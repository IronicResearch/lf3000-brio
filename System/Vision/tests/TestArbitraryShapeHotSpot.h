#include <cxxtest/TestSuite.h>
#include <UnitTestUtils.h>
#include <UnitTestVisionUtils.h>

#include <Vision/VNArbitraryShapeHotSpot.h>
#include <Vision/VNVisionTypes.h>

#include <opencv2/opencv.hpp>

static const float ONE_HALF = 0.50f;
static const float ONE_FOURTHS = 0.25f;
static const float ONE_EIGHTHS = 0.125f;

using namespace LeapFrog::Brio;
using namespace LF::Vision;

class TestArbitraryShapeHotSpot : public CxxTest::TestSuite, TestSuiteBase, UnitTestVisionUtils {
	public:
		TestArbitraryShapeHotSpot() {
			init();
		}

		~TestArbitraryShapeHotSpot() {
			cleanup();
		}

		void testConstructorWithNoArgumentShouldSucceed() {
			PRINT_TEST_NAME();
			std::auto_ptr<VNArbitraryShapeHotSpot> ArbitraryShapeHotSpot (new
				VNArbitraryShapeHotSpot);
			TS_ASSERT(ArbitraryShapeHotSpot.get());
		}

		void testConstructorWithRectSameSizeAsMatFilterImageFullOnScreenShouldSucceed() {
			PRINT_TEST_NAME();
			std::auto_ptr<VNArbitraryShapeHotSpot> ArbitraryShapeHotSpot (new
				VNArbitraryShapeHotSpot(RectNonZeroFullOnScreen, MatFilterImageMediumNonZero));
			TS_ASSERT(ArbitraryShapeHotSpot.get());
		}

		void testConstructorWithRectSameSizeAsMatFilterImagePartOnScreenShouldSucceed() {
			PRINT_TEST_NAME();
			std::auto_ptr<VNArbitraryShapeHotSpot> ArbitraryShapeHotSpot (new
				VNArbitraryShapeHotSpot(RectNonZeroPartOnScreen, MatFilterImageMediumNonZero));
			TS_ASSERT(ArbitraryShapeHotSpot.get());
		}

		void testConstructorWithRectSameSizeAsMatFilterImageFullOffScreenShouldSucceed() {
			PRINT_TEST_NAME();
			std::auto_ptr<VNArbitraryShapeHotSpot> ArbitraryShapeHotSpot (new
				VNArbitraryShapeHotSpot(RectNonZeroFullOffScreen, MatFilterImageMediumNonZero));
			TS_ASSERT(ArbitraryShapeHotSpot.get());
		}

		void testConstructorWithRectSmallerThanMatFilterImageFullOnScreenShouldSucceed() {
			PRINT_TEST_NAME();
			std::auto_ptr<VNArbitraryShapeHotSpot> ArbitraryShapeHotSpot (new
				VNArbitraryShapeHotSpot(ScaleRect(RectNonZeroFullOnScreen,sfSmall), MatFilterImageMediumNonZero));
			TS_ASSERT(ArbitraryShapeHotSpot.get());
		}

		void testConstructorWithRectSmallerThanMatFilterImagePartOnScreenShouldSucceed() {
			PRINT_TEST_NAME();
			std::auto_ptr<VNArbitraryShapeHotSpot> ArbitraryShapeHotSpot (new
				VNArbitraryShapeHotSpot(ScaleRect(RectNonZeroPartOnScreen,sfSmall), MatFilterImageMediumNonZero));
			TS_ASSERT(ArbitraryShapeHotSpot.get());
		}

		void testConstructorWithRectSmallerThanMatFilterImageFullOffScreenShouldSucceed() {
			PRINT_TEST_NAME();
			std::auto_ptr<VNArbitraryShapeHotSpot> ArbitraryShapeHotSpot (new
				VNArbitraryShapeHotSpot(ScaleRect(RectNonZeroFullOffScreen,sfSmall), MatFilterImageMediumNonZero));
			TS_ASSERT(ArbitraryShapeHotSpot.get());
		}

		void testConstructorWithRectLargerThanMatFilterImageFullOnScreenShouldSucceed() {
			PRINT_TEST_NAME();
			std::auto_ptr<VNArbitraryShapeHotSpot> ArbitraryShapeHotSpot (new
				VNArbitraryShapeHotSpot(ScaleRect(RectNonZeroFullOnScreen,sfLarge), MatFilterImageMediumNonZero));
			TS_ASSERT(ArbitraryShapeHotSpot.get());
		}

		void testConstructorWithRectLargerThanMatFilterImagePartOnScreenShouldSucceed() {
			PRINT_TEST_NAME();
			std::auto_ptr<VNArbitraryShapeHotSpot> ArbitraryShapeHotSpot (new
				VNArbitraryShapeHotSpot(ScaleRect(RectNonZeroPartOnScreen,sfLarge), MatFilterImageMediumNonZero));
			TS_ASSERT(ArbitraryShapeHotSpot.get());
		}

		void testConstructorWithRectLargerThanMatFilterImageFullOffScreenShouldSucceed() {
			PRINT_TEST_NAME();
			std::auto_ptr<VNArbitraryShapeHotSpot> ArbitraryShapeHotSpot (new
				VNArbitraryShapeHotSpot(ScaleRect(RectNonZeroFullOffScreen,sfLarge), MatFilterImageMediumNonZero));
			TS_ASSERT(ArbitraryShapeHotSpot.get());
		}

