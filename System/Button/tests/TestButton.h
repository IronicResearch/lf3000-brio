// TestButton.h

#include <cxxtest/TestSuite.h>
#include <boost/scoped_ptr.hpp>
#include <ButtonMPI.h>
#include <EventMPI.h>
#include <EventListener.h>
#include <SystemErrors.h>
#include <SystemEvents.h>
#include <UnitTestUtils.h>


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
		CString			empty;
		CURI			emptyu;
		ConstPtrCString	pName = &empty;
		ConstPtrCURI	pURI = &emptyu;
		
		TS_ASSERT_EQUALS( kNoErr, btnmgr_->GetMPIVersion(version) );
		TS_ASSERT_EQUALS( kNoErr, btnmgr_->GetMPIName(pName) );
		TS_ASSERT_EQUALS( version, MakeVersion(0, 1) );
		TS_ASSERT_EQUALS( *pName, "ButtonMPI" );

		TS_ASSERT_EQUALS( kNoErr, btnmgr_->GetModuleVersion(version) );
		TS_ASSERT_EQUALS( version, MakeVersion(0, 1) );
		TS_ASSERT_EQUALS( kNoErr, btnmgr_->GetModuleName(pName) );
		TS_ASSERT_EQUALS( *pName, "Button" );
		TS_ASSERT_EQUALS( kNoErr, btnmgr_->GetModuleOrigin(pURI) );
		TS_ASSERT_EQUALS( *pURI, "Button FIXME" );
	}
	
	//------------------------------------------------------------------------
	void testPollForState( )
	{
		tButtonData	data;
		data.buttonState		= kBadButtonState;
		data.buttonTransition	= kBadButtonState;
		TS_ASSERT_EQUALS( kNoErr, btnmgr_->GetButtonState(data) );
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
		tListenerId	id;
		boost::scoped_ptr<CEventMPI> eventmgr(new CEventMPI());
		TS_ASSERT_EQUALS( kNoErr, eventmgr->RegisterEventListener(&handler_) );
		usleep(100);
		TS_ASSERT_EQUALS( handler_.data_.buttonState, kBadButtonState );
		TS_ASSERT_EQUALS( handler_.data_.buttonTransition, kBadButtonState );
	}

};
