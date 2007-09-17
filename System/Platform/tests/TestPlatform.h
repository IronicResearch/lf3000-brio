// TestPlatform.h

#include <cxxtest/TestSuite.h>
#include <boost/scoped_ptr.hpp>
#include <PlatformMPI.h>
#include <EventMPI.h>
#include <EventListener.h>
#include <SystemErrors.h>
#include <SystemEvents.h>
#include <UnitTestUtils.h>

LF_USING_BRIO_NAMESPACE()

const U32 kBadPlatformState = 0xBAADF00D;


//============================================================================
// MyEventListener
//============================================================================
const tEventType kMyHandledTypes[] = { kAllPlatformEvents };
					
//----------------------------------------------------------------------------
class MyPlatformEventListener : public IEventListener
{
public:
	MyPlatformEventListener( ) : IEventListener(kMyHandledTypes, ArrayCount(kMyHandledTypes)) {}
	
	virtual tEventStatus Notify( const IEventMessage &msg )
	{
		U16 size = msg.GetSizeInBytes();
		TS_ASSERT_EQUALS( size, sizeof(CPlatformMessage) );
		type_ = msg.GetEventType();
		const CPlatformMessage& platformMsg = reinterpret_cast<const CPlatformMessage&>(msg);
		tPlatformData state = platformMsg.GetPlatformState();
		memcpy(&data_, &state, sizeof(data_));
		return kEventStatusOKConsumed;
	}
	tEventType	type_;
	tPlatformData data_;
};



//============================================================================
// TestPlatformMgr functions
//============================================================================
class TestPlatformMgr : public CxxTest::TestSuite, TestSuiteBase
{
private:
	CPlatformMPI*			platformMgr_;
	MyPlatformEventListener	 handler_;
public:
	//------------------------------------------------------------------------
	void setUp( )
	{
		platformMgr_ = new CPlatformMPI();
		handler_.data_.platformState = kBadPlatformState;
	}

	//------------------------------------------------------------------------
	void tearDown( )
	{
		delete platformMgr_; 
	}
	
	//------------------------------------------------------------------------
	void testWasCreated( )
	{
		TS_ASSERT( platformMgr_ != NULL );
		TS_ASSERT( platformMgr_->IsValid() == true );
	}
	
	//------------------------------------------------------------------------
	void testCoreMPI( )
	{
		tVersion		version;
		const CString*	pName;
		const CURI*		pURI;
		
		pName = platformMgr_->GetMPIName();
		TS_ASSERT_EQUALS( *pName, "PlatformMPI" );
		version = platformMgr_->GetModuleVersion();
		TS_ASSERT_EQUALS( version, 2 );
		pName = platformMgr_->GetModuleName();
		TS_ASSERT_EQUALS( *pName, "Platform" );
		pURI = platformMgr_->GetModuleOrigin();
		TS_ASSERT_EQUALS( *pURI, "/LF/System/Platform" );
	}
	
	//------------------------------------------------------------------------
	void testPollForState( )
	{
		tPlatformData data;
		data.platformState = kBadPlatformState;
		data = platformMgr_->GetPlatformState();
		TS_ASSERT_DIFFERS( data.platformState, kBadPlatformState );
	}

	//------------------------------------------------------------------------
	void testSetupCallback( )
	{
		// Doesn't really test much, just that making the calls to
		// setup the handler succeed.  So long as the EventMPI gets
		// unit tested, we shouldn't need to do much more here.
		//
		boost::scoped_ptr<CEventMPI> eventmgr(new CEventMPI());
		TS_ASSERT_EQUALS( kNoErr, eventmgr->RegisterEventListener(&handler_) );
		usleep(100);
		TS_ASSERT_EQUALS( handler_.data_.platformState, kBadPlatformState );
	}

};
