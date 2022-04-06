
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

		//ShowData(sourceImage, "sourceImage"); // Debugging

		cv::Scalar min(3.0, 3.0, 3.0);
		cv::Scalar max(7.0, 7.0, 7.0);

		cv::Mat resultInRange;
		// Comments from cv::inRange header:
		//! set mask elements for those array elements which are within the element-specific bounding box
		// (dst = lowerb <= src && src < upperb)
		cv::inRange(sourceImage, min, max, resultInRange);
		//ShowData(resultInRange, "resultInRange");  // Debugging

		cv::Mat resultInRange3;
		inRange3(sourceImage, min, max, resultInRange3);
		//ShowData(resultInRange3, "resultInRange3");  // Debugging

		TS_ASSERT_SAME_DATA(resultInRange.data, resultInRange3.data, numPoints);
	}


private:

	// For debugging
	void ShowData(cv::Mat& image, const char* message = ""){

		LeapFrog::Brio::CDebugMPI debug(kGroupVision);

		debug.DebugOut(kDbgLvlCritical, "ShowData start: %s\n", message);
		debug.DebugOut(kDbgLvlCritical, "index\timage\n");

		uchar numPoints = image.total();
		uchar* data = image.data;

		for (int i = 0; i < numPoints; i++){
			switch(image.type()){
				case CV_8U:
					debug.DebugOut(kDbgLvlCritical, "%d\t%d\n", i, data[i]);
					break;
				case CV_8UC3:
					debug.DebugOut(kDbgLvlCritical, "%d\t%d, %d, %d\n", i, data[3*i + 0], data[3*i + 1], data[3*i + 2]);
					break;

				default:
					debug.DebugOut(kDbgLvlCritical, "ERROR in ShowData. Default case hit. Type not handled: %d\n", image.type());
			}
		}

		debug.DebugOut(kDbgLvlCritical, "ShowData end\n");
	}
};
