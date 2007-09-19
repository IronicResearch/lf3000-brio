// TestUSBDevice.h

#include <cxxtest/TestSuite.h>
#include <boost/scoped_ptr.hpp>
#include <USBDeviceMPI.h>
#include <USBDeviceTypes.h>
#include <EventMPI.h>
#include <EventListener.h>
#include <SystemErrors.h>
#include <SystemEvents.h>
#include <UnitTestUtils.h>

LF_USING_BRIO_NAMESPACE()

const U32 kBadUSBDeviceState = 0xBAADF00D;


//============================================================================
// MyEventListener
//============================================================================
const tEventType kMyHandledTypes[] = { kAllUSBDeviceEvents };
					
//----------------------------------------------------------------------------
class MyUSBEventListener : public IEventListener
{
public:
	MyUSBEventListener( ) : IEventListener(kMyHandledTypes, ArrayCount(kMyHandledTypes)) {}
	
	virtual tEventStatus Notify( const IEventMessage &msg )
	{
		U16 size		= msg.GetSizeInBytes();
		TS_ASSERT_EQUALS( size, sizeof(CUSBDeviceMessage) );
		type_ = msg.GetEventType();
		const CUSBDeviceMessage& usbmsg = reinterpret_cast<const CUSBDeviceMessage&>(msg);
		tUSBDeviceData state = usbmsg.GetUSBDeviceState();
		memcpy(&data_, &state, sizeof(data_));
		return kEventStatusOKConsumed;
	}
	tEventType	type_;
	tUSBDeviceData	data_;
};



//============================================================================
// TestUSBMgr functions
//============================================================================
class TestUSBMgr : public CxxTest::TestSuite, TestSuiteBase
{
private:
	CUSBDeviceMPI*				usbmgr_;
	MyUSBEventListener		handler_;
public:
	//------------------------------------------------------------------------
	void setUp( )
	{
		usbmgr_ = new CUSBDeviceMPI();
		handler_.data_.USBDeviceState = kBadUSBDeviceState;
	}

	//------------------------------------------------------------------------
	void tearDown( )
	{
		delete usbmgr_; 
	}
	
	//------------------------------------------------------------------------
	void testWasCreated( )
	{
		TS_ASSERT( usbmgr_ != NULL );
		TS_ASSERT( usbmgr_->IsValid() == true );
	}
	
	//------------------------------------------------------------------------
	void testCoreMPI( )
	{
		tVersion		version;
		const CString*	pName;
		const CURI*		pURI;
		
		pName = usbmgr_->GetMPIName();
		TS_ASSERT_EQUALS( *pName, "USBDeviceMPI" );
		version = usbmgr_->GetModuleVersion();
		TS_ASSERT_EQUALS( version, 2 );
		pName = usbmgr_->GetModuleName();
		TS_ASSERT_EQUALS( *pName, "USBDevice" );
		pURI = usbmgr_->GetModuleOrigin();
		TS_ASSERT_EQUALS( *pURI, "/LF/System/USBDevice" );
	}
	
	//------------------------------------------------------------------------
	void testPollForState( )
	{
		tUSBDeviceData	data;
		data.USBDeviceState		= kBadUSBDeviceState;
		data = usbmgr_->GetUSBDeviceState();
		TS_ASSERT_DIFFERS( data.USBDeviceState, kBadUSBDeviceState );
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
		TS_ASSERT_EQUALS( handler_.data_.USBDeviceState, kBadUSBDeviceState );
	}

	//------------------------------------------------------------------------
	void testDisableUSB( )
	{
		// Note: If device is connected to a Linux host, we may not be able to
		// disable.	 This is why we err == kUSBDeviceFailure does not count as a
		// test failure.
		tErrType err;
		tUSBDeviceData data = usbmgr_->GetUSBDeviceState();
		if(data.USBDeviceSupports & kUSBDeviceIsMassStorage) {
			err = usbmgr_->DisableUSBDeviceDrivers(kUSBDeviceIsMassStorage);
			TS_ASSERT(err == kNoErr || err == kUSBDeviceFailure);
		}
	}

	//------------------------------------------------------------------------
	void testEnableUSB( )
	{
		tErrType err;
		tUSBDeviceData data = usbmgr_->GetUSBDeviceState();
		if(data.USBDeviceSupports & kUSBDeviceIsMassStorage) {
			err = usbmgr_->EnableUSBDeviceDrivers(kUSBDeviceIsMassStorage);
			TS_ASSERT_EQUALS(err, kNoErr);
		}
	}

	//------------------------------------------------------------------------
	void testEnabledReportedProperly( )
	{
		tErrType err;
		tUSBDeviceData data = usbmgr_->GetUSBDeviceState();
		if(data.USBDeviceSupports & kUSBDeviceIsMassStorage) {
			err = usbmgr_->EnableUSBDeviceDrivers(kUSBDeviceIsMassStorage);
			TS_ASSERT_EQUALS(err, kNoErr);
            tUSBDeviceData data = usbmgr_->GetUSBDeviceState();
            TS_ASSERT(data.USBDeviceDriver & kUSBDeviceIsMassStorage);
        }
	}

};
