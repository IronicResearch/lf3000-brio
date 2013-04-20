// TestButton.h

#include <cxxtest/TestSuite.h>
#include <boost/scoped_ptr.hpp>
#include <ButtonMPI.h>
#include <EventMPI.h>
#include <EventListener.h>
#include <SystemErrors.h>
#include <SystemEvents.h>
#include <UnitTestUtils.h>
#include <KeypadTypes.h>

//For memcpy
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

#include "Utility.h"


LF_USING_BRIO_NAMESPACE()

const U32 kBadButtonState = 0xBAADF00D;


//============================================================================
// MyEventListener
//============================================================================
const tEventType kMyHandledTypes[] = { kAllButtonEvents, kAllTouchEvents, kAllKeypadEvents };
					
//----------------------------------------------------------------------------
class MyBtnEventListener : public IEventListener
{
public:
	MyBtnEventListener( ) : IEventListener(kMyHandledTypes, ArrayCount(kMyHandledTypes)) {}
	
	virtual tEventStatus Notify( const IEventMessage &msg )
	{
		U16 size		= msg.GetSizeInBytes();
		type_ = msg.GetEventType();
		if (type_ == kButtonStateChanged)
		{
			TS_ASSERT_EQUALS( size, sizeof(CButtonMessage) );
			const CButtonMessage& btnmsg = reinterpret_cast<const CButtonMessage&>(msg);
			tButtonData state = btnmsg.GetButtonState();
			memcpy(&data_, &state, sizeof(data_));
			return kEventStatusOKConsumed;
		}
		if (type_ == kTouchStateChanged)
		{
			TS_ASSERT_EQUALS( size, sizeof(CTouchMessage) );
			const CTouchMessage& tmsg = reinterpret_cast<const CTouchMessage&>(msg);
			td_ = tmsg.GetTouchState();
			printf("%d, %d, %d\n", td_.touchState, td_.touchX, td_.touchY);
			return kEventStatusOKConsumed;
		}
		if (type_ == kTouchEventMultiTouch)
		{
			TS_ASSERT_EQUALS( size, sizeof(CTouchMessage) );
			const CTouchMessage& tmsg = reinterpret_cast<const CTouchMessage&>(msg);
			mtd_ = tmsg.GetMultiTouchState();
			printf("%d: %d, %d, %d, %d, %d\n", mtd_.id, mtd_.td.touchState, mtd_.td.touchX, mtd_.td.touchY, mtd_.touchWidth, mtd_.touchHeight);
			return kEventStatusOKConsumed;
		}
		if (type_ == kKeypadStateChanged)
		{
			TS_ASSERT_EQUALS( size, sizeof(CKeypadMessage) );
			const CKeypadMessage& kmsg = reinterpret_cast<const CKeypadMessage&>(msg);
			tKeypadData kd = kmsg.GetKeypadState();
			tWChar key = (tWChar)kd.keyCode;
			printf("%02x: %c (@%d.%d)\n", (int)kd.keyCode, (char)key, (int)kd.time.seconds, (int)kd.time.microSeconds);
			return kEventStatusOKConsumed;
		}
		return kEventStatusOK;
	}
	tEventType	type_;
	tButtonData	data_;
	tTouchData  td_;
	tMultiTouchData mtd_;
};



