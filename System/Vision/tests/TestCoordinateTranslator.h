// TestCoordinateTranslator

#include <cxxtest/TestSuite.h>
#include <VNCoordinateTranslator.h>
#include <sstream>
//For strcmp
#include <string.h>
#include <UnitTestUtils.h>

static const float kVNVisionTestEpsilon = 0.0001;

//============================================================================
// TestCoordinateTranslator functions
//============================================================================
class TestCoordinateTranslator : public CxxTest::TestSuite, TestSuiteBase {
private:
  LF::Vision::VNCoordinateTranslator *translator_;

public:
  void setFrames(void) {
    LeapFrog::Brio::tRect vframe;
    vframe.left = 0;
    vframe.top = 0;
    vframe.right = 640;
    vframe.bottom = 480;
    translator_->SetVisionFrame(vframe);

    LeapFrog::Brio::tRect dframe;
    dframe.left = 25;
    dframe.top = 50;
    dframe.right = 1025;
    dframe.bottom = 650;
    translator_->SetDisplayFrame(dframe);
  }
  //------------------------------------------------------------------------
  void setUp(void) {
    translator_ = LF::Vision::VNCoordinateTranslator::Instance();
    setFrames();
  }

  //------------------------------------------------------------------------
  void tearDown(void){
  }

  //------------------------------------------------------------------------
  void testWasCreated(void) {
    PRINT_TEST_NAME();
    TS_ASSERT(translator_ != NULL);
  }

  //------------------------------------------------------------------------
  void testPointFromVisionToDisplay(void) {
    PRINT_TEST_NAME();
    LF::Vision::VNPoint p;
    p.x = 50; p.y = 50;
    LF::Vision::VNPoint pn = translator_->FromVisionToDisplay(p);
    TS_ASSERT_DELTA(pn.x, 103, kVNVisionTestEpsilon);
    TS_ASSERT_DELTA(pn.y, 112, kVNVisionTestEpsilon);
  }

  //------------------------------------------------------------------------
  void testPointFromDisplayToVision(void) {
    PRINT_TEST_NAME();
    LF::Vision::VNPoint p;
    p.x = 710; p.y = 500;
    LF::Vision::VNPoint pn = translator_->FromDisplayToVision(p);
    TS_ASSERT_DELTA(pn.x, 438, kVNVisionTestEpsilon);
    TS_ASSERT_DELTA(pn.y, 360, kVNVisionTestEpsilon);
  }

  //------------------------------------------------------------------------
  void testRectFromVisionToDisplay(void) {
    PRINT_TEST_NAME();
    LeapFrog::Brio::tRect r;
    r.left = 10;
    r.right = 80;
    r.top = 0;
    r.bottom = 100;
    LeapFrog::Brio::tRect rn = translator_->FromVisionToDisplay(r);
    TS_ASSERT_DELTA(rn.left, 40, kVNVisionTestEpsilon);
    TS_ASSERT_DELTA(rn.right, 150, kVNVisionTestEpsilon);
    TS_ASSERT_DELTA(rn.top, 50, kVNVisionTestEpsilon);
    TS_ASSERT_DELTA(rn.bottom, 175, kVNVisionTestEpsilon);
  }

  //------------------------------------------------------------------------
  void testRectFromDisplayToVision(void) {
    PRINT_TEST_NAME();
    LeapFrog::Brio::tRect r;
    r.left = 900;
    r.right = 950;
    r.top = 50;
    r.bottom = 650;
    LeapFrog::Brio::tRect rn = translator_->FromDisplayToVision(r);
    TS_ASSERT_DELTA(rn.left, 560, kVNVisionTestEpsilon);
    TS_ASSERT_DELTA(rn.right, 592, kVNVisionTestEpsilon);
    TS_ASSERT_DELTA(rn.top, 0, kVNVisionTestEpsilon);
    TS_ASSERT_DELTA(rn.bottom, 480, kVNVisionTestEpsilon);
  }

};

// EOF
