// TestRsrcMgr.h

#include <cxxtest/TestSuite.h>
#include <boost/shared_array.hpp>
#include <RsrcMgrMPI.h>
#include <EventMessage.h>
#include <SystemErrors.h>
#include <SystemEvents.h>


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
class TestRsrcMgr : public CxxTest::TestSuite 
{
private:
	CRsrcMgrMPI*		rsrcmgr_;
	MyRsrcEventListener	handler_;
public:
	//------------------------------------------------------------------------
	void setUp( )
	{
		rsrcmgr_ = new CRsrcMgrMPI();
	}

	//------------------------------------------------------------------------
	void tearDown( )
	{
		rsrcmgr_->CloseAllDevices();
		delete rsrcmgr_; 
	}
	
	//------------------------------------------------------------------------
	void resetHandler( )
	{
		handler_.Reset();
	}

	//------------------------------------------------------------------------
	bool handlerIsReset( )
	{
		return handler_.GetEventMsg() == NULL;
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
		CString			empty;
		ConstPtrCString	pName = &empty;
		ConstPtrCURI	pURI = &empty;
		
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->GetMPIVersion(version) );
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->GetMPIName(pName) );
		TS_ASSERT_EQUALS( version, MakeVersion(0, 1) );
		TS_ASSERT_EQUALS( *pName, "RsrcMgrMPI" );

		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->GetModuleVersion(version) );
		TS_ASSERT_EQUALS( version, MakeVersion(0, 1) );
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->GetModuleName(pName) );
		TS_ASSERT_EQUALS( *pName, "RsrcMgr Module" );
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->GetModuleOrigin(pURI) );
		TS_ASSERT_EQUALS( *pURI, "RsrcMgr URI" );
	}
	
	//------------------------------------------------------------------------
	void testOpenCloseDevices( )
	{
		TS_ASSERT( handlerIsReset() );
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->OpenAllDevices() );
		TS_ASSERT( !handlerIsReset() );
		TS_ASSERT_EQUALS( kResourceAllDevicesOpenedEvent, handler_.GetEventMsg()->GetEventType() );
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->CloseAllDevices() );

		resetHandler();
		MyRsrcEventListener	overrideHandler;
		TS_ASSERT( handlerIsReset() );
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->OpenAllDevices(kNoOptionFlags, &overrideHandler) );
		TS_ASSERT( handlerIsReset() );
		TS_ASSERT_EQUALS( kResourceAllDevicesOpenedEvent, overrideHandler.GetEventMsg()->GetEventType() );
	}

	//------------------------------------------------------------------------
	void testMissingRsrc( )
	{
		tRsrcHndl		handle;
		ConstPtrCString	pStr;
		ConstPtrCURI	pURI;
//		tRsrcID			id;
		tRsrcType		type;
		tVersion		version;
		U32				size;
		tPtr			ptr;
		TS_ASSERT( handlerIsReset() );
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->OpenAllDevices() );
		TS_ASSERT( !handlerIsReset() );
		resetHandler();
		TS_ASSERT( handlerIsReset() );
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->SetDefaultURIPath("./testrsrcs") );
		TS_ASSERT_EQUALS( kResourceNotFoundErr, rsrcmgr_->FindRsrc("bogus_resource_name", handle) );
		TS_ASSERT_EQUALS( kResourceInvalidErr, rsrcmgr_->GetRsrcURI(handle, pURI) );
//		TS_ASSERT_EQUALS( kResourceInvalidErr, rsrcmgr_->GetRsrcID(handle, id) );
		TS_ASSERT_EQUALS( kResourceInvalidErr, rsrcmgr_->GetRsrcType(handle, type) );
		TS_ASSERT_EQUALS( kResourceInvalidErr, rsrcmgr_->GetRsrcVersion(handle, version) );
		TS_ASSERT_EQUALS( kResourceInvalidErr, rsrcmgr_->GetRsrcVersionStr(handle, pStr) );
		TS_ASSERT_EQUALS( kResourceInvalidErr, rsrcmgr_->GetRsrcPackedSize(handle, size) );
		TS_ASSERT_EQUALS( kResourceInvalidErr, rsrcmgr_->GetRsrcUnpackedSize(handle, size) );
		TS_ASSERT_EQUALS( kResourceInvalidErr, rsrcmgr_->GetRsrcPtr(handle, ptr) );
		TS_ASSERT( handlerIsReset() );
	}

	//------------------------------------------------------------------------
	void testFindAndLoadTextRsrc( )
	{
		tRsrcHndl		handle;
		ConstPtrCString	pStr;
		ConstPtrCURI	pURI;
//		tRsrcID			id;
		tRsrcType		type;
		tVersion		version;
		U32				size;
		tPtr			ptr;
		TS_ASSERT( handlerIsReset() );
		const U32		kSizeHelloWorldText	= 30;
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->OpenAllDevices() );
		TS_ASSERT( !handlerIsReset() );
		TS_ASSERT_EQUALS( kResourceAllDevicesOpenedEvent, handler_.GetEventMsg()->GetEventType() );
		resetHandler();
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->SetDefaultURIPath("./testrsrcs") );
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->FindRsrc( "HelloWorldText.txt", handle) );
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->GetRsrcURI(handle, pURI) );
		TS_ASSERT_EQUALS( *pURI, "testrsrcs/HelloWorldText.txt" );	//FIXME: find way to specify whole path from unit test
		// FIXME/tp: Remove ID concept?
