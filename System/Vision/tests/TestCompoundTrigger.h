
// Unit tests for VNCompoundTrigger
// Reference: Test plan:
// http://wiki.leapfrog.com/display/CTD/Testing+Plans
// VisionTestingPlanv2.xlsx

#include <Vision/VNCompoundTrigger.h>
#include <Vision/VNSpatialTrigger.h>
#include <Vision/VNTemporalTriggering.h>
#include <UnitTestUtils.h>

#include <cxxtest/TestSuite.h>
#include <memory>

using namespace LF::Vision;

class TestCompoundTrigger : public CxxTest::TestSuite,
							LeapFrog::Brio::TestSuiteBase {

  public:

	TestCompoundTrigger() {
		init();
	}

	~TestCompoundTrigger() {
		cleanup();
	}

	//	valid spatial and valid temporal.    Expected: success
	void testConstructorValidSpatialValidTemporalShouldSucceed() {
		PRINT_TEST_NAME();
		std::auto_ptr<VNCompoundTrigger>  obj(new VNCompoundTrigger(VALID_SPATIAL, VALID_TEMPORAL));
		TS_ASSERT( obj.get() );
	}

	//	NULL spatial and valid temporal   Expected: Success creation
	// TODO revisit for possible addition of asserts in production code.
	// For now, the code checks for non NULL
	// Triggered() returns false if either are NULL
	void testConstructorNullSpatialValidTemporalShouldSucceed() {
		PRINT_TEST_NAME();
		std::auto_ptr<VNCompoundTrigger>  obj(new VNCompoundTrigger(NULL_SPATIAL, VALID_TEMPORAL));
		TS_ASSERT( obj.get() );
	}

	//	valid spatial and NULL temporal   Expected: Success creation
	// TODO revisit for possible addition of asserts in production code.
	// For now, the code checks for non NULL
	// Triggered() returns false if either are NULL
	void testConstructorValidSpatialNullTemporalShouldSucceed() {
		PRINT_TEST_NAME();
		std::auto_ptr<VNCompoundTrigger>  obj(new VNCompoundTrigger(VALID_SPATIAL, NULL_TEMPORAL));
		TS_ASSERT( obj.get() );
	}

	//	NULL spatial and NULL temporal    Expected: Success creation
	// TODO revisit for possible addition of asserts in production code.
	// For now, the code checks for non NULL.
	// Triggered() returns false if either are NULL
	void testConstructorNullSpatialNULLTemporalShouldSucceed() {
		PRINT_TEST_NAME();
		std::auto_ptr<VNCompoundTrigger>  obj(new VNCompoundTrigger(NULL_SPATIAL, NULL_TEMPORAL));
		TS_ASSERT( obj.get() );
	}

	//	valid spatial invalid temporal    Expected: ?
	// No test added for this, on purpose.
	// We can't think of in invalid state that is not null
	// That was covered by:
	// testConstructorValidSpatialNullTemporalShouldSucceed

	//	invalid spatial valid temporal    Expected: ?
	// No test added for this, on purpose.
	// We can't think of in invalid state that is not null
	// That was covered by:
	// testConstructorNullSpatialValidTemporalShouldSucceed


private:

	void init(){
		NULL_SPATIAL = NULL;
		NULL_TEMPORAL = NULL;
		VALID_SPATIAL = new VNSpatialTriggerForTest();
		VALID_TEMPORAL = new VNTemporalTriggeringForTest();
	}

	void cleanup() {
		if (VALID_SPATIAL)
			delete VALID_SPATIAL;

		if (VALID_TEMPORAL)
			delete VALID_TEMPORAL;
	}


	class VNSpatialTriggerForTest : public VNSpatialTrigger {
	public:
		virtual bool Triggered(const VNHotSpot *hotSpot){
			return false;
		}

	};

	class VNTemporalTriggeringForTest : public VNTemporalTriggering {
	public:

		virtual bool Triggered(bool triggered){
			return false;
		}
	};

	VNSpatialTrigger* NULL_SPATIAL;
	VNSpatialTrigger* VALID_SPATIAL;
	VNTemporalTriggering* NULL_TEMPORAL;
	VNTemporalTriggering* VALID_TEMPORAL;
};
