#include <cxxtest/TestSuite.h>
#include <UnitTestUtils.h>
#include <UnitTestVisionUtils.h>

#include <Vision/VNVirtualTouch.h>

#include <VNVirtualTouchPIMPL.h>
#include <VNIntegralImage.h>
#define VN_PROFILE 1
#include <VNProfiler.h>
#include <sstream>
//For strcmp
#include <string.h>
#include <UnitTestUtils.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>

using namespace LF::Vision;

#define VN_TEST_WIDTH 640
#define VN_TEST_HEIGHT 480

static const float ONE_FOURTHS = 0.25f;

class TestVirtualTouch : public CxxTest::TestSuite, TestSuiteBase, UnitTestVisionUtils {
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

		TestVirtualTouch() {
			init();
		}

		~TestVirtualTouch() {
			cleanup();
		}

		void testConstructionWithNoArgumentShouldSucceed() {
			PRINT_TEST_NAME();
			std::auto_ptr<VNVirtualTouch> VirtualTouch (new VNVirtualTouch());
			TS_ASSERT(VirtualTouch.get());
		}

		void testConstructionWithLearningRateBiggerThanZeroSmallerEqualToOneShouldSucceed() {
			PRINT_TEST_NAME();
			std::auto_ptr<VNVirtualTouch> VirtualTouch0 (new VNVirtualTouch(0.1f));
			TS_ASSERT(VirtualTouch0.get());
			std::auto_ptr<VNVirtualTouch> VirtualTouch1 (new VNVirtualTouch(1.0f));
			TS_ASSERT(VirtualTouch1.get());
		}

		void testConstructionWithLearningRateEqualZeroShouldSucceed() {
			PRINT_TEST_NAME();
			std::auto_ptr<VNVirtualTouch> VirtualTouch (new VNVirtualTouch(0.0f));
			TS_ASSERT(VirtualTouch.get());
		}

		void testConstructionWithLearningRateBiggerThanOneShouldSucceed() {
			PRINT_TEST_NAME();
			std::auto_ptr<VNVirtualTouch> VirtualTouch (new VNVirtualTouch(1.2f));
			TS_ASSERT(VirtualTouch.get());
		}

		void testConstructionWithLearningRateSmallerThanZeroShouldSucceed() {
			PRINT_TEST_NAME();
			std::auto_ptr<VNVirtualTouch> VirtualTouch (new VNVirtualTouch(-0.1f));
			TS_ASSERT(VirtualTouch.get());
		}

		void testInitializeWithZeroWidthZeroHeightShouldSucceed() {
			PRINT_TEST_NAME();
			VNVirtualTouch VirtualTouch;
			VirtualTouch.Initialize(0, 0);
		}

		void testInitializeWithLessThanZeroWidthBiggerThanZeroHeightShouldSucceed() {
			PRINT_TEST_NAME();
			VNVirtualTouch VirtualTouch;
			VirtualTouch.Initialize(-48, 48);
		}

		void testInitializeWithLessThanZeroWidthLessThanZeroHeightShouldSucceed() {
			PRINT_TEST_NAME();
			VNVirtualTouch VirtualTouch;
			VirtualTouch.Initialize(-48, -48);
		}

		void testInitializeWithBiggerThanZeroWidthBiggerThanZeroHeightShouldSucceed() {
			PRINT_TEST_NAME();
			VNVirtualTouch VirtualTouch;
			VirtualTouch.Initialize(48, 48);
		}

		void testSetLearningRateWithRateEqualZeroShouldSucceed() {
			PRINT_TEST_NAME();
			VNVirtualTouch VirtualTouch;
			float learningRate = VirtualTouch.GetLearningRate();
			VirtualTouch.SetLearningRate(0.0f);
			TS_ASSERT_EQUALS(learningRate, VirtualTouch.GetLearningRate());
		}

		void testSetLearningRateWithRateLessThanZeroShouldSucceed() {
			PRINT_TEST_NAME();
			VNVirtualTouch VirtualTouch;
			float learningRate = VirtualTouch.GetLearningRate();
			VirtualTouch.SetLearningRate(-0.1f);
			TS_ASSERT_EQUALS(learningRate, VirtualTouch.GetLearningRate());
		}

