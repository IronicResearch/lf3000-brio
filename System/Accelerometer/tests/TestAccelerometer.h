// TestAccelerometer.h

#include <cxxtest/TestSuite.h>
#include <boost/scoped_ptr.hpp>
#include <AccelerometerMPI.h>
#include <EventMPI.h>
#include <EventListener.h>
#include <SystemErrors.h>
#include <SystemEvents.h>
#include <UnitTestUtils.h>

//For memcpy
#include <string.h>

LF_USING_BRIO_NAMESPACE()

const S32 kBadAccelerometerState = 0xBAADF00D;

//============================================================================
// MyEventListener
//============================================================================
const tEventType kMyHandledTypes[] = { kAllAccelerometerEvents };
					
//----------------------------------------------------------------------------
class MyAclmtrEventListener : public IEventListener
{
public:
	MyAclmtrEventListener( ) : IEventListener(kMyHandledTypes, ArrayCount(kMyHandledTypes)) {}
	
	virtual tEventStatus Notify( const IEventMessage &msg )
	{
		U16 size		= msg.GetSizeInBytes();
		TS_ASSERT_EQUALS( size, sizeof(CAccelerometerMessage) );
		type_ = msg.GetEventType();
		const CAccelerometerMessage& aclmsg = reinterpret_cast<const CAccelerometerMessage&>(msg);
		if (type_ == kAccelerometerDataChanged)
		{
			tAccelerometerData acldata = aclmsg.GetAccelerometerData();
			memcpy(&data_, &acldata, sizeof(data_));
		}
		if (type_ == kOrientationChanged)
		{
			orient_ = aclmsg.GetOrientation();
		}
		return kEventStatusOKConsumed;
	}
	tEventType			type_;
	tAccelerometerData	data_;
	S32					orient_;
};



//============================================================================
// TestAclmtrMgr functions
//============================================================================
class TestAclmtrMgr : public CxxTest::TestSuite, TestSuiteBase
{
private:
	CAccelerometerMPI*		aclmtr_;
//	CEventMPI				evtmgr_;
	MyAclmtrEventListener	handler_;
public:
	//------------------------------------------------------------------------
	void setUp( )
	{
		aclmtr_ = new CAccelerometerMPI();
		handler_.data_.accelX	= kBadAccelerometerState;
		handler_.data_.accelY	= kBadAccelerometerState;
		handler_.data_.accelZ	= kBadAccelerometerState;
		handler_.orient_		= kBadAccelerometerState;
	}

	//------------------------------------------------------------------------
	void tearDown( )
	{
		delete aclmtr_;
		sleep(1);
	}
	
	//------------------------------------------------------------------------
	void testWasCreated( )
	{
		PRINT_TEST_NAME();
		
		TS_ASSERT( aclmtr_ != NULL );
		TS_ASSERT( aclmtr_->IsValid() == true );
	}
	
	//------------------------------------------------------------------------
	void testCoreMPI( )
	{
		PRINT_TEST_NAME();
		
		tVersion		version;
		const CString*	pName;
		const CURI*		pURI;
		
		pName = aclmtr_->GetMPIName();
		TS_ASSERT_EQUALS( *pName, "AccelerometerMPI" );
		version = aclmtr_->GetModuleVersion();
		TS_ASSERT_EQUALS( version, 3 );
		pName = aclmtr_->GetModuleName();
		TS_ASSERT_EQUALS( *pName, "Accelerometer" );
		pURI = aclmtr_->GetModuleOrigin();
		TS_ASSERT_EQUALS( *pURI, "/LF/System/Accelerometer" );
	}
	
	//------------------------------------------------------------------------
	void testPollForState( )
	{
		PRINT_TEST_NAME();
		
		tAccelerometerData	data;
		data.accelX		= kBadAccelerometerState;
		data.accelY		= kBadAccelerometerState;
		data.accelZ		= kBadAccelerometerState;
		data = aclmtr_->GetAccelerometerData();
		TS_ASSERT_DIFFERS( data.accelX, kBadAccelerometerState );
		TS_ASSERT_DIFFERS( data.accelY, kBadAccelerometerState );
		TS_ASSERT_DIFFERS( data.accelZ, kBadAccelerometerState );
	}

	//------------------------------------------------------------------------
	void testSetupCallback( )
	{
		PRINT_TEST_NAME();
		
		TS_ASSERT_EQUALS( kNoErr, aclmtr_->RegisterEventListener(&handler_) );
		sleep(1);
		TS_ASSERT_EQUALS( handler_.type_, kAccelerometerDataChanged );
		TS_ASSERT_DIFFERS( handler_.data_.accelX, kBadAccelerometerState );
		TS_ASSERT_DIFFERS( handler_.data_.accelY, kBadAccelerometerState );
		TS_ASSERT_DIFFERS( handler_.data_.accelZ, kBadAccelerometerState );
		TS_ASSERT_EQUALS( kNoErr, aclmtr_->UnregisterEventListener(&handler_) );
	}

