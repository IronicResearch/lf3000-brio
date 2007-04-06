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
//#include <KernelMPI.h>
#include <ResourceMPI.h>
#include <ResourcePriv.h>
//#include <ModuleRsrc.h>
#include <EventListener.h>

#include <stdio.h>
#include <string.h> // for strcmp
#include <dirent.h>
#include <sys/stat.h>
#include <linux/stat.h>
#include <stdio.h>

#include <map>		// FIXME: replace with non-STL implementation

const tVersion	kMPIVersion = 	MakeVersion(0,1);
const CString	kMpiName = 		"ResourceMPI";
const CURI		kModuleURI =	"URI";
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
	//----------------------------------------------------------------------------
	CResourceImpl* RetrieveImpl(U32 id)
	{
		MPIMap::iterator it = gMPIMap.find(id);
		if (it != gMPIMap.end())
			return &(it->second);
//FIXME		assert();
		return NULL;
	}

	//----------------------------------------------------------------------------
	tRsrcDescriptor* GetRsrcDescriptor(tRsrcHndl hndl)
	{
		U32 rsrcNum = (U32)hndl;
	
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
		
		hndl = (tRsrcHndl) -1;
		if (count < rsrcDescPtrArraySize)
		{
			if (pimpl->lastSearchType == kRsrcSearchTypeByURI)
				cstrTemp = pimpl->lastSearchPattern.uri;
				
			for (; count < rsrcDescPtrArraySize && !bFound; ++count)
			{
				switch (pimpl->lastSearchType)
				{
					case kRsrcSearchTypeUndefined:
						break;
					case kRsrcSearchTypeByURI:
						if (rsrcDescPtrArray[count].uri == cstrTemp)
						{
							bFound = true;
							hndl = (tRsrcHndl) count;
						}
						break;
					case kRsrcSearchTypeByID:
						if (rsrcDescPtrArray[count].id == pimpl->lastSearchPattern.id)
						{
							bFound = true;
							hndl = (tRsrcHndl) count;
						}
						break;
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
}



//----------------------------------------------------------------------------
CResourceModule::CResourceModule()
{
	if (devOpenCount == 0)		// if initial construction
	{
		struct tRsrcDeviceDescriptor tempDevice;
		CString URIpath;
		
		DefaultURI = "/home/lfu/LeapFrog/";
		
		URIpath = DefaultURI + "System/";
		strncpy(tempDevice.uriBase, URIpath.c_str(), MAX_RSRC_URI_SIZE);
		AddDeviceEntry(&tempDevice);
		URIpath = DefaultURI + "Applic/";
		strncpy(tempDevice.uriBase, URIpath.c_str(), MAX_RSRC_URI_SIZE);
		AddDeviceEntry(&tempDevice);
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
tErrType CResourceModule::GetModuleVersion(tVersion &version) const
{
	version = kMPIVersion;
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CResourceModule::GetModuleName(ConstPtrCString &pName) const
{
	pName = &kMpiName;
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CResourceModule::GetModuleOrigin(ConstPtrCURI &pURI) const
{
	pURI = &kModuleURI;
	return kNoErr;
}

//----------------------------------------------------------------------------
U32 CResourceModule::Register( )
{
	static U32 sNextIndex = 0;
	CResourceImpl	temp;
	gMPIMap.insert(MPIMap::value_type(++sNextIndex, temp));
	return sNextIndex;
}

//----------------------------------------------------------------------------
tErrType CResourceModule::SetDefaultURIPath(U32 id, const CURI &pURIPath)
{
	CResourceImpl* pimpl = RetrieveImpl(id);
	if (pimpl == NULL)
		return kResourceInvalidMPIIdErr;
		
	pimpl->mDefaultURI = pURIPath;
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CResourceModule::SetDefaultEventHandler(U32 id, 
									const IEventListener *pEventListener,
									tEventContext callerContext)
{
	CResourceImpl* pimpl = RetrieveImpl(id);
	if (pimpl == NULL)
		return kResourceInvalidMPIIdErr;
		
	pimpl->mpEventListener = pEventListener;
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
Boolean	CResourceModule::RsrcIsLoaded(tRsrcHndl hndl)
{
	return (GetRsrcDescriptor(hndl) != NULL) ? true : false;
}

//----------------------------------------------------------------------------
tErrType	CResourceModule::GetNumRsrcs(U32 id, U32 *pCount, const CURI *pURIPath)
{
	CResourceImpl* pimpl = RetrieveImpl(id);
	if (pimpl == NULL)
		return kResourceInvalidMPIIdErr;

	if (pCount == NULL)
		return kInvalidParamErr;

	*pCount = rsrcDescPtrArraySize; 
	return kNoErr;
}
 	
//----------------------------------------------------------------------------
tErrType	CResourceModule::GetNumRsrcs(U32 id, tRsrcType type, U32 *pCount, 
						const CURI *pURIPath)
{
	CResourceImpl* pimpl = RetrieveImpl(id);
	if (pimpl == NULL)
		return kResourceInvalidMPIIdErr;

	U32 index=0, count=0;
	tRsrcDescriptor *pRsrcDesc;

	if (pCount == NULL)
		return kInvalidParamErr;

	while (index < rsrcDescPtrArraySize)
	{
		if (rsrcDescPtrArray[index].type == type)
			count++;
		index++;
	}

	*pCount = count; 
	return kNoErr;
}
 
//----------------------------------------------------------------------------
tErrType	CResourceModule::FindRsrc(U32 id, const CURI &rsrcURI, tRsrcHndl &hndl,
						const CURI *pURIPath)
{
	CResourceImpl* pimpl = RetrieveImpl(id);
	if (pimpl == NULL)
		return kResourceInvalidMPIIdErr;
		
	CURI searchPath;
	 
	if (pURIPath != NULL)
	{
//		searchPath = *pURIPath + "/" + rsrcURI;
		return kInvalidParamErr;
	}
	else
	{
		searchPath = pimpl->mDefaultURI + "/" + rsrcURI;
	}
	
	// setup/save the search parameters for the FindNextRsrc function
	pimpl->lastSearchHndl = (tRsrcHndl) 0;
	pimpl->lastSearchType = kRsrcSearchTypeByURI;
	strncpy(pimpl->lastSearchPattern.uri, searchPath.c_str(), MAX_RSRC_URI_SIZE);
	
	hndl = FindRsrcRecord(pimpl);
	
	return (GetRsrcDescriptor(hndl) != NULL) ? kNoErr : kResourceNotFoundErr;
}
 
//----------------------------------------------------------------------------
tErrType	CResourceModule::FindRsrc(U32 id, tRsrcID rsrcID, tRsrcHndl &hndl,
						const CURI *pURIPath)
{
	CResourceImpl* pimpl = RetrieveImpl(id);
	if (pimpl == NULL)
		return kResourceInvalidMPIIdErr;
		
	// setup/save the search parameters for the FindNextRsrc function
	pimpl->lastSearchHndl = (tRsrcHndl) 0;
	pimpl->lastSearchType = kRsrcSearchTypeByID;
	pimpl->lastSearchPattern.id = rsrcID;
	
	hndl = FindRsrcRecord(pimpl);
	
	return (GetRsrcDescriptor(hndl) != NULL) ? kNoErr : kResourceNotFoundErr;
}

//----------------------------------------------------------------------------
tErrType	CResourceModule::FindRsrcs(U32 id, tRsrcHndl &hndl, const CURI *pURIPath)
{
	CResourceImpl* pimpl = RetrieveImpl(id);
	if (pimpl == NULL)
		return kResourceInvalidMPIIdErr;
		
	Boolean bFound = false;
	U32 count = (U32)hndl;
	
	// save the search parameters for the FindNextRsrc function
	pimpl->lastSearchHndl = hndl;
	pimpl->lastSearchType = kRsrcSearchTypeByHandle;
	
	if (count < rsrcDescPtrArraySize)
	{
		bFound = true;	
	}

	if (!bFound) 
	{
		pimpl->lastSearchHndl = (tRsrcHndl)(-1);
		return kResourceNotFoundErr;
	}

	return kNoErr;
}
 
//----------------------------------------------------------------------------
tErrType	CResourceModule::FindRsrcs(U32 id, tRsrcType type, tRsrcHndl &hndl, 
						const CURI *pURIPath)
{
	CResourceImpl* pimpl = RetrieveImpl(id);
	if (pimpl == NULL)
		return kResourceInvalidMPIIdErr;
		
	// setup/save the search parameters for the FindNextRsrc function
	pimpl->lastSearchHndl = (tRsrcHndl) 0;
	pimpl->lastSearchType = kRsrcSearchTypeByType;
	pimpl->lastSearchPattern.type = type;
	
	hndl = FindRsrcRecord(pimpl);
	
	return (GetRsrcDescriptor(hndl) != NULL) ? kNoErr : kResourceNotFoundErr;
}
 
//----------------------------------------------------------------------------
tErrType	CResourceModule::FindNextRsrc(U32 id, tRsrcHndl &hndl)
{
	CResourceImpl* pimpl = RetrieveImpl(id);
	if (pimpl == NULL)
		return kResourceInvalidMPIIdErr;
		
	hndl = FindRsrcRecord(pimpl);
	
	return (GetRsrcDescriptor(hndl) != NULL) ? kNoErr : kResourceNotFoundErr;
}
 
//----------------------------------------------------------------------------
tErrType	CResourceModule::GetRsrcURI(U32 id, tRsrcHndl hndl, ConstPtrCURI& pURI)
{
	CResourceImpl* pimpl = RetrieveImpl(id);
	if (pimpl == NULL)
		return kResourceInvalidMPIIdErr;
		
	tRsrcDescriptor* pRsrc;

	if ((pRsrc = GetRsrcDescriptor(hndl)) == NULL)
		return kResourceInvalidErr;

	pimpl->cstrReturnVal = pRsrc->uri;
	pURI = &pimpl->cstrReturnVal;

	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType	CResourceModule::GetRsrcName(U32 id, tRsrcHndl hndl, ConstPtrCString& pName)
{
	CResourceImpl* pimpl = RetrieveImpl(id);
	if (pimpl == NULL)
		return kResourceInvalidMPIIdErr;
		
	tRsrcDescriptor* pRsrc;

	if ((pRsrc = GetRsrcDescriptor(hndl)) == NULL)
		return kResourceInvalidErr;

	pimpl->cstrReturnVal = pRsrc->name;
	pName = &pimpl->cstrReturnVal;

	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType 	CResourceModule::GetRsrcID(tRsrcHndl hndl, tRsrcID &id)
{
	tRsrcDescriptor* pRsrc;

	if ((pRsrc = GetRsrcDescriptor(hndl)) == NULL)
		return kResourceInvalidErr;

	id = pRsrc->id;
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType 	CResourceModule::GetRsrcType(tRsrcHndl hndl, tRsrcType &rsrcType)
{
	tRsrcDescriptor* pRsrc;

	if ((pRsrc = GetRsrcDescriptor(hndl)) == NULL)
		return kResourceInvalidErr;

	rsrcType = pRsrc->type;
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType 	CResourceModule::GetRsrcVersion(tRsrcHndl hndl, tVersion &version)
{
	tRsrcDescriptor* pRsrc;

	if ((pRsrc = GetRsrcDescriptor(hndl)) == NULL)
		return kResourceInvalidErr;

	version = pRsrc->version;
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType 	CResourceModule::GetRsrcVersionStr(tRsrcHndl hndl, ConstPtrCString &pVersionStr)	
{
	tRsrcDescriptor* pRsrc;

	if ((pRsrc = GetRsrcDescriptor(hndl)) == NULL)
		return kResourceInvalidErr;

	static CString sTemp = "0.1";
	pVersionStr = &sTemp;

	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType 	CResourceModule::GetRsrcPackedSize(tRsrcHndl hndl, U32 &size)
{
	tRsrcDescriptor* pRsrc;

	if ((pRsrc = GetRsrcDescriptor(hndl)) == NULL)
		return kResourceInvalidErr;

	size = pRsrc->packedSize;
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType 	CResourceModule::GetRsrcUnpackedSize(tRsrcHndl hndl, U32 &size)
{
	tRsrcDescriptor* pRsrc;

	if ((pRsrc = GetRsrcDescriptor(hndl)) == NULL)
		return kResourceInvalidErr;

	size = pRsrc->unpackedSize;
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType  	CResourceModule::GetRsrcPtr(tRsrcHndl hndl, tPtr &pRsrcPtr)
{
	tRsrcDescriptor* pRsrc;

	if ((pRsrc = GetRsrcDescriptor(hndl)) == NULL)
		return kResourceInvalidErr;		

	if (pRsrcPtr == NULL)
		return kInvalidParamErr;			// invalid pointer supplied

	if (pRsrc->pRsrc == NULL)
		return kResourceNotLoadedErr;

	pRsrcPtr = pRsrc->pRsrc;
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType  	CResourceModule::OpenAllDevices(tOptionFlags openOptions,
						const IEventListener *pEventHandler,
						tEventContext eventContext)
{
	if (rscrOpenCount == 0)		// if initial construction
	{
		// foreach identified device
		for (U16 i = 0; i < rsrcDeviceArraySize; i++)
		{
			ScanDirForRscr(rsrcDeviceArray[i].uriBase);
		}
	}
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
tErrType  	CResourceModule::OpenRsrc(tRsrcHndl hndl, 
						tOptionFlags openOptions,
						const IEventListener *pEventHandler,
						tEventContext eventContext)
{
	tRsrcDescriptor* pRsrc;
	
	if ((pRsrc = GetRsrcDescriptor(hndl)) == NULL)
		return kResourceInvalidErr;
		
	if (pRsrc->useCount == 0)
	{
		// resource is not opened, open it now
		pRsrc->pFile = fopen(pRsrc->uri, "rb");
		
		// register open for later cleanup 
	}
	if (pRsrc->pFile != NULL)
	{
		pRsrc->useCount++;		
		return kNoErr;
	}
	
	return kResourceInvalidErr;
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
tErrType  	CResourceModule::ReadRsrc(tRsrcHndl hndl, void* pBuffer, U32 numBytesRequested,
						U32 *pNumBytesActual,
						tOptionFlags readOptions,
						const IEventListener *pEventHandler,
						tEventContext eventContext)
{
	tRsrcDescriptor* pRsrc;
	
	if ((pRsrc = GetRsrcDescriptor(hndl)) == NULL)
		return kResourceInvalidErr;
		
	if (pBuffer == NULL || numBytesRequested == 0)
		return kInvalidParamErr;			// invalid pointer supplied
		
	if (pRsrc->pFile != NULL)
	{
		fread(pBuffer, sizeof(char), numBytesRequested, pRsrc->pFile);
		return kNoErr;
	}
	return kResourceNotOpenErr;	
}  

//----------------------------------------------------------------------------
tErrType  	CResourceModule::LoadRsrc(tRsrcHndl hndl, tOptionFlags loadOptions,
							const IEventListener *pEventHandler,
						tEventContext eventContext)
{
	tRsrcDescriptor* pRsrc;
	FILE *rFile;
	unsigned char *cptr;

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
	if (pRsrc->pRsrc != NULL)
	{
		pRsrc->useCount++;		
		return kNoErr;
	}

	return kResourceInvalidErr;
}  

//----------------------------------------------------------------------------
tErrType  	CResourceModule::UnloadRsrc(tRsrcHndl hndl, 
						tOptionFlags unloadOptions,
						const IEventListener *pEventHandler,
						tEventContext eventContext)
{
	tRsrcDescriptor* pRsrc;
	
	if ((pRsrc = GetRsrcDescriptor(hndl)) == NULL)
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
tErrType CResourceModule::GetNumDevices(U16 *pCount)
{
	*pCount = 2; 
	return kNoErr;
}
//----------------------------------------------------------------------------
tErrType CResourceModule::GetNumDevices(tDeviceType type, U16 *pCount)
{
	*pCount = 2; 
	return kNoErr;
}
//----------------------------------------------------------------------------
tErrType CResourceModule::FindDevice(tDeviceHndl *pHndl)
{
	return kNoErr;
}
//----------------------------------------------------------------------------
tErrType CResourceModule::FindDevice(tDeviceType type, tDeviceHndl *pHndl)
{
	return kNoErr;
}
//----------------------------------------------------------------------------
tErrType CResourceModule::FindNextDevice(tDeviceHndl *pHndl)
{
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CResourceModule::GetDeviceName(tDeviceHndl hndl, const CString **ppName)
{
	return kNoErr;
}
//----------------------------------------------------------------------------
tErrType CResourceModule::GetDeviceType(tDeviceHndl hndl, tDeviceType *pType)
{
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CResourceModule::OpenDevice(tDeviceHndl hndl, 
									tOptionFlags openOptions,
									const IEventListener *pEventHandler,
									tEventContext eventContext)  
{
	return kNoErr;
}
//----------------------------------------------------------------------------
tErrType CResourceModule::CloseDevice(tDeviceHndl hndl)
{
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CResourceModule::GetNumRsrcPackages(U32 *pCount, 
									tRsrcPackageType type, 
									const CURI *pURIPath)
{
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CResourceModule::FindRsrcPackage(const CURI *pPackageURI,
									tRsrcPackageHndl *pHndl,
									const CURI *pURIPath)
{
	return kNoErr;
}
//----------------------------------------------------------------------------
tErrType CResourceModule::FindRsrcPackages(tRsrcPackageType type,
									tRsrcPackageHndl *pHndl, 
									const CURI *pURIPath)	
{
	return kNoErr;
}
//----------------------------------------------------------------------------
tErrType CResourceModule::FindNextRsrcPackage(tRsrcPackageHndl *pHndl)
{
	return kNoErr;
}

	// Getting package info
//----------------------------------------------------------------------------
tErrType CResourceModule::GetRsrcPackageURI(tRsrcPackageHndl hndl, const CURI **ppURI)
{
	return kNoErr;
}
//----------------------------------------------------------------------------
tErrType CResourceModule::GetRsrcPackageName(tRsrcPackageHndl hndl, const CString **ppName)
{
	return kNoErr;
}
//----------------------------------------------------------------------------
tErrType CResourceModule::GetRsrcPackageType(tRsrcPackageHndl hndl, tRsrcPackageType *pType)
{
	return kNoErr;
}
//----------------------------------------------------------------------------
tErrType CResourceModule::GetRsrcPackageVersion(tRsrcPackageHndl hndl, tVersion *pVersion)
{
	return kNoErr;
}
//----------------------------------------------------------------------------
tErrType CResourceModule::GetRsrcPackageVersionStr(tRsrcPackageHndl hndl, 
									const CString **ppVersionStr)
{
	return kNoErr;
}
//----------------------------------------------------------------------------
tErrType CResourceModule::GetRsrcPackageSizeUnpacked(tRsrcPackageHndl hndl, U32 *pSize)
{
	return kNoErr;
}
//----------------------------------------------------------------------------
tErrType CResourceModule::GetRsrcPackageSizePacked(tRsrcPackageHndl hndl, U32 *pSize)
{
	return kNoErr;
}

	// Opening & closing packages to find resources within them
//----------------------------------------------------------------------------
tErrType CResourceModule::OpenRsrcPackage(tRsrcPackageHndl hndl, 
									tOptionFlags openOptions,
									const IEventListener *pEventHandler,
									tEventContext eventContext)  
{
	return kNoErr;
}
//----------------------------------------------------------------------------
tErrType CResourceModule::CloseRsrcPackage(tRsrcPackageHndl hndl)
{
	return kNoErr;
}

	// Loading & unloading packages
//----------------------------------------------------------------------------
tErrType CResourceModule::LoadRsrcPackage(tRsrcPackageHndl hndl, 
									tOptionFlags loadOptions,
									const IEventListener *pEventHandler,
									tEventContext eventContext)  
{
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CResourceModule::UnloadRsrcPackage(tRsrcPackageHndl hndl, 
									tOptionFlags unloadOptions,
									const IEventListener *pEventHandler,
									tEventContext eventContext)  
{
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CResourceModule::SeekRsrc(tRsrcHndl hndl, U32 numSeekBytes, 
									tOptionFlags seekOptions)
{
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CResourceModule::WriteRsrc(tRsrcHndl hndl, const void *pBuffer, 
									U32 numBytesRequested, U32 *pNumBytesActual,
									tOptionFlags writeOptions,
									const IEventListener *pEventHandler,
									tEventContext eventContext)  
{
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CResourceModule::GetRsrcRefCount(tRsrcHndl hndl)
{
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CResourceModule::NewRsrc(tRsrcType rsrcType, void* pRsrc, tRsrcHndl *pHndl)
{
	return kNoErr;
}

//----------------------------------------------------------------------------
tErrType CResourceModule::DeleteRsrc(tRsrcHndl hndl)
{
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
