#include <cxxtest/TestSuite.h>
#include <VNYUYV2RGB.h>
#define VN_PROFILE 1
#include <VNProfiler.h>
#include <sstream>
//For strcmp
#include <string.h>
#include <UnitTestUtils.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>

class TestYUYV2RGB : public CxxTest::TestSuite, LeapFrog::Brio::TestSuiteBase {
	private:
		cv::Mat 	baseYUYVImage_;
		static int imageWidth_;
		static int imageHeight_;
		static int numIterations_;
	public:

		void loadYUYVImage( const char* path, cv::Mat& mat ) {
			std::ifstream file;
			file.open( path, std::ios::in | std::ios::binary );
			if( file.is_open() ) {
				file.seekg(0, file.end);
				std::streampos sz = file.tellg();
				file.seekg(0, file.beg);
				//std::cout << "image size: " << sz << " kb" << std::endl;
				printf( "\nimage size: %d\n", (int)sz);
				TS_ASSERT( sz == imageWidth_ * imageHeight_ * 2 );
				char* buffer = new char[sz];
				file.read(buffer,sz);
				mat.data = (unsigned char*)buffer;
				file.close();
				
				//mat has only ptr to data, not copy, so no delete [] buffer;
				TS_TRACE("Succesfully loaded test YUYV image...");
			} else {
				TS_ASSERT( !"could not open test YUYV image" );
			}
		}

		void saveImageRaw( const char* path, cv::Mat& mat ) {
			std::ofstream file;
			file.open( path, std::ios::out | std::ios::binary );
			if( file.is_open() ) {
				file.write((char*)mat.data, mat.size().area() * mat.elemSize() );
				file.close();
			} else {
				TS_ASSERT( !"could not open image for writing" );
			}
		}
		//------------------------------------------------------------------------
		void setUp(void) {
			baseYUYVImage_.create(TestYUYV2RGB::imageWidth_, TestYUYV2RGB::imageHeight_, CV_8UC2);
			loadYUYVImage( "/LF/Base/Brio/bin/data/test1-640x480-1FPS-YUY2.raw", baseYUYVImage_ ); // BUGBUG: hardwired path
		}

		//------------------------------------------------------------------------
		void tearDown(void) {
			if( baseYUYVImage_.data ) {
				delete[] baseYUYVImage_.data;
				baseYUYVImage_.data = 0;
			}
		}

		//------------------------------------------------------------------------
		void testWasCreated(void) {
			PRINT_TEST_NAME();
		}
		
		void testYUYV2RGBOpenCV(void) {
			PRINT_TEST_NAME();

			PROF_RESET();

			cv::Mat rgb(TestYUYV2RGB::imageWidth_, TestYUYV2RGB::imageHeight_, CV_8UC3);
			for(int i = 0; i < TestYUYV2RGB::numIterations_; i++ ) {
			
				PROF_BLOCK_START("OpenCV YUYV 2 RGB");
				cv::cvtColor(baseYUYVImage_, rgb, CV_YUV2RGB_YUYV);
				PROF_BLOCK_END();
			}

			PROF_PRINT_REPORT();			
		}

		void testYUYV2RGBFast(void) {
			PRINT_TEST_NAME();

			PROF_RESET();

			cv::Mat rgb(TestYUYV2RGB::imageWidth_, TestYUYV2RGB::imageHeight_, CV_8UC3);
			for(int i = 0; i < TestYUYV2RGB::numIterations_; i++ ) {
			
				PROF_BLOCK_START("Fast YUYV 2 RGB");
				LF::Vision::YUYV2RGB( baseYUYVImage_,  rgb );
				PROF_BLOCK_END();
			}

			PROF_PRINT_REPORT();			
		}


		void testCompareVersions(void) {
			PRINT_TEST_NAME();

			cv::Mat rgbOpenCV(TestYUYV2RGB::imageWidth_, TestYUYV2RGB::imageHeight_, CV_8UC3);
			cv::cvtColor(baseYUYVImage_, rgbOpenCV, CV_YUV2RGB_YUYV);

			cv::Mat rgbFast(TestYUYV2RGB::imageWidth_, TestYUYV2RGB::imageHeight_, CV_8UC3);
			LF::Vision::YUYV2RGB( baseYUYVImage_, rgbFast );

		
			cv::Mat diff(TestYUYV2RGB::imageWidth_, TestYUYV2RGB::imageHeight_, CV_8UC3);
			cv::absdiff(rgbOpenCV, rgbFast, diff);
			cv::Scalar sum = cv::sum(diff);

			saveImageRaw( "opencv.rgb", rgbOpenCV );
			saveImageRaw( "fast.rgb", rgbFast );
			saveImageRaw( "diff.rgb", diff );
			
			printf( "\naverage channel diff: (%f, %f, %f)\n", sum[0]/diff.size().area(), sum[1]/diff.size().area(), sum[2]/diff.size().area() );
		}
};

int TestYUYV2RGB::imageWidth_ = 640;
int TestYUYV2RGB::imageHeight_ = 480;
int TestYUYV2RGB::numIterations_ = 200;