		void testConstructorWithZeroRectNonZeroMatFilterImageFullOnScreenShouldSucceed() {
			PRINT_TEST_NAME();
			TS_TRACE("No test yet for Constructor with zero size rect and non-zero size mat filter image (full onscreen);\n"
			"Current behavior: OpenCV Error: Assertion failed (dsize.area() || (inv_scale_x > 0 && inv_scale_y > 0))\n"
			"in resize. Expected: ?");
			/*
			std::auto_ptr<VNArbitraryShapeHotSpot> ArbitraryShapeHotSpot (new
				VNArbitraryShapeHotSpot(RectZero, MatFilterImageMediumNonZero));
			TS_ASSERT(ArbitraryShapeHotSpot.get());
			*/
		}

		void testConstructorWithZeroRectNonZeroMatFilterImagePartOnScreenShouldSucceed() {
			PRINT_TEST_NAME();
			TS_TRACE("No test yet for Constructor with zero size rect and non-zero size mat filter image (partially offscreen);\n"
			"Current behavior: OpenCV Error: Assertion failed (dsize.area() || (inv_scale_x > 0 && inv_scale_y > 0))\n"
			"in resize. Expected: ?");
			/*
			std::auto_ptr<VNArbitraryShapeHotSpot> ArbitraryShapeHotSpot (new
				VNArbitraryShapeHotSpot(RectZero, MatFilterImageMediumNonZero));
			TS_ASSERT(ArbitraryShapeHotSpot.get());
			*/
		}

		void testConstructorWithZeroRectNonZeroMatFilterImageFullOffScreenShouldSucceed() {
			PRINT_TEST_NAME();
			TS_TRACE("No test yet for Constructor with zero size rect and non-zero size mat filter image (full offscreen);\n"
			"Current behavior: OpenCV Error: Assertion failed (dsize.area() || (inv_scale_x > 0 && inv_scale_y > 0))\n"
			"in resize. Expected: ?");
			/*
			std::auto_ptr<VNArbitraryShapeHotSpot> ArbitraryShapeHotSpot (new
				VNArbitraryShapeHotSpot(RectZero, MatFilterImageMediumNonZero));
			TS_ASSERT(ArbitraryShapeHotSpot.get());
			*/
		}

		void testConstructorWithNonZeroRectZeroMatFilterImageFullOnScreenShouldSucceed() {
			PRINT_TEST_NAME();
			TS_TRACE("No test yet for Constructor with non-zero size rect and zero size mat filter image (full onscreen);\n"
			"Current behavior: OpenCV Error: Assertion failed (ssize.area() > 0) in resize. Expected: ?");
			/*
			std::auto_ptr<VNArbitraryShapeHotSpot> ArbitraryShapeHotSpot (new
				VNArbitraryShapeHotSpot(RectNonZeroFullOnScreen, MatFilterImageZero));
			TS_ASSERT(ArbitraryShapeHotSpot.get());
			*/
		}

		void testConstructorWithNonZeroRectZeroMatFilterImagePartOnScreenShouldSucceed() {
			PRINT_TEST_NAME();
			TS_TRACE("No test yet for Constructor with non-zero size rect and zero size mat filter image (partially offscreen);\n"
			"Current behavior: OpenCV Error: Assertion failed (ssize.area() > 0) in resize. Expected: ?");
			/*
			std::auto_ptr<VNArbitraryShapeHotSpot> ArbitraryShapeHotSpot (new
				VNArbitraryShapeHotSpot(RectNonZeroPartOnScreen, MatFilterImageZero));
			TS_ASSERT(ArbitraryShapeHotSpot.get());
			*/
		}

		void testConstructorWithNonZeroRectZeroMatFilterImageFullOffScreenShouldSucceed() {
			PRINT_TEST_NAME();
			TS_TRACE("No test yet for Constructor with non-zero size rect and zero size mat filter image (full offscreen);\n"
			"Current behavior: OpenCV Error: Assertion failed (ssize.area() > 0) in resize. Expected: ?");
			/*
			std::auto_ptr<VNArbitraryShapeHotSpot> ArbitraryShapeHotSpot (new
				VNArbitraryShapeHotSpot(RectNonZeroFullOffScreen, MatFilterImageZero));
			TS_ASSERT(ArbitraryShapeHotSpot.get());
			*/
		}

		void testConstructorWithRectSameSizeAsFontSurfFilterImageFullOnScreenShouldSucceed() {
			PRINT_TEST_NAME();
			std::auto_ptr<VNArbitraryShapeHotSpot> ArbitraryShapeHotSpot (new
				VNArbitraryShapeHotSpot(RectNonZeroFullOnScreen, FontSurfFilterImageMediumNonZero));
			TS_ASSERT(ArbitraryShapeHotSpot.get());
		}

		void testConstructorWithRectSameSizeAsFontSurfFilterImagePartOnScreenShouldSucceed() {
			PRINT_TEST_NAME();
			std::auto_ptr<VNArbitraryShapeHotSpot> ArbitraryShapeHotSpot (new
				VNArbitraryShapeHotSpot(RectNonZeroPartOnScreen, FontSurfFilterImageMediumNonZero));
			TS_ASSERT(ArbitraryShapeHotSpot.get());
		}