		void testSetLearningRateWithRateEqualZeroPointFiveShouldSucceed() {
			PRINT_TEST_NAME();
			VNVirtualTouch VirtualTouch;
			VirtualTouch.SetLearningRate(0.5f);
			TS_ASSERT_EQUALS(0.5f, VirtualTouch.GetLearningRate());
		}

		void testSetLearningRateWithRateEqualOneShouldSucceed() {
			PRINT_TEST_NAME();
			VNVirtualTouch VirtualTouch;
			VirtualTouch.SetLearningRate(1.0f);
			TS_ASSERT_EQUALS(1.0f, VirtualTouch.GetLearningRate());
		}

		void testSetLearningRateWithRateBiggerThanOneShouldSucceed() {
			PRINT_TEST_NAME();
			VNVirtualTouch VirtualTouch;
			float learningRate = VirtualTouch.GetLearningRate();
			VirtualTouch.SetLearningRate(1.2f);
			TS_ASSERT_EQUALS(learningRate, VirtualTouch.GetLearningRate());
		}

		void testGetLearningRateAfterConstructionWithNoArgumentShouldSucceed() {
			PRINT_TEST_NAME();
			VNVirtualTouch VirtualTouch;
			TS_ASSERT_EQUALS(kVNDefaultVirtualTouchLearningRate, VirtualTouch.GetLearningRate());
		}

		void testGetLearningRateAfterConstructionWithZeroLearningRateShouldSucceed() {
			PRINT_TEST_NAME();
			VNVirtualTouch VirtualTouch (0.0f);
			TS_ASSERT_EQUALS(kVNDefaultVirtualTouchLearningRate, VirtualTouch.GetLearningRate());
		}

		void testGetLearningRateAfterConstructionWithLessThanZeroLearningRateShouldSucceed() {
			PRINT_TEST_NAME();
			VNVirtualTouch VirtualTouch (-0.1f);
			TS_ASSERT_EQUALS(kVNDefaultVirtualTouchLearningRate, VirtualTouch.GetLearningRate());
		}

		void testGetLearningRateAfterConstructionWithZeroPointFiveLearningRateShouldSucceed() {
			PRINT_TEST_NAME();
			VNVirtualTouch VirtualTouch (0.5f);
			TS_ASSERT_EQUALS(0.5f, VirtualTouch.GetLearningRate());
		}

		void testGetLearningRateAfterConstructionWithOneLearningRateShouldSucceed() {
			PRINT_TEST_NAME();
			VNVirtualTouch VirtualTouch (1.0f);
			TS_ASSERT_EQUALS(1.0f, VirtualTouch.GetLearningRate());
		}

		void testGetLearningRateAfterConstructionWithBiggerThanOneLearningRateShouldSucceed() {
			PRINT_TEST_NAME();
			VNVirtualTouch VirtualTouch (1.2f);
			TS_ASSERT_EQUALS(kVNDefaultVirtualTouchLearningRate, VirtualTouch.GetLearningRate());
		}

		void testGetLearningRateAfterSetLearningRateWithZeroShouldSucceed() {
			PRINT_TEST_NAME();
			VNVirtualTouch VirtualTouch;
			float learningRate = VirtualTouch.GetLearningRate();
			VirtualTouch.SetLearningRate (0.0f);
			TS_ASSERT_EQUALS(learningRate, VirtualTouch.GetLearningRate());
		}

		void testGetLearningRateAfterSetLearningRateWithLessThanZeroShouldSucceed() {
			PRINT_TEST_NAME();
			VNVirtualTouch VirtualTouch;
			float learningRate = VirtualTouch.GetLearningRate();
			VirtualTouch.SetLearningRate (-0.1f);
			TS_ASSERT_EQUALS(learningRate, VirtualTouch.GetLearningRate());
		}

		void testGetLearningRateAfterSetLearningRateWithZeroPointFiveShouldSucceed() {
			PRINT_TEST_NAME();
			VNVirtualTouch VirtualTouch;
			VirtualTouch.SetLearningRate (0.5f);
			TS_ASSERT_EQUALS(0.5f, VirtualTouch.GetLearningRate());
		}

