
// Unit tests for VNPointTrigger
// Reference: Test plan:
// http://wiki.leapfrog.com/display/CTD/Testing+Plans
// VisionTestingPlanv2.xlsx

#include <Vision/VNPointTrigger.h>
#include <UnitTestUtils.h>
#include <Vision/VNRectHotSpot.h>
#include <Vision/VNVisionTypes.h>

#include <cxxtest/TestSuite.h>
#include <memory>

using namespace LeapFrog::Brio;
using namespace LF::Vision;

const S16 WAND_X_DEFAULT = -10000;
const S16 WAND_Y_DEFAULT = -10000;


class TestPointTrigger : public CxxTest::TestSuite,
							LeapFrog::Brio::TestSuiteBase {

  public:

	TestPointTrigger()
  	  : wandPosition(WAND_X_DEFAULT, WAND_Y_DEFAULT), rectSize(100) {
	}

	void testConstructorDefaultShouldSucceed() {
		PRINT_TEST_NAME();
		std::auto_ptr<VNPointTrigger>  trigger(new VNPointTrigger);
		TS_ASSERT( trigger.get() );
	}

	//	Triggered with NULL	Expect: FALSE
	void testTriggeredWithNullExpectFalse() {
		PRINT_TEST_NAME();
		std::auto_ptr<VNPointTrigger>  trigger(new VNPointTrigger);
		VNHotSpot *hotSpot = NULL;
		TS_ASSERT( !trigger->Triggered(hotSpot) );
	}

	//	Triggered with valid hotspot with wand inside of bounding area	Expect: TRUE
	void testTriggeredWithValidHotspotWandInsideTriggeredExpectTrue() {
		PRINT_TEST_NAME();
		std::auto_ptr<VNPointTrigger>  trigger(new VNPointTrigger);

		// Rect centered around the default wand location
		float rectSize = 100.0f;
		LeapFrog::Brio::tRect hotSpotRect;
		hotSpotRect.left = wandPosition.x -rectSize/2;
		hotSpotRect.right = wandPosition.x + rectSize/2;
		hotSpotRect.top = wandPosition.y -rectSize/2;
		hotSpotRect.bottom = wandPosition.y + rectSize/2;

		VNRectHotSpot  hotSpot(hotSpotRect);
		TS_ASSERT( trigger->Triggered(&hotSpot) );
	}

	//	Triggered with valid hotspot with wand on edge of bounding area	Expect: FALSE
	// TODO scrutinize.  Currently acts binary near edge. On or off.
	void testTriggeredWithValidHotspotWandOnEdgeTriggeredExpectFalse() {
		PRINT_TEST_NAME();

		std::auto_ptr<VNPointTrigger>  trigger(new VNPointTrigger);

		// Rect on edge of default wand location
		int closeToEdge = 1;
		LeapFrog::Brio::tRect hotSpotRect;
		hotSpotRect.left = wandPosition.x + closeToEdge;
		hotSpotRect.right = wandPosition.x + rectSize + closeToEdge;
		hotSpotRect.top = wandPosition.y + closeToEdge;
		hotSpotRect.bottom = wandPosition.y + rectSize + closeToEdge;

		VNRectHotSpot  hotSpot(hotSpotRect);
		TS_ASSERT( !trigger->Triggered(&hotSpot) );
	}

	//	Triggered with valid hotspot with wand outside of bounding area	Expect: FALSE
	void testTriggeredWithValidHotspotWithWandOutsideExpectFalse() {
		PRINT_TEST_NAME();
		std::auto_ptr<VNPointTrigger>  trigger(new VNPointTrigger);

		// Rect outside default wand location
		LeapFrog::Brio::tRect hotSpotRect;
		hotSpotRect.left = wandPosition.x + rectSize;
		hotSpotRect.right = wandPosition.x + 2*rectSize;
		hotSpotRect.top = wandPosition.y + rectSize;
		hotSpotRect.bottom = wandPosition.y + 2* rectSize;

		VNRectHotSpot  hotSpot(hotSpotRect);
		TS_ASSERT( !trigger->Triggered(&hotSpot) );
	}

private:
	const VNPoint wandPosition;  // stays constant, at default for tests.
	const S16 rectSize;
};
