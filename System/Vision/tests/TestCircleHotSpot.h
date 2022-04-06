
// Unit tests for VNCircleHotSpot
// Reference: Test plan:
// http://wiki.leapfrog.com/display/CTD/Testing+Plans
// VisionTestingPlanv2.xlsx

#include <Vision/VNCircleHotSpot.h>
#include <UnitTestUtils.h>
#include <UnitTestVisionUtils.h>

#include <cxxtest/TestSuite.h>
#include <memory>

using namespace LF::Vision;

class TestCircleHotSpot : public CxxTest::TestSuite,
							LeapFrog::Brio::TestSuiteBase {

  public:

	TestCircleHotSpot() {
		init();
	}

	// Tests of Constructors -----------------------------------

	void testConstructorDefaultShouldSucceed() {
		PRINT_TEST_NAME();
		std::auto_ptr<VNCircleHotSpot>  spot(new VNCircleHotSpot);
		TS_ASSERT( spot.get() );
		TS_ASSERT_DELTA(0.0,spot->GetRadius(), 0.01f)
		assertCenterPointsEqual(CenterPointZeroZero, spot->GetCenter());
	}

	void testConstructorCircleFullyOnScreenShouldSucceed() {
		PRINT_TEST_NAME();
		float radiusFullyOnScreen = vUtils.GetScreenHeight()/4.0f;
		std::auto_ptr<VNCircleHotSpot>  spot(new VNCircleHotSpot(CenterPointZeroZero,
																	radiusFullyOnScreen));
		TS_ASSERT( spot.get() );
	}

	void testConstructorCirclePartiallyOffScreenShouldSucceed() {
		PRINT_TEST_NAME();
		float radiusPartiallyOffScreen = vUtils.GetScreenHeight();
		std::auto_ptr<VNCircleHotSpot>  spot(new VNCircleHotSpot(CenterPointZeroZero,
																	radiusPartiallyOffScreen));
		TS_ASSERT( spot.get() );
	}

	void testConstructorCircleFullyOffScreenShouldSucceed() {
		PRINT_TEST_NAME();
		float radiusFullyOffScreen = vUtils.GetScreenHeight()/4;
		std::auto_ptr<VNCircleHotSpot>  spot(new VNCircleHotSpot(CenterOffScreen,
																	radiusFullyOffScreen));
		TS_ASSERT( spot.get() );
		TS_ASSERT_DELTA(radiusFullyOffScreen,spot->GetRadius(), 0.01f)
		assertCenterPointsEqual(CenterOffScreen, spot->GetCenter());
	}

	void testConstructorAnyCenterLocationZeroRadiusShouldSucceed() {
		PRINT_TEST_NAME();
		float radius = 0.0f;
		std::auto_ptr<VNCircleHotSpot>  spot(new VNCircleHotSpot(CenterOffScreen, radius));
		TS_ASSERT_DELTA(0.0,spot->GetRadius(), 0.01f)
	}

	// test of the constructor with radius < 0.0.
	void testConstructorAnyCenterNegativeRadiusShouldSucceedRadiusZero() {
		PRINT_TEST_NAME();
		float radius = -10.0f;
		std::auto_ptr<VNCircleHotSpot>  spot(new VNCircleHotSpot(CenterOffScreen, radius));
		TS_ASSERT_DELTA(0.0,spot->GetRadius(), 0.01f)
	}

	// Tests of SetCenter() -----------------------------------

	void testSetCenterInCenterZeroZeroShouldSucceed() {
		PRINT_TEST_NAME();
		VNCircleHotSpot spot;
		spot.SetCenter(CenterPointZeroZero);
		assertCenterPointsEqual(CenterPointZeroZero, spot.GetCenter());
	}

	void testSetCenterInScreenBoundsShouldSucceed() {
		PRINT_TEST_NAME();
		VNCircleHotSpot spot;
		spot.SetCenter(CenterOnScreenNonZero);
		assertCenterPointsEqual(CenterOnScreenNonZero, spot.GetCenter());
	}

	void testSetCenterOutOFScreenBoundsShouldSucceed() {
		PRINT_TEST_NAME();
		VNCircleHotSpot spot;
		spot.SetCenter(CenterOffScreen);
		assertCenterPointsEqual(CenterOffScreen, spot.GetCenter());
	}

	// Tests of GetCenter() -----------------------------------

	// Test GetCenter after default construction.
	// Expect: Success, radius = 0, center = (0, 0)
	// No test needed. Already covered by:
	// testConstructorDefaultShouldSucceed()

	// Test GetCenter after construction with parameters
	// Expect: Success, radius and center to match inputs
	// No test needed. Already covered by:
	// testConstructorCircleFullyOffScreenShouldSucceed()

	// Test omitted here: testGetCenterAfterSetCenterShouldMatch
	// Justification: Already tested earlier by
	// testSetRadiusPositiveShouldSucceed();

	// Tests of SetRadius() -----------------------------------

	void testSetRadiusPositiveShouldSucceed() {
		PRINT_TEST_NAME();
		VNCircleHotSpot spot;
		float goalRadius = 10.0f;
		spot.SetRadius(goalRadius);
		TS_ASSERT_DELTA(goalRadius,spot.GetRadius(), 0.1f)
	}

	void testSetRadiusNegativeShouldSucceedZero() {
		PRINT_TEST_NAME();
		VNCircleHotSpot spot;
		float goalRadius = -10.0f;
		spot.SetRadius(goalRadius);
		TS_ASSERT_DELTA(0.0f,spot.GetRadius(), 0.1f)
	}

	void testSetRadiusZeroShouldSucceedZero() {
		PRINT_TEST_NAME();
		VNCircleHotSpot spot;
		float goalRadius = 0.0f;
		spot.SetRadius(goalRadius);
		TS_ASSERT_DELTA(0.0f,spot.GetRadius(), 0.1f)
	}

	// Tests of GetRadius() -----------------------------------

	// Test GetRadius after default construction.
	// Expect: Success, radius = 0, center = (0, 0)
	// No test needed. Already covered by:
	// testConstructorDefaultShouldSucceed()

	// Test GetRadius after construction with parameters, radius > 0
	// Expect: Success, radius and center to match inputs
	// No test needed. Already covered by:
	// testConstructorCircleFullyOffScreenShouldSucceed()


	// Test GetRadius call after construction with parameters and an invalid raidus(<=0)
	// Expect: Success, radius == 0
	// No test needed. Already covered by:
	// testConstructorAnyCenterLocationZeroRadiusShouldSucceed() {
	// testConstructorAnyCenterNegativeRadiusShouldSucceedRadiusZero() {

	// Test omitted here: call GetCenter after SetRadius with successful completion
	// Already tested earlier by testSetRadiusPositiveShouldSucceed()

	// Original description:
	// Inputs: call after SetRadius with unsuccessful completion
	// Expected: should match radius value prior to making call to SetRadius
	// Test omitted.  No obvious way to create unsuccessful completions.
	// But passing in radius < 0, results in radius == 0.


  private:

	void init(){
		CenterPointZeroZero.x = 0;
		CenterPointZeroZero.y = 0;

		CenterOnScreenNonZero.x = vUtils.GetScreenWidth()/4;
		CenterOnScreenNonZero.y = vUtils.GetScreenHeight()/4;

		CenterOffScreen.x = vUtils.GetScreenWidth();
		CenterOffScreen.y = vUtils.GetScreenHeight();
	}

	void assertCenterPointsEqual(const VNPoint& center1, const VNPoint& center2) {
		TS_ASSERT_EQUALS(center1.x, center2.x);
		TS_ASSERT_EQUALS(center1.y, center2.y);
	}

	UnitTestVisionUtils vUtils;
	VNPoint CenterPointZeroZero;
	VNPoint CenterOnScreenNonZero;
	VNPoint CenterOffScreen;
};
