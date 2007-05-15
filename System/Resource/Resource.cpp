//==============================================================================
// $Source: $
//
// Copyright (c) 2002-2006 LeapFrog Enterprises, Inc.
// All Rights Reserved
//==============================================================================
//
// File:
//		Resource.cpp
//
// Description:
//		Implements the Module Public Interface (MPI) for the Brio Kernel module.
//
//==============================================================================

#include <CoreTypes.h>
#include <SystemTypes.h>
#include <SystemErrors.h>

#include <CoreMPI.h>
#include <DebugMPI.h>
//#include <KernelMPI.h>
#include <ResourceMPI.h>
#include <ResourcePriv.h>
#include <EventListener.h>
#include <EventMPI.h>

#include <stdio.h>
#include <string.h> // for strcmp
#include <dirent.h>
#include <sys/stat.h>
//#include <linux/stat.h>	// FIXME/dm: missing on embedded target -- extraneous?
#include <stdio.h>

#include <map>		// FIXME: replace with non-STL implementation

const tVersion	kModuleVersion = 	2;
const CString	kModuleName = 		"Resource";
const CURI		kModuleURI =		"/LF/System/Resource";
const U32		kRsrcDescBlockInc = 256;
const U32		kRscrDeviceBlockInc = 16;

//----------------------------------------------------------------------------

// device tracking variables
tRsrcDeviceDescriptor* rsrcDeviceArray = NULL;
U32 rsrcDeviceBlockSize = 0;
U32 rsrcDeviceArraySize = 0;
int devOpenCount = 0;
// module resource tracking variables
tRsrcDescriptor* rsrcDescPtrArray = NULL;
U32 rsrcDescBlockSize = 0;
U32 rsrcDescPtrArraySize = 0;
int rscrOpenCount = 0;
CURI DefaultURI;

class CResourceImpl {
public:
	CResourceImpl() : lastSearchHndl(0), lastSearchType(kRsrcSearchTypeUndefined),
						mDefaultURI(DefaultURI), mpEventListener(NULL)	{}
	tRsrcHndl			lastSearchHndl;
	tRsrcSearchType		lastSearchType;
	tRsrcSearchPattern	lastSearchPattern;
	CURI				mDefaultURI;
	CString 			cstrReturnVal;
	const IEventListener*	mpEventListener;
};

typedef std::map<U32, CResourceImpl>	MPIMap;// FIXME: replace with non-STL implementation

MPIMap gMPIMap;


namespace
{


//==============================================================================
// Setup search paths
//==============================================================================
	//----------------------------------------------------------------------------
	CResourceImpl* RetrieveImpl(U32 id)
	{
		// FIXME/tp: multithreading issues
		MPIMap::iterator it = gMPIMap.find(id);
		if (it != gMPIMap.end())
			return &(it->second);
		CDebugMPI	dbg(kGroupResource);
		dbg.Assert(false, "Resource configuration failure, unregistered MPI id!");
	}

	//----------------------------------------------------------------------------
	tRsrcDescriptor* GetRsrcDescriptor(tRsrcHndl hndl)
	{
		U32 rsrcNum = static_cast<U32>(hndl-1);
	
		if (rsrcNum > rsrcDescPtrArraySize)
			return NULL;
		return &rsrcDescPtrArray[rsrcNum];
	}
	
	//----------------------------------------------------------------------------
	tRsrcHndl FindRsrcRecord( CResourceImpl *pimpl )
	{
		Boolean bFound = false;
		U32 count = (U32) pimpl->lastSearchHndl;
		CString cstrTemp;
		tRsrcHndl hndl;
		
		hndl = kInvalidRsrcHndl;
		if (count <= rsrcDescPtrArraySize)
		{
			if (pimpl->lastSearchType == kRsrcSearchTypeByURI)
				cstrTemp = pimpl->lastSearchPattern.uri;
				
			for (; count <= rsrcDescPtrArraySize && !bFound; ++count)
			{
				switch (pimpl->lastSearchType)
				{
					case kRsrcSearchTypeUndefined:
						break;
					case kRsrcSearchTypeByURI:
						if (rsrcDescPtrArray[count-1].uri == cstrTemp)
						{
							bFound = true;
							hndl = (tRsrcHndl) count;
						}
						break;
						/* FIXME/tp: resolve ByID issues
					case kRsrcSearchTypeByID:
						if (rsrcDescPtrArray[count].id == pimpl->lastSearchPattern.id)
						{
							bFound = true;
							hndl = (tRsrcHndl) count;
						}
						break;
						*/
					case kRsrcSearchTypeByHandle:
						break;
					case kRsrcSearchTypeByType:
						if (rsrcDescPtrArray[count].type == pimpl->lastSearchPattern.type)
						{
							bFound = true;
							hndl = (tRsrcHndl) count;
						}
						break;
				}
			}
		}
		pimpl->lastSearchHndl = hndl;
		return hndl;
	}
	
