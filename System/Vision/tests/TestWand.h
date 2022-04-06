
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
		cv::Mat yuyvImages[6];

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
		void testWandHueAccuracy() {
			PRINT_TEST_NAME();

			// static HWControllerLEDColor kHWControllerLEDGreen    = (1 << 0);
		  // static HWControllerLEDColor kHWControllerLEDRed      = (1 << 1);
		  // static HWControllerLEDColor kHWControllerLEDBlue     = (1 << 2);
		  // static HWControllerLEDColor kHWControllerLEDYellow   = (1 << 3);
		  // static HWControllerLEDColor kHWControllerLEDCyan     = (1 << 4);
		  // static HWControllerLEDColor kHWControllerLEDMagenta  = (1 << 5);

			loadYUYVImage( "/LF/Base/Brio/bin/data/lf-wand-test-640x480-green-yuy2.raw", yuyvImages[0] );
			loadYUYVImage( "/LF/Base/Brio/bin/data/lf-wand-test-640x480-red-yuy2.raw", yuyvImages[1] );
			loadYUYVImage( "/LF/Base/Brio/bin/data/lf-wand-test-640x480-blue-yuy2.raw", yuyvImages[2] );
			loadYUYVImage( "/LF/Base/Brio/bin/data/lf-wand-test-640x480-yellow-yuy2.raw", yuyvImages[3] );
			loadYUYVImage( "/LF/Base/Brio/bin/data/lf-wand-test-640x480-cyan-yuy2.raw", yuyvImages[4] );
			loadYUYVImage( "/LF/Base/Brio/bin/data/lf-wand-test-640x480-magenta-yuy2.raw", yuyvImages[5] );

			PROF_RESET();


			cv::Mat output(cv::Size(VN_TEST_WIDTH, VN_TEST_HEIGHT), CV_8U);

			/// test green
			wand_->pimpl_->SetColor(LF::Hardware::kHWControllerLEDGreen);
			wandTrackerPIMPL_->Execute( yuyvImages[0], output);
			saveGrayImage( "/tmp/green-output.gray", output );
			if( wand_->IsVisible() ) {
				LF::Vision::VNPoint p = wand_->GetLocation();
				printf("found geen wand location: %d, %d\n", p.x, p.y);
			} else {
				printf("\aERROR could not find GREEN wand\n");
			}


			/// test red
			wand_->pimpl_->SetColor(LF::Hardware::kHWControllerLEDRed);
			wandTrackerPIMPL_->Execute( yuyvImages[1], output);
			saveGrayImage( "/tmp/red-output.gray", output );

			if( wand_->IsVisible() ) {
				LF::Vision::VNPoint p = wand_->GetLocation();
				printf("found red wand location: %d, %d\n", p.x, p.y);
			} else {
				printf("\aERROR could not find RED wand\n");
			}

			/// test blue
			wand_->pimpl_->SetColor(LF::Hardware::kHWControllerLEDBlue);
			wandTrackerPIMPL_->Execute( yuyvImages[2], output);
			saveGrayImage( "/tmp/blue-output.gray", output );
			if( wand_->IsVisible() ) {
				LF::Vision::VNPoint p = wand_->GetLocation();
				printf("found blue wand location: %d, %d\n", p.x, p.y);
			} else {
				printf("\aERROR could not find BLUE wand\n");
			}

			/// test yellow
			wand_->pimpl_->SetColor(LF::Hardware::kHWControllerLEDYellow);
			wandTrackerPIMPL_->Execute( yuyvImages[3], output);

			if( wand_->IsVisible() ) {
				LF::Vision::VNPoint p = wand_->GetLocation();
				printf("found yellow wand location: %d, %d\n", p.x, p.y);
			} else {
				printf("\aERROR could not find YELLOW wand\n");
			}

			/// test cyan
			wand_->pimpl_->SetColor(LF::Hardware::kHWControllerLEDCyan);
			wandTrackerPIMPL_->Execute( yuyvImages[4], output);

			if( wand_->IsVisible() ) {
				LF::Vision::VNPoint p = wand_->GetLocation();
				printf("found cyan wand location: %d, %d\n", p.x, p.y);
			} else {
				printf("\aERROR could not find CYAN wand\n");
			}

			/// test magenta
			wand_->pimpl_->SetColor(LF::Hardware::kHWControllerLEDMagenta);
			wandTrackerPIMPL_->Execute( yuyvImages[5], output);

			if( wand_->IsVisible() ) {
				LF::Vision::VNPoint p = wand_->GetLocation();
				printf("found magenta wand location: %d, %d\n", p.x, p.y);
			} else {
				printf("\aERROR could not find MAGENTA wand\n");
			}

		}



		//------------------------------------------------------------------------
		void testWandPerformance() {
			PRINT_TEST_NAME();

			loadYUYVImage( "/LF/Base/Brio/bin/data/lf-wand-test-640x480-green-yuy2.raw", yuyvImages[0] );
			loadYUYVImage( "/LF/Base/Brio/bin/data/lf-wand-test-640x480-green-yuy2.raw", yuyvImages[1] );
			loadYUYVImage( "/LF/Base/Brio/bin/data/lf-wand-test-640x480-green-yuy2.raw", yuyvImages[2] );


			PROF_RESET();

			cv::Mat output(cv::Size(VN_TEST_WIDTH, VN_TEST_HEIGHT), CV_8U);
			for( int i = 0; i < 250; i++ ) {
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

		void saveGrayImage( const char* path, cv::Mat& mat ) {
			std::ofstream file;
			file.open( path, std::ios::out | std::ios::binary );
			if( file.is_open() ) {
				file.write( (char*)mat.data, mat.total() );
				file.close();
			} else {
				TS_ASSERT( !"COULD NOT WRITE GRAY IMAGE");
			}
		}




};
