// TestResourceBlocking.h

#include <cxxtest/TestSuite.h>
#include <boost/shared_array.hpp>
#include <ResourceMPI.h>
#include <DebugMPI.h>
#include <EventMessage.h>
#include <SystemErrors.h>
#include <SystemEvents.h>
#include <EventListener.h>
#include <EventMPI.h>
#include <UnitTestUtils.h>

LF_USING_BRIO_NAMESPACE()

//============================================================================
// MyRsrcEventListener
//============================================================================
CURI	gPkg = "LF/Brio/UnitTest/Resource";
CURI	gPkgA = "LF/Brio/UnitTest/Resource/A";
CURI	gPkgB = "LF/Brio/UnitTest/Resource/B";
CURI	gPkgC = "LF/Brio/UnitTest/Resource/C";
/*
template <typename T, typename D>
class scoped_resource
{
private:
	boost::shared_ptr<void> rsrc_;

public:
	explicit scoped_resource(T rsrc)
	{
		rsrc_ = boost::shared_ptr<T>(rsrc, D(mpi));
	}

    operator T() const { return static_cast<T>rsrc_.get(); }
};

struct RsrcHndlDeleter
{
	explicit RsrcHndlDeleter(CResourceMPI& mpi) : mpi_(mpi) {}
 	void operator()(void * p) { mpi_.UnloadRsrc(static_cast<T>(p)) }
 private:
 	CResourceMPI&	mpi_;
};

typedef scoped_resource<tRsrcHndl, RsrcHndlDeleter> RsrcHndl;

RsrcHndl rsrc(mpi.LoadRsrc(hndl), mpi);
*/

//============================================================================
// TestRsrcMgr functions
//============================================================================
class TestRsrcBlocking : public CxxTest::TestSuite, TestSuiteBase
{
private:
	CResourceMPI*		rsrcmgr_;
public:
	//------------------------------------------------------------------------
	void setUp( )
	{
		rsrcmgr_ = new CResourceMPI;
	}

	//------------------------------------------------------------------------
	void tearDown( )
	{
		rsrcmgr_->CloseAllDevices();
		delete rsrcmgr_; 
	}
	
	//------------------------------------------------------------------------
	void testWasCreated( )
	{
		TS_ASSERT( rsrcmgr_ != NULL );
		TS_ASSERT( rsrcmgr_->IsValid() == true );
	}
	
	//------------------------------------------------------------------------
	void testCoreMPI( )
	{
		tVersion		version;
		const CString*	pName;
		const CURI*		pURI;
		
		pName = rsrcmgr_->GetMPIName();
		TS_ASSERT_EQUALS( *pName, "ResourceMPI" );
		version = rsrcmgr_->GetModuleVersion();
		TS_ASSERT_EQUALS( version, 2 );
		pName = rsrcmgr_->GetModuleName();
		TS_ASSERT_EQUALS( *pName, "Resource" );
		pURI = rsrcmgr_->GetModuleOrigin();
		TS_ASSERT_EQUALS( *pURI, "/LF/System/Resource" );
	}
	