		void testConstructorWithRectSameSizeAsFontSurfFilterImageFullOffScreenShouldSucceed() {
			PRINT_TEST_NAME();
			std::auto_ptr<VNArbitraryShapeHotSpot> ArbitraryShapeHotSpot (new
				VNArbitraryShapeHotSpot(RectNonZeroFullOffScreen, FontSurfFilterImageMediumNonZero));
			TS_ASSERT(ArbitraryShapeHotSpot.get());
		}

		void testConstructorWithRectSmallerThanFontSurfFilterImageFullOnScreenShouldSucceed() {
			PRINT_TEST_NAME();
			std::auto_ptr<VNArbitraryShapeHotSpot> ArbitraryShapeHotSpot (new
				VNArbitraryShapeHotSpot(ScaleRect(RectNonZeroFullOnScreen,sfSmall), FontSurfFilterImageMediumNonZero));
			TS_ASSERT(ArbitraryShapeHotSpot.get());
		}

		void testConstructorWithRectSmallerThanFontSurfFilterImagePartOnScreenShouldSucceed() {
			PRINT_TEST_NAME();
			std::auto_ptr<VNArbitraryShapeHotSpot> ArbitraryShapeHotSpot (new
				VNArbitraryShapeHotSpot(ScaleRect(RectNonZeroPartOnScreen,sfSmall), FontSurfFilterImageMediumNonZero));
			TS_ASSERT(ArbitraryShapeHotSpot.get());
		}

		void testConstructorWithRectSmallerThanFontSurfFilterImageFullOffScreenShouldSucceed() {
			PRINT_TEST_NAME();
			std::auto_ptr<VNArbitraryShapeHotSpot> ArbitraryShapeHotSpot (new
				VNArbitraryShapeHotSpot(ScaleRect(RectNonZeroFullOffScreen,sfSmall), FontSurfFilterImageMediumNonZero));
			TS_ASSERT(ArbitraryShapeHotSpot.get());
		}

		void testConstructorWithRectLargerThanFontSurfFilterImageFullOnScreenShouldSucceed() {
			PRINT_TEST_NAME();
			std::auto_ptr<VNArbitraryShapeHotSpot> ArbitraryShapeHotSpot (new
				VNArbitraryShapeHotSpot(ScaleRect(RectNonZeroFullOnScreen,sfLarge), FontSurfFilterImageMediumNonZero));
			TS_ASSERT(ArbitraryShapeHotSpot.get());
		}

		void testConstructorWithRectLargerThanFontSurfFilterImagePartOnScreenShouldSucceed() {
			PRINT_TEST_NAME();
			std::auto_ptr<VNArbitraryShapeHotSpot> ArbitraryShapeHotSpot (new
				VNArbitraryShapeHotSpot(ScaleRect(RectNonZeroPartOnScreen,sfLarge), FontSurfFilterImageMediumNonZero));
			TS_ASSERT(ArbitraryShapeHotSpot.get());
		}

		void testConstructorWithRectLargerThanFontSurfFilterImageFullOffScreenShouldSucceed() {
			PRINT_TEST_NAME();
			std::auto_ptr<VNArbitraryShapeHotSpot> ArbitraryShapeHotSpot (new
				VNArbitraryShapeHotSpot(ScaleRect(RectNonZeroFullOffScreen,sfLarge), FontSurfFilterImageMediumNonZero));
			TS_ASSERT(ArbitraryShapeHotSpot.get());
		}

		void testConstructorWithZeroRectNonZeroFontSurfFilterImageFullOnScreenShouldSucceed() {
			PRINT_TEST_NAME();
			TS_TRACE("No test yet for Constructor with zero size rect and non-zero size fontsurf filter image (full onscreen);\n"
			"Current behavior: OpenCV Error: Assertion failed (dsize.area() || (inv_scale_x > 0 && inv_scale_y > 0))\n"
			"in resize. Expected: ?");
			/*
			std::auto_ptr<VNArbitraryShapeHotSpot> ArbitraryShapeHotSpot (new
				VNArbitraryShapeHotSpot(RectZero, FontSurfFilterImageMediumNonZero));
			TS_ASSERT(ArbitraryShapeHotSpot.get());
			*/
		}

		void testConstructorWithZeroRectNonZeroFontSurfFilterImagePartOnScreenShouldSucceed() {
			PRINT_TEST_NAME();
			TS_TRACE("No test yet for Constructor with zero size rect and non-zero size fontsurf filter image (partially offscreen);\n"
			"Current behavior: OpenCV Error: Assertion failed (dsize.area() || (inv_scale_x > 0 && inv_scale_y > 0))\n"
			"in resize. Expected: ?");
			/*
			std::auto_ptr<VNArbitraryShapeHotSpot> ArbitraryShapeHotSpot (new
				VNArbitraryShapeHotSpot(RectZero, FontSurfFilterImageMediumNonZero));
			TS_ASSERT(ArbitraryShapeHotSpot.get());
			*/
		}

		void testConstructorWithZeroRectNonZeroFontSurfFilterImageFullOffScreenShouldSucceed() {
			PRINT_TEST_NAME();
			TS_TRACE("No test yet for Constructor with zero size rect and non-zero size fontsurf filter image (full offscreen);\n"
			"Current behavior: OpenCV Error: Assertion failed (dsize.area() || (inv_scale_x > 0 && inv_scale_y > 0))\n"
			"in resize. Expected: ?");
			/*
			std::auto_ptr<VNArbitraryShapeHotSpot> ArbitraryShapeHotSpot (new
				VNArbitraryShapeHotSpot(RectZero, FontSurfFilterImageMediumNonZero));
			TS_ASSERT(ArbitraryShapeHotSpot.get());
			*/
		}

