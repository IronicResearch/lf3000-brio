// TestPower.h

#include <cxxtest/TestSuite.h>
#include <boost/scoped_ptr.hpp>
#include <PowerMPI.h>
#include <EventMPI.h>
#include <EventListener.h>
#include <SystemErrors.h>
#include <SystemEvents.h>
#include <UnitTestUtils.h>

LF_USING_BRIO_NAMESPACE()

const enum tPowerState kBadPowerState = (enum tPowerState)0xBAADF00D;


//============================================================================
// MyEventListener
//============================================================================
const tEventType kMyHandledTypes[] = { kAllPowerEvents };
					
//----------------------------------------------------------------------------
class MyPwrEventListener : public IEventListener
{
public:
	MyPwrEventListener( ) : IEventListener(kMyHandledTypes, ArrayCount(kMyHandledTypes)) {}
	
	virtual tEventStatus Notify( const IEventMessage &msg )
	{
		U16 size		= msg.GetSizeInBytes();
		TS_ASSERT_EQUALS( size, sizeof(CPowerMessage) );
		type_ = msg.GetEventType();
		const CPowerMessage& pwrmsg = reinterpret_cast<const CPowerMessage&>(msg);
		tPowerData state;
		state.powerState = pwrmsg.GetPowerState();
		memcpy(&data_, &state, sizeof(data_));
		return kEventStatusOKConsumed;
	}
	tEventType	type_;
	tPowerData	data_;
};



//============================================================================
// TestPwrMgr functions
//============================================================================
class TestPwrMgr : public CxxTest::TestSuite, TestSuiteBase
{
private:
	CPowerMPI*				pwrmgr_;
	CEventMPI				evtmgr_;
	MyPwrEventListener		handler_;
public:
	//------------------------------------------------------------------------
	void setUp( )
	{
		pwrmgr_ = new CPowerMPI();
		handler_.data_.powerState		= kBadPowerState;
	}

	//------------------------------------------------------------------------
	void tearDown( )
	{
		delete pwrmgr_; 
		sleep(1);
	}
	
	//------------------------------------------------------------------------
	void testWasCreated( )
	{
		TS_ASSERT( pwrmgr_ != NULL );
		TS_ASSERT( pwrmgr_->IsValid() == true );
	}
	
	//------------------------------------------------------------------------
	void testCoreMPI( )
	{
		tVersion		version;
		const CString*	pName;
		const CURI*		pURI;
		
		pName = pwrmgr_->GetMPIName();
		TS_ASSERT_EQUALS( *pName, "PowerMPI" );
		version = pwrmgr_->GetModuleVersion();
		TS_ASSERT_EQUALS( version, 2 );
		pName = pwrmgr_->GetModuleName();
		TS_ASSERT_EQUALS( *pName, "Power" );
		pURI = pwrmgr_->GetModuleOrigin();
		TS_ASSERT_EQUALS( *pURI, "/LF/System/Power" );
	}
	
	//------------------------------------------------------------------------
	void testPollForState( )
	{
		tPowerData	data;
		data.powerState	= kBadPowerState;
		data.powerState = pwrmgr_->GetPowerState();
		TS_ASSERT_DIFFERS( data.powerState, kBadPowerState );
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
		TS_ASSERT_EQUALS( handler_.data_.powerState, kBadPowerState );
	}

};