		void testGetLearningRateAfterSetLearningRateWithOneShouldSucceed() {
			PRINT_TEST_NAME();
			VNVirtualTouch VirtualTouch;
			VirtualTouch.SetLearningRate (1.0f);
			TS_ASSERT_EQUALS(1.0f, VirtualTouch.GetLearningRate());
		}

		void testGetLearningRateAfterSetLearningRateWithBiggerThanOneShouldSucceed() {
			PRINT_TEST_NAME();
			VNVirtualTouch VirtualTouch;
			float learningRate = VirtualTouch.GetLearningRate();
			VirtualTouch.SetLearningRate (1.2f);
			TS_ASSERT_EQUALS(learningRate, VirtualTouch.GetLearningRate());
		}

		void testExecuteShouldSucceed() {
			PRINT_TEST_NAME();
			VNVirtualTouch VirtualTouch;
			VirtualTouch.Execute(MatInputImage, MatOutputImage);
		}

		//------------------------------------------------------------------------
		void testVirtualTouchPerformance() {
			PRINT_TEST_NAME();
/*
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
 */
		}

		void testIntegralImagePerformance() {
			
			PRINT_TEST_NAME();

			PROF_RESET();

			cv::Mat input(cv::Size(VN_TEST_WIDTH, VN_TEST_HEIGHT), CV_8U);
			cv::Mat output;
			for( int i = 0; i < 250; i++ ) {
				cv::randu(input, cv::Scalar::all(0), cv::Scalar::all(255));
				PROF_BLOCK_START("OpenCV integral");
				cv::integral( input, output, -1 );
				PROF_BLOCK_END();
			}

			for( int i = 0; i < 250; i++ ) {
				cv::randu(input, cv::Scalar::all(0), cv::Scalar::all(255));
				PROF_BLOCK_START("Fast integral");
				LF::Vision::IntegralImage( input, output );
				PROF_BLOCK_END();
			}
			
			PROF_PRINT_REPORT();
		}
	
		void testIntegralImageToOpenCV() {
		
			PRINT_TEST_NAME();
		
			cv::Mat input(cv::Size(VN_TEST_WIDTH, VN_TEST_HEIGHT), CV_8U);
			cv::Mat output1(input.size(), CV_32S), output2(input.size(), CV_32S);
		
		
			// generate random image
			cv::randu(input, cv::Scalar::all(0), cv::Scalar::all(255));
		
			// opencv
			cv::integral( input, output1, -1 );
		
			// fast
			LF::Vision::IntegralImage( input, output2 );
		
			printf("width = %d height = %d\n", input.size().width, input.size().height);
		
			for ( int y = 0; y < input.size().width; y++ ) {
				for ( int x = 0; x < input.size().height; x++ ) {
					int a = output1.at<int32_t>(x+1,y+1); // NOTE: the format of OpenCV adds a row and column
					int b = output2.at<int32_t>(x,y);
					if ( a != b ) {
						printf( "(%d,%d): %d != %d\n", x, y, a, b );
					}
				}
			}
		}

	private:
		void init() {
			int screenWidth = GetScreenWidth();
			int screenHeight = GetScreenHeight();
			
			// Mat image (size = 1/4 * screenWidth , 1/4 * screenHeight)
			CreateDataImage (dataImage, ONE_FOURTHS * screenWidth, ONE_FOURTHS * screenHeight);
			MatInputImage = cv::Mat (cv::Size(ONE_FOURTHS * screenWidth, ONE_FOURTHS * screenHeight), CV_8UC3, dataImage);
			
			MatOutputImage = cv::Mat (cv::Size(ONE_FOURTHS * screenWidth, ONE_FOURTHS * screenHeight), CV_8U);
		}
		
		void cleanup() {
			delete dataImage;
		}

		void CreateDataImage (unsigned char*& data, int width, int height) {
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
		
	private:
		unsigned char *dataImage;
		cv::Mat MatInputImage;
		cv::Mat MatOutputImage;
};