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

// Okay, here's the sinful situation: Lightning (and perhaps other platforms)
// require that no application is running while USB is enabled.  Consistent with
// this requirement, the constructor for the USB manager asserts out if the USB
// is enabled.  So none of these unit tests work any more on Lightning.  Revert
// to svn 1980 to bring back the tests if necessary

					
//============================================================================
// TestUSBMgr functions
//============================================================================
class TestUSBMgr : public CxxTest::TestSuite, TestSuiteBase
{

public:
	//------------------------------------------------------------------------
	void setUp( )
	{

	}

	//------------------------------------------------------------------------
	void tearDown( )
	{

	}

	void testDummy( )
	{
		PRINT_TEST_NAME();
		
		TS_ASSERT(1);
	}
	
};
