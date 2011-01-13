#ifndef LF_BRIO_WRAPPERS_H
#define LF_BRIO_WRAPPERS_H

#include <stdio.h>
#include <string>	
#include <fstream> 
#include <fcntl.h>
#include <sys/stat.h>

using namespace std;
LF_USING_BRIO_NAMESPACE()

//============================================================================
// EFdWrapper class
//============================================================================
class EFdWrapper
{
	// Wrap a file descriptor so a file is closed in the dtor.
	// Also provide a ctor that opens the file.
	// Useful when file I/O errors may cause many different exit points
	// from a routine that has an open file descriptor.
public:
	EFdWrapper( int fd = -1 );
	EFdWrapper( const string& strFile, int oflag, int pmode = S_IWRITE );
	~EFdWrapper( );
	
	bool Open( const string& strFile, int oflag, int pmode = S_IWRITE );
	void Close( );
	void Assign( int fd )	{ Close (); mfd = fd; }
	bool IsValid( )			{ return (mfd != -1); }
	operator int( )			{ return mfd; }
	
private:
	EFdWrapper( const EFdWrapper& );
	EFdWrapper& operator=( const EFdWrapper& );
	
	int		mfd;
};

//----------------------------------------------------------------------------
// EFdWrapper ctors
//----------------------------------------------------------------------------
inline EFdWrapper::EFdWrapper( int fd ) : mfd (fd)
{
	// Create from an open file descriptor
}
	
inline EFdWrapper::EFdWrapper( const string& strFile, int oflag, int pmode )
	: mfd (-1)
{
	// Open the named file
	Open (strFile, oflag, pmode);
}
	
//----------------------------------------------------------------------------
// EFdWrapper dtor
//----------------------------------------------------------------------------
inline EFdWrapper::~EFdWrapper( )
{
	// Cleanup by closing the file
	Close ();
}
	
//----------------------------------------------------------------------------
// EFdWrapper::Open( )
//----------------------------------------------------------------------------
inline bool EFdWrapper::Open( const string& strFile, int oflag, int pmode )
{
	// Close any previous files
	Close ();
	
	// Open the named file
	if( !strFile.empty() )
		mfd = open (strFile.c_str(), oflag, pmode);
	return IsValid ();
}
	
//----------------------------------------------------------------------------
// EFdWrapper::Close( )
//----------------------------------------------------------------------------
inline void EFdWrapper::Close( )
{
	// Close the file
	if( mfd != -1 )
		//VERIFY assure(close (mfd) == 0);
		close(mfd);
	mfd = -1;
}

#endif 

//eof
