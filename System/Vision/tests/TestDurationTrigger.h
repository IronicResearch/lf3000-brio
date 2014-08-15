
// Unit tests for VNDurationTrigger
// Reference: Test plan:
// http://wiki.leapfrog.com/display/CTD/Testing+Plans
// VisionTestingPlanv2.xlsx

#include <Vision/VNDurationTrigger.h>
#include <UnitTestUtils.h>

#include <cxxtest/TestSuite.h>
#include <memory>

using namespace LF::Vision;

class TestDurationTrigger : public CxxTest::TestSuite,
							LeapFrog::Brio::TestSuiteBase {

  public:

	TestDurationTrigger() {
		init();
	}

	// Constructor tests
	//  Constructor void    Expected: success
	void testConstructorDefaultShouldSucceedZero() {
		PRINT_TEST_NAME();
		std::auto_ptr<VNDurationTrigger>  trigger(new VNDurationTrigger);
		TS_ASSERT( trigger.get() );
		TS_ASSERT_DELTA(kVNDurationTriggerDefault, trigger->GetDuration(), TOLERANCE );
	}

	//	Constructor  duration > 0     Expected: success
	void testConstructorDurationPositiveShouldSucceed() {
		PRINT_TEST_NAME();
		std::auto_ptr<VNDurationTrigger>  trigger(new VNDurationTrigger(POSITIVE));
		TS_ASSERT_DELTA(POSITIVE, trigger->GetDuration(), TOLERANCE );
	}

	//	Constructor  duration < 0     Expected: succeed, duration == 0
	// TODO maybe enable production asserts,
	// once the test can disable it via CDebugMPI debug(kGroupVision);
	void testConstructorDurationNegativeShouldSucceedZero() {
		PRINT_TEST_NAME();
		VNDurationTrigger trigger(NEGATIVE);
		TS_ASSERT_DELTA(ZERO, trigger.GetDuration(), TOLERANCE );
	}

	//	Constructor  duration == 0     Expected: Succeed, Zero
	void testConstructorDurationZeroShouldSucceedZero() {
		PRINT_TEST_NAME();
		VNDurationTrigger trigger(ZERO);
		TS_ASSERT_DELTA(ZERO, trigger.GetDuration(), TOLERANCE );
	}

	//	SetDuration   > 0     Success
	void testSetDurationPositiveShouldSucceed() {
		PRINT_TEST_NAME();
		VNDurationTrigger trigger;
		trigger.SetDuration(POSITIVE);
		TS_ASSERT_DELTA(POSITIVE, trigger.GetDuration(), TOLERANCE);
	}

	//	SetDuration   < 0     Expected: Success, Duration = 0.0f
	void testSetDurationNegativeShouldSucceedZero() {
		PRINT_TEST_NAME();
		VNDurationTrigger trigger;
		trigger.SetDuration(NEGATIVE);
		TS_ASSERT_DELTA(ZERO, trigger.GetDuration(), TOLERANCE);
	}

	//	SetDuration == 0     Expected: Success, Duration = 0.0f
	void testSetDurationZeroSucceedZero() {
		PRINT_TEST_NAME();
		VNDurationTrigger trigger;
		trigger.SetDuration(ZERO);
		TS_ASSERT_DELTA(ZERO, trigger.GetDuration(), TOLERANCE);
	}

	//	GetDuration tests

	//	call after constructor with no input
	// Expect: Succeed, duration ==kVNDurationTriggerDefault
	// No test needed. Already covered by:
	// testConstructorDefaultShouldSucceedZero

	//	call after constructor with duration > 0	should match value passed in to constructor
	// No test needed. Already convered by:
	//testConstructorDurationPositiveShouldSucceed

	//	call after constructor with duration < 0	Succeed, duration == 0.0f
	// No test needed. Already covered by:
	// testConstructorDurationNegativeShouldSucceedZero

	//	call after constructor with duration == 0	Succeed, duration == 0.0f
	// No test needed. Already covered by:
	// testConstructorDurationZeroShouldSucceedZero


	//	call after SetDuration with duration > 0	should match value passed in to SetDuration
	// No test needed. Already tested by:
	// testSetDurationPositiveShouldSucceed()

	//	call after SetDuration with duration < 0	Succeed, duration == 0.0f
	// No test needed. Already covered by:
	// testSetDurationNegativeShouldSucceedZero

	//	call after SetDuration with duration == 0	Succeed, duration == 0.0f
	// No test needed. Already covered by:
	// testSetDurationZeroSucceedZero


private:

	void init(){
		POSITIVE = 10.0f;
		NEGATIVE = -10.0f;
		ZERO = 0.0f;
		TOLERANCE = 0.01f;
	}

	float POSITIVE;
	float NEGATIVE;
	float ZERO;
	float TOLERANCE;
};
