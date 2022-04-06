// TestTranslatorBase

#include <cxxtest/TestSuite.h>
#include <VNTranslatorBase.h>
#include <sstream>
//For strcmp
#include <string.h>
#include <UnitTestUtils.h>

static const float kVNTranslatorBaseTestEpsilon = 0.0001;

//============================================================================
// TestTranslatorBase functions
//============================================================================
class TestTranslatorBase : public CxxTest::TestSuite, TestSuiteBase {
private:
  LF::Vision::VNTranslatorBase *translator_;

public:
  void setFrames(void) {
    LeapFrog::Brio::tRect vframe;
    vframe.left = 0;
    vframe.top = 0;
    vframe.right = 640;
    vframe.bottom = 480;
    translator_->SetDestFrame(vframe);

    LeapFrog::Brio::tRect dframe;
    dframe.left = 25;
    dframe.top = 50;
    dframe.right = 1025;
    dframe.bottom = 650;
    translator_->SetSourceFrame(dframe);
  }
  //------------------------------------------------------------------------
  void setUp(void) {
    translator_ = new LF::Vision::VNTranslatorBase();
    setFrames();
  }

  //------------------------------------------------------------------------
  void tearDown(void){
    if (translator_) delete translator_;
  }

  //------------------------------------------------------------------------
  void testWasCreated(void) {
    PRINT_TEST_NAME();
    TS_ASSERT(translator_ != NULL);
  }

  //------------------------------------------------------------------------
  void testPointFromDestToSource(void) {
    PRINT_TEST_NAME();
    LF::Vision::VNPoint p;
    p.x = 50; p.y = 50;
    LF::Vision::VNPoint pn = translator_->FromDestToSource(p);
    TS_ASSERT_DELTA(pn.x, 103, kVNTranslatorBaseTestEpsilon);
    TS_ASSERT_DELTA(pn.y, 112, kVNTranslatorBaseTestEpsilon);
  }

  //------------------------------------------------------------------------
  void testPointFromSourceToDest(void) {
    PRINT_TEST_NAME();
    LF::Vision::VNPoint p;
    p.x = 710; p.y = 500;
    LF::Vision::VNPoint pn = translator_->FromSourceToDest(p);
    TS_ASSERT_DELTA(pn.x, 438, kVNTranslatorBaseTestEpsilon);
    TS_ASSERT_DELTA(pn.y, 360, kVNTranslatorBaseTestEpsilon);
  }

  //------------------------------------------------------------------------
  void testCVPointFromDestToSource(void) {
    PRINT_TEST_NAME();
    cv::Point p(50,50);
    cv::Point pn = translator_->FromDestToSource(p);
    TS_ASSERT_DELTA(pn.x, 103, kVNTranslatorBaseTestEpsilon);
    TS_ASSERT_DELTA(pn.y, 112, kVNTranslatorBaseTestEpsilon);
  }

  //------------------------------------------------------------------------
  void testCVPointFromSourceToDest(void) {
    PRINT_TEST_NAME();
    cv::Point p(710, 500);
    cv::Point pn = translator_->FromSourceToDest(p);
    TS_ASSERT_DELTA(pn.x, 438, kVNTranslatorBaseTestEpsilon);
    TS_ASSERT_DELTA(pn.y, 360, kVNTranslatorBaseTestEpsilon);
  }

  //------------------------------------------------------------------------
  void testRectFromDestToSource(void) {
    PRINT_TEST_NAME();
    LeapFrog::Brio::tRect r;
    r.left = 10;
    r.right = 80;
    r.top = 0;
    r.bottom = 100;
    LeapFrog::Brio::tRect rn = translator_->FromDestToSource(r);
    TS_ASSERT_DELTA(rn.left, 40, kVNTranslatorBaseTestEpsilon);
    TS_ASSERT_DELTA(rn.right, 150, kVNTranslatorBaseTestEpsilon);
    TS_ASSERT_DELTA(rn.top, 50, kVNTranslatorBaseTestEpsilon);
    TS_ASSERT_DELTA(rn.bottom, 175, kVNTranslatorBaseTestEpsilon);
  }

  //------------------------------------------------------------------------
  void testRectFromSourceToDest(void) {
    PRINT_TEST_NAME();
    LeapFrog::Brio::tRect r;
    r.left = 900;
    r.right = 950;
    r.top = 50;
    r.bottom = 650;
    LeapFrog::Brio::tRect rn = translator_->FromSourceToDest(r);
    TS_ASSERT_DELTA(rn.left, 560, kVNTranslatorBaseTestEpsilon);
    TS_ASSERT_DELTA(rn.right, 592, kVNTranslatorBaseTestEpsilon);
    TS_ASSERT_DELTA(rn.top, 0, kVNTranslatorBaseTestEpsilon);
    TS_ASSERT_DELTA(rn.bottom, 480, kVNTranslatorBaseTestEpsilon);
  }

};

// EOF