		void testConstructorWithNonZeroRectZeroFontSurfFilterImageFullOnScreenShouldSucceed() {
			PRINT_TEST_NAME();
			TS_TRACE("No test yet for Constructor with non-zero size rect and zero size fontsurf filter image (full onscreen);\n"
			"Current behavior: OpenCV Error: Assertion failed (ssize.area() > 0) in resize. Expected: ?");
			/*
			std::auto_ptr<VNArbitraryShapeHotSpot> ArbitraryShapeHotSpot (new
				VNArbitraryShapeHotSpot(RectNonZeroFullOnScreen, FontSurfFilterImageZero));
			TS_ASSERT(ArbitraryShapeHotSpot.get());
			*/
		}

		void testConstructorWithNonZeroRectZeroFontSurfFilterImagePartOnScreenShouldSucceed() {
			PRINT_TEST_NAME();
			TS_TRACE("No test yet for Constructor with non-zero size rect and zero size fontsurf filter image (partially offscreen);\n"
			"Current behavior: OpenCV Error: Assertion failed (ssize.area() > 0) in resize. Expected: ?");
			/*
			std::auto_ptr<VNArbitraryShapeHotSpot> ArbitraryShapeHotSpot (new
				VNArbitraryShapeHotSpot(RectNonZeroPartOnScreen, FontSurfFilterImageZero));
			TS_ASSERT(ArbitraryShapeHotSpot.get());
			*/
		}

		void testConstructorWithNonZeroRectZeroFontSurfFilterImageFullOffScreenShouldSucceed() {
			PRINT_TEST_NAME();
			TS_TRACE("No test yet for Constructor with non-zero size rect and zero size fontsurf filter image (full offscreen);\n"
			"Current behavior: OpenCV Error: Assertion failed (ssize.area() > 0) in resize. Expected: ?");
			/*
			std::auto_ptr<VNArbitraryShapeHotSpot> ArbitraryShapeHotSpot (new
				VNArbitraryShapeHotSpot(RectNonZeroFullOffScreen, FontSurfFilterImageZero));
			TS_ASSERT(ArbitraryShapeHotSpot.get());
			*/
		}

		void testSetFilterImageWithZeroMatFilterImageFullOnScreenShouldSucceed() {
			PRINT_TEST_NAME();
			TS_TRACE("No test yet for SetFilterImage with zero size mat filter image (full onscreen);\n"
			"Current behavior: OpenCV Error: Assertion failed (ssize.area() > 0) in resize. Expected: ?");
			/*
			VNArbitraryShapeHotSpot ArbitraryShapeHotSpot(RectNonZeroFullOnScreen, MatFilterImageMediumNonZero);
			ArbitraryShapeHotSpot.SetFilterImage(MatFilterImageZero);
			ASSERT_FILTERIMAGE_EQUALS(MatFilterImageZero, ArbitraryShapeHotSpot.GetFilterImage());
			*/
		}

		void testSetFilterImageWithZeroMatFilterImagePartOnScreenShouldSucceed() {
			PRINT_TEST_NAME();
			TS_TRACE("No test yet for SetFilterImage with zero size mat filter image (partially offscreen);\n"
			"Current behavior: OpenCV Error: Assertion failed (ssize.area() > 0) in resize. Expected: ?");
			/*
			VNArbitraryShapeHotSpot ArbitraryShapeHotSpot(RectNonZeroPartOnScreen, MatFilterImageMediumNonZero);
			ArbitraryShapeHotSpot.SetFilterImage(MatFilterImageZero);
			ASSERT_FILTERIMAGE_EQUALS(MatFilterImageZero, ArbitraryShapeHotSpot.GetFilterImage());
			*/
		}

		void testSetFilterImageWithZeroMatFilterImageFullOffScreenShouldSucceed() {
			PRINT_TEST_NAME();
			TS_TRACE("No test yet for SetFilterImage with zero size mat filter image (full offscreen);\n"
			"Current behavior: OpenCV Error: Assertion failed (ssize.area() > 0) in resize. Expected: ?");
			/*
			VNArbitraryShapeHotSpot ArbitraryShapeHotSpot(RectNonZeroFullOffScreen, MatFilterImageMediumNonZero);
			ArbitraryShapeHotSpot.SetFilterImage(MatFilterImageZero);
			ASSERT_FILTERIMAGE_EQUALS(MatFilterImageZero, ArbitraryShapeHotSpot.GetFilterImage());
			*/
		}

		void testSetFilterImageWithNonZeroMatFilterImageFullOnScreenShouldSucceed() {
			PRINT_TEST_NAME();
			VNArbitraryShapeHotSpot ArbitraryShapeHotSpot(RectNonZeroFullOnScreen, MatFilterImageMediumNonZero);
			ArbitraryShapeHotSpot.SetFilterImage(MatFilterImageMediumNonZero);
			ASSERT_FILTERIMAGE_EQUALS(MatFilterImageMediumNonZero, ArbitraryShapeHotSpot.GetFilterImage());
		}

		void testSetFilterImageWithNonZeroMatFilterImagePartOnScreenShouldSucceed() {
			PRINT_TEST_NAME();
			VNArbitraryShapeHotSpot ArbitraryShapeHotSpot(RectNonZeroPartOnScreen, MatFilterImageMediumNonZero);
			ArbitraryShapeHotSpot.SetFilterImage(MatFilterImageMediumNonZero);
			ASSERT_FILTERIMAGE_EQUALS(MatFilterImageMediumNonZero, ArbitraryShapeHotSpot.GetFilterImage());
		}