	//------------------------------------------------------------------------
	void testOpenCloseAllDevices( )
	{
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->OpenAllDevices() );
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->CloseAllDevices() );
	}

	//------------------------------------------------------------------------
	void testDoubleCloseAllDevices( )
	{
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->OpenAllDevices() );
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->CloseAllDevices() );
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->CloseAllDevices() );
	}

	//------------------------------------------------------------------------
	void testOpenCloseSingleDevices()
	{
		//TODO: Implement on future (non-Lightning) platform
	}

	//------------------------------------------------------------------------
	void testPackageEnumerations()
	{
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->OpenAllDevices() );
		rsrcmgr_->SetDefaultURIPath(gPkg);
		TS_ASSERT_EQUALS( static_cast<U16>(2), rsrcmgr_->GetNumPackages() );
		rsrcmgr_->SetDefaultURIPath(gPkgA);
		TS_ASSERT_EQUALS( static_cast<U16>(1), rsrcmgr_->GetNumPackages() );
		rsrcmgr_->SetDefaultURIPath(gPkgB);
		TS_ASSERT_EQUALS( static_cast<U16>(1), rsrcmgr_->GetNumPackages() );
		rsrcmgr_->SetDefaultURIPath(gPkgC);
		TS_ASSERT_EQUALS( static_cast<U16>(0), rsrcmgr_->GetNumPackages() );
		rsrcmgr_->SetDefaultURIPath(gPkg);
		TS_ASSERT_EQUALS( static_cast<U16>(2), rsrcmgr_->GetNumPackages() );
		
		tPackageHndl pkg;
		pkg = rsrcmgr_->FindFirstPackage(kPackageTypeAll, &gPkgC);
		TS_ASSERT_EQUALS( kInvalidPackageHndl, pkg );
		TS_ASSERT_THROWS( rsrcmgr_->GetPackageURI(pkg), UnitTestAssertException );
		TS_ASSERT_THROWS( rsrcmgr_->GetPackageType(pkg), UnitTestAssertException );
		TS_ASSERT_THROWS( rsrcmgr_->GetPackageVersion(pkg), UnitTestAssertException );
		TS_ASSERT_EQUALS( kInvalidPackageHndl, rsrcmgr_->FindNextPackage() );

		// TODO: test different types and versions when supported
		pkg = rsrcmgr_->FindFirstPackage(kPackageTypeAll, &gPkgA);
		TS_ASSERT_DIFFERS( kInvalidPackageHndl, pkg );
		TS_ASSERT_EQUALS( *rsrcmgr_->GetPackageURI(pkg), "LF/Brio/UnitTest/Resource/A/CountTest" );
		TS_ASSERT_EQUALS( rsrcmgr_->GetPackageType(pkg), kPackageTypeInvalid );
		TS_ASSERT_EQUALS( rsrcmgr_->GetPackageVersion(pkg), 1 );
		TS_ASSERT_EQUALS( kInvalidPackageHndl, rsrcmgr_->FindNextPackage() );
		
		rsrcmgr_->SetDefaultURIPath(gPkgB);
		pkg = rsrcmgr_->FindFirstPackage(kPackageTypeAll);
		TS_ASSERT_DIFFERS( kInvalidPackageHndl, pkg );
		TS_ASSERT_EQUALS( *rsrcmgr_->GetPackageURI(pkg), "LF/Brio/UnitTest/Resource/B/LoadTest" );
		TS_ASSERT_EQUALS( rsrcmgr_->GetPackageType(pkg), kPackageTypeInvalid );
		TS_ASSERT_EQUALS( rsrcmgr_->GetPackageVersion(pkg), 1 );
		TS_ASSERT_EQUALS( kInvalidPackageHndl, rsrcmgr_->FindNextPackage() );
		
		rsrcmgr_->SetDefaultURIPath(gPkg);
		pkg = rsrcmgr_->FindFirstPackage(kPackageTypeAll);
		TS_ASSERT_DIFFERS( kInvalidPackageHndl, pkg );
		TS_ASSERT_DIFFERS( kInvalidPackageHndl, rsrcmgr_->FindNextPackage() );
		TS_ASSERT_EQUALS( kInvalidPackageHndl, rsrcmgr_->FindNextPackage() );
	}

	//------------------------------------------------------------------------
	void testFindPackage()
	{
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->OpenAllDevices() );

		tPackageHndl pkg;
		pkg = rsrcmgr_->FindPackage("CountTest");
		TS_ASSERT_EQUALS( kInvalidPackageHndl, pkg );

		pkg = rsrcmgr_->FindPackage("CountTest", &gPkgA);
		TS_ASSERT_DIFFERS( kInvalidPackageHndl, pkg );
		TS_ASSERT_EQUALS( *rsrcmgr_->GetPackageURI(pkg), "LF/Brio/UnitTest/Resource/A/CountTest" );
		TS_ASSERT_EQUALS( rsrcmgr_->GetPackageType(pkg), kPackageTypeInvalid );
		TS_ASSERT_EQUALS( rsrcmgr_->GetPackageVersion(pkg), 1 );

		// TODO: test different types and versions when supported
		pkg = rsrcmgr_->FindPackage("LoadTest", &gPkgB);
		TS_ASSERT_DIFFERS( kInvalidPackageHndl, pkg );
		TS_ASSERT_EQUALS( *rsrcmgr_->GetPackageURI(pkg), "LF/Brio/UnitTest/Resource/B/LoadTest" );
		TS_ASSERT_EQUALS( rsrcmgr_->GetPackageType(pkg), kPackageTypeInvalid );
		TS_ASSERT_EQUALS( rsrcmgr_->GetPackageVersion(pkg), 1 );
	
		rsrcmgr_->SetDefaultURIPath(gPkgA);
		pkg = rsrcmgr_->FindPackage("CountTest");
		TS_ASSERT_DIFFERS( kInvalidPackageHndl, pkg );

		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->CloseAllDevices() );
	}

	//------------------------------------------------------------------------
	void testFindPackageFlexibleDefaultPathSyntax()
	{
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->OpenAllDevices() );

		CURI d1("LF/Brio/UnitTest/Resource/A");
		CURI d2("LF/Brio/UnitTest/Resource/A/");
		CURI d3("/LF/Brio/UnitTest/Resource/A");
		CURI d4("/LF/Brio/UnitTest/Resource/A/");
		
		tPackageHndl pkg1 = rsrcmgr_->FindPackage("CountTest", &d1);
		TS_ASSERT_DIFFERS( kInvalidPackageHndl, pkg1 );

		tPackageHndl pkg2 = rsrcmgr_->FindPackage("CountTest", &d2);
		TS_ASSERT_DIFFERS( kInvalidPackageHndl, pkg2 );

		tPackageHndl pkg3 = rsrcmgr_->FindPackage("CountTest", &d3);
		TS_ASSERT_DIFFERS( kInvalidPackageHndl, pkg3 );

		tPackageHndl pkg4 = rsrcmgr_->FindPackage("CountTest", &d4);
		TS_ASSERT_DIFFERS( kInvalidPackageHndl, pkg4 );

		rsrcmgr_->SetDefaultURIPath(d1);
		tPackageHndl pkg5 = rsrcmgr_->FindPackage("CountTest");
		TS_ASSERT_DIFFERS( kInvalidPackageHndl, pkg5 );

		rsrcmgr_->SetDefaultURIPath(d2);
		tPackageHndl pkg6 = rsrcmgr_->FindPackage("CountTest");
		TS_ASSERT_DIFFERS( kInvalidPackageHndl, pkg6 );

		rsrcmgr_->SetDefaultURIPath(d3);
		tPackageHndl pkg7 = rsrcmgr_->FindPackage("CountTest");
		TS_ASSERT_DIFFERS( kInvalidPackageHndl, pkg7 );

		rsrcmgr_->SetDefaultURIPath(d4);
		tPackageHndl pkg8 = rsrcmgr_->FindPackage("CountTest");
		TS_ASSERT_DIFFERS( kInvalidPackageHndl, pkg8 );
	}

	//------------------------------------------------------------------------
	void testGetNumRsrcs()
	{
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->OpenAllDevices() );
		TS_ASSERT_EQUALS( static_cast<U32>(0), rsrcmgr_->GetNumRsrcs() );
		rsrcmgr_->SetDefaultURIPath(gPkgA);
		tPackageHndl pkg = rsrcmgr_->FindPackage("CountTest");
		TS_ASSERT_DIFFERS( kInvalidPackageHndl, pkg );
		TS_ASSERT_EQUALS( static_cast<U32>(0), rsrcmgr_->GetNumRsrcs() );
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->OpenPackage(pkg) );
		TS_ASSERT_EQUALS( static_cast<U32>(5), rsrcmgr_->GetNumRsrcs() );
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->ClosePackage(pkg) );
		TS_ASSERT_EQUALS( static_cast<U32>(0), rsrcmgr_->GetNumRsrcs() );
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->CloseAllDevices() );
	}
	
	//------------------------------------------------------------------------
	void testDoubleClosePackage( )
	{
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->OpenAllDevices() );
		tPackageHndl pkg = rsrcmgr_->FindPackage("CountTest", &gPkgA);
		TS_ASSERT_DIFFERS( kInvalidPackageHndl, pkg );
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->OpenPackage(pkg) );
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->ClosePackage(pkg) );
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->ClosePackage(pkg) );
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->CloseAllDevices() );
	}

	//------------------------------------------------------------------------
	void testGetNumRsrcsByType()
	{
		// FIXME/tp: Implement for Lightning
	}
	
	//------------------------------------------------------------------------
	void testGetNumRsrcsWithOverlappingPackages()
	{
		//FIXME/tp: Test it!
		// Count should the sum minus the count of intersecting resources
	}
	
	// FIXME/tp: Test what happens when devices/pkgs/resources are not explicitly freed but CResourceMPI goes out of scope.
	// FIXME/tp: Work through correct smart handles for devices/pkgs/resources.
		
	//------------------------------------------------------------------------
	void testMissingRsrc( )
	{
		tRsrcHndl		handle;
		
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->OpenAllDevices() );
		tPackageHndl pkg = rsrcmgr_->FindPackage("CountTest", &gPkgA);
		TS_ASSERT_DIFFERS( kInvalidPackageHndl, pkg );
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->OpenPackage(pkg) );

		handle = rsrcmgr_->FindRsrc("bogus_resource_name");
		TS_ASSERT_EQUALS( kInvalidRsrcHndl, handle );
		// The following calls assert when provided an invalid handle
		TS_ASSERT_THROWS( rsrcmgr_->GetURI(handle), UnitTestAssertException );
		TS_ASSERT_THROWS( rsrcmgr_->GetVersion(handle), UnitTestAssertException );
		TS_ASSERT_THROWS( rsrcmgr_->GetPackedSize(handle), UnitTestAssertException );
		TS_ASSERT_THROWS( rsrcmgr_->GetUnpackedSize(handle), UnitTestAssertException );
		TS_ASSERT_THROWS( rsrcmgr_->GetPtr(handle), UnitTestAssertException );

	}

	//------------------------------------------------------------------------
	void testFindAndLoadTextRsrc( )
	{
		tRsrcHndl		handle;
		ConstPtrCURI	pURI;
		tPtr			ptr = (void*) 1;

		const U32		kSizeHelloWorldText	= 21;
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->OpenAllDevices() );
		tPackageHndl pkg = rsrcmgr_->FindPackage("LoadTest", &gPkgB);
		rsrcmgr_->SetDefaultURIPath(gPkgB);
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->OpenPackage(pkg) );
		handle = rsrcmgr_->FindRsrc("one");
		TS_ASSERT_DIFFERS( kInvalidRsrcHndl, handle );
		pURI = rsrcmgr_->GetURI(handle);
		TS_ASSERT_EQUALS( "LF/Brio/UnitTest/Resource/B/one", *pURI );
