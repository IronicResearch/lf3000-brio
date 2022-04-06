
// Unit tests for VNWandTracker
// Reference: Test plan:
// http://wiki.leapfrog.com/display/CTD/Testing+Plans
// VisionTestingPlanv2.xlsx

#include <cxxtest/TestSuite.h>
#include <UnitTestUtils.h>

#include <Vision/VNWandTracker.h>

using namespace LeapFrog::Brio;
using namespace LF::Vision;

static const int VN_TEST_WIDTH = 640;
static const int VN_TEST_HEIGHT = 480;

class TestWandTracker : public CxxTest::TestSuite,
						LeapFrog::Brio::TestSuiteBase {
		
	public:
		
		TestWandTracker() {
		}
		
		void testConstructorDefaultShouldSucceed() {
			PRINT_TEST_NAME();
			std::auto_ptr<VNWandTracker> tracker(new VNWandTracker);
			TS_ASSERT( tracker.get() );
		}
		
		void testConstructorSettingSimpleParamsShouldSucceed() {
			PRINT_TEST_NAME();
			VNInputParameters params;
			params.push_back(std::pair<LeapFrog::Brio::CString, float>("VNWTAreaToStartScaling", 1100.f));
			params.push_back(std::pair<LeapFrog::Brio::CString, float>("VNWTMinPercentToScale", 0.25f));
			params.push_back(std::pair<LeapFrog::Brio::CString, float>("VNWTMinWandArea", 45.f));
			
			std::auto_ptr<VNWandTracker> tracker(new VNWandTracker(&params));
			TS_ASSERT( tracker.get() );
		}
		
		void testConstructorSettingMoreSimpleParamsShouldSucceed() {
			PRINT_TEST_NAME();
			VNInputParameters params;
			params.push_back(std::pair<LeapFrog::Brio::CString, float>("VNWTUseWandScaling", 1100.f));
			params.push_back(std::pair<LeapFrog::Brio::CString, float>("VNWTNumFramesCacheLoc", 0.25f));
			params.push_back(std::pair<LeapFrog::Brio::CString, float>("VNWTSmoothingAlpha", 45.f));
			
			std::auto_ptr<VNWandTracker> tracker(new VNWandTracker(&params));
			TS_ASSERT( tracker.get() );
		}
		
		void testConstructorSettingRandomParamsShouldSucceed() {
			PRINT_TEST_NAME();
			VNInputParameters params;
			params.push_back(std::pair<LeapFrog::Brio::CString, float>("MyCatIsOnFire", 3.1415f));
			params.push_back(std::pair<LeapFrog::Brio::CString, float>("jeairjoufaknvklajfjds", 564510.654f));
			params.push_back(std::pair<LeapFrog::Brio::CString, float>("WowAnotherRandomString", 6465.3f));
			
			std::auto_ptr<VNWandTracker> tracker(new VNWandTracker(&params));
			TS_ASSERT( tracker.get() );
		}
		
		void testConstructorSettingNegScalingParamsShouldSucceed() {
			PRINT_TEST_NAME();
			VNInputParameters params;
			params.push_back(std::pair<LeapFrog::Brio::CString, float>("VNWTAreaToStartScaling", -100.f));
			params.push_back(std::pair<LeapFrog::Brio::CString, float>("VNWTMinPercentToScale", 0.25f));
			params.push_back(std::pair<LeapFrog::Brio::CString, float>("VNWTMinWandArea", 45.f));
			
			std::auto_ptr<VNWandTracker> tracker(new VNWandTracker(&params));
			TS_ASSERT( tracker.get() );
		}
		
		void testConstructorSettingNegUseWandScalingParamsShouldSucceed() {
			PRINT_TEST_NAME();
			VNInputParameters params;
			params.push_back(std::pair<LeapFrog::Brio::CString, float>("VNWTUseWandScaling", -100.f));
			params.push_back(std::pair<LeapFrog::Brio::CString, float>("VNWTNumFramesCacheLoc", 0.25f));
			params.push_back(std::pair<LeapFrog::Brio::CString, float>("VNWTSmoothingAlpha", 45.f));
			
			std::auto_ptr<VNWandTracker> tracker(new VNWandTracker(&params));
			TS_ASSERT( tracker.get() );
		}

		void testConstructorSettingNegMinPercentParamsShouldSucceed() {
			PRINT_TEST_NAME();
			VNInputParameters params;
			params.push_back(std::pair<LeapFrog::Brio::CString, float>("VNWTAreaToStartScaling", 1200.f));
			params.push_back(std::pair<LeapFrog::Brio::CString, float>("VNWTMinPercentToScale", -0.25f));
			params.push_back(std::pair<LeapFrog::Brio::CString, float>("VNWTMinWandArea", 45.f));
			
			std::auto_ptr<VNWandTracker> tracker(new VNWandTracker(&params));
			TS_ASSERT( tracker.get() );
		}
		
		void testConstructorSettingNegNumFramesParamsShouldSucceed() {
			PRINT_TEST_NAME();
			VNInputParameters params;
			params.push_back(std::pair<LeapFrog::Brio::CString, float>("VNWTUseWandScaling", 1100.f));
			params.push_back(std::pair<LeapFrog::Brio::CString, float>("VNWTNumFramesCacheLoc", -0.25f));
			params.push_back(std::pair<LeapFrog::Brio::CString, float>("VNWTSmoothingAlpha", 45.f));
			
			std::auto_ptr<VNWandTracker> tracker(new VNWandTracker(&params));
			TS_ASSERT( tracker.get() );
		}
		
		void testConstructorSettingNegAreaParamsShouldSucceed() {
			PRINT_TEST_NAME();
			VNInputParameters params;
			params.push_back(std::pair<LeapFrog::Brio::CString, float>("VNWTAreaToStartScaling", 1200.f));
			params.push_back(std::pair<LeapFrog::Brio::CString, float>("VNWTMinPercentToScale", 0.25f));
			params.push_back(std::pair<LeapFrog::Brio::CString, float>("VNWTMinWandArea", -45.f));
			
			std::auto_ptr<VNWandTracker> tracker(new VNWandTracker(&params));
			TS_ASSERT( tracker.get() );
		}
		
		void testConstructorSettingNegSmoothingAlphaParamsShouldSucceed() {
			PRINT_TEST_NAME();
			VNInputParameters params;
			params.push_back(std::pair<LeapFrog::Brio::CString, float>("VNWTUseWandScaling", 1100.f));
			params.push_back(std::pair<LeapFrog::Brio::CString, float>("VNWTNumFramesCacheLoc", 0.25f));
			params.push_back(std::pair<LeapFrog::Brio::CString, float>("VNWTSmoothingAlpha", -45.f));
			
			std::auto_ptr<VNWandTracker> tracker(new VNWandTracker(&params));
			TS_ASSERT( tracker.get() );
		}
		
		void testInitializeZeroZeroShouldSucceed() {
			PRINT_TEST_NAME();
			std::auto_ptr<VNWandTracker> tracker(new VNWandTracker);
			TS_ASSERT( tracker.get() );
			
			tracker->Initialize(0, 0);
		}

		void testInitializePosPosShouldSucceed() {
			PRINT_TEST_NAME();
			std::auto_ptr<VNWandTracker> tracker(new VNWandTracker);
			TS_ASSERT( tracker.get() );
			
			tracker->Initialize(100, 100);
		}
		
		void testSetAutomaticWandScalingVoidShouldSucceed() {
			PRINT_TEST_NAME();
			std::auto_ptr<VNWandTracker> tracker(new VNWandTracker);
						
			tracker->SetAutomaticWandScaling();
			
			TS_ASSERT(!(tracker->GetAutomaticWandScaling()));
		}

		void testSetAutomaticWandScalingTrueShouldSucceed() {
			PRINT_TEST_NAME();
			std::auto_ptr<VNWandTracker> tracker(new VNWandTracker);
						
			tracker->SetAutomaticWandScaling(true);
			
			TS_ASSERT(tracker->GetAutomaticWandScaling());
		}


		void testSetAutomaticWandScalingFalseShouldSucceed() {
			PRINT_TEST_NAME();
			std::auto_ptr<VNWandTracker> tracker(new VNWandTracker);
						
			tracker->SetAutomaticWandScaling(false);
			
			TS_ASSERT(!(tracker->GetAutomaticWandScaling()));
		}
		
		void testGetAutomaticWandScalingNullConstructionShouldSucceed() {
			PRINT_TEST_NAME();
			std::auto_ptr<VNWandTracker> tracker(new VNWandTracker);
						
			TS_ASSERT(!(tracker->GetAutomaticWandScaling()));
		}
		
		
		void testGetAutomaticWandScalingParamConstructionShouldSucceed() {
			PRINT_TEST_NAME();
									
			VNInputParameters params;
			params.push_back(std::pair<LeapFrog::Brio::CString, float>("VNWTAreaToStartScaling", 1100.f));
			params.push_back(std::pair<LeapFrog::Brio::CString, float>("VNWTMinPercentToScale", 0.25f));
			params.push_back(std::pair<LeapFrog::Brio::CString, float>("VNWTMinWandArea", 45.f));
			
			std::auto_ptr<VNWandTracker> tracker(new VNWandTracker(&params));
			
			TS_ASSERT(!(tracker->GetAutomaticWandScaling()));
		}
		
		void testGetAutomaticWandScalingAfterSetVoidShouldSucceed() {
			PRINT_TEST_NAME();
			std::auto_ptr<VNWandTracker> tracker(new VNWandTracker);
						
			tracker->SetAutomaticWandScaling();
			
			TS_ASSERT(!(tracker->GetAutomaticWandScaling()));
		}

		void testGetAutomaticWandScalingAfterSetFalseShouldSucceed() {
			PRINT_TEST_NAME();
			std::auto_ptr<VNWandTracker> tracker(new VNWandTracker);
						
			tracker->SetAutomaticWandScaling(false);
			
			TS_ASSERT(!(tracker->GetAutomaticWandScaling()));
		}
		
		void testGetAutomaticWandScalingAfterSetTrueShouldSucceed() {
			PRINT_TEST_NAME();
			std::auto_ptr<VNWandTracker> tracker(new VNWandTracker);
						
			tracker->SetAutomaticWandScaling(true);
			
			TS_ASSERT(tracker->GetAutomaticWandScaling());
		}
		
		
		void testExecuteBlankInputShouldSucceed() {
			PRINT_TEST_NAME();
			std::auto_ptr<VNWandTracker> tracker(new VNWandTracker);
			TS_ASSERT( tracker.get() );
			
			cv::Mat inputImage( VN_TEST_WIDTH, VN_TEST_HEIGHT,  CV_8UC2, cvScalar(0.0, 0.0, 0.0 ,0.0));
			cv::Mat outputImage(cv::Size(VN_TEST_WIDTH, VN_TEST_HEIGHT), CV_8U);
			
			tracker->Execute(inputImage, outputImage);
			
			TS_ASSERT(isImageBlank(outputImage));
		}

private:				
		bool isImageBlank(cv::Mat& mat) {			
			for(int y = 0; y < mat.rows; ++y)
			{
				unsigned char *scanner = mat.data + (y * mat.step);
				for(int x = 0; x < mat.cols; ++x, ++scanner)
				{
					if(*scanner) return false;
				}
			}
			
			return true;
		}
};