		void testSetFilterImageWithNonZeroMatFilterImageFullOffScreenShouldSucceed() {
			PRINT_TEST_NAME();
			VNArbitraryShapeHotSpot ArbitraryShapeHotSpot(RectNonZeroFullOffScreen, MatFilterImageMediumNonZero);
			ArbitraryShapeHotSpot.SetFilterImage(MatFilterImageMediumNonZero);
			ASSERT_FILTERIMAGE_EQUALS(MatFilterImageMediumNonZero, ArbitraryShapeHotSpot.GetFilterImage());
		}

		void testSetFilterImageWithDifferentSizeMatFilterImageFullOnScreenShouldSucceed() {
			PRINT_TEST_NAME();
			VNArbitraryShapeHotSpot ArbitraryShapeHotSpot(ScaleRect(RectNonZeroFullOnScreen,sfLarge), MatFilterImageMediumNonZero);
			ArbitraryShapeHotSpot.SetFilterImage(MatFilterImageMediumNonZero);
			ASSERT_FILTERIMAGE_EQUALS(MatFilterImageMediumNonZero, ArbitraryShapeHotSpot.GetFilterImage());
		}

		void testSetFilterImageWithDifferentSizeMatFilterImagePartOnScreenShouldSucceed() {
			PRINT_TEST_NAME();
			VNArbitraryShapeHotSpot ArbitraryShapeHotSpot(ScaleRect(RectNonZeroPartOnScreen,sfLarge), MatFilterImageMediumNonZero);
			ArbitraryShapeHotSpot.SetFilterImage(MatFilterImageMediumNonZero);
			ASSERT_FILTERIMAGE_EQUALS(MatFilterImageMediumNonZero, ArbitraryShapeHotSpot.GetFilterImage());
		}

		void testSetFilterImageWithDifferentSizeMatFilterImageFullOffScreenShouldSucceed() {
			PRINT_TEST_NAME();
			VNArbitraryShapeHotSpot ArbitraryShapeHotSpot(ScaleRect(RectNonZeroFullOffScreen,sfLarge), MatFilterImageMediumNonZero);
			ArbitraryShapeHotSpot.SetFilterImage(MatFilterImageMediumNonZero);
			ASSERT_FILTERIMAGE_EQUALS(MatFilterImageMediumNonZero, ArbitraryShapeHotSpot.GetFilterImage());
		}

		void testSetFilterImageWithZeroFontSurfFilterImageFullOnScreenShouldSucceed() {
			PRINT_TEST_NAME();
			TS_TRACE("No test yet for SetFilterImage with zero size fontsurf filter image (full onscreen);\n"
			"Current behavior: OpenCV Error: Assertion failed (ssize.area() > 0) in resize. Expected: ?");
			/*
			VNArbitraryShapeHotSpot ArbitraryShapeHotSpot(RectNonZeroFullOnScreen, FontSurfFilterImageMediumNonZero);
			ArbitraryShapeHotSpot.SetFilterImage(FontSurfFilterImageZero);
			*/
		}

		void testSetFilterImageWithZeroFontSurfFilterImagePartOnScreenShouldSucceed() {
			PRINT_TEST_NAME();
			TS_TRACE("No test yet for SetFilterImage with zero size fontsurf filter image (partially offscreen);\n"
			"Current behavior: OpenCV Error: Assertion failed (ssize.area() > 0) in resize. Expected: ?");
			/*
			VNArbitraryShapeHotSpot ArbitraryShapeHotSpot(RectNonZeroPartOnScreen, FontSurfFilterImageMediumNonZero);
			ArbitraryShapeHotSpot.SetFilterImage(FontSurfFilterImageZero);
			*/
		}

		void testSetFilterImageWithZeroFontSurfFilterImageFullOffScreenShouldSucceed() {
			PRINT_TEST_NAME();
			TS_TRACE("No test yet for SetFilterImage with zero size fontsurf filter image (full offscreen);\n"
			"Current behavior: OpenCV Error: Assertion failed (ssize.area() > 0) in resize. Expected: ?");
			/*
			VNArbitraryShapeHotSpot ArbitraryShapeHotSpot(RectNonZeroFullOffScreen, FontSurfFilterImageMediumNonZero);
			ArbitraryShapeHotSpot.SetFilterImage(FontSurfFilterImageZero);
			*/
		}

		void testSetFilterImageWithNonZeroFontSurfFilterImageFullOnScreenShouldSucceed() {
			PRINT_TEST_NAME();
			VNArbitraryShapeHotSpot ArbitraryShapeHotSpot(RectNonZeroFullOnScreen, FontSurfFilterImageMediumNonZero);
			ArbitraryShapeHotSpot.SetFilterImage(FontSurfFilterImageMediumNonZero);
			ASSERT_FILTERIMAGE_EQUALS(MatFilterImageMediumFontSurfReferencePoint, ArbitraryShapeHotSpot.GetFilterImage());
		}

		void testSetFilterImageWithNonZeroFontSurfFilterImagePartOnScreenShouldSucceed() {
			PRINT_TEST_NAME();
			VNArbitraryShapeHotSpot ArbitraryShapeHotSpot(RectNonZeroPartOnScreen, FontSurfFilterImageMediumNonZero);
			ArbitraryShapeHotSpot.SetFilterImage(FontSurfFilterImageMediumNonZero);
			ASSERT_FILTERIMAGE_EQUALS(MatFilterImageMediumFontSurfReferencePoint, ArbitraryShapeHotSpot.GetFilterImage());
		}

