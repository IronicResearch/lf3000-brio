
// Unit tests for VNInRange3
// Reference: Test plan:
// http://wiki.leapfrog.com/display/CTD/Testing+Plans
// VisionTestingPlanv2.xlsx

#include "VNInRange3.h"
#include <DebugMPI.h>
#include <Vision/VNVisionTypes.h>

#include <UnitTestUtils.h>
#include <cxxtest/TestSuite.h>
//#include <memory>

using namespace LF::Vision;

class TestInRange3 : public CxxTest::TestSuite,
							LeapFrog::Brio::TestSuiteBase {

  public:

	void testInRange3VersusInRangesResultImageShouldBeEqual() {
		PRINT_TEST_NAME();

		// TODO Debug and fix. Not working right yet.
		LeapFrog::Brio::CDebugMPI debug(kGroupVision);

		const int ROWS = 10;
		const int COLS = 1;
		const int numPoints = ROWS*COLS;
		const int VALUES_PER_POINT = 3;
		const int dataSize = VALUES_PER_POINT* numPoints;
		uchar data[dataSize];  // three values per point

		// Generate fake data. 10 points with 3 values of the index
		for (int i=0; i < numPoints; i++){
			data[3*i+0] = data[3*i+1] = data[3*i+2] = (uchar)(i);
		}

		cv::Mat sourceImage(ROWS, COLS, CV_8UC3, &data);

		ShowData(sourceImage, "sourceImage"); // TODO remove after working

		uchar EMPTY_DATA1[dataSize] = {0};  // To clean out memory
		uchar EMPTY_DATA2[dataSize] = {0};  // To clean out memory

		cv::Scalar min(3.0, 3.0, 3.0);
		cv::Scalar max(7.0, 7.0, 7.0);

		//cv::Mat resultInRange(ROWS, COLS, CV_8UC3,0);
		cv::Mat resultInRange(ROWS, COLS, CV_8UC3, &EMPTY_DATA1);
		//resultInRange.create( sourceImage.size(), CV_8UC3, EMPTY_DATA);
		//resultInRange = 0; // clean out junk memory. Is this needed for the later comparison?

		// Comments from cv::inRange header:
		//! set mask elements for those array elements which are within the element-specific bounding box
		// (dst = lowerb <= src && src < upperb)
		cv::inRange(sourceImage, min, max, resultInRange);
		ShowData(resultInRange, "resultInRange");  // TODO remove after working

		//cv::Mat resultInRange3(ROWS, COLS, CV_8UC3, 0);
		cv::Mat resultInRange3(ROWS, COLS, CV_8UC3, &EMPTY_DATA2);
		//resultInRange3.create( sourceImage.size(), CV_8UC3, EMPTY_DATA);
		//resultInRange3 = 0;  // clean out junk memory. Is this needed for the later comparison?

		inRange3(sourceImage, min, max, resultInRange3);
		ShowData(resultInRange3, "resultInRange3");  // TODO remove after working

		//assertMatImagesAreEqual(resultInRange, resultInRange3, dataSize);

		TS_WARN("This test is not complete yet. It needs debugging and scrutiny.");
		TS_ASSERT_SAME_DATA(resultInRange.data, resultInRange3.data, dataSize);
	}


private:

	// For debugging
	void ShowData(cv::Mat& image, const char* message = ""){

		LeapFrog::Brio::CDebugMPI debug(kGroupVision);

		debug.DebugOut(kDbgLvlCritical, "ShowData start: %s\n", message);
		debug.DebugOut(kDbgLvlCritical, "index\tdimage\n");

		uchar numPoints = image.total();
		uchar* data = image.data;

		for (int i = 0; i < numPoints; i++){
			debug.DebugOut(kDbgLvlCritical, "%d\t%d, %d, %d\n", i, data[3*i + 0], data[3*i + 1], data[3*i + 2]);
		}

		debug.DebugOut(kDbgLvlCritical, "ShowData end\n");
	}


// For debugging.  Needs scrutiny and likely fixes.
//	void assertMatImagesAreEqual(cv::Mat& image1, cv::Mat& image2, int size){
//		TS_ASSERT_EQUALS( image1.size(), image2.size());
//
//		//TS_ASSERT_SAME_DATA(image1.data, image2.data, size); //image2.size());
//		LeapFrog::Brio::CDebugMPI debug(kGroupVision);
//
//		bool areEqual = false;
//
//		size_t SIZE = image1.total();
//
//		for (int i = 0; i < SIZE; i++){
//			if ( *(image1.data + i) != *(image2.data + i)) {
//				areEqual = false;
//
//				char image1Point = (char)(*(image1.data + i));
//				char image2Point = (char)(*(image2.data + i));
//				debug.DebugOut(kDbgLvlCritical, "index = %d; image1: %d; image2: %d\n",
//														i, image1Point, image2Point);
//				break;
//			} else {
//				areEqual = true;
//			}
//		}
//
//		if (!areEqual) {
//			// Show the contents
//			debug.DebugOut(kDbgLvlCritical, "index\tdimage1\timage2\n");
//
//			for (int i = 0; i < SIZE; i++){
//				char image1Point = (char)(*(image1.data + i));
//				char image2Point = (char)(*(image2.data + i));
//				debug.DebugOut(kDbgLvlCritical, "%d\t%d\t%d\n", i, image1Point, image2Point);
//			}
//		}
//
//		TS_ASSERT(areEqual);
//	}
};
