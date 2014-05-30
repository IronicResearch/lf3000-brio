
#include <cxxtest/TestSuite.h>
#include <Vision/VNWand.h>
#include <VNWandPIMPL.h>
#include <Hardware/HWControllerTypes.h>
#include <VNWandTrackerPIMPL.h>

#define VN_PROFILE 1
#include <VNProfiler.h>
#include <sstream>
//For strcmp
#include <string.h>
#include <UnitTestUtils.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>

#define VN_TEST_WIDTH	640
#define VN_TEST_HEIGHT	480

class TestWand : public CxxTest::TestSuite, LeapFrog::Brio::TestSuiteBase {
	private:
		LF::Vision::VNWandTrackerPIMPL* wandTrackerPIMPL_;
		LF::Vision::VNWand* wand_;
		LF::Vision::VNInputParameters inputParameters_;
		cv::Mat yuyvImages[3];
	
	friend LF::Vision::VNWand::VNWand();
	
	public:
	
	

		//------------------------------------------------------------------------
		void setUp(void) {
			wand_ = new LF::Vision::VNWand(); // TODO: setup wand
			wand_->pimpl_->SetColor(LF::Hardware::kHWControllerLEDGreen);
			wandTrackerPIMPL_ = new LF::Vision::VNWandTrackerPIMPL(wand_, &inputParameters_);

		}

		//------------------------------------------------------------------------
		void tearDown(void) {
			delete wandTrackerPIMPL_;
		}

		//------------------------------------------------------------------------
		void testWandPerformance() {
			PRINT_TEST_NAME();
			
			loadYUYVImage( "/LF/Base/Brio/bin/data/wand1-640x480.yuyv", yuyvImages[0] );
			loadYUYVImage( "/LF/Base/Brio/bin/data/wand2-640x480.yuyv", yuyvImages[1] );
			loadYUYVImage( "/LF/Base/Brio/bin/data/wand3-640x480.yuyv", yuyvImages[2] );


			PROF_RESET();

			cv::Mat input(VN_TEST_WIDTH,VN_TEST_HEIGHT,CV_8UC3);
			cv::Mat output(VN_TEST_WIDTH, VN_TEST_HEIGHT, CV_8U);
			for( int i = 0; i < 250; i++ ) {
				cv::randu(input, cv::Scalar::all(0), cv::Scalar::all(255));
				PROF_BLOCK_START("Wand Execute");
				wandTrackerPIMPL_->Execute( yuyvImages[rand() % 2], output);
				PROF_BLOCK_END();
			}

			PROF_PRINT_REPORT();
 
		}
	
		void loadYUYVImage( const char* path, cv::Mat& mat ) {
			mat.create( cv::Size(VN_TEST_WIDTH, VN_TEST_HEIGHT), CV_8UC2);
			std::ifstream file;
			file.open( path, std::ios::in | std::ios::binary );
			if( file.is_open() ) {
				file.seekg(0, file.end);
				std::streampos sz = file.tellg();
				file.seekg(0, file.beg);
				//std::cout << "image size: " << sz << " kb" << std::endl;
				printf( "\nimage size: %d\n", (int)sz);
				TS_ASSERT( sz == VN_TEST_WIDTH * VN_TEST_HEIGHT * 2 );
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



		
};