	//----------------------------------------------------------------------------
	U32 IncreaseDeviceBlockSize()
	{
		U32 newSize = rsrcDeviceBlockSize + kRscrDeviceBlockInc;
		
		struct tRsrcDeviceDescriptor* nb = new struct tRsrcDeviceDescriptor[newSize];
		// copy old data (if any) into new block
		if (rsrcDeviceBlockSize > 0)
		{
			memcpy(nb, (void *) rsrcDeviceArray, sizeof(tRsrcDeviceDescriptor) * rsrcDeviceBlockSize);
		}
		// initialize the extension area
		memset((void *) &nb[rsrcDeviceBlockSize], 0, sizeof(tRsrcDeviceDescriptor) * kRscrDeviceBlockInc);
		// free old block (if any)
		if (rsrcDeviceBlockSize > 0)
		{
			delete []rsrcDeviceArray;
		}
		// copy pointer to new block into global pointer
		rsrcDeviceArray = nb;
		
		return newSize;
	}
	
	//----------------------------------------------------------------------------
	void AddDeviceEntry( struct tRsrcDeviceDescriptor* tempDevice )
	{
		if (rsrcDeviceArraySize >= rsrcDeviceBlockSize)
		{
			rsrcDeviceBlockSize = IncreaseDeviceBlockSize();
		}
		// scan through the device list, add only if unique
		for (U32 i = 0; i < rsrcDeviceArraySize; i++)
		{
			if (strncmp(rsrcDeviceArray[i].uriBase, tempDevice->uriBase,
				MAX_RSRC_URI_SIZE) == 0)
				return;
		}
		rsrcDeviceArray[rsrcDeviceArraySize].uriBase;
		strncpy(rsrcDeviceArray[rsrcDeviceArraySize].uriBase, tempDevice->uriBase, MAX_RSRC_URI_SIZE);
		rsrcDeviceArraySize++;
	}
	
	//----------------------------------------------------------------------------
	U32 IncreaseDescriptorBlockSize()
	{
		U32 newSize = rsrcDescBlockSize + kRsrcDescBlockInc;
		
		struct tRsrcDescriptor* nb = new struct tRsrcDescriptor[newSize];
		// copy old data (if any) into new block
		if (rsrcDescBlockSize > 0)
		{
			memcpy(nb, (void *) rsrcDescPtrArray, sizeof(tRsrcDescriptor) * rsrcDescBlockSize);
		}
		// initialize the extension area
		memset((void *) &nb[rsrcDescBlockSize], 0, sizeof(tRsrcDescriptor) * kRsrcDescBlockInc);
		// free old block (if any)
		if (rsrcDescBlockSize > 0)
		{
			delete []rsrcDescPtrArray;
		}
		// copy pointer to new block into global pointer
		rsrcDescPtrArray = nb;
		
		return newSize;
	}
	
	//----------------------------------------------------------------------------
	void AddRsrcEntry(struct tRsrcDescriptor* tempDescriptor )
	{
		struct tRsrcDescriptor* currDescriptor;
		
		if (rsrcDescPtrArraySize >= rsrcDescBlockSize)
		{
			rsrcDescBlockSize = IncreaseDescriptorBlockSize();
		}
		currDescriptor = &rsrcDescPtrArray[rsrcDescPtrArraySize++];
		// copy tempDescriptor data in rsrcDescArray
		currDescriptor->type = tempDescriptor->type;
		currDescriptor->version = tempDescriptor->version;
		currDescriptor->packedSize = tempDescriptor->packedSize;
		currDescriptor->unpackedSize = tempDescriptor->unpackedSize;
		currDescriptor->id = rsrcDescPtrArraySize - 1;
		currDescriptor->pRsrc = NULL;
		currDescriptor->pFile = NULL;
		currDescriptor->useCount = 0;	// initial load/unload count
		strncpy(currDescriptor->uri, tempDescriptor->uri, MAX_RSRC_URI_SIZE);
		strncpy(currDescriptor->name, tempDescriptor->name, MAX_RSRC_NAME_SIZE);
	}
	
	//----------------------------------------------------------------------------
	void ScanDirForRscr(char *path)
	{
		DIR *dp;
		struct dirent *ep;
		struct tRsrcDescriptor tempDescrp;
		struct stat fileStat;
		struct tRsrcDeviceDescriptor tempDevice;
		CString cstrTemp;
		FILE *rFile;
				
		// open the specified directory
		dp = opendir( path );
		if (dp != NULL)
		{
			// loop through the specified directory, processing recource entries
			while ((ep = readdir( dp )))
			{
				// skip over '.' and '..' entries
				if (strcmp(ep->d_name, ".") == 0 || strcmp(ep->d_name, "..") == 0)
					continue;
				// build the fully resolved filename (URI)
				cstrTemp = path;
				cstrTemp += ep->d_name;
				// stat the entry to get size and type (used to determine if item is a directory)
				stat(cstrTemp.c_str(), &fileStat);
				
				// is the entry a sub directory
				if (S_ISDIR( fileStat.st_mode))
				{
					// fill-in and add a new device entry for the sub directory found
					cstrTemp += "/";
					strncpy(tempDevice.uriBase, cstrTemp.c_str(), MAX_RSRC_URI_SIZE);
					AddDeviceEntry(&tempDevice);
				}
				else if (strstr(ep->d_name, ".rsrc"))	// is the entry a resource descriptor
				{
					// read resource description file
					rFile = fopen(cstrTemp.c_str(), "rb");
					if (rFile != NULL)
					{
						// Note: pointing to a tRsrcDescriptor structure, but only
						// reading tRsrcFileDescriptor size structure (missing pRsrc,
						// pFile and useCount entries, as these are initialized later)
						fread((void *) &tempDescrp, sizeof(tRsrcFileDescriptor), 1, rFile);
					}
					// add resource block entry (init pRsrc, pFile and useCount)
					AddRsrcEntry(&tempDescrp);
				}
			}
			closedir(dp);
		}
		else
		{
			printf("ERROR - unable to open directory - %s\n", path);
		}
	}
	
#ifndef EMULATION
	// FIXME/tp: separeate into emulation-specific subfolder files
	CURI InitializeBaseResourceURIs()
	{
		tRsrcDeviceDescriptor tempDevice;
		CString URIpath;
		
		CURI defaultURI = "/home/lfu/LeapFrog/";
		
		URIpath = defaultURI + "System/";
		strncpy(tempDevice.uriBase, URIpath.c_str(), MAX_RSRC_URI_SIZE);
		AddDeviceEntry(&tempDevice);
		URIpath = defaultURI + "Applic/";
		strncpy(tempDevice.uriBase, URIpath.c_str(), MAX_RSRC_URI_SIZE);
		AddDeviceEntry(&tempDevice);
		return defaultURI;
	}
	
#else // !EMULATION