//============================================================================
// TestBtnMgr functions
//============================================================================
class TestBtnMgr : public CxxTest::TestSuite, TestSuiteBase
{
private:
	CButtonMPI*				btnmgr_;
	CEventMPI				evtmgr_;
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
		sleep(1);
	}
	
	//------------------------------------------------------------------------
	void testWasCreated( )
	{
		PRINT_TEST_NAME();
		
		TS_ASSERT( btnmgr_ != NULL );
		TS_ASSERT( btnmgr_->IsValid() == true );
	}
	
	//------------------------------------------------------------------------
	void testCoreMPI( )
	{
		PRINT_TEST_NAME();
		
		tVersion		version;
		const CString*	pName;
		const CURI*		pURI;
		
		tPlatformCaps test = kCapsTouchscreen;
		TS_ASSERT(HasPlatformCapability(test));

		pName = btnmgr_->GetMPIName();
		TS_ASSERT_EQUALS( *pName, "ButtonMPI" );
		version = btnmgr_->GetModuleVersion();
		TS_ASSERT_EQUALS( version, 3 );
		pName = btnmgr_->GetModuleName();
		TS_ASSERT_EQUALS( *pName, "Button" );
		pURI = btnmgr_->GetModuleOrigin();
		TS_ASSERT_EQUALS( *pURI, "/LF/System/Button" );
	}
	
	//------------------------------------------------------------------------
	void testPollForState( )
	{
		PRINT_TEST_NAME();
		
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
		PRINT_TEST_NAME();
		
		boost::scoped_ptr<CEventMPI> eventmgr(new CEventMPI());
		TS_ASSERT_EQUALS( kNoErr, eventmgr->RegisterEventListener(&handler_) );
		usleep(100);
		TS_ASSERT_EQUALS( handler_.data_.buttonState, kBadButtonState );
		TS_ASSERT_EQUALS( handler_.data_.buttonTransition, kBadButtonState );
	}

	//------------------------------------------------------------------------
	void XXXXtestTouchRate( )
	{
#ifndef EMULATION
		U32	rate;
		rate = btnmgr_->GetTouchRate();
		TS_ASSERT_DIFFERS( 0, rate );
		rate = kTouchRateMin;
		TS_ASSERT_EQUALS( kNoErr, btnmgr_->SetTouchRate(rate) );
		rate = btnmgr_->GetTouchRate();
		TS_ASSERT_EQUALS( kTouchRateMin, rate );
		rate = kTouchRateMax;
		TS_ASSERT_EQUALS( kNoErr, btnmgr_->SetTouchRate(rate) );
		rate = btnmgr_->GetTouchRate();
		TS_ASSERT_EQUALS( kTouchRateMax, rate );
		rate = kTouchRateDefault;
		TS_ASSERT_EQUALS( kNoErr, btnmgr_->SetTouchRate(rate) );
		rate = btnmgr_->GetTouchRate();
		TS_ASSERT_EQUALS( kTouchRateDefault, rate );
#endif
	}

	//------------------------------------------------------------------------
	void testTouchMode( )
	{
		PRINT_TEST_NAME();
		
#ifndef EMULATION
		tTouchMode mode;
		mode = btnmgr_->GetTouchMode();
		TS_ASSERT_EQUALS( kTouchModeDefault, mode );
		mode = kTouchModeDrawing;
		TS_ASSERT_EQUALS( kNoErr, btnmgr_->SetTouchMode(mode) );
		mode = btnmgr_->GetTouchMode();
		TS_ASSERT_EQUALS( kTouchModeDrawing, mode );
		mode = kTouchModeDefault;
		TS_ASSERT_EQUALS( kNoErr, btnmgr_->SetTouchMode(mode) );
		mode = btnmgr_->GetTouchMode();
		TS_ASSERT_EQUALS( kTouchModeDefault, mode );
#endif
	}

	//------------------------------------------------------------------------
	void testTouchEvents( )
	{
		PRINT_TEST_NAME();

#ifndef EMULATION
		tTouchMode mode;
		mode = btnmgr_->GetTouchMode();
		TS_ASSERT_EQUALS( kTouchModeDefault, mode );
		if (HasPlatformCapability(kCapsMultiTouch))
			TS_ASSERT_EQUALS( kNoErr, btnmgr_->SetTouchMode(kTouchModeMultiTouch) );
		boost::scoped_ptr<CEventMPI> eventmgr(new CEventMPI());
		TS_ASSERT_EQUALS( kNoErr, eventmgr->RegisterEventListener(&handler_) );
		printf("**** start pressing touchscreen *****\n");
		for (int i = 0; i < 30; i++)
			sleep(1);
#endif
	}

	//------------------------------------------------------------------------
	void testVirtualKeypadEvents( )
	{
		PRINT_TEST_NAME();

		CString keyseq = "The Quick Brown Fox Jumped over the Lazy Dog.\n";
		boost::scoped_ptr<CEventMPI> eventmgr(new CEventMPI());
		TS_ASSERT_EQUALS( kNoErr, eventmgr->RegisterEventListener(&handler_) );

		for (int i = 0; i < keyseq.length(); i++)
		{
			tKeypadData keyrec;
			keyrec.keyCode = keyseq.at(i);
			keyrec.keyState = 1;
			struct timeval tv;
			gettimeofday(&tv, NULL);
			keyrec.time.seconds = tv.tv_sec;
			keyrec.time.microSeconds = tv.tv_usec;
			CKeypadMessage keymsg(keyrec);
			eventmgr->PostEvent(keymsg, 255, &handler_);
			usleep(random() % 100000);
		}
	}
};
