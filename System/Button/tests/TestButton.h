// TestButton.h

#include <cxxtest/TestSuite.h>
#include <boost/scoped_ptr.hpp>
#include <ButtonMPI.h>
#include <EventMPI.h>
#include <EventListener.h>
#include <SystemErrors.h>
#include <SystemEvents.h>
#include <UnitTestUtils.h>

LF_USING_BRIO_NAMESPACE()

const U32 kBadButtonState = 0xBAADF00D;


//============================================================================
// MyEventListener
//============================================================================
const tEventType kMyHandledTypes[] = { kAllButtonEvents };
					
//----------------------------------------------------------------------------
class MyBtnEventListener : public IEventListener
{
public:
	MyBtnEventListener( ) : IEventListener(kMyHandledTypes, ArrayCount(kMyHandledTypes)) {}
	
	virtual tEventStatus Notify( const IEventMessage &msg )
	{
		U16 size		= msg.GetSizeInBytes();
		TS_ASSERT_EQUALS( size, sizeof(CButtonMessage) );
		type_ = msg.GetEventType();
		const CButtonMessage& btnmsg = reinterpret_cast<const CButtonMessage&>(msg);
		tButtonData state = btnmsg.GetButtonState();
		memcpy(&data_, &state, sizeof(data_));
		return kEventStatusOKConsumed;
	}
	tEventType	type_;
	tButtonData	data_;
};



//============================================================================
// TestBtnMgr functions
//============================================================================
class TestBtnMgr : public CxxTest::TestSuite, TestSuiteBase
{
private:
	CButtonMPI*				btnmgr_;
	MyBtnEventListener		handler_;
public:
	//------------------------------------------------------------------------
	void setUp( )
	{
		btnmgr_ = new CButtonMPI();
		handler_.data_.buttonState		= kBadButtonState;
		handler_.data_.buttonTransition	= kBadButtonState;
	}

	//------------------------------------------------------------------------
	void tearDown( )
	{
		delete btnmgr_; 
	}
	
	//------------------------------------------------------------------------
	void testWasCreated( )
	{
		TS_ASSERT( btnmgr_ != NULL );
		TS_ASSERT( btnmgr_->IsValid() == true );
	}
	
	//------------------------------------------------------------------------
	void testCoreMPI( )
	{
		tVersion		version;
		const CString*	pName;
		const CURI*		pURI;
		
		pName = btnmgr_->GetMPIName();
		TS_ASSERT_EQUALS( *pName, "ButtonMPI" );
		version = btnmgr_->GetModuleVersion();
		TS_ASSERT_EQUALS( version, 2 );
		pName = btnmgr_->GetModuleName();
		TS_ASSERT_EQUALS( *pName, "Button" );
		pURI = btnmgr_->GetModuleOrigin();
		TS_ASSERT_EQUALS( *pURI, "/LF/System/Button" );
	}
	
	//------------------------------------------------------------------------
	void testPollForState( )
	{
		tButtonData	data;
		data.buttonState		= kBadButtonState;
		data.buttonTransition	= kBadButtonState;
		data = btnmgr_->GetButtonState();
		TS_ASSERT_DIFFERS( data.buttonState, kBadButtonState );
		TS_ASSERT_DIFFERS( data.buttonTransition, kBadButtonState );
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
		TS_ASSERT_EQUALS( handler_.data_.buttonState, kBadButtonState );
		TS_ASSERT_EQUALS( handler_.data_.buttonTransition, kBadButtonState );
	}

};