//		type = rsrcmgr_->GetType(handle);
//		TS_ASSERT_EQUALS( type, kTextFile );
		TS_ASSERT_EQUALS( 1, rsrcmgr_->GetVersion(handle) );
		TS_ASSERT_EQUALS( kSizeHelloWorldText, rsrcmgr_->GetPackedSize(handle) );
		TS_ASSERT_EQUALS( kSizeHelloWorldText, rsrcmgr_->GetUnpackedSize(handle) );
		
		// not loaded yet test
		TS_ASSERT_THROWS( rsrcmgr_->GetPtr(handle), UnitTestAssertException );
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->LoadRsrc(handle) );
		ptr = rsrcmgr_->GetPtr(handle);
		CString	helloWorld = CString(reinterpret_cast<char*>(ptr), 0, kSizeHelloWorldText);
		TS_ASSERT_EQUALS( helloWorld, "Hello from App1.rsrc\n" );
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->UnloadRsrc(handle) );
		TS_ASSERT_THROWS( rsrcmgr_->GetPtr(handle), UnitTestAssertException );
	}

	//------------------------------------------------------------------------
	void testFindAndOpenBinaryRsrc( )
	{
		tRsrcHndl		handle;
		ConstPtrCURI	pURI;
		const U32		kBellAudioSize	= 3000;
		const U32		kBufSize	= 80;
		U8 				buffer[kBufSize];
		
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->OpenAllDevices() );
		tPackageHndl pkg = rsrcmgr_->FindPackage("LoadTest", &gPkgB);
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->OpenPackage(pkg) );
		rsrcmgr_->SetDefaultURIPath(gPkgB);
		handle = rsrcmgr_->FindRsrc("five");
		TS_ASSERT_DIFFERS( kInvalidRsrcHndl, handle );
		pURI = rsrcmgr_->GetURI(handle);
		TS_ASSERT_EQUALS( "LF/Brio/UnitTest/Resource/B/five", *pURI );