		void testSetFilterImageWithNonZeroFontSurfFilterImageFullOffScreenShouldSucceed() {
			PRINT_TEST_NAME();
			VNArbitraryShapeHotSpot ArbitraryShapeHotSpot(RectNonZeroFullOffScreen, FontSurfFilterImageMediumNonZero);
			ArbitraryShapeHotSpot.SetFilterImage(FontSurfFilterImageMediumNonZero);
			ASSERT_FILTERIMAGE_EQUALS(MatFilterImageMediumFontSurfReferencePoint, ArbitraryShapeHotSpot.GetFilterImage());
		}

		void testSetFilterImageWithDifferentSizeFontSurfFilterImageFullOnScreenShouldSucceed() {
			PRINT_TEST_NAME();
			VNArbitraryShapeHotSpot ArbitraryShapeHotSpot(ScaleRect(RectNonZeroFullOnScreen,sfLarge), FontSurfFilterImageMediumNonZero);
			ArbitraryShapeHotSpot.SetFilterImage(FontSurfFilterImageMediumNonZero);
			ASSERT_FILTERIMAGE_EQUALS(MatFilterImageMediumFontSurfReferencePoint, ArbitraryShapeHotSpot.GetFilterImage());
		}

		void testSetFilterImageWithDifferentSizeFontSurfFilterImagePartOnScreenShouldSucceed() {
			PRINT_TEST_NAME();
			VNArbitraryShapeHotSpot ArbitraryShapeHotSpot(ScaleRect(RectNonZeroPartOnScreen,sfLarge), FontSurfFilterImageMediumNonZero);
			ArbitraryShapeHotSpot.SetFilterImage(FontSurfFilterImageMediumNonZero);
			ASSERT_FILTERIMAGE_EQUALS(MatFilterImageMediumFontSurfReferencePoint, ArbitraryShapeHotSpot.GetFilterImage());
		}

		void testSetFilterImageWithDifferentSizeFontSurfFilterImageFullOffScreenShouldSucceed() {
			PRINT_TEST_NAME();
			VNArbitraryShapeHotSpot ArbitraryShapeHotSpot(ScaleRect(RectNonZeroFullOffScreen,sfLarge), FontSurfFilterImageMediumNonZero);
			ArbitraryShapeHotSpot.SetFilterImage(FontSurfFilterImageMediumNonZero);
			ASSERT_FILTERIMAGE_EQUALS(MatFilterImageMediumFontSurfReferencePoint, ArbitraryShapeHotSpot.GetFilterImage());
		}

		void testGetFilterImageAfterConstructorWithNoArgumentShouldSucceed() {
			PRINT_TEST_NAME();
			VNArbitraryShapeHotSpot ArbitraryShapeHotSpot;
			ASSERT_FILTERIMAGE_EQUALS(MatFilterImageZero, ArbitraryShapeHotSpot.GetFilterImage());
		}

		void testGetFilterImageAfterConstructorWithMatFilterImageFullOnScreenShouldSucceed() {
			PRINT_TEST_NAME();
			VNArbitraryShapeHotSpot ArbitraryShapeHotSpot(RectNonZeroFullOnScreen, MatFilterImageMediumNonZero);
			ASSERT_FILTERIMAGE_EQUALS(MatFilterImageMediumNonZero, ArbitraryShapeHotSpot.GetFilterImage());
		}

		void testGetFilterImageAfterConstructorWithMatFilterImagePartOnScreenShouldSucceed() {
			PRINT_TEST_NAME();
			VNArbitraryShapeHotSpot ArbitraryShapeHotSpot(RectNonZeroPartOnScreen, MatFilterImageMediumNonZero);
			ASSERT_FILTERIMAGE_EQUALS(MatFilterImageMediumNonZero, ArbitraryShapeHotSpot.GetFilterImage());
		}

		void testGetFilterImageAfterConstructorWithMatFilterImageFullOffScreenShouldSucceed() {
			PRINT_TEST_NAME();
			VNArbitraryShapeHotSpot ArbitraryShapeHotSpot(RectNonZeroFullOffScreen, MatFilterImageMediumNonZero);
			ASSERT_FILTERIMAGE_EQUALS(MatFilterImageMediumNonZero, ArbitraryShapeHotSpot.GetFilterImage());
		}

		void testGetFilterImageAfterSetMatFilterImageFullOnScreenShouldSucceed() {
			PRINT_TEST_NAME();
			VNArbitraryShapeHotSpot ArbitraryShapeHotSpot(RectNonZeroFullOnScreen, MatFilterImageSmallNonZero);
			ArbitraryShapeHotSpot.SetFilterImage(MatFilterImageMediumNonZero);
			ASSERT_FILTERIMAGE_EQUALS(MatFilterImageMediumNonZero, ArbitraryShapeHotSpot.GetFilterImage());
		}

		void testGetFilterImageAfterSetMatFilterImagePartOnScreenShouldSucceed() {
			PRINT_TEST_NAME();
			VNArbitraryShapeHotSpot ArbitraryShapeHotSpot(RectNonZeroPartOnScreen, MatFilterImageSmallNonZero);
			ArbitraryShapeHotSpot.SetFilterImage(MatFilterImageMediumNonZero);
			ASSERT_FILTERIMAGE_EQUALS(MatFilterImageMediumNonZero, ArbitraryShapeHotSpot.GetFilterImage());
		}

