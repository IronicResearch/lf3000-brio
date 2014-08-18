
// Unit tests for VNOcclusionTrigger
// Reference: Test plan:
// http://wiki.leapfrog.com/display/CTD/Testing+Plans
// VisionTestingPlanv2.xlsx

#include <Vision/VNOcclusionTrigger.h>
#include <Vision/VNArbitraryShapeHotSpot.h>
#include <Vision/VNRectHotSpot.h>
#include <Vision/VNVisionTypes.h>

#include <UnitTestUtils.h>
#include <cxxtest/TestSuite.h>
#include <memory>

using namespace LeapFrog::Brio;
using namespace LF::Vision;

const S16 WAND_X_DEFAULT = -10000;
const S16 WAND_Y_DEFAULT = -10000;


class TestOcclusionTrigger : public CxxTest::TestSuite,
							LeapFrog::Brio::TestSuiteBase {

  public:

	TestOcclusionTrigger()
	  : rectSize(100) {
		init();
	}

	//	VNOcclusionTrigger(float percentOccluded)
	//	void	successful object construction
	void testConstructorDefaultShouldSucceed() {
		PRINT_TEST_NAME();
		std::auto_ptr<VNOcclusionTrigger>  trigger(new VNOcclusionTrigger);
		TS_ASSERT( trigger.get() );
	}

	//	1 <= percentOccluded > 0	successful object construction
	void testConstructorValidOcculsionShouldSucceed() {
		PRINT_TEST_NAME();
		std::auto_ptr<VNOcclusionTrigger>  trigger(
				new VNOcclusionTrigger(VALID_OCCLUSION));
		TS_ASSERT( trigger.get() );
	}

	//	percentOccluded < 0
	void testConstructorNegativeOcculsionShouldSucceedOcclusionZero() {
		PRINT_TEST_NAME();
		VNOcclusionTrigger trigger(NEGATIVE_OCCLUSION);
		TS_ASSERT_DELTA( ZERO_OCCLUSION, trigger.GetOcclusionTriggerPercentage(), TOLERANCE );
	}

	//	percentOccluded == 0
	void testConstructorZeroOcculsionShouldSucceedOcclusionZero() {
		PRINT_TEST_NAME();
		VNOcclusionTrigger trigger(ZERO_OCCLUSION);
		TS_ASSERT_DELTA( ZERO_OCCLUSION, trigger.GetOcclusionTriggerPercentage(), TOLERANCE );
	}

	//	percentOccluded > 1
	void testConstructorTooBigOcculsionShouldSucceedOcclusionFull() {
		PRINT_TEST_NAME();
		VNOcclusionTrigger trigger(TOO_BIG_OCCLUSION);
		TS_ASSERT_DELTA( FULL_OCCLUSION, trigger.GetOcclusionTriggerPercentage(),  TOLERANCE );
	}

	//	SetOcclusionTriggerPercentage(float occlusionPercent)
	//	1 <= percentOccluded > 0	successful
	void testSetOcclusionValidShouldSucceed() {
		PRINT_TEST_NAME();
		VNOcclusionTrigger  trigger;
		trigger.SetOcclusionTriggerPercentage(VALID_OCCLUSION);
		TS_ASSERT_DELTA(VALID_OCCLUSION, trigger.GetOcclusionTriggerPercentage(), TOLERANCE );
	}

	//	percentOccluded < 0
	void testSetOcclusionNegativeShouldSucceedOcclusionZero() {
		PRINT_TEST_NAME();
		VNOcclusionTrigger  trigger;
		trigger.SetOcclusionTriggerPercentage(NEGATIVE_OCCLUSION);
		TS_ASSERT_DELTA(ZERO_OCCLUSION, trigger.GetOcclusionTriggerPercentage(), TOLERANCE );
	}

	//	percentOccluded == 0
	void testSetOcclusionZeroShouldSucceedOcclusionZero() {
		PRINT_TEST_NAME();
		VNOcclusionTrigger  trigger;
		trigger.SetOcclusionTriggerPercentage(ZERO_OCCLUSION);
		TS_ASSERT_DELTA(ZERO_OCCLUSION, trigger.GetOcclusionTriggerPercentage(), TOLERANCE );
	}

	//	percentOccluded > 1
	void testSetOcclusionTooBigShouldSucceedOcclusionFull() {
		PRINT_TEST_NAME();
		VNOcclusionTrigger  trigger;
		trigger.SetOcclusionTriggerPercentage(TOO_BIG_OCCLUSION);
		TS_ASSERT_DELTA(FULL_OCCLUSION, trigger.GetOcclusionTriggerPercentage(), TOLERANCE );
	}

	//	Triggered(VNHotSpot *)

	//	Triggered with NULL	Expect: FALSE
	void testTriggeredWithNullExpectFalse() {
		PRINT_TEST_NAME();
		VNOcclusionTrigger  trigger;
		VNHotSpot *hotSpot = NULL;
		TS_ASSERT( !trigger.Triggered(hotSpot) );
	}

	//	valid hotspot with diff image with percent diff < occlusionPercent	FALSE
	void testTriggeredWithDiffLessThanOcclusionExpectFalse() {
		PRINT_TEST_NAME();

		// Rect based on origin needed. Upper left is (0,0)
		const int rectSize = 100;
		LeapFrog::Brio::tRect hotSpotRect;
		hotSpotRect.left = 0;
		hotSpotRect.right = rectSize;
		hotSpotRect.top = 0;
		hotSpotRect.bottom = rectSize;

		const int dataSize = rectSize*rectSize;
		char data[dataSize];

		// Occlude data up to a certain index, 30%
		int topAccludedIndex = int(dataSize * 0.30);

		for (int i=0; i < dataSize; i++){

			if ( i < topAccludedIndex){
				data[i] = kVNMaxPixelValue;
			}
			else {
				data[i] = kVNMinPixelValue;
			}
		}

		cv::Mat diffImage(rectSize, rectSize, CV_8U, &data);

		VNRectHotSpot hotSpot(hotSpotRect);

		VNOcclusionTrigger trigger;
		trigger.SetOcclusionTriggerPercentage(0.4);

		hotSpot.SetTrigger(&trigger);
		hotSpot.Trigger(diffImage);

		TS_ASSERT( !trigger.Triggered(&hotSpot) );
	}

	//	valid hotspot with diff image with percent diff == occlusionPercent	FALSE
	void testTriggeredWithDiffEqualToOcclusionExpectFalse() {
		PRINT_TEST_NAME();
		// Rect based on origin needed. Upper left is (0,0)
		const int rectSize = 100;
		LeapFrog::Brio::tRect hotSpotRect;
		hotSpotRect.left = 0;
		hotSpotRect.right = rectSize;
		hotSpotRect.top = 0;
		hotSpotRect.bottom = rectSize;

		const int dataSize = rectSize*rectSize;
		char data[dataSize];

		// Occlude data up to a certain index, 30%
		int topAccludedIndex = int(dataSize * 0.30);

		for (int i=0; i < dataSize; i++){

			if ( i < topAccludedIndex){
				data[i] = kVNMaxPixelValue;
			}
			else {
				data[i] = kVNMinPixelValue;
			}
		}

		cv::Mat diffImage(rectSize, rectSize, CV_8U, &data);

		VNRectHotSpot hotSpot(hotSpotRect);

		VNOcclusionTrigger trigger;
		trigger.SetOcclusionTriggerPercentage(0.3);

		hotSpot.SetTrigger(&trigger);
		hotSpot.Trigger(diffImage);

		TS_ASSERT( !trigger.Triggered(&hotSpot) );
	}

	//	valid hotspot with diff image with percent diff > occlusionPercent	TRUE
	void testTriggeredWithDiffGreaterThanOcclusionExpectTrue() {
		PRINT_TEST_NAME();
		// Rect based on origin needed. Upper left is (0,0)
		const int rectSize = 100;
		LeapFrog::Brio::tRect hotSpotRect;
		hotSpotRect.left = 0;
		hotSpotRect.right = rectSize;
		hotSpotRect.top = 0;
		hotSpotRect.bottom = rectSize;

		const int dataSize = rectSize*rectSize;
		char data[dataSize];

		// Occlude data up to a certain index, 60%
		int topAccludedIndex = int(dataSize * 0.60);

		for (int i=0; i < dataSize; i++){

			if ( i < topAccludedIndex){
				data[i] = kVNMaxPixelValue;
			}
			else {
				data[i] = kVNMinPixelValue;
			}
		}

		cv::Mat diffImage(rectSize, rectSize, CV_8U, &data);

		VNRectHotSpot hotSpot(hotSpotRect);

		VNOcclusionTrigger trigger;
		trigger.SetOcclusionTriggerPercentage(0.4);

		hotSpot.SetTrigger(&trigger);
		hotSpot.Trigger(diffImage);

		TS_ASSERT( trigger.Triggered(&hotSpot) );
	}

	//  TODO Add these test cases
	void testMoreTestCasesTodoAdd() {
		PRINT_TEST_NAME();
		TS_WARN("TODO: More test cases to complete. See list in test file." );
	}

	//	//	GetOcclusionTriggerPercent(void)
	//	call after constructor with no input	should match kVNDefaultPercentOccludedToTrigger value
	//	call after constructor with 1 >= percentOccluded > 0	should match value passed in to constructor
	//	call after constructor with percentOccluded < 0	?
	//	call after constructor with percentOccluded > 1	?
	//	call after constructor with percentOccluded == 0	?
	//	call after Set with 1 >= percentOccluded > 0	should match value passed in to SetOcclusionTriggerPercent
	//	call after Set with percentOccluded < 0	?
	//	call after Set with percentOccluded == 0	?
	//	call after Set with percentOccluded > 1	?
	//
		//	GetOccludedPercent(void)
	//	call will no previous call to Tirggered	0
	//	call after call to Triggered with percent occluded < threshold	match the percent occluded in diff image in hot spot
	//	call after call to Triggered with percent occluded == threshold	match the percent occluded in diff image in hot spot
	//	call after call to Triggered with percent occluded > threshold	match the percent occluded in diff image in hot spot
	//


private:

	void init(){
		VALID_OCCLUSION = 0.5f;
		NEGATIVE_OCCLUSION = -1.0f;
		TOO_BIG_OCCLUSION = 2.0f;  // Greater than 1
		ZERO_OCCLUSION = 0.0f;
		FULL_OCCLUSION = 1.0f;
		TOLERANCE = 0.0001f;
	}

	float VALID_OCCLUSION;
	float NEGATIVE_OCCLUSION;
	float TOO_BIG_OCCLUSION;
	float ZERO_OCCLUSION;
	float FULL_OCCLUSION;

	float TOLERANCE;
	const S16 rectSize;
};
