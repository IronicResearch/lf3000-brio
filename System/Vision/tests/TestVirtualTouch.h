
#include <cxxtest/TestSuite.h>
#include <VNVirtualTouchPIMPL.h>
#define VN_PROFILE 1
#include <VNProfiler.h>
#include <sstream>
//For strcmp
#include <string.h>
#include <UnitTestUtils.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>

#define VN_TEST_WIDTH 640
#define VN_TEST_HEIGHT 480

class TestVirtualTouch : public CxxTest::TestSuite, LeapFrog::Brio::TestSuiteBase {
	private:
		LF::Vision::VNVirtualTouchPIMPL* virtualTouch_;
	public:


		//------------------------------------------------------------------------
		void setUp(void) {
			virtualTouch_ = new LF::Vision::VNVirtualTouchPIMPL(0.13f);
		}

		//------------------------------------------------------------------------
		void tearDown(void) {
			delete virtualTouch_;
		}

		//------------------------------------------------------------------------
		void testVirtualTouchPerformance() {
			PRINT_TEST_NAME();

			PROF_RESET();

			cv::Mat input(VN_TEST_WIDTH,VN_TEST_HEIGHT,CV_8UC3);
			cv::Mat output(VN_TEST_WIDTH, VN_TEST_HEIGHT, CV_8U);
			for( int i = 0; i < 250; i++ ) {
				cv::randu(input, cv::Scalar::all(0), cv::Scalar::all(255));
				PROF_BLOCK_START("Virtual Touch Execute");
				virtualTouch_->Execute(input, output);
				PROF_BLOCK_END();
			}

			PROF_PRINT_REPORT();
		}

		void testIntegralImagePerformance() {
			PRINT_TEST_NAME();

			PROF_RESET();

			cv::Mat input(VN_TEST_WIDTH, VN_TEST_HEIGHT, CV_8U);
			cv::Mat output;
			for( int i = 0; i < 250; i++ ) {
				cv::randu(input, cv::Scalar::all(0), cv::Scalar::all(255));
				PROF_BLOCK_START("integral");
				cv::integral( input, output, -1 );
				PROF_BLOCK_END();
			}

			PROF_PRINT_REPORT();
		}

		
};

