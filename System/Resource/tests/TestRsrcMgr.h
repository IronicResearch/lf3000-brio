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
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->GetNumRsrcs( count ) );
		TS_ASSERT_EQUALS( count, 36 );
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->CloseAllDevices() );
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
		
		resetHandler(handler_);
		TS_ASSERT( handlerIsReset(handler_) );
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->OpenAllDevices() );
		TS_ASSERT( !handlerIsReset(handler_) );
		resetHandler(handler_);
		TS_ASSERT( handlerIsReset(handler_) );
		rsrcmgr_->SetDefaultURIPath("./testrsrcs");
		TS_ASSERT_EQUALS( kResourceNotFoundErr, rsrcmgr_->FindRsrc(handle, "bogus_resource_name") );
		TS_ASSERT_EQUALS( kResourceInvalidErr, rsrcmgr_->GetRsrcURI(pURI, handle) );
//		TS_ASSERT_EQUALS( kResourceInvalidErr, rsrcmgr_->GetRsrcID(id, handle) );
		TS_ASSERT_EQUALS( kResourceInvalidErr, rsrcmgr_->GetRsrcType(type, handle) );
		TS_ASSERT_EQUALS( kResourceInvalidErr, rsrcmgr_->GetRsrcVersion(version, handle) );
		TS_ASSERT_EQUALS( kResourceInvalidErr, rsrcmgr_->GetRsrcVersionStr(pStr, handle) );
		TS_ASSERT_EQUALS( kResourceInvalidErr, rsrcmgr_->GetRsrcPackedSize(size, handle) );
		TS_ASSERT_EQUALS( kResourceInvalidErr, rsrcmgr_->GetRsrcUnpackedSize(size, handle) );
		TS_ASSERT_EQUALS( kResourceInvalidErr, rsrcmgr_->GetRsrcPtr(ptr, handle) );
		TS_ASSERT( handlerIsReset(handler_) );
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
		tPtr			ptr = (void*) 1;
		TS_ASSERT( handlerIsReset(handler_) );
		const U32		kSizeHelloWorldText	= 17;
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->OpenAllDevices() );
		TS_ASSERT( !handlerIsReset(handler_) );
		TS_ASSERT_EQUALS( kResourceAllDevicesOpenedEvent, handler_.GetEventMsg()->GetEventType() );
		resetHandler(handler_);
//		srcmgr_->SetDefaultURIPath("./testrsrcs");
		rsrcmgr_->SetDefaultURIPath("../../../Lightning/Samples/BrioCubs/apprsrc");
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->FindRsrc( handle, "app1.txt") );
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->GetRsrcURI(pURI, handle) );
//		TS_ASSERT_EQUALS( *pURI, "testrsrcs/HelloWorldText.txt" );	//FIXME: find way to specify whole path from unit test
		TS_ASSERT_EQUALS( *pURI, "/home/lfu/workspace/Brio2/apprsrc/app4.txt" );	//FIXME: find way to specify whole path from unit test
		// FIXME/tp: Remove ID concept?
//		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->GetRsrcID(id, handle) 21);
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->GetRsrcType(type, handle) );
//		TS_ASSERT_EQUALS( type, kTextFile );
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->GetRsrcVersion(version, handle) );
		TS_ASSERT_EQUALS( version, 2 );
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->GetRsrcVersionStr(pStr, handle) );
		TS_ASSERT_EQUALS( *pStr, "0.1" );
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->GetRsrcPackedSize(size, handle) );
		TS_ASSERT_EQUALS( size, kSizeHelloWorldText );
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->GetRsrcUnpackedSize(size, handle) );
		TS_ASSERT_EQUALS( size, kSizeHelloWorldText );
		TS_ASSERT_EQUALS( kResourceNotLoadedErr, rsrcmgr_->GetRsrcPtr(ptr, handle) );
		
		TS_ASSERT( handlerIsReset(handler_) );
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->LoadRsrc(handle) );
		TS_ASSERT( !handlerIsReset(handler_) );
		TS_ASSERT_EQUALS( kResourceLoadedEvent, handler_.GetEventMsg()->GetEventType() );
		resetHandler(handler_);
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->GetRsrcPtr(ptr, handle) );
		CString	helloWorld = reinterpret_cast<char*>(ptr);
		TS_ASSERT_EQUALS( helloWorld, "Hello World Text" );
		TS_ASSERT( handlerIsReset(handler_) );
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->UnloadRsrc(handle) );
		TS_ASSERT( !handlerIsReset(handler_) );
		TS_ASSERT_EQUALS( kResourceUnloadedEvent, handler_.GetEventMsg()->GetEventType() );
		resetHandler(handler_);
		TS_ASSERT_EQUALS( kResourceNotLoadedErr, rsrcmgr_->GetRsrcPtr(ptr, handle) );
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
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->FindRsrc( handle, "app5.bin") );
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->GetRsrcURI(pURI, handle) );
		TS_ASSERT_EQUALS( *pURI, "/home/lfu/workspace/Brio2/apprsrc/Applic2/app5.bin" );
		// FIXME/tp: Remove ID concept?
//		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->GetRsrcID(hid, andle) );
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->GetRsrcType(type, handle) );
//		TS_ASSERT_EQUALS( type, kTextFile );
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->GetRsrcVersion(version, handle) );
		TS_ASSERT_EQUALS( version, 2 );
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->GetRsrcVersionStr(pStr, handle) );
		TS_ASSERT_EQUALS( *pStr, "0.1" );
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->GetRsrcPackedSize(size, handle) );
		TS_ASSERT_EQUALS( size, kBellAudioSize );
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->GetRsrcUnpackedSize(size, handle) );
		TS_ASSERT_EQUALS( size, kBellAudioSize );
		TS_ASSERT_EQUALS( kResourceNotLoadedErr, rsrcmgr_->GetRsrcPtr(ptr, handle) );
		TS_ASSERT_EQUALS( kResourceNotOpenErr, rsrcmgr_->ReadRsrc(handle, buffer, kBufSize) );
		
		TS_ASSERT( handlerIsReset(handler_) );
		TS_ASSERT_EQUALS( kNoErr, rsrcmgr_->OpenRsrc(handle) );
		TS_ASSERT( !handlerIsReset(handler_) );
		TS_ASSERT_EQUALS( kResourceOpenedEvent, handler_.GetEventMsg()->GetEventType() );
		resetHandler(handler_);
		TS_ASSERT_EQUALS( kResourceNotLoadedErr, rsrcmgr_->GetRsrcPtr(ptr, handle) );
	
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