		void testGetFilterImageAfterSetMatFilterImageFullOffScreenShouldSucceed() {
			PRINT_TEST_NAME();
			VNArbitraryShapeHotSpot ArbitraryShapeHotSpot(RectNonZeroFullOffScreen, MatFilterImageSmallNonZero);
			ArbitraryShapeHotSpot.SetFilterImage(MatFilterImageMediumNonZero);
			ASSERT_FILTERIMAGE_EQUALS(MatFilterImageMediumNonZero, ArbitraryShapeHotSpot.GetFilterImage());
		}

		void testGetFilterImageAfterConstructorWithFontSurfFilterImageFullOnScreenShouldSucceed() {
			PRINT_TEST_NAME();
			VNArbitraryShapeHotSpot ArbitraryShapeHotSpot(RectNonZeroFullOnScreen, FontSurfFilterImageMediumNonZero);
			ASSERT_FILTERIMAGE_EQUALS(MatFilterImageMediumFontSurfReferencePoint, ArbitraryShapeHotSpot.GetFilterImage());
		}

		void testGetFilterImageAfterConstructorWithFontSurfFilterImagePartOnScreenShouldSucceed() {
			PRINT_TEST_NAME();
			VNArbitraryShapeHotSpot ArbitraryShapeHotSpot(RectNonZeroPartOnScreen, FontSurfFilterImageMediumNonZero);
			ASSERT_FILTERIMAGE_EQUALS(MatFilterImageMediumFontSurfReferencePoint, ArbitraryShapeHotSpot.GetFilterImage());
		}

		void testGetFilterImageAfterConstructorWithFontSurfFilterImageFullOffScreenShouldSucceed() {
			PRINT_TEST_NAME();
			VNArbitraryShapeHotSpot ArbitraryShapeHotSpot(RectNonZeroFullOffScreen, FontSurfFilterImageMediumNonZero);
			ASSERT_FILTERIMAGE_EQUALS(MatFilterImageMediumFontSurfReferencePoint, ArbitraryShapeHotSpot.GetFilterImage());
		}

		void testGetFilterImageAfterSetFontSurfFilterImageFullOnScreenShouldSucceed() {
			PRINT_TEST_NAME();
			VNArbitraryShapeHotSpot ArbitraryShapeHotSpot(RectNonZeroFullOnScreen, FontSurfFilterImageSmallNonZero);
			ArbitraryShapeHotSpot.SetFilterImage(FontSurfFilterImageMediumNonZero);
			ASSERT_FILTERIMAGE_EQUALS(MatFilterImageMediumFontSurfReferencePoint, ArbitraryShapeHotSpot.GetFilterImage());
		}

		void testGetFilterImageAfterSetFontSurfFilterImagePartOnScreenShouldSucceed() {
			PRINT_TEST_NAME();
			VNArbitraryShapeHotSpot ArbitraryShapeHotSpot(RectNonZeroPartOnScreen, FontSurfFilterImageSmallNonZero);
			ArbitraryShapeHotSpot.SetFilterImage(FontSurfFilterImageMediumNonZero);
			ASSERT_FILTERIMAGE_EQUALS(MatFilterImageMediumFontSurfReferencePoint, ArbitraryShapeHotSpot.GetFilterImage());
		}

		void testGetFilterImageAfterSetFontSurfFilterImageFullOffScreenShouldSucceed() {
			PRINT_TEST_NAME();
			VNArbitraryShapeHotSpot ArbitraryShapeHotSpot(RectNonZeroFullOffScreen, FontSurfFilterImageSmallNonZero);
			ArbitraryShapeHotSpot.SetFilterImage(FontSurfFilterImageMediumNonZero);
			ASSERT_FILTERIMAGE_EQUALS(MatFilterImageMediumFontSurfReferencePoint, ArbitraryShapeHotSpot.GetFilterImage());
		}