//		type = rsrcmgr_->GetType(handle);
//		TS_ASSERT_EQUALS( type, kTextFile );
		TS_ASSERT_EQUALS( 5, rsrcmgr_->GetVersion(handle) );
		TS_ASSERT_EQUALS( kBellAudioSize, rsrcmgr_->GetPackedSize(handle) );
		TS_ASSERT_EQUALS( kBellAudioSize * 2, rsrcmgr_->GetUnpackedSize(handle) );
		
		// not loaded test
		TS_ASSERT_THROWS( rsrcmgr_->GetPtr(handle), UnitTestAssertException );
		TS_ASSERT_THROWS( rsrcmgr_->ReadRsrc(handle, buffer, kBufSize), UnitTestAssertException );
		
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->OpenRsrc(handle, kOpenRsrcOptionRead) );
		TS_ASSERT_THROWS( rsrcmgr_->GetPtr(handle), UnitTestAssertException );
	
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->ReadRsrc(handle, buffer, kBufSize) );
		for(unsigned int i=0; i < kBufSize; ++i)
		{
			// Report only a single failure in the buffer mismatch
			if(buffer[i] != i)
			{
				TS_ASSERT_EQUALS( buffer[i], i );
				break;
			}
		}
		
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->CloseRsrc(handle) );
	}
	
	void xtestFindOpenAndWriteBinaryRsrc( )
	{
		// FIXME/tp: Reenable after 6/15 delivery
		tRsrcHndl		handle;
		U32				size;
		const U32		kBufSize	= 80;
		U8 				buffer[kBufSize];
		U8 				buffer_inv[kBufSize];
		U8 				buffer_inv_test[kBufSize];
		
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->OpenAllDevices() );
		rsrcmgr_->SetDefaultURIPath("/home/lfu/workspace/Brio2/apprsrc/Applic2");
		handle = rsrcmgr_->FindRsrc("app5.bin");
		TS_ASSERT_DIFFERS( kInvalidRsrcHndl, handle );
		
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->OpenRsrc(handle,
			(kOpenRsrcOptionRead | kOpenRsrcOptionWrite) ) );

		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->ReadRsrc(handle, buffer, kBufSize) );

		for(unsigned int i=0; i < kBufSize; ++i)
		{
			// Report only a single failure in the buffer mismatch
			if(buffer[i] != i)
			{
				TS_ASSERT_EQUALS( buffer[i], i );
				break;
			}
		}
		// seek forward from current position (80 from current, or 160 from start)
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->SeekRsrc(handle, kBufSize, kSeekRsrcOptionCur) );
		// write inverted data to the buffer
		for(unsigned int i=0; i < kBufSize; ++i)
		{
			buffer_inv[i] = ~(i + 160);
		}
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->WriteRsrc(handle, buffer_inv, kBufSize, &size) );
		TS_ASSERT_EQUALS( kBufSize, size );
		// seek back to the start of the file
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->SeekRsrc(handle, 0, kSeekRsrcOptionSet) );
		// read/verify normal data (check for no overwrite)
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->ReadRsrc(handle, buffer, kBufSize) );
		for(unsigned int i=0; i < kBufSize; ++i)
		{
			// Report only a single failure in the buffer mismatch
			if(buffer[i] != i)
			{
				TS_ASSERT_EQUALS( buffer[i], i );
				break;
			}
		}
		// seek to modified region (SET)
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->SeekRsrc(handle, kBufSize*2, kSeekRsrcOptionSet) );
		// read/verify inverted data
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->ReadRsrc(handle, buffer_inv_test, kBufSize) );
		for(unsigned int i=0; i < kBufSize; ++i)
		{
			// Report only a single failure in the buffer mismatch
			if(buffer_inv_test[i] != buffer_inv[i])
			{
				TS_ASSERT_EQUALS( buffer_inv_test[i], buffer_inv[i] );
				break;
			}
		}
		// seek back to modified region (SET)
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->SeekRsrc(handle, kBufSize*2, kSeekRsrcOptionSet) );
		// write original data to the file
		for(unsigned int i=0; i < kBufSize; ++i)
		{
			buffer_inv[i] = i + 160;
		}
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->WriteRsrc(handle, buffer_inv, kBufSize, &size) );
		TS_ASSERT_EQUALS( kBufSize, size );
		// seek back to the start of the file
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->SeekRsrc(handle, 0) );
		// read/verify the original data
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->ReadRsrc(handle, buffer, kBufSize) );
		for(unsigned int i=0; i < kBufSize; ++i)
		{
			// Report only a single failure in the buffer mismatch
			if(buffer[i] != i)
			{
				TS_ASSERT_EQUALS( buffer[i], i );
				break;
			}
		}
		// seek to the restored region of the file (CUR)
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->SeekRsrc(handle, kBufSize, kSeekRsrcOptionCur) );
		// read/verify the restored data
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->ReadRsrc(handle, buffer_inv_test, kBufSize) );
		for(unsigned int i=0; i < kBufSize; ++i)
		{
			// Report only a single failure in the buffer mismatch
			if(buffer_inv_test[i] != buffer_inv[i])
			{
				TS_ASSERT_EQUALS( buffer_inv_test[i], buffer_inv[i] );
				break;
			}
		}
		
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->CloseRsrc(handle) );
	}
};

// Totest: packages containing packages
// packages containing resources with URIs with alternate bases

