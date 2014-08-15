
// Unit tests for VNIntervalTrigger
// Reference: Test plan:
// http://wiki.leapfrog.com/display/CTD/Testing+Plans
// VisionTestingPlanv2.xlsx

#include <Vision/VNIntervalTrigger.h>
//#include <Vision/VNVisionTypes.h>
//#include <DebugMPI.h>

#include <UnitTestUtils.h>
#include <cxxtest/TestSuite.h>
#include <memory>

using namespace LF::Vision;

class TestIntervalTrigger : public CxxTest::TestSuite,
							LeapFrog::Brio::TestSuiteBase {

  public:

	TestIntervalTrigger() {
		init();
	}

	// Constructor tests
	//  Constructor void    Expected: success
	void testConstructorDefaultShouldSucceed() {
		PRINT_TEST_NAME();
		std::auto_ptr<VNIntervalTrigger>  obj(new VNIntervalTrigger);
		TS_ASSERT( obj.get() );
	}

	//	Constructor  interval > 0     Expected: success
	void testConstructorIntervalPositiveShouldSucceed() {
		PRINT_TEST_NAME();
		std::auto_ptr<VNIntervalTrigger>  obj(new VNIntervalTrigger(POSITIVE));
		TS_ASSERT( obj.get() );
	}

	//	Constructor  interval < 0     Expected: Success but interval set to 0.0f
	// TODO maybe enable production asserts,
	// once the test can disable it via CDebugMPI debug(kGroupVision);
	void testConstructorIntervalNegativeShouldSucceedSetIntervalToZero() {
		PRINT_TEST_NAME();
		std::auto_ptr<VNIntervalTrigger>  trigger(new VNIntervalTrigger(NEGATIVE));
		TS_ASSERT_DELTA( 0.0f, trigger->GetInterval(), TOLERANCE );
	}

	//	Constructor  interval == 0     Expected: Success
	void testConstructorIntervalZeroNeedsShouldSucceed() {
		PRINT_TEST_NAME();
		std::auto_ptr<VNIntervalTrigger>  trigger(new VNIntervalTrigger(0.0f));
		TS_ASSERT_DELTA( 0.0f, trigger->GetInterval(), TOLERANCE );
	}

	//	SetInterval   > 0     Success
	void testSetIntervalPositiveShouldSucceed() {
		PRINT_TEST_NAME();
		VNIntervalTrigger trigger;
		trigger.SetInterval(POSITIVE);
		TS_ASSERT_DELTA(POSITIVE, trigger.GetInterval(), TOLERANCE);
	}

	//	SetInterval   < 0     Expected: Success, but interval == 0
	// TODO maybe enable production asserts,
	// once the test can disable it via CDebugMPI debug(kGroupVision);
	void testSetIntervalNegativeShouldSucceedZero() {
		PRINT_TEST_NAME();
		VNIntervalTrigger trigger;
		trigger.SetInterval(NEGATIVE);
		TS_ASSERT_DELTA(0.0f, trigger.GetInterval(), TOLERANCE);
	}

	//	SetInterval == 0     Expected: Success, inrerval 0.0f
	void testSetIntervalZeroShouldSucceedZero() {
		PRINT_TEST_NAME();
		VNIntervalTrigger trigger;
		trigger.SetInterval(0.0f);
		TS_ASSERT_DELTA(0.0f, trigger.GetInterval(), TOLERANCE);
	}

	//	GetInterval tests

	//	call after constructor with no input	should match kVNIntervalTriggerDefault value
	void testGetIntervalAfterConstructorNoInputShouldMatchDefaultValue() {
		VNIntervalTrigger trigger;
		TS_ASSERT_DELTA(kVNIntervalTriggerDefault, trigger.GetInterval(), TOLERANCE);
	}

	//	call after constructor with interval > 0	should match value passed in to constructor
	void testGetIntervalAfterConstructorPositiveInputShouldMatchInterval() {
		VNIntervalTrigger trigger(POSITIVE);
		TS_ASSERT_DELTA(POSITIVE, trigger.GetInterval(), TOLERANCE);
	}

	//	call after constructor with interval < 0	Expect, interval == 0.0f
	void testGetIntervalAfterConstructorNegativeInputShouldSucceedZero() {
		VNIntervalTrigger trigger(NEGATIVE);
		TS_ASSERT_DELTA(0.0f, trigger.GetInterval(), TOLERANCE);
	}

	//	call after constructor with interval == 0	Expeect:  Succeed, zero
	void testGetIntervalAfterConstructorZeroInputSucceedZero() {
		VNIntervalTrigger trigger(0.0f);
		TS_ASSERT_DELTA(0.0f, trigger.GetInterval(), TOLERANCE);
	}

	//	call after SetInterval with interval > 0	should match value passed in to SetInterval
	// No test needed. Already tested by:
	// testSetIntervalPositiveShouldSucceed()

	//	call after SetInterval with interval < 0	Expect: 0.0f
	// No test needed. Already tested by:
	// testSetIntervalNegativeShouldSucceedZero

	//	call after SetInterval with interval == 0	Expect: 0.0f
	// No test needed. Already tested by:
	// testSetIntervalZeroShouldSucceedZero

private:

	void init(){
		POSITIVE = 10.0f;
		NEGATIVE = -10.0f;
		TOLERANCE = 0.01f;
	}

	float POSITIVE;
	float NEGATIVE;
	float TOLERANCE;
};