	private:
		void init() {
			int screenWidth = GetScreenWidth();
			int screenHeight = GetScreenHeight();

			// Rectangle area (size = 1/2 * screenWidth , 1/2 * screenHeight) in center on screen
			SetRectDimension(RectNonZeroFullOnScreen, ONE_FOURTHS * screenWidth, ONE_FOURTHS * screenHeight, ONE_HALF * screenWidth, ONE_HALF * screenHeight);

			// Medium rectangle area (size = 1/2 * screenWidth , 1/2 * screenHeight) in center left partial on screen
			SetRectDimension(RectNonZeroPartOnScreen, -ONE_FOURTHS * screenWidth, ONE_FOURTHS * screenHeight, ONE_HALF * screenWidth, ONE_HALF * screenHeight);

			// Medium rectangle area (size = 1/2 * screenWidth , 1/2 * screenHeight) in center right off screen
			SetRectDimension(RectNonZeroFullOffScreen, (1 + ONE_FOURTHS) * screenWidth, ONE_FOURTHS * screenHeight, ONE_HALF * screenWidth, ONE_HALF * screenHeight);

			// Zero rectangle area (size = 0, 0) in top left on screen
			SetRectDimension(RectZero, 0, 0, 0, 0);

			// Mat filter image (size = 1/4 * screenWidth , 1/4 * screenHeight)
			CreateDataFilterImage (dataSmallFilterImage, ONE_FOURTHS * screenWidth, ONE_FOURTHS * screenHeight);
			MatFilterImageSmallNonZero = cv::Mat (cv::Size(ONE_FOURTHS * screenWidth, ONE_FOURTHS * screenHeight), CV_8U, dataSmallFilterImage);
			
			// Mat filter image (size = 1/2 * screenWidth , 1/2 * screenHeight)
			CreateDataFilterImage (dataMediumFilterImage, ONE_HALF * screenWidth, ONE_HALF * screenHeight);
			MatFilterImageMediumNonZero = cv::Mat (cv::Size(ONE_HALF * screenWidth, ONE_HALF * screenHeight), CV_8U, dataMediumFilterImage);

			// Mat filter image (size = 0, 0)
			MatFilterImageZero = cv::Mat (0, 0, CV_8U);

			// FontSurf filter image (size = 1/2 * screenHeight , 1/2 * screenHeight)
			SetFontSurfFilterImage(FontSurfFilterImageMediumNonZero, ONE_HALF * screenWidth, ONE_HALF * screenHeight);

			// FontSurf filter image (size = 1/4 * screenHeight , 1/4 * screenHeight)
			SetFontSurfFilterImage(FontSurfFilterImageSmallNonZero, ONE_FOURTHS * screenWidth, ONE_FOURTHS * screenHeight);

			// FontSurf filter image (size = 0, 0)
			SetFontSurfFilterImage(FontSurfFilterImageZero, 0, 0);
		
			// Mat filter image reference point for fontsurf filter
			VNArbitraryShapeHotSpot ArbitraryShapeHotSpot(RectNonZeroFullOffScreen, FontSurfFilterImageMediumNonZero);
			MatFilterImageMediumFontSurfReferencePoint = ArbitraryShapeHotSpot.GetFilterImage();
		}

		void cleanup() {
			delete[] dataSmallFilterImage;
			delete[] dataMediumFilterImage;
		}

		void ASSERT_FILTERIMAGE_EQUALS(const cv::Mat& filterImage0, const cv::Mat& filterImage1) {
			TS_ASSERT(filterImage0.size().width == filterImage1.size().width);
			TS_ASSERT(filterImage0.size().height == filterImage1.size().height);
						
			cv::Mat matDiff;
			cv::compare(filterImage0, filterImage1, matDiff, cv::CMP_NE);
			TS_ASSERT_EQUALS(0, cv::countNonZero(matDiff));
		}

		enum RectSizeFactor { sfSmall, sfLarge };

		tRect ScaleRect (tRect Rect, RectSizeFactor sizeFactor) {
			int screenWidth = GetScreenWidth();
			int screenHeight = GetScreenHeight();
			int widthDiff = 0;
			int heightDiff = 0;

			switch (sizeFactor) {
				case sfSmall:
					widthDiff = -ONE_EIGHTHS * screenWidth;
					heightDiff = -ONE_EIGHTHS * screenHeight;
					break;
				case sfLarge:
					widthDiff = ONE_EIGHTHS * screenWidth;
					heightDiff = ONE_EIGHTHS * screenHeight;
					break;
				default:
					break;
			}
			
			tRect scaledRect;
			scaledRect.left = Rect.left - widthDiff;
			scaledRect.right = Rect.right + widthDiff;
			scaledRect.top = Rect.top - heightDiff;
			scaledRect.bottom = Rect.bottom + heightDiff;
			
			return scaledRect;
		}

		void SetRectDimension (tRect& Rect, int left, int top, int width, int height) {
			Rect.left = left;
			Rect.right = left + width;
			Rect.top = top;
			Rect.bottom = top + height;
		}
		
		void CreateDataFilterImage (unsigned char*& data, int width, int height) {
			data = (unsigned char*)(malloc(width * height * sizeof(unsigned char)));
			for (int i = 0; i < width; ++i) {
				for (int j = 0; j < height; ++j) {
					if (j >= i)
						data[i*height+j] = kVNMaxPixelValue;
					else
						data[i*height+j] = kVNMinPixelValue;
				}
			}
		}

		void SetFontSurfFilterImage (tFontSurf& fontSurfFilterImage, int width, int height) {
			fontSurfFilterImage.width  = width;
			fontSurfFilterImage.height = height;
			fontSurfFilterImage.pitch  = fontSurfFilterImage.width * 4;
			fontSurfFilterImage.buffer = static_cast<U8*>(malloc(fontSurfFilterImage.pitch * fontSurfFilterImage.height));
			fontSurfFilterImage.format = kPixelFormatARGB8888;
			memset(fontSurfFilterImage.buffer, 0, fontSurfFilterImage.pitch * fontSurfFilterImage.height);
		}


	private:
		tRect RectZero;
		tRect RectNonZeroFullOnScreen;
		tRect RectNonZeroPartOnScreen;
		tRect RectNonZeroFullOffScreen;
		
		unsigned char *dataSmallFilterImage;
		unsigned char *dataMediumFilterImage;

		cv::Mat MatFilterImageZero;
		cv::Mat MatFilterImageSmallNonZero;
		cv::Mat MatFilterImageMediumNonZero;
		cv::Mat MatFilterImageMediumFontSurfReferencePoint;
		
		tFontSurf FontSurfFilterImageZero;
		tFontSurf FontSurfFilterImageSmallNonZero;
		tFontSurf FontSurfFilterImageMediumNonZero;
};