	#include <unistd.h>

	inline CURI InitializeBaseResourceURIs()
	{
		tRsrcDeviceDescriptor tempDevice;
		char buf[MAX_RSRC_URI_SIZE];
		if( getcwd(buf, MAX_RSRC_URI_SIZE) == NULL )
			;//FIXME: bail
		CURI defaultURI = buf;
		defaultURI += "/apprsrc/";
// printf("InitializeBaseResourceURIs: %s\n", defaultURI.c_str());
		strncpy(tempDevice.uriBase, defaultURI.c_str(), MAX_RSRC_URI_SIZE);
		AddDeviceEntry(&tempDevice);
		return defaultURI;
	}
	
#endif // !EMULATION
}



//----------------------------------------------------------------------------
CResourceModule::CResourceModule() : dbg_(kGroupResource)
{
	if (devOpenCount == 0)		// if initial construction
	{
		DefaultURI = InitializeBaseResourceURIs();
	}
	devOpenCount++;
}

//----------------------------------------------------------------------------
CResourceModule::~CResourceModule()
{
	if (--devOpenCount == 0)
	{
		if (rsrcDeviceBlockSize > 0)
		{
			delete []rsrcDeviceArray;
		}
		rsrcDeviceArraySize = 0;
		rsrcDeviceBlockSize = 0;
	}
}

//----------------------------------------------------------------------------
Boolean	CResourceModule::IsValid() const
{
	return true;
}

//----------------------------------------------------------------------------
tVersion CResourceModule::GetModuleVersion() const
{
	return kModuleVersion;
}

//----------------------------------------------------------------------------
const CString* CResourceModule::GetModuleName() const
{
	return &kModuleName;
}

//----------------------------------------------------------------------------
const CURI* CResourceModule::GetModuleOrigin() const
{
	return &kModuleURI;
}

//----------------------------------------------------------------------------
U32 CResourceModule::Register( )
{
	// FIXME/tp: Handle threading issues through lock
	static U32 sNextIndex = 0;
	CResourceImpl	temp;
	gMPIMap.insert(MPIMap::value_type(++sNextIndex, temp));
	return sNextIndex;
}

//----------------------------------------------------------------------------
void CResourceModule::SetDefaultURIPath(U32 id, const CURI &pURIPath)
{
	CResourceImpl* pimpl = RetrieveImpl(id);
	pimpl->mDefaultURI = pURIPath;
#ifdef EMULATION
	if( pURIPath.at(pURIPath.length()-1) != '/' )
		pimpl->mDefaultURI += "/";
#else
	// FIXME/dm: Implement general CString handling for embedded target
	const char* str = pURIPath.c_str();
	int	  len = pURIPath.size();
	if (str[len-1] != '/')	
		pimpl->mDefaultURI += "/";
	
#endif
}