	//------------------------------------------------------------------------
	void testAccelerometerRate( )
	{
		U32	rate;
		rate = aclmtr_->GetAccelerometerRate();
		TS_ASSERT_DIFFERS( 0, rate );
		rate = kAccelerometerRateMin;
		TS_ASSERT_EQUALS( kNoErr, aclmtr_->SetAccelerometerRate(rate) );
		rate = aclmtr_->GetAccelerometerRate();
		TS_ASSERT_EQUALS( kAccelerometerRateMin, rate );
		rate = kAccelerometerRateMax;
		TS_ASSERT_EQUALS( kNoErr, aclmtr_->SetAccelerometerRate(rate) );
		rate = aclmtr_->GetAccelerometerRate();
		TS_ASSERT_EQUALS( kAccelerometerRateMax, rate );
		rate = kAccelerometerRateDefault;
		TS_ASSERT_EQUALS( kNoErr, aclmtr_->SetAccelerometerRate(rate) );
		rate = aclmtr_->GetAccelerometerRate();
		TS_ASSERT_EQUALS( kAccelerometerRateDefault, rate );
	}

	//------------------------------------------------------------------------
	void testAccelerometerMode( )
	{
		PRINT_TEST_NAME();
		
		tAccelerometerMode mode;
		mode = aclmtr_->GetAccelerometerMode();
		TS_ASSERT_EQUALS( kAccelerometerModeDisabled, mode );
		mode = kAccelerometerModeContinuous;
		TS_ASSERT_EQUALS( kNoErr, aclmtr_->SetAccelerometerMode(mode) );
		mode = aclmtr_->GetAccelerometerMode();
		TS_ASSERT_EQUALS( kAccelerometerModeContinuous, mode );
		mode = kAccelerometerModeOrientation;
		TS_ASSERT_EQUALS( kNoErr, aclmtr_->SetAccelerometerMode(mode) );
		mode = aclmtr_->GetAccelerometerMode();
		TS_ASSERT_EQUALS( kAccelerometerModeOrientation, mode );
		mode = kAccelerometerModeOneShot;
		TS_ASSERT_EQUALS( kNoErr, aclmtr_->SetAccelerometerMode(mode) );
		mode = aclmtr_->GetAccelerometerMode();
		TS_ASSERT_EQUALS( kAccelerometerModeOneShot, mode );
		mode = kAccelerometerModeDisabled;
		TS_ASSERT_EQUALS( kNoErr, aclmtr_->SetAccelerometerMode(mode) );
		mode = aclmtr_->GetAccelerometerMode();
		TS_ASSERT_EQUALS( kAccelerometerModeDisabled, mode );
	}

	//------------------------------------------------------------------------
	void testOrientation( )
	{
		PRINT_TEST_NAME();

		TS_ASSERT_EQUALS( kNoErr, aclmtr_->RegisterEventListener(&handler_) );
		TS_ASSERT_EQUALS( kNoErr, aclmtr_->SetAccelerometerMode(kAccelerometerModeOrientation) );
		sleep(1);
		TS_ASSERT_DIFFERS( handler_.orient_, kBadAccelerometerState );
		TS_ASSERT_EQUALS( handler_.orient_, aclmtr_->GetOrientation() );
		TS_ASSERT_EQUALS( kNoErr, aclmtr_->UnregisterEventListener(&handler_) );
	}

	//------------------------------------------------------------------------
	void testOneShot( )
	{
		PRINT_TEST_NAME();

		TS_ASSERT_EQUALS( kNoErr, aclmtr_->RegisterEventListener(&handler_) );
		TS_ASSERT_EQUALS( kNoErr, aclmtr_->SetAccelerometerMode(kAccelerometerModeOneShot) );
		sleep(1);
		TS_ASSERT_EQUALS( handler_.type_, kAccelerometerDataChanged );
		TS_ASSERT_DIFFERS( handler_.data_.accelX, kBadAccelerometerState );
		TS_ASSERT_DIFFERS( handler_.data_.accelY, kBadAccelerometerState );
		TS_ASSERT_DIFFERS( handler_.data_.accelZ, kBadAccelerometerState );
		tAccelerometerData data = handler_.data_;
		sleep(1);
		TS_ASSERT_EQUALS( handler_.data_.accelX, data.accelX );
		TS_ASSERT_EQUALS( handler_.data_.accelY, data.accelY );
		TS_ASSERT_EQUALS( handler_.data_.accelZ, data.accelZ );
		TS_ASSERT_EQUALS( kNoErr, aclmtr_->UnregisterEventListener(&handler_) );
	}
};
