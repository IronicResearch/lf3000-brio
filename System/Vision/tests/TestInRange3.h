
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

		const int ROWS = 10;
		const int COLS = 1;
		//const int PLANES = 10;
		//const int dataSize = ROWS*COLS*PLANES;
		const int dataSize = ROWS*COLS;
		//cv::Scalar_<uchar> data[dataSize];
		uchar data[dataSize][3];  // three values per point

		// The goal is to generate 10 points with 3 values of the index

		for (int i=0; i < dataSize; i++){
			data[i][0] = (uchar)(i);
			data[i][1] = (uchar)(i);
			data[i][2] = (uchar)(i);
		}

		cv::Mat sourceImage(ROWS, COLS, CV_8UC3, &data);

		//const int dataSize = 15;

		//cv::Mat sourceImage = (cv::Mat_<uchar>(2,3) << 1, 1, 1, 9, 9, 9, 12, 12, 12, 18, 18, 18, 25, 25, 25, 40, 40, 40);

		//Mat(int ndims, const int* sizes, int type, void* data, const size_t* steps=0);
		//Mat::Mat(Size size, int type, void* data, size_t step=AUTO_STEP)

		// Wanted to use this constructor but not compiling. Seems confused about another version.
		//cv::Mat sourceImage(dataSize, CV_8U,  &data);

//		int sizes[3];
//		sizes[0] = ROWS;
//		sizes[1] = COLS;
//		sizes[2] = PLANES;
//		cv::Mat sourceImage(3, sizes, CV_8U, &data);

		//Mat::Mat(Size size, int type, void* data, size_t step=AUTO_STEP)
		//cv::Mat sourceImage(dataSize, CV_8UC3, &data);

		ShowData(sourceImage, "sourceImage"); // TODO remove after working


		cv::Scalar min(3.0, 3.0, 3.0);
		cv::Scalar max(7.0, 7.0, 7.0);


		cv::Mat resultInRange;

		//resultPerOpenCV.create( sourceImage.size(), CV_8U );
		// Comments from cv::inRange header:
		//! set mask elements for those array elements which are within the element-specific bounding box
		// (dst = lowerb <= src && src < upperb)
		cv::inRange(sourceImage, min, max, resultInRange);
		ShowData(resultInRange, "resultInRange");  // TODO remove after working

		cv::Mat resultInRange3;
		//resultPerInRange3Function.create( sourceImage.size(), CV_8U );
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

		uchar SIZE = image.total();

		for (uchar i = 0; i < SIZE; i++){
			uchar* imagePoint = (image.data + i);
			//cv::Scalar_<uchar> imagePoint = *(image.data + i);
			debug.DebugOut(kDbgLvlCritical, "%d\t%d, %d, %d\n", i, imagePoint[0], imagePoint[1], imagePoint[2]);
		}

		debug.DebugOut(kDbgLvlCritical, "ShowData end\n");
	}

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