//----------------------------------------------------------------------------
tErrType CResourceModule::SetDefaultListener(U32 id, const IEventListener *pListener)
{
	CResourceImpl* pimpl = RetrieveImpl(id);
	pimpl->mpEventListener = pListener;
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType 	CResourceModule::AddRsrcRef(tRsrcHndl hndl)
{
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType	CResourceModule::DeleteRsrcRef(tRsrcHndl hndl)
{
	return kNoErr;
}

//----------------------------------------------------------------------------
Boolean	CResourceModule::RsrcIsLoaded(tRsrcHndl hndl) const
{
	return (GetRsrcDescriptor(hndl) != NULL) ? true : false;
}

//----------------------------------------------------------------------------
void CResourceModule::SetSearchScope(U32 id, eSearchScope scope)
{
	if (scope != kOpenPackagesAndDevices)
		dbg_.DebugOut(kDbgLvlCritical, 
			"Currently only support kOpenPackagesAndDevices search scope\n");
}
//----------------------------------------------------------------------------
eSearchScope CResourceModule::GetSearchScope(U32 id) const
{
	return kOpenPackagesAndDevices;
}
//----------------------------------------------------------------------------
U32 CResourceModule::GetNumRsrcs(U32 id, const CURI *pURIPath) const
{
	CResourceImpl* pimpl = RetrieveImpl(id);
	return rsrcDescPtrArraySize; 
}
 	
//----------------------------------------------------------------------------
U32 CResourceModule::GetNumRsrcs(U32 id, tRsrcType type, 
								const CURI *pURIPath) const
{
	CResourceImpl* pimpl = RetrieveImpl(id);
	U32 count = 0;
	U32 index=0;
	tRsrcDescriptor *pRsrcDesc;

	while (index < rsrcDescPtrArraySize)
	{
		if (rsrcDescPtrArray[index].type == type)
			count++;
		index++;
	}

	return count;
}
 
//----------------------------------------------------------------------------
tRsrcHndl CResourceModule::FindRsrc(U32 id, const CURI &rsrcURI,
									const CURI *pURIPath) const
{
	CResourceImpl* pimpl = RetrieveImpl(id);
	CURI searchPath = pURIPath ? *pURIPath : pimpl->mDefaultURI;
	searchPath += rsrcURI;
	 	
	// setup/save the search parameters for the FindNextRsrc function
	// FIXME/tp: URIs are unique, so what would "NextRsrc" mean for this function?
	pimpl->lastSearchHndl = (tRsrcHndl) 1;
	pimpl->lastSearchType = kRsrcSearchTypeByURI;
	strncpy(pimpl->lastSearchPattern.uri, searchPath.c_str(), MAX_RSRC_URI_SIZE);
	
	return FindRsrcRecord(pimpl);
}

/* FIXME/tp: Reslove problems with ID scheme
//----------------------------------------------------------------------------
tRsrcHndl CResourceModule::FindFirstRsrc(U32 id, tRsrcHndl &hndl, tRsrcID rsrcID,
									const CURI *pURIPath) const
{
	CResourceImpl* pimpl = RetrieveImpl(id);
		
	// setup/save the search parameters for the FindNextRsrc function
	pimpl->lastSearchHndl = (tRsrcHndl) 0;
	pimpl->lastSearchType = kRsrcSearchTypeByID;
	pimpl->lastSearchPattern.id = rsrcID;
	
	hndl = FindRsrcRecord(pimpl);
	
	return (GetRsrcDescriptor(hndl) != NULL) ? kNoErr : kResourceNotFoundErr;
}
*/

//----------------------------------------------------------------------------
tRsrcHndl CResourceModule::FindFirstRsrc(U32 id, const CURI *pURIPath) const
{
	CResourceImpl* pimpl = RetrieveImpl(id);
	// save the search parameters for the FindNextRsrc function
	pimpl->lastSearchHndl = 1;
	pimpl->lastSearchType = kRsrcSearchTypeByHandle;
	return FindRsrcRecord(pimpl);
}
 
//----------------------------------------------------------------------------
tRsrcHndl CResourceModule::FindFirstRsrc(U32 id, tRsrcType type, 
										const CURI *pURIPath) const
{
	CResourceImpl* pimpl = RetrieveImpl(id);
		
	// setup/save the search parameters for the FindNextRsrc function
	pimpl->lastSearchHndl = 1;
	pimpl->lastSearchType = kRsrcSearchTypeByType;
	pimpl->lastSearchPattern.type = type;
	
	return FindRsrcRecord(pimpl);
}
 
//----------------------------------------------------------------------------
tRsrcHndl CResourceModule::FindNextRsrc(U32 id) const
{
	CResourceImpl* pimpl = RetrieveImpl(id);
	return FindRsrcRecord(pimpl);
}
 
//----------------------------------------------------------------------------
const CURI* CResourceModule::GetRsrcURI(U32 id, tRsrcHndl hndl) const
{
	CResourceImpl* pimpl = RetrieveImpl(id);
	tRsrcDescriptor* pRsrc = GetRsrcDescriptor(hndl);
	
	if (pRsrc == NULL)
	{
		dbg_.DebugOut(kDbgLvlCritical, "GetRsrcURI() called with invalid resource handle\n");
		return &kNullURI;
	}

	pimpl->cstrReturnVal = pRsrc->uri;
	return &pimpl->cstrReturnVal;
}

//----------------------------------------------------------------------------
const CString* CResourceModule::GetRsrcName(U32 id, tRsrcHndl hndl) const
{
	CResourceImpl* pimpl = RetrieveImpl(id);
	tRsrcDescriptor* pRsrc = GetRsrcDescriptor(hndl);
	
	if (pRsrc == NULL)
	{
		dbg_.DebugOut(kDbgLvlCritical, "GetRsrcName() called with invalid resource handle\n");
		return &kNullString;
	}

	pimpl->cstrReturnVal = pRsrc->name;
	return &pimpl->cstrReturnVal;
}

//----------------------------------------------------------------------------
tRsrcType CResourceModule::GetRsrcType(tRsrcHndl hndl) const
{
	tRsrcDescriptor* pRsrc = GetRsrcDescriptor(hndl);
	if (pRsrc == NULL)
	{
		dbg_.DebugOut(kDbgLvlCritical, "GetRsrcType() called with invalid resource handle\n");
		return kInvalidRsrcType;
	}

	return pRsrc->type;
}

//----------------------------------------------------------------------------
tVersion CResourceModule::GetRsrcVersion(tRsrcHndl hndl) const
{
	tRsrcDescriptor* pRsrc = GetRsrcDescriptor(hndl);
	if (pRsrc == NULL)
	{
		dbg_.DebugOut(kDbgLvlCritical, "GetRsrcVersion() called with invalid resource handle\n");
		return kUndefinedVersion;
	}

	return pRsrc->version;
}

//----------------------------------------------------------------------------
const CString* CResourceModule::GetRsrcVersionStr(tRsrcHndl hndl) const
{
	tRsrcDescriptor* pRsrc = GetRsrcDescriptor(hndl);
	if (pRsrc == NULL)
	{
		dbg_.DebugOut(kDbgLvlCritical, "GetRsrcVersionStr() called with invalid resource handle\n");
		return &kNullString;
	}

	static CString sTemp;
	char buf[16];
	sprintf(buf, "%d", pRsrc->version);
	sTemp = buf;
//	static CString sTemp = "2";
	return &sTemp;
}

//----------------------------------------------------------------------------
U32 CResourceModule::GetRsrcPackedSize(tRsrcHndl hndl) const
{
	tRsrcDescriptor* pRsrc = GetRsrcDescriptor(hndl);
	if (pRsrc == NULL)
	{
		dbg_.DebugOut(kDbgLvlCritical, "GetRsrcPackedSize() called with invalid resource handle\n");
		return 0;
	}

	return pRsrc->packedSize;

}

//----------------------------------------------------------------------------
U32 CResourceModule::GetRsrcUnpackedSize(tRsrcHndl hndl) const
{
	tRsrcDescriptor* pRsrc = GetRsrcDescriptor(hndl);
	if (pRsrc == NULL)
	{
		dbg_.DebugOut(kDbgLvlCritical, "GetRsrcUnpackedSize() called with invalid resource handle\n");
		return 0;
	}

	return pRsrc->unpackedSize;
}

//----------------------------------------------------------------------------
tPtr CResourceModule::GetRsrcPtr(tRsrcHndl hndl) const
{
	tRsrcDescriptor* pRsrc = GetRsrcDescriptor(hndl);
	if (pRsrc == NULL)
	{
		dbg_.DebugOut(kDbgLvlCritical, "GetRsrcPtr() called with invalid resource handle\n");
		return 0;
	}
	
	if (pRsrc->pRsrc == NULL)
		dbg_.DebugOut(kDbgLvlCritical, "GetRsrcPtr() called when resource not yet loaded!\n");

	return pRsrc->pRsrc;
}

//----------------------------------------------------------------------------
tErrType  	CResourceModule::OpenAllDevices(U32 id, tOptionFlags openOptions,
						const IEventListener *pListener)
{
	const tEventPriority	kPriorityTBD = 0;
	IEventListener *pActiveListener;
	
	CResourceImpl* pimpl = RetrieveImpl(id);
	pActiveListener = (IEventListener *) pimpl->mpEventListener;
	if( pListener != kNull )
		pActiveListener = (IEventListener *) pListener;
		
	if (rscrOpenCount == 0)		// if initial construction
	{
		// foreach identified device
		for (U16 i = 0; i < rsrcDeviceArraySize; i++)
		{
			ScanDirForRscr(rsrcDeviceArray[i].uriBase);
		}
	}
	tResourceMsgDat	data;
	data = 0;

	CEventMPI	event;
	CResourceEventMessage	msg(kResourceAllDevicesOpenedEvent, data);
	event.PostEvent(msg, kPriorityTBD, pActiveListener);
	
	rscrOpenCount++;
	return kNoErr;
}
  
//----------------------------------------------------------------------------
tErrType  	CResourceModule::CloseAllDevices()
{
	if (rscrOpenCount > 0)
	{
		if (--rscrOpenCount == 0)
		{
			if (rsrcDescBlockSize > 0)
			{
				delete []rsrcDescPtrArray;
			}
			rsrcDescPtrArraySize = 0;
			rsrcDescBlockSize = 0;
		}
	}
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType  	CResourceModule::OpenRsrc(U32 id, tRsrcHndl hndl, 
						tOptionFlags openOptions,
						const IEventListener *pListener)
{
	tRsrcDescriptor* pRsrc;
	const tEventPriority	kPriorityTBD = 0;
	IEventListener *pActiveListener;
	
	CResourceImpl* pimpl = RetrieveImpl(id);
	pActiveListener = (IEventListener *) pimpl->mpEventListener;
	if( pListener != kNull )
		pActiveListener = (IEventListener *) pListener;
	
	if ((pRsrc = GetRsrcDescriptor(hndl)) == NULL)
		return kResourceInvalidErr;
		
	if (pRsrc->useCount == 0)
	{
		// resource is not opened, open it now (open according to type)
		CString fileOptions;
		
		if(! (openOptions & kOpenRsrcOptionWrite) )
		{
			fileOptions = "rb";		// no write, open for read only
		}
		else
		{
			if( openOptions & kOpenRsrcOptionRead )
			{
				fileOptions = "rb+";	// open for reading & writing
			}
			else
			{
				fileOptions = "wb";		// open for write only (truncate existing)
			}
		}
		
		pRsrc->pFile = fopen(pRsrc->uri, fileOptions.c_str());
		
		// register open for later cleanup 
	}
	if (pRsrc->pFile == NULL)
	{
		return kResourceInvalidErr;
	}
	pRsrc->useCount++;		
	
	tResourceMsgDat	data;
	data = 0;

	CEventMPI	event;
	CResourceEventMessage	msg(kResourceOpenedEvent, data);
	event.PostEvent(msg, kPriorityTBD, pActiveListener);
	
	return kNoErr;
}  

//----------------------------------------------------------------------------
tErrType  	CResourceModule::CloseRsrc(tRsrcHndl hndl)
{
	tRsrcDescriptor* pRsrc;
	
	if ((pRsrc = GetRsrcDescriptor(hndl)) == NULL)
		return kResourceInvalidErr;
		
	if (pRsrc->useCount == 1)
	{
		// resource opened and this is the last user, close file
		if (pRsrc->pFile != NULL)
		{
			fclose(pRsrc->pFile);
			pRsrc->pFile = NULL;
			// unregister open for later cleanup 
		}
		pRsrc->useCount = 0;
		return kNoErr;
	}
	else if (pRsrc->useCount > 1)
	{
		pRsrc->useCount--;		
		return kNoErr;
	}
	return kResourceInvalidErr;
}

//----------------------------------------------------------------------------
tErrType  	CResourceModule::ReadRsrc(U32 id, tRsrcHndl hndl, void* pBuffer, U32 numBytesRequested,
						U32 *pNumBytesActual,
						tOptionFlags readOptions,
						const IEventListener *pListener) const
{
	tRsrcDescriptor* pRsrc;
	const tEventPriority	kPriorityTBD = 0;
	IEventListener *pActiveListener;
	
	CResourceImpl* pimpl = RetrieveImpl(id);
	pActiveListener = (IEventListener *) pimpl->mpEventListener;
	if( pListener != kNull )
		pActiveListener = (IEventListener *) pListener;
	
	if ((pRsrc = GetRsrcDescriptor(hndl)) == NULL)
		return kResourceInvalidErr;
		
	if (pBuffer == NULL || numBytesRequested == 0)
		return kInvalidParamErr;			// invalid pointer supplied
		
	if (pRsrc->pFile == NULL)
	{
		return kResourceNotOpenErr;	
	}
	fread(pBuffer, sizeof(char), numBytesRequested, pRsrc->pFile);
	
	tResourceMsgDat	data;
	data = 0;

	CEventMPI	event;
	CResourceEventMessage	msg(kResourceReadDoneEvent, data);
	event.PostEvent(msg, kPriorityTBD, pActiveListener);
	return kNoErr;
}  

//----------------------------------------------------------------------------
tErrType  	CResourceModule::LoadRsrc(U32 id, tRsrcHndl hndl, tOptionFlags loadOptions,
							const IEventListener *pListener)
{
	tRsrcDescriptor* pRsrc;
	FILE *rFile;
	unsigned char *cptr;
	const tEventPriority	kPriorityTBD = 0;
	IEventListener *pActiveListener;
	
	CResourceImpl* pimpl = RetrieveImpl(id);
	pActiveListener = (IEventListener *) pimpl->mpEventListener;
	if( pListener != kNull )
		pActiveListener = (IEventListener *) pListener;
		
	if ((pRsrc = GetRsrcDescriptor(hndl)) == NULL)
		return kResourceInvalidErr;
		
	if (pRsrc->useCount == 0)
	{
		// resource data not loaded, create space, set ptr and load data
		if (pRsrc->unpackedSize > 0)
		{
			pRsrc->pRsrc = new unsigned char[pRsrc->unpackedSize+1];
			// load data into newly allocated space
			rFile = fopen(pRsrc->uri, "rb");
			if (rFile != NULL)
			{
				fread(pRsrc->pRsrc, sizeof(char), pRsrc->unpackedSize, rFile);
				fclose(rFile);
				
				// kill ending newline (not sure if this is necessary)
				cptr = (unsigned char *) pRsrc->pRsrc;
				if (cptr[pRsrc->unpackedSize-1] == '\n')
				{
					 cptr[pRsrc->unpackedSize-1] = '\0';
				}
			}
			// register load for later cleanup 
		}
	}
	if (pRsrc->pRsrc == NULL)
	{
		return kResourceInvalidErr;
	}
	pRsrc->useCount++;		

	tResourceMsgDat	data;
	data = 0;

	CEventMPI	event;
	CResourceEventMessage	msg(kResourceLoadedEvent, data);
	event.PostEvent(msg, kPriorityTBD, pActiveListener);
	
	return kNoErr;
	
}  

//----------------------------------------------------------------------------
tErrType  	CResourceModule::UnloadRsrc(U32 id, tRsrcHndl hndl, 
						tOptionFlags unloadOptions,
						const IEventListener *pListener)
{
	tRsrcDescriptor* pRsrc;
	const tEventPriority	kPriorityTBD = 0;
	IEventListener *pActiveListener;
	
	CResourceImpl* pimpl = RetrieveImpl(id);
	pActiveListener = (IEventListener *) pimpl->mpEventListener;
	if( pListener != kNull )
		pActiveListener = (IEventListener *) pListener;

	if ((pRsrc = GetRsrcDescriptor(hndl)) == NULL)
		return kResourceInvalidErr;
		
	if (pRsrc->useCount == 0)
		return kResourceInvalidErr;
		
	if (pRsrc->useCount == 1)
	{
		// resource data loaded, free space, clear ptr
		if (pRsrc->pRsrc != NULL)
		{
			delete (unsigned char *) pRsrc->pRsrc;
			pRsrc->pRsrc = NULL;
			// unregister load for later cleanup 
		}
		pRsrc->useCount = 0;
	}
	else if (pRsrc->useCount > 1)
	{
		pRsrc->useCount--;		
	}
	
	tResourceMsgDat	data;
	data = 0;

	CEventMPI	event;
	CResourceEventMessage	msg(kResourceUnloadedEvent, data);
	event.PostEvent(msg, kPriorityTBD, pActiveListener);

	return kNoErr;
}

//----------------------------------------------------------------------------
U16 CResourceModule::GetNumDevices() const
{
	return 2; 
}
//----------------------------------------------------------------------------
U16 CResourceModule::GetNumDevices(tDeviceType type) const
{
	return 2; 
}

//----------------------------------------------------------------------------
tDeviceHndl CResourceModule::FindFirstDevice(tDeviceType type) const
{
	dbg_.DebugOut(kDbgLvlCritical, "FindFirstDevice not implemented\n");
	return kInvalidDeviceHndl;
}
//----------------------------------------------------------------------------
tDeviceHndl CResourceModule::FindFirstDevice() const
{
	dbg_.DebugOut(kDbgLvlCritical, "FindFirstDevice not implemented\n");
	return kInvalidDeviceHndl;
}
//----------------------------------------------------------------------------
tDeviceHndl CResourceModule::FindNextDevice() const
{
	dbg_.DebugOut(kDbgLvlCritical, "FindNextDevice not implemented\n");
	return kInvalidDeviceHndl;
}

//----------------------------------------------------------------------------
const CString* CResourceModule::GetDeviceName(tDeviceHndl hndl) const
{
	dbg_.DebugOut(kDbgLvlCritical, "GetDeviceName not implemented\n");
	return &kNullString;
}
//----------------------------------------------------------------------------
tDeviceType CResourceModule::GetDeviceType(tDeviceHndl hndl) const
{
	dbg_.DebugOut(kDbgLvlCritical, "GetDeviceType not implemented\n");
	return kRsrcDeviceTypeUndefined;
}

//----------------------------------------------------------------------------
tErrType CResourceModule::OpenDevice(tDeviceHndl hndl, 
									tOptionFlags openOptions,
									const IEventListener *pListener)  
{
	dbg_.DebugOut(kDbgLvlCritical, "OpenDevice not implemented\n");
	return kNoErr;
}
//----------------------------------------------------------------------------
tErrType CResourceModule::CloseDevice(tDeviceHndl hndl)
{
	dbg_.DebugOut(kDbgLvlCritical, "CloseDevice not implemented\n");
	return kNoErr;
}

//----------------------------------------------------------------------------
U32 CResourceModule::GetNumRsrcPackages(tRsrcPackageType type, 
										const CURI *pURIPath) const
{
	dbg_.DebugOut(kDbgLvlCritical, "GetNumRsrcPackages not implemented\n");
	return 0;
}

//----------------------------------------------------------------------------
tRsrcPackageHndl CResourceModule::FindRsrcPackage(const CURI& packageURI,
												const CURI *pURIPath) const
{
	dbg_.DebugOut(kDbgLvlCritical, "FindRsrcPackage not implemented\n");
	return kInvalidRsrcPackageHndl;
}
//----------------------------------------------------------------------------
tRsrcPackageHndl CResourceModule::FindFirstRsrcPackage(tRsrcPackageType type,
												const CURI *pURIPath) const
{
	dbg_.DebugOut(kDbgLvlCritical, "FindFirstRsrcPackage not implemented\n");
	return kInvalidRsrcPackageHndl;
}
//----------------------------------------------------------------------------
tRsrcPackageHndl CResourceModule::FindNextRsrcPackage() const
{
	dbg_.DebugOut(kDbgLvlCritical, "FindNextRsrcPackage not implemented\n");
	return kInvalidRsrcPackageHndl;
}

	// Getting package info
//----------------------------------------------------------------------------
const CURI* CResourceModule::GetRsrcPackageURI(tRsrcPackageHndl hndl) const
{
	dbg_.DebugOut(kDbgLvlCritical, "GetRsrcPackageURI not implemented\n");
	return &kNullURI;
}
//----------------------------------------------------------------------------
const CString* CResourceModule::GetRsrcPackageName(tRsrcPackageHndl hndl) const
{
	dbg_.DebugOut(kDbgLvlCritical, "GetRsrcPackageName not implemented\n");
	return &kNullString;
}
//----------------------------------------------------------------------------
tRsrcPackageType CResourceModule::GetRsrcPackageType(tRsrcPackageHndl hndl) const
{
	dbg_.DebugOut(kDbgLvlCritical, "GetRsrcPackageType not implemented\n");
	return kRsrcPackageTypeUndefined;
}
//----------------------------------------------------------------------------
tVersion CResourceModule::GetRsrcPackageVersion(tRsrcPackageHndl hndl) const
{
	dbg_.DebugOut(kDbgLvlCritical, "GetRsrcPackageVersion not implemented\n");
	return kUndefinedVersion;
}
//----------------------------------------------------------------------------
const CString* CResourceModule::GetRsrcPackageVersionStr(tRsrcPackageHndl hndl) const
{
	dbg_.DebugOut(kDbgLvlCritical, "GetRsrcPackageVersionStr not implemented\n");
	return &kNullString;
}
//----------------------------------------------------------------------------
U32 CResourceModule::GetRsrcPackageSizeUnpacked(tRsrcPackageHndl hndl) const
{
	dbg_.DebugOut(kDbgLvlCritical, "GetRsrcPackageSizeUnpacked not implemented\n");
	return 0;
}
//----------------------------------------------------------------------------
U32 CResourceModule::GetRsrcPackageSizePacked(tRsrcPackageHndl hndl) const
{
	dbg_.DebugOut(kDbgLvlCritical, "GetRsrcPackageSizePacked not implemented\n");
	return 0;
}

	// Opening & closing packages to find resources within them
//----------------------------------------------------------------------------
tErrType CResourceModule::OpenRsrcPackage(tRsrcPackageHndl hndl, 
									tOptionFlags openOptions,
									const IEventListener *pListener)  
{
	dbg_.DebugOut(kDbgLvlCritical, "OpenRsrcPackage not implemented\n");
	return kNoErr;
}
//----------------------------------------------------------------------------
tErrType CResourceModule::CloseRsrcPackage(tRsrcPackageHndl hndl)
{
	dbg_.DebugOut(kDbgLvlCritical, "CloseRsrcPackage not implemented\n");
	return kNoErr;
}

	// Loading & unloading packages
//----------------------------------------------------------------------------
tErrType CResourceModule::LoadRsrcPackage(tRsrcPackageHndl hndl, 
									tOptionFlags loadOptions,
									const IEventListener *pListener)  
{
	dbg_.DebugOut(kDbgLvlCritical, "LoadRsrcPackage not implemented\n");
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CResourceModule::UnloadRsrcPackage(tRsrcPackageHndl hndl, 
									tOptionFlags unloadOptions,
									const IEventListener *pListener)  
{
	dbg_.DebugOut(kDbgLvlCritical, "UnloadRsrcPackage not implemented\n");
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CResourceModule::SeekRsrc(tRsrcHndl hndl, U32 numSeekBytes, 
									tOptionFlags seekOptions) const
{
	tRsrcDescriptor* pRsrc;
	
	if ((pRsrc = GetRsrcDescriptor(hndl)) == NULL)
		return kResourceInvalidErr;
		
	if (pRsrc->pFile == NULL)
	{
		return kResourceNotOpenErr;	
	}
	fseek ( pRsrc->pFile, numSeekBytes, seekOptions );

	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CResourceModule::WriteRsrc(U32 id, tRsrcHndl hndl, const void *pBuffer, 
									U32 numBytesRequested, U32 *pNumBytesActual,
									tOptionFlags writeOptions,
									const IEventListener *pListener) const
{
	tRsrcDescriptor* pRsrc;
	const tEventPriority	kPriorityTBD = 0;
	IEventListener *pActiveListener;
	
	CResourceImpl* pimpl = RetrieveImpl(id);
	if (pimpl == NULL)
		return kResourceInvalidMPIIdErr;
		
	pActiveListener = (IEventListener *) pimpl->mpEventListener;
	if( pListener != kNull )
		pActiveListener = (IEventListener *) pListener;
	
	if ((pRsrc = GetRsrcDescriptor(hndl)) == NULL)
		return kResourceInvalidErr;
		
	if (pBuffer == NULL || numBytesRequested == 0 || pNumBytesActual == NULL )
		return kInvalidParamErr;			// invalid pointer supplied
		
	if (pRsrc->pFile == NULL)
	{
		return kResourceNotOpenErr;	
	}
	*pNumBytesActual = fwrite(pBuffer, sizeof(char), numBytesRequested, pRsrc->pFile);
	
	tResourceMsgDat	data;
	data = 0;

	CEventMPI	event;
	CResourceEventMessage	msg(kResourceWriteDoneEvent, data);
	event.PostEvent(msg, kPriorityTBD, pActiveListener);
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CResourceModule::GetRsrcRefCount(tRsrcHndl hndl)
{
	dbg_.DebugOut(kDbgLvlCritical, "GetRsrcRefCount not implemented\n");
	return kNoErr;
}

//----------------------------------------------------------------------------
tRsrcHndl CResourceModule::NewRsrc(tRsrcType rsrcType, void* pData)
{
	dbg_.DebugOut(kDbgLvlCritical, "NewRsrc not implemented\n");
	return kInvalidRsrcHndl;
}

//----------------------------------------------------------------------------
tErrType CResourceModule::DeleteRsrc(tRsrcHndl hndl)
{
	dbg_.DebugOut(kDbgLvlCritical, "DeleteRsrc not implemented\n");
	return kNoErr;
}



//============================================================================
// Instance management interface for the Module Manager
//============================================================================

static CResourceModule*	sinst = NULL;
//------------------------------------------------------------------------
extern "C" ICoreModule* CreateInstance(tVersion version)
{
	if (sinst == NULL)
		sinst = new CResourceModule;
	return sinst;
}
	
//------------------------------------------------------------------------
extern "C" void DestroyInstance(ICoreModule* ptr)
{
//		assert(ptr == sinst);
	delete sinst;
	sinst = NULL;
}

// EOF
