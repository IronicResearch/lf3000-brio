//============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//============================================================================
//
// File:
//		Loader.cpp
//
// Description:
//		Implements Utility functions for Dataset Loader.
//
//============================================================================
#include <stdio.h>
#include <vector>	
#include <string>	
#include <fstream> 
#include <fcntl.h>
#include <math.h>
#include <SystemTypes.h>
#include <CoreTypes.h>	
#include <boost/shared_array.hpp>	

#include "Wrappers.h"
#include "Utility.h"

LF_BEGIN_BRIO_NAMESPACE()

/****************************************************************************
 * 																			*
 * Dataset loader functions.												*
 * 		StripTrailing() -remove whitespace when their second parameter      *
 * 						 is defaulted, but can also be used to remove 		*
 * 						 leading and trailing quote characters, etc.		*
 * 		FileSize(a)		-returns size of a.                         		*
 * 		OffsetsToPtrs()	-convert the offsets into pointers for a given   	*
 * 						 pointer address.									*
 * 		LoadDataset()	-loads the dataset into RAM, fixes up any pointers	*
 * 						 and returns a pointer to the dataset.	            *
 * **************************************************************************/
//----------------------------------------------------------------------------
string StripTrailing( const string& in, 
						const string& remove /* = std_string(_T(" \t\r\n")) */ )
{
	string out(in);
	while( out.size() && out.size() == out.find_last_of(remove) + 1 )
		out = out.substr(0, out.size() - 1);
	return out;
}

//----------------------------------------------------------------------------
// FileSize()
//----------------------------------------------------------------------------
size_t FileSize( const string& file )
{
	struct stat status;
	if( 0 == stat(StripTrailing(file, kDirSeparatorString).c_str(), &status) )
		return status.st_size;
	return 0;
}

string GetFileExtension(const string& file)
{
	size_t nPos = file.rfind('.', file.length());
	if (nPos != string::npos) {
		return(file.substr(nPos+1, file.length() - nPos));
	}
	return("");
}

//----------------------------------------------------------------------------
void OffsetsToPtrs( U8* pData, const PointerOffsets& ptr_offsets )
{
	U8* pBase = pData;
	for( PointerOffsets::const_iterator it = ptr_offsets.begin();
			it != ptr_offsets.end(); ++it )
	{
		*((U32*)(pBase + *it)) += (U32)pBase;
	}
}

//----------------------------------------------------------------------------
tAppRsrcDataSet* LoadDataset(const string& dsbinPath, const string& rbinPath)
{
	// TODO: Validate file paths!
	if (0 == FileSize(dsbinPath) || 0 == FileSize(rbinPath))
		return NULL;
	
	string ext1("dsetBin"), ext2("relinkBin");
	if ((ext1.compare(GetFileExtension(dsbinPath)) != 0) || (ext2.compare(GetFileExtension(rbinPath)) != 0))
				return NULL;
		
	boost::shared_array<U8> buf(new U8[FileSize(dsbinPath)]);
	EFdWrapper fd(dsbinPath, O_RDONLY | O_BINARY);
	read(fd, buf.get(), FileSize(dsbinPath));
		
	vector<U32> pointers;
	int size = FileSize(rbinPath)/sizeof(U32);
	U32* p = new U32[size];
		
	pointers.resize(size);
		
	EFdWrapper in(rbinPath, O_RDONLY, O_BINARY);
	read(in, p, FileSize(rbinPath));
	std::copy(p, p+size, pointers.begin());
	
	OffsetsToPtrs(buf.get(), pointers);
	tAppRsrcDataSet* pMD = (tAppRsrcDataSet*)buf.get();
			
	delete[] p;
	return pMD;
}

LF_END_BRIO_NAMESPACE()

// EOF