//		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->GetRsrcID(handle, id) );
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->GetRsrcType(handle, type) );
//		TS_ASSERT_EQUALS( type, kTextFile );
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->GetRsrcVersion(handle, version) );
		TS_ASSERT_EQUALS( version, MakeVersion(1, 0) );
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->GetRsrcVersionStr(handle, pStr) );
//		TS_ASSERT_EQUALS( *pStr, "1.0" );
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->GetRsrcPackedSize(handle, size) );
		TS_ASSERT_EQUALS( size, kSizeHelloWorldText );
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->GetRsrcUnpackedSize(handle, size) );
		TS_ASSERT_EQUALS( size, kSizeHelloWorldText );
		TS_ASSERT_EQUALS( kResourceNotLoadedErr, rsrcmgr_->GetRsrcPtr(handle, ptr) );
		
		TS_ASSERT( handlerIsReset() );
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->LoadRsrc(handle) );
		TS_ASSERT( !handlerIsReset() );
		TS_ASSERT_EQUALS( kResourceLoadedEvent, handler_.GetEventMsg()->GetEventType() );
		resetHandler();
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->GetRsrcPtr(handle, ptr) );
		CString	helloWorld = reinterpret_cast<char*>(ptr);
		TS_ASSERT_EQUALS( helloWorld, "Hello World Text" );
		TS_ASSERT( handlerIsReset() );
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->UnloadRsrc(handle) );
		TS_ASSERT( !handlerIsReset() );
		TS_ASSERT_EQUALS( kResourceUnloadedEvent, handler_.GetEventMsg()->GetEventType() );
		resetHandler();
		TS_ASSERT_EQUALS( kResourceNotLoadedErr, rsrcmgr_->GetRsrcPtr(handle, ptr) );
	}

	//------------------------------------------------------------------------
	void testFindAndOpenBinaryRsrc( )
	{
		tRsrcHndl		handle;
		ConstPtrCString	pStr;
		ConstPtrCURI	pURI;
//		tRsrcID			id;
		tRsrcType		type;
		tVersion		version;
		U32				size;
		tPtr			ptr;
		const U32		kBellAudioSize	= 3000;
		const U32		kBufSize	= 80;
		U8 				buffer[kBufSize];
		
		TS_ASSERT( handlerIsReset() );
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->OpenAllDevices() );
		TS_ASSERT( !handlerIsReset() );
		TS_ASSERT_EQUALS( kResourceAllDevicesOpenedEvent, handler_.GetEventMsg()->GetEventType() );
		resetHandler();
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->SetDefaultURIPath("./testrsrcs") );
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->FindRsrc("Dummy.bin", handle) );
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->GetRsrcURI(handle, pURI) );
		TS_ASSERT_EQUALS( *pURI, "testrsrcs/Dummy.bin" );
		// FIXME/tp: Remove ID concept?
//		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->GetRsrcID(handle, id) );
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->GetRsrcType(handle, type) );
//		TS_ASSERT_EQUALS( type, kTextFile );
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->GetRsrcVersion(handle, version) );
		TS_ASSERT_EQUALS( version, MakeVersion(1, 0) );
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->GetRsrcVersionStr(handle, pStr) );
//		TS_ASSERT_EQUALS( *pStr, "1.0" );
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->GetRsrcPackedSize(handle, size) );
		TS_ASSERT_EQUALS( size, kBellAudioSize );
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->GetRsrcUnpackedSize(handle, size) );
		TS_ASSERT_EQUALS( size, kBellAudioSize );
		TS_ASSERT_EQUALS( kResourceNotLoadedErr, rsrcmgr_->GetRsrcPtr(handle, ptr) );
		TS_ASSERT_EQUALS( kResourceNotOpenErr, rsrcmgr_->ReadRsrc(handle, buffer, kBufSize) );
		
		TS_ASSERT( handlerIsReset() );
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->OpenRsrc(handle) );
		TS_ASSERT( !handlerIsReset() );
		TS_ASSERT_EQUALS( kResourceNotOpenErr, handler_.GetEventMsg()->GetEventType() );
		resetHandler();
		TS_ASSERT_EQUALS( kResourceNotLoadedErr, rsrcmgr_->GetRsrcPtr(handle, ptr) );
	
		TS_ASSERT( handlerIsReset() );
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->ReadRsrc(handle, buffer, kBufSize) );
		TS_ASSERT( !handlerIsReset() );
		TS_ASSERT_EQUALS( kResourceReadDoneEvent, handler_.GetEventMsg()->GetEventType() );
		resetHandler();
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
