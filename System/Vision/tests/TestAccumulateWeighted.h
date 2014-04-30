#include <cxxtest/TestSuite.h>
#include <VNAccumulate.h>
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

class TestAccumulateWeighted : public CxxTest::TestSuite, LeapFrog::Brio::TestSuiteBase {
	private:
	public:


		//------------------------------------------------------------------------
		void setUp(void) {
		}

		//------------------------------------------------------------------------
		void tearDown(void) {
		}

		//------------------------------------------------------------------------
		void testAccumulateWeightedPerformanceFixedPoint() {
			PRINT_TEST_NAME();

			PROF_RESET();

			cv::Mat dst(VN_TEST_WIDTH,VN_TEST_HEIGHT,CV_32S);
			cv::randu(dst, cv::Scalar::all(0), cv::Scalar::all(255));

			cv::Mat src(VN_TEST_WIDTH,VN_TEST_HEIGHT,CV_8U);
			cv::randu(src, cv::Scalar::all(0), cv::Scalar::all(255));
			
			for(int i = 0; i < 250; i++ ) {
			
				PROF_BLOCK_START("Fixed Point accumulateWeighted");
				LF::Vision::accumulateWeightedFixedPoint(src, dst, 0.13f);
				PROF_BLOCK_END();
			}

			PROF_PRINT_REPORT();			
		}

		void testAccumulateWeightedPerformanceOpenCV() {
			PRINT_TEST_NAME();

			PROF_RESET();

			cv::Mat dst(VN_TEST_WIDTH,VN_TEST_HEIGHT,CV_32F);
			cv::randu(dst, cv::Scalar::all(0), cv::Scalar::all(255));

			cv::Mat src(VN_TEST_WIDTH,VN_TEST_HEIGHT,CV_8U);
			cv::randu(src, cv::Scalar::all(0), cv::Scalar::all(255));
			
			for(int i = 0; i < 250; i++ ) {
			
				PROF_BLOCK_START("OpenCV (float) accumulateWeighted");
				cv::accumulateWeighted(src, dst, 0.13f);
				PROF_BLOCK_END();
			}

			PROF_PRINT_REPORT();			
		}
		
		void testAccumulatedWeightedFixedToOpenCV() {
			
			PRINT_TEST_NAME();
			const float alpha = 0.12f;

			// generate destination random matrix
			cv::Mat rand8(VN_TEST_WIDTH,VN_TEST_HEIGHT,CV_8U);
			cv::randu(rand8, cv::Scalar::all(0), cv::Scalar::all(255));
			
			cv::Mat dstF32(VN_TEST_WIDTH,VN_TEST_HEIGHT,CV_32F);
			rand8.convertTo(dstF32, CV_32F);
			cv::Mat dstS32(VN_TEST_WIDTH,VN_TEST_HEIGHT,CV_32S);
			rand8.convertTo(dstS32, CV_32S);

			// create a source random matrix
			cv::randu(rand8, cv::Scalar::all(0), cv::Scalar::all(255));

			// run fixed point accumulate weighted
			LF::Vision::accumulateWeightedFixedPoint(rand8, dstS32, alpha);

			// run the opencv accumulate weighted
			cv::accumulateWeighted(rand8, dstF32, alpha);

			cv::Mat resultFixed(VN_TEST_WIDTH,VN_TEST_HEIGHT,CV_8U);
			dstS32.convertTo(resultFixed, CV_8U); 
			cv::Mat resultFloat(VN_TEST_WIDTH,VN_TEST_HEIGHT,CV_8U);
			dstF32.convertTo(resultFloat,CV_8U);


			cv::Mat diff(VN_TEST_WIDTH,VN_TEST_HEIGHT, CV_8U);
			cv::absdiff(resultFloat, resultFixed, diff);
			cv::Scalar sum = cv::sum(diff);

			printf( "\ntotal diff: %f\naverage diff: %f\n", sum[0], sum[0]/diff.size().area() );
		}
		
};

