// TestRsrcMgr.h

#include <cxxtest/TestSuite.h>
#include <boost/shared_array.hpp>
#include <ResourceMPI.h>
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
const tEventType kMyHandledTypes[] = { kAllResourceEvents };						
//----------------------------------------------------------------------------
class MyRsrcEventListener : public IEventListener
{

public:
	MyRsrcEventListener( ) 
		: IEventListener(kMyHandledTypes, ArrayCount(kMyHandledTypes))
	{		
	}
	
	virtual tEventStatus Notify( const IEventMessage& msg )
	{
		U16 size		= msg.GetSizeInBytes();
		msg_			= boost::shared_array<U8>(new U8[size]);
		memcpy(msg_.get(), &msg, size);
		return kEventStatusOKConsumed;
	}
	
	const IEventMessage* GetEventMsg() const
	{
		return reinterpret_cast<const IEventMessage*>(msg_.get());
	}
	
	void Reset()
	{
		msg_.reset();
	}
	
private:
	boost::shared_array<U8>	msg_;
};



//============================================================================
// TestRsrcMgr functions
//============================================================================
class TestRsrcMgr : public CxxTest::TestSuite, TestSuiteBase
{
private:
	CResourceMPI*		rsrcmgr_;
	MyRsrcEventListener	handler_;
public:
	//------------------------------------------------------------------------
	void setUp( )
	{
		rsrcmgr_ = new CResourceMPI(&handler_);
	}

	//------------------------------------------------------------------------
	void tearDown( )
	{
		rsrcmgr_->CloseAllDevices();
		delete rsrcmgr_; 
	}
	
	//------------------------------------------------------------------------
	void resetHandler( MyRsrcEventListener& handler )
	{
		handler.Reset();
	}

	//------------------------------------------------------------------------
	bool handlerIsReset( MyRsrcEventListener& handler )
	{
		return handler.GetEventMsg() == NULL;
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
	void testOpenCloseDevices( )
	{
		TS_ASSERT( handlerIsReset(handler_) );
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->OpenAllDevices() );
		TS_ASSERT( !handlerIsReset(handler_) );
		TS_ASSERT_EQUALS( kResourceAllDevicesOpenedEvent, handler_.GetEventMsg()->GetEventType() );
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->CloseAllDevices() );

		resetHandler(handler_);
		MyRsrcEventListener	overrideHandler;
		TS_ASSERT( handlerIsReset(handler_) );
		TS_ASSERT( handlerIsReset(overrideHandler) );
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->OpenAllDevices(kNoOptionFlags, &overrideHandler) );
		TS_ASSERT( handlerIsReset(handler_) );
		TS_ASSERT( !handlerIsReset(overrideHandler) );
		
		TS_ASSERT_EQUALS( kResourceAllDevicesOpenedEvent, overrideHandler.GetEventMsg()->GetEventType() );
		resetHandler(overrideHandler);
		TS_ASSERT( handlerIsReset(overrideHandler) );
	}

	//------------------------------------------------------------------------
	void testGetNumRsrcs()
	{
		U32				count;
		
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->OpenAllDevices() );
		TS_ASSERT_EQUALS( 36, rsrcmgr_->GetNumRsrcs( ) );
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->CloseAllDevices() );
	}
	
	//------------------------------------------------------------------------
	void testMissingRsrc( )
	{
		tRsrcHndl		handle;
		ConstPtrCString	pStr;
		ConstPtrCURI	pURI;
		tRsrcType		type;
		tVersion		version;
		U32				size;
		tPtr			ptr;
		
		resetHandler(handler_);
		TS_ASSERT( handlerIsReset(handler_) );
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->OpenAllDevices() );
		TS_ASSERT( !handlerIsReset(handler_) );
		resetHandler(handler_);
		TS_ASSERT( handlerIsReset(handler_) );
		rsrcmgr_->SetDefaultURIPath("./testrsrcs");
		handle = rsrcmgr_->FindRsrc("bogus_resource_name");
		TS_ASSERT_EQUALS( kInvalidRsrcHndl, handle );
		// generates low-level assert
//		pURI = rsrcmgr_->GetRsrcURI(handle);
//		TS_ASSERT_EQUALS( "", *pURI );
		// generates low-level assert
//		TS_ASSERT_EQUALS( kUndefinedVersion, rsrcmgr_->GetRsrcVersion(handle) );
//		const CString*	pVer;
		// generates low-level assert
//		pVer = rsrcmgr_->GetRsrcVersionStr(handle);
//		TS_ASSERT_EQUALS( *pVer, "" );
		// generates low-level assert
//		TS_ASSERT_EQUALS( 0, rsrcmgr_->GetRsrcPackedSize(handle) );
		// generates low-level assert
//		TS_ASSERT_EQUALS( 0, rsrcmgr_->GetRsrcUnpackedSize(handle) );
		// generates low-level assert
//		TS_ASSERT_EQUALS( (void*) NULL, rsrcmgr_->GetRsrcPtr(handle) );
		TS_ASSERT( handlerIsReset(handler_) );
	}

	//------------------------------------------------------------------------
	void testFindAndLoadTextRsrc( )
	{
		tRsrcHndl		handle;
		ConstPtrCString	pStr;
		ConstPtrCURI	pURI;
		tRsrcType		type;
		tVersion		version;
		U32				size;
		tPtr			ptr = (void*) 1;
		TS_ASSERT( handlerIsReset(handler_) );
		const U32		kSizeHelloWorldText	= 17;
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->OpenAllDevices() );
		TS_ASSERT( !handlerIsReset(handler_) );
		TS_ASSERT_EQUALS( kResourceAllDevicesOpenedEvent, handler_.GetEventMsg()->GetEventType() );
		resetHandler(handler_);
		rsrcmgr_->SetDefaultURIPath("/home/lfu/workspace/Brio2/apprsrc/");
		handle = rsrcmgr_->FindRsrc("app4.txt");
		TS_ASSERT_DIFFERS( kInvalidRsrcHndl, handle );
		pURI = rsrcmgr_->GetRsrcURI(handle);
		TS_ASSERT_EQUALS( "/home/lfu/workspace/Brio2/apprsrc/app4.txt", *pURI );
//		type = rsrcmgr_->GetRsrcType(handle);
//		TS_ASSERT_EQUALS( type, kTextFile );
		TS_ASSERT_EQUALS( 4, rsrcmgr_->GetRsrcVersion(handle) );
		const CString*	pVer;
		pVer = rsrcmgr_->GetRsrcVersionStr(handle);
		TS_ASSERT_EQUALS( *pVer, "4" );
		TS_ASSERT_EQUALS( kSizeHelloWorldText, rsrcmgr_->GetRsrcPackedSize(handle) );
		TS_ASSERT_EQUALS( kSizeHelloWorldText, rsrcmgr_->GetRsrcUnpackedSize(handle) );
		// not loaded yet test
		// generates low-level assert
//		TS_ASSERT_EQUALS( (void*) NULL, rsrcmgr_->GetRsrcPtr(handle) );
		
		TS_ASSERT( handlerIsReset(handler_) );
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->LoadRsrc(handle) );
		TS_ASSERT( !handlerIsReset(handler_) );
		TS_ASSERT_EQUALS( kResourceLoadedEvent, handler_.GetEventMsg()->GetEventType() );
		resetHandler(handler_);
		ptr = rsrcmgr_->GetRsrcPtr(handle);
		CString	helloWorld = reinterpret_cast<char*>(ptr);
		TS_ASSERT_EQUALS( helloWorld, "Hello World Text" );
		TS_ASSERT( handlerIsReset(handler_) );
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->UnloadRsrc(handle) );
		TS_ASSERT( !handlerIsReset(handler_) );
		TS_ASSERT_EQUALS( kResourceUnloadedEvent, handler_.GetEventMsg()->GetEventType() );
		resetHandler(handler_);
//		TS_ASSERT_EQUALS( kResourceNotLoadedErr, rsrcmgr_->GetRsrcPtr(ptr, handle) );
		// generates low-level assert
//		TS_ASSERT_EQUALS( (void*) NULL, rsrcmgr_->GetRsrcPtr(handle) );
	}

	//------------------------------------------------------------------------
	void testFindAndOpenBinaryRsrc( )
	{
		tRsrcHndl		handle;
		ConstPtrCString	pStr;
		ConstPtrCURI	pURI;
		tRsrcType		type;
		tVersion		version;
		U32				size;
		tPtr			ptr = (void*) 1;
		const U32		kBellAudioSize	= 3000;
		const U32		kBufSize	= 80;
		U8 				buffer[kBufSize];
		
		TS_ASSERT( handlerIsReset(handler_) );
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->OpenAllDevices() );
		TS_ASSERT( !handlerIsReset(handler_) );
		TS_ASSERT_EQUALS( kResourceAllDevicesOpenedEvent, handler_.GetEventMsg()->GetEventType() );
		resetHandler(handler_);
		rsrcmgr_->SetDefaultURIPath("/home/lfu/workspace/Brio2/apprsrc/Applic2");
		handle = rsrcmgr_->FindRsrc("app5.bin");
		TS_ASSERT_DIFFERS( kInvalidRsrcHndl, handle );
		pURI = rsrcmgr_->GetRsrcURI(handle);
		TS_ASSERT_EQUALS( "/home/lfu/workspace/Brio2/apprsrc/Applic2/app5.bin", *pURI );
//		type = rsrcmgr_->GetRsrcType(handle);
//		TS_ASSERT_EQUALS( type, kTextFile );
		TS_ASSERT_EQUALS( 25, rsrcmgr_->GetRsrcVersion(handle) );
		const CString*	pVer;
		pVer = rsrcmgr_->GetRsrcVersionStr(handle);
		TS_ASSERT_EQUALS( *pVer, "25" );
		TS_ASSERT_EQUALS( kBellAudioSize, rsrcmgr_->GetRsrcPackedSize(handle) );
		TS_ASSERT_EQUALS( kBellAudioSize, rsrcmgr_->GetRsrcUnpackedSize(handle) );
		// not loaded test
		// generates low-level assert
//		TS_ASSERT_EQUALS( (void*) NULL, rsrcmgr_->GetRsrcPtr(handle) );
		TS_ASSERT_EQUALS( kResourceNotOpenErr, rsrcmgr_->ReadRsrc(handle, buffer, kBufSize) );
		
		TS_ASSERT( handlerIsReset(handler_) );
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->OpenRsrc(handle, kOpenRsrcOptionRead) );
		TS_ASSERT( !handlerIsReset(handler_) );
		TS_ASSERT_EQUALS( kResourceOpenedEvent, handler_.GetEventMsg()->GetEventType() );
		resetHandler(handler_);
//		TS_ASSERT_EQUALS( kResourceNotLoadedErr, rsrcmgr_->GetRsrcPtr(ptr, handle) );
		// generates low-level assert
//		TS_ASSERT_EQUALS( (void*) NULL, rsrcmgr_->GetRsrcPtr(handle) );
	
		TS_ASSERT( handlerIsReset(handler_) );
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->ReadRsrc(handle, buffer, kBufSize) );
		TS_ASSERT( !handlerIsReset(handler_) );
		TS_ASSERT_EQUALS( kResourceReadDoneEvent, handler_.GetEventMsg()->GetEventType() );
		resetHandler(handler_);
		for(int i=0; i < kBufSize; ++i)
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
	
	void testFindOpenAndWriteBinaryRsrc( )
	{
		tRsrcHndl		handle;
		U32				size;
		const U32		kBufSize	= 80;
		U8 				buffer[kBufSize];
		U8 				buffer_inv[kBufSize];
		U8 				buffer_inv_test[kBufSize];
		
		TS_ASSERT( handlerIsReset(handler_) );
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->OpenAllDevices() );
		TS_ASSERT( !handlerIsReset(handler_) );
		TS_ASSERT_EQUALS( kResourceAllDevicesOpenedEvent, handler_.GetEventMsg()->GetEventType() );
		resetHandler(handler_);
		rsrcmgr_->SetDefaultURIPath("/home/lfu/workspace/Brio2/apprsrc/Applic2");
		handle = rsrcmgr_->FindRsrc("app5.bin");
		TS_ASSERT_DIFFERS( kInvalidRsrcHndl, handle );
		
		TS_ASSERT( handlerIsReset(handler_) );
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->OpenRsrc(handle,
			(kOpenRsrcOptionRead | kOpenRsrcOptionWrite) ) );
		TS_ASSERT( !handlerIsReset(handler_) );
		TS_ASSERT_EQUALS( kResourceOpenedEvent, handler_.GetEventMsg()->GetEventType() );
		resetHandler(handler_);

		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->ReadRsrc(handle, buffer, kBufSize) );
		TS_ASSERT( !handlerIsReset(handler_) );
		TS_ASSERT_EQUALS( kResourceReadDoneEvent, handler_.GetEventMsg()->GetEventType() );
		resetHandler(handler_);
		for(int i=0; i < kBufSize; ++i)
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
		for(int i=0; i < kBufSize; ++i)
		{
			buffer_inv[i] = ~(i + 160);
		}
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->WriteRsrc(handle, buffer_inv, kBufSize, &size) );
		TS_ASSERT_EQUALS( kBufSize, size );
		TS_ASSERT( !handlerIsReset(handler_) );
		TS_ASSERT_EQUALS( kResourceWriteDoneEvent, handler_.GetEventMsg()->GetEventType() );
		resetHandler(handler_);
		// seek back to the start of the file
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->SeekRsrc(handle, 0, kSeekRsrcOptionSet) );
		// read/verify normal data (check for no overwrite)
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->ReadRsrc(handle, buffer, kBufSize) );
		TS_ASSERT( !handlerIsReset(handler_) );
		TS_ASSERT_EQUALS( kResourceReadDoneEvent, handler_.GetEventMsg()->GetEventType() );
		resetHandler(handler_);
		for(int i=0; i < kBufSize; ++i)
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
		TS_ASSERT( !handlerIsReset(handler_) );
		TS_ASSERT_EQUALS( kResourceReadDoneEvent, handler_.GetEventMsg()->GetEventType() );
		resetHandler(handler_);
		for(int i=0; i < kBufSize; ++i)
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
		for(int i=0; i < kBufSize; ++i)
		{
			buffer_inv[i] = i + 160;
		}
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->WriteRsrc(handle, buffer_inv, kBufSize, &size) );
		TS_ASSERT_EQUALS( kBufSize, size );
		TS_ASSERT( !handlerIsReset(handler_) );
		TS_ASSERT_EQUALS( kResourceWriteDoneEvent, handler_.GetEventMsg()->GetEventType() );
		resetHandler(handler_);
		// seek back to the start of the file
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->SeekRsrc(handle, 0) );
		// read/verify the original data
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->ReadRsrc(handle, buffer, kBufSize) );
		TS_ASSERT( !handlerIsReset(handler_) );
		TS_ASSERT_EQUALS( kResourceReadDoneEvent, handler_.GetEventMsg()->GetEventType() );
		resetHandler(handler_);
		for(int i=0; i < kBufSize; ++i)
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
		TS_ASSERT( !handlerIsReset(handler_) );
		TS_ASSERT_EQUALS( kResourceReadDoneEvent, handler_.GetEventMsg()->GetEventType() );
		resetHandler(handler_);
		for(int i=0; i < kBufSize; ++i)
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

	//------------------------------------------------------------------------
	void testFindAndLoadTextRsrcOverloadListener( )
	{
		// TODO: Make Load/UnloadRsrc calls with explicit listener parameter
		// and make sure appropriate listeners get called
	}
		
	//------------------------------------------------------------------------
	void testFindAndOpenBinaryRsrcOverloadListener( )
	{
		// TODO: Make Open/Close/ReadRsrc calls with explicit listener parameter
		// and make sure appropriate listeners get called
	}
	
};
