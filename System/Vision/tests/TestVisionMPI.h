#include <cxxtest/TestSuite.h>
#include <UnitTestUtils.h>
#include <UnitTestVisionUtils.h>

#include <Vision/VNAlgorithm.h>
#include <Vision/VNVirtualTouch.h>
#include <Vision/VNVisionMPI.h>
#include <Vision/VNRectHotSpot.h>


#include <DisplayMPI.h>

#include <memory>

using namespace LeapFrog::Brio;
using namespace LF::Vision;

static const float ONE_FOURTHS = 0.25f;
static const float ONE_HALF = 0.50f;

class TestVisionMPI : public CxxTest::TestSuite, TestSuiteBase, UnitTestVisionUtils {
	public:
		TestVisionMPI() {
			init();
		}

		~TestVisionMPI() {
			cleanup();
		}

		void setUp(void) {
			usleep(1000);
		}

		void testConstructorWithNoArgumentShouldSucceed() {
			PRINT_TEST_NAME();
			VNVisionMPI VisionMPI;
			
		}
		
		void testGetAlgorithmAfterConstructionShouldSucceed() {
			PRINT_TEST_NAME();
			VNVisionMPI VisionMPI;
			TS_ASSERT(VisionMPI.GetAlgorithm() == NULL);
		}

		void testGetAlgorithmAfterSetAlgorithmWithNullAlgorithmShouldSucceed() {
			PRINT_TEST_NAME();
			VNVisionMPI VisionMPI;
			VisionMPI.SetAlgorithm(NULL);
			TS_ASSERT(VisionMPI.GetAlgorithm() == NULL);
		}

		void testGetAlgorithmAfterSetAlgorithmWithValidAlgorithmShouldSucceed() {
			PRINT_TEST_NAME();
			VNVisionMPI VisionMPI;
			VisionMPI.SetAlgorithm(Algorithm);
			TS_ASSERT(VisionMPI.GetAlgorithm() == Algorithm);
		}

		void testSetAlgorithmWithNullAlgorithmShouldSucceed() {
			PRINT_TEST_NAME();
			VNVisionMPI VisionMPI;
			VisionMPI.SetAlgorithm(NULL);
			TS_ASSERT(VisionMPI.GetAlgorithm() == NULL);
		}

		void testSetAlgorithmWithValidAlgorithmShouldSucceed() {
			PRINT_TEST_NAME();
			VNVisionMPI VisionMPI;
			VisionMPI.SetAlgorithm(Algorithm);
			TS_ASSERT(VisionMPI.GetAlgorithm() == Algorithm);
		}

		void testAddHotSpotWithNullHotSpotShouldSucceed() {
			PRINT_TEST_NAME();
			TS_WARN("There is currently no way to confirm whether this is successful.\n"
				"Need to expose std::vector<const VNHotSpot*> hotSpots_ in VNVisionMPIPIMPL.");
			VNVisionMPI VisionMPI;
			VisionMPI.AddHotSpot(NULL);
		}

		void testAddHotSpotWithValidHotSpotShouldSucceed() {
			PRINT_TEST_NAME();
			TS_WARN("There is currently no way to confirm whether this is successful.\n"
				"Need to expose std::vector<const VNHotSpot*> hotSpots_ in VNVisionMPIPIMPL.");
			VNVisionMPI VisionMPI;
			std::auto_ptr<VNRectHotSpot> RectHotSpot (new VNRectHotSpot);
			TS_ASSERT(RectHotSpot.get());
			VisionMPI.AddHotSpot(dynamic_cast<VNHotSpot*>(RectHotSpot.get()));
		}

		void testRemoveHotSpotWithNullHotSpotShouldSucceed() {
			PRINT_TEST_NAME();
			TS_WARN("There is currently no way to confirm whether this is successful.\n"
				"Need to expose std::vector<const VNHotSpot*> hotSpots_ in VNVisionMPIPIMPL.");
			VNVisionMPI VisionMPI;
			std::auto_ptr<VNRectHotSpot> RectHotSpot (new VNRectHotSpot);
			TS_ASSERT(RectHotSpot.get());
			VisionMPI.RemoveHotSpot(NULL);
		}

		void testRemoveHotSpotWithHotSpotNotInTheListShouldSucceed() {
			PRINT_TEST_NAME();
			TS_WARN("There is currently no way to confirm whether this is successful.\n"
				"Need to expose std::vector<const VNHotSpot*> hotSpots_ in VNVisionMPIPIMPL.");
			VNVisionMPI VisionMPI;
			std::auto_ptr<VNRectHotSpot> RectHotSpotAdded (new VNRectHotSpot);
			TS_ASSERT(RectHotSpotAdded.get());
			VisionMPI.AddHotSpot(dynamic_cast<VNHotSpot*>(RectHotSpotAdded.get()));
			std::auto_ptr<VNRectHotSpot> RectHotSpotToBeRemoved (new VNRectHotSpot);
			TS_ASSERT(RectHotSpotToBeRemoved.get());
			VisionMPI.RemoveHotSpot(dynamic_cast<VNHotSpot*>(RectHotSpotToBeRemoved.get()));
		}

		void testRemoveHotSpotWithHotSpotInTheListShouldSucceed() {
			PRINT_TEST_NAME();
			TS_WARN("There is currently no way to confirm whether this is successful.\n"
				"Need to expose std::vector<const VNHotSpot*> hotSpots_ in VNVisionMPIPIMPL.");
			VNVisionMPI VisionMPI;
			std::auto_ptr<VNRectHotSpot> RectHotSpot (new VNRectHotSpot);
			TS_ASSERT(RectHotSpot.get());
			VisionMPI.AddHotSpot(dynamic_cast<VNHotSpot*>(RectHotSpot.get()));
			VisionMPI.RemoveHotSpot(dynamic_cast<VNHotSpot*>(RectHotSpot.get()));
		}

		void testRemoveHotSpotByIDWithNonMatchingIDShouldSucceed() {
			PRINT_TEST_NAME();
			TS_WARN("There is currently no way to confirm whether this is successful.\n"
				"Need to expose std::vector<const VNHotSpot*> hotSpots_ in VNVisionMPIPIMPL.");
			VNVisionMPI VisionMPI;
			std::auto_ptr<VNRectHotSpot> RectHotSpot (new VNRectHotSpot);
			TS_ASSERT(RectHotSpot.get());
			U32 latestID = -1;
			VisionMPI.RemoveHotSpotByID(latestID);
		}

		void testRemoveHotSpotByIDWithOneHotSpotMatchingIDShouldSucceed() {
			PRINT_TEST_NAME();
			TS_WARN("There is currently no way to confirm whether this is successful.\n"
				"Need to expose std::vector<const VNHotSpot*> hotSpots_ in VNVisionMPIPIMPL.");
			VNVisionMPI VisionMPI;
			std::auto_ptr<VNRectHotSpot> RectHotSpot (new VNRectHotSpot);
			TS_ASSERT(RectHotSpot.get());
			RectHotSpot->SetTag(ULONG_MAX);
			VisionMPI.RemoveHotSpotByID(ULONG_MAX);
		}

		void testRemoveHotSpotByIDWithNineHotSpotMatchingIDShouldSucceed() {
			PRINT_TEST_NAME();
			TS_WARN("There is currently no way to confirm whether this is successful.\n"
				"Need to expose std::vector<const VNHotSpot*> hotSpots_ in VNVisionMPIPIMPL.");
			VNVisionMPI VisionMPI;
			for (int i = 0; i < 9; i++) {
				std::auto_ptr<VNRectHotSpot> RectHotSpot (new VNRectHotSpot);
				TS_ASSERT(RectHotSpot.get());
				RectHotSpot->SetTag(ULONG_MAX);
				VisionMPI.AddHotSpot(dynamic_cast<VNHotSpot*>(RectHotSpot.get()));
			}
			VisionMPI.RemoveHotSpotByID(ULONG_MAX);
		}

		void testRemoveAllHotSpotsWithNoHotSpotInListShouldSucceed() {
			PRINT_TEST_NAME();
			TS_WARN("There is currently no way to confirm whether this is successful.\n"
				"Need to expose std::vector<const VNHotSpot*> hotSpots_ in VNVisionMPIPIMPL.");
			VNVisionMPI VisionMPI;
			VisionMPI.RemoveAllHotSpots();
		}

		void testRemoveAllHotSpotsWithOneHotSpotInListShouldSucceed() {
			PRINT_TEST_NAME();
			TS_WARN("There is currently no way to confirm whether this is successful.\n"
				"Need to expose std::vector<const VNHotSpot*> hotSpots_ in VNVisionMPIPIMPL.");
			VNVisionMPI VisionMPI;
			std::auto_ptr<VNRectHotSpot> RectHotSpot (new VNRectHotSpot);
			TS_ASSERT(RectHotSpot.get());
			VisionMPI.AddHotSpot(dynamic_cast<VNHotSpot*>(RectHotSpot.get()));
			VisionMPI.RemoveAllHotSpots();
		}

		void testRemoveAllHotSpotsWithMultipleHotSpotInListShouldSucceed() {
			PRINT_TEST_NAME();
			TS_WARN("There is currently no way to confirm whether this is successful.\n"
				"Need to expose std::vector<const VNHotSpot*> hotSpots_ in VNVisionMPIPIMPL.");
			VNVisionMPI VisionMPI;
			
			for (int i = 0; i < 9; i++) {
				std::auto_ptr<VNRectHotSpot> RectHotSpot (new VNRectHotSpot);
				TS_ASSERT(RectHotSpot.get());
				RectHotSpot->SetTag(ULONG_MAX);
				VisionMPI.AddHotSpot(dynamic_cast<VNHotSpot*>(RectHotSpot.get()));
			}
			VisionMPI.RemoveAllHotSpots();
		}

		void testStartWithNullVideoSurfFalseDispatchSynchronouslyNullRectShouldSucceed() {
			PRINT_TEST_NAME();
			VNVisionMPI VisionMPI;
			TS_ASSERT(VisionMPI.Stop());
			tErrType error = VisionMPI.Start(NULL, false, NULL);
			TS_ASSERT(error != kNoErr);
			TS_ASSERT( VisionMPI.Stop());
		}

		void testStartWithNullVideoSurfFalseDispatchSynchronouslySmallerThanVisionFrameRectShouldSucceed() {
			PRINT_TEST_NAME();
			VNVisionMPI VisionMPI;
			TS_ASSERT(VisionMPI.Stop());
			tErrType error = VisionMPI.Start(NULL, false, displayRectSmallerThanVisionFrame);
			TS_ASSERT(error != kNoErr);
			TS_ASSERT( VisionMPI.Stop());
		}

		void testStartWithNullVideoSurfFalseDispatchSynchronouslySameAsVisionFrameRectShouldSucceed() {
			PRINT_TEST_NAME();
			VNVisionMPI VisionMPI;
			TS_ASSERT(VisionMPI.Stop());
			tErrType error = VisionMPI.Start(NULL, false, displayRectSameAsVisionFrame);
			TS_ASSERT(error != kNoErr);
			TS_ASSERT( VisionMPI.Stop());
		}

		void testStartWithNullVideoSurfFalseDispatchSynchronouslyBiggerThanVisionFrameRectShouldSucceed() {
			PRINT_TEST_NAME();
			VNVisionMPI VisionMPI;
			TS_ASSERT(VisionMPI.Stop());
			tErrType error = VisionMPI.Start(NULL, false, displayRectBiggerThanVisionFrame);
			TS_ASSERT(error != kNoErr);
			TS_ASSERT( VisionMPI.Stop());
		}

		void testStartWithNullVideoSurfTrueDispatchSynchronouslyNullRectShouldSucceed() {
			PRINT_TEST_NAME();
			VNVisionMPI VisionMPI;
			TS_ASSERT(VisionMPI.Stop());
			tErrType error = VisionMPI.Start(NULL, true, NULL);
			TS_ASSERT(error != kNoErr);
			TS_ASSERT( VisionMPI.Stop());
		}

		void testStartWithNullVideoSurfTrueDispatchSynchronouslySmallerThanVisionFrameRectShouldSucceed() {
			PRINT_TEST_NAME();
			VNVisionMPI VisionMPI;
			TS_ASSERT(VisionMPI.Stop());
			tErrType error = VisionMPI.Start(NULL, true, displayRectSmallerThanVisionFrame);
			TS_ASSERT(error != kNoErr);
			TS_ASSERT( VisionMPI.Stop());
		}

		void testStartWithNullVideoSurfTrueDispatchSynchronouslySameAsVisionFrameRectShouldSucceed() {
			PRINT_TEST_NAME();
			VNVisionMPI VisionMPI;
			TS_ASSERT(VisionMPI.Stop());
			tErrType error = VisionMPI.Start(NULL, true, displayRectSameAsVisionFrame);
			TS_ASSERT(error != kNoErr);
			TS_ASSERT( VisionMPI.Stop());
		}

		void testStartWithNullVideoSurfTrueDispatchSynchronouslyBiggerThanVisionFrameRectShouldSucceed() {
			PRINT_TEST_NAME();
			VNVisionMPI VisionMPI;
			TS_ASSERT(VisionMPI.Stop());
			tErrType error = VisionMPI.Start(NULL, true, displayRectBiggerThanVisionFrame);
			TS_ASSERT(error != kNoErr);
			TS_ASSERT( VisionMPI.Stop());
		}

		void testStartWithSmallerThanVisionRectVideoSurfFalseDispatchSynchronouslyNullRectShouldSucceed() {
			PRINT_TEST_NAME();
			VNVisionMPI VisionMPI;
			TS_ASSERT(VisionMPI.Stop());
			tErrType error = VisionMPI.Start(videoSurfSmallerThanVisionFrame, false, NULL);
			TS_ASSERT(error != kNoErr);
			TS_ASSERT( VisionMPI.Stop());
		}

		void testStartWithSmallerThanVisionRectVideoSurfFalseDispatchSynchronouslySmallerThanVisionFrameRectShouldSucceed() {
			PRINT_TEST_NAME();
			VNVisionMPI VisionMPI;
			TS_ASSERT(VisionMPI.Stop());
			tErrType error = VisionMPI.Start(videoSurfSmallerThanVisionFrame, false, displayRectSmallerThanVisionFrame);
			TS_ASSERT(error != kNoErr);
			TS_ASSERT( VisionMPI.Stop());
		}

		void testStartWithSmallerThanVisionRectVideoSurfFalseDispatchSynchronouslySameAsVisionFrameRectShouldSucceed() {
			PRINT_TEST_NAME();
			VNVisionMPI VisionMPI;
			TS_ASSERT(VisionMPI.Stop());
			tErrType error = VisionMPI.Start(videoSurfSmallerThanVisionFrame, false, displayRectSameAsVisionFrame);
			TS_ASSERT(error != kNoErr);
			TS_ASSERT( VisionMPI.Stop());
		}

		void testStartWithSmallerThanVisionRectVideoSurfFalseDispatchSynchronouslyBiggerThanVisionFrameRectShouldSucceed() {
			PRINT_TEST_NAME();
			VNVisionMPI VisionMPI;
			TS_ASSERT(VisionMPI.Stop());
			tErrType error = VisionMPI.Start(videoSurfSmallerThanVisionFrame, false, displayRectBiggerThanVisionFrame);
			TS_ASSERT(error != kNoErr);
			TS_ASSERT( VisionMPI.Stop());
		}

		void testStartWithSmallerThanVisionRectVideoSurfTrueDispatchSynchronouslyNullRectShouldSucceed() {
			PRINT_TEST_NAME();
			VNVisionMPI VisionMPI;
			TS_ASSERT(VisionMPI.Stop());
			tErrType error = VisionMPI.Start(videoSurfSmallerThanVisionFrame, true, NULL);
			TS_ASSERT(error != kNoErr);
			TS_ASSERT( VisionMPI.Stop());
		}

		void testStartWithSmallerThanVisionRectVideoSurfTrueDispatchSynchronouslySmallerThanVisionFrameRectShouldSucceed() {
			PRINT_TEST_NAME();
			VNVisionMPI VisionMPI;
			TS_ASSERT(VisionMPI.Stop());
			tErrType error = VisionMPI.Start(videoSurfSmallerThanVisionFrame, true, displayRectSmallerThanVisionFrame);
			TS_ASSERT(error != kNoErr);
			TS_ASSERT( VisionMPI.Stop());
		}

		void testStartWithSmallerThanVisionRectVideoSurfTrueDispatchSynchronouslySameAsVisionFrameRectShouldSucceed() {
			PRINT_TEST_NAME();
			VNVisionMPI VisionMPI;
			TS_ASSERT(VisionMPI.Stop());
			tErrType error = VisionMPI.Start(videoSurfSmallerThanVisionFrame, true, displayRectSameAsVisionFrame);
			TS_ASSERT(error != kNoErr);
			TS_ASSERT( VisionMPI.Stop());
		}

		void testStartWithSmallerThanVisionRectVideoSurfTrueDispatchSynchronouslyBiggerThanVisionFrameRectShouldSucceed() {
			PRINT_TEST_NAME();
			VNVisionMPI VisionMPI;
			TS_ASSERT(VisionMPI.Stop());
			tErrType error = VisionMPI.Start(videoSurfSmallerThanVisionFrame, true, displayRectBiggerThanVisionFrame);
			TS_ASSERT(error != kNoErr);
			TS_ASSERT( VisionMPI.Stop());
		}

		void testStartWithBiggerThanVisionRectVideoSurfFalseDispatchSynchronouslyNullRectShouldSucceed() {
			PRINT_TEST_NAME();
			VNVisionMPI VisionMPI;
			TS_ASSERT(VisionMPI.Stop());
			tErrType error = VisionMPI.Start(videoSurfBiggerThanVisionFrame, false, NULL);
			TS_ASSERT(error == kNoErr);
			TS_ASSERT( VisionMPI.Stop());
		}

		void testStartWithBiggerThanVisionRectVideoSurfFalseDispatchSynchronouslySmallerThanVisionFrameRectShouldSucceed() {
			PRINT_TEST_NAME();
			VNVisionMPI VisionMPI;
			tErrType error = VisionMPI.Start(videoSurfBiggerThanVisionFrame, false, displayRectSmallerThanVisionFrame);
			TS_ASSERT(error == kNoErr);
			TS_ASSERT( VisionMPI.Stop());
		}

		void testStartWithBiggerThanVisionRectVideoSurfFalseDispatchSynchronouslySameAsVisionFrameRectShouldSucceed() {
			PRINT_TEST_NAME();
			VNVisionMPI VisionMPI;
			tErrType error = VisionMPI.Start(videoSurfBiggerThanVisionFrame, false, displayRectSameAsVisionFrame);
			TS_ASSERT(error == kNoErr);
			TS_ASSERT( VisionMPI.Stop());
		}

		void testStartWithBiggerThanVisionRectVideoSurfFalseDispatchSynchronouslyBiggerThanVisionFrameRectShouldSucceed() {
			PRINT_TEST_NAME();
			VNVisionMPI VisionMPI;
			tErrType error = VisionMPI.Start(videoSurfBiggerThanVisionFrame, false, displayRectBiggerThanVisionFrame);
			TS_ASSERT(error == kNoErr);
			TS_ASSERT( VisionMPI.Stop());
		}

		void testStartWithBiggerThanVisionRectVideoSurfTrueDispatchSynchronouslyNullRectShouldSucceed() {
			PRINT_TEST_NAME();
			VNVisionMPI VisionMPI;
			tErrType error = VisionMPI.Start(videoSurfBiggerThanVisionFrame, true, NULL);
			TS_ASSERT(error == kNoErr);
			TS_ASSERT( VisionMPI.Stop());
		}

		void testStartWithBiggerThanVisionRectVideoSurfTrueDispatchSynchronouslySmallerThanVisionFrameRectShouldSucceed() {
			PRINT_TEST_NAME();
			VNVisionMPI VisionMPI;
			tErrType error = VisionMPI.Start(videoSurfBiggerThanVisionFrame, true, displayRectSmallerThanVisionFrame);
			TS_ASSERT(error == kNoErr);
			TS_ASSERT( VisionMPI.Stop());
		}

		void testStartWithBiggerThanVisionRectVideoSurfTrueDispatchSynchronouslySameAsVisionFrameRectShouldSucceed() {
			PRINT_TEST_NAME();
			VNVisionMPI VisionMPI;
			tErrType error = VisionMPI.Start(videoSurfBiggerThanVisionFrame, true, displayRectSameAsVisionFrame);
			TS_ASSERT(error == kNoErr);
			TS_ASSERT( VisionMPI.Stop());
		}

		void testStartWithBiggerThanVisionRectVideoSurfTrueDispatchSynchronouslyBiggerThanVisionFrameRectShouldSucceed() {
			PRINT_TEST_NAME();
			VNVisionMPI VisionMPI;
			tErrType error = VisionMPI.Start(videoSurfBiggerThanVisionFrame, true, displayRectBiggerThanVisionFrame);
			TS_ASSERT(error == kNoErr);
			TS_ASSERT( VisionMPI.Stop());
		}

		void testStartWithSameAsVisionRectVideoSurfFalseDispatchSynchronouslyNullRectShouldSucceed() {
			PRINT_TEST_NAME();
			VNVisionMPI VisionMPI;
			tErrType error = VisionMPI.Start(videoSurfSameAsVisionFrame, false, NULL);
			TS_ASSERT(error == kNoErr);
			TS_ASSERT( VisionMPI.Stop());
		}

		void testStartWithSameAsVisionRectVideoSurfFalseDispatchSynchronouslySmallerThanVisionFrameRectShouldSucceed() {
			PRINT_TEST_NAME();
			VNVisionMPI VisionMPI;
			tErrType error = VisionMPI.Start(videoSurfSameAsVisionFrame, false, displayRectSmallerThanVisionFrame);
			TS_ASSERT(error == kNoErr);
			TS_ASSERT( VisionMPI.Stop());
		}

		void testStartWithSameAsVisionRectVideoSurfFalseDispatchSynchronouslySameAsVisionFrameRectShouldSucceed() {
			PRINT_TEST_NAME();
			VNVisionMPI VisionMPI;
			tErrType error = VisionMPI.Start(videoSurfSameAsVisionFrame, false, displayRectSameAsVisionFrame);
			TS_ASSERT(error == kNoErr);
			TS_ASSERT( VisionMPI.Stop());
		}

		void testStartWithSameAsVisionRectVideoSurfFalseDispatchSynchronouslyBiggerThanVisionFrameRectShouldSucceed() {
			PRINT_TEST_NAME();
			VNVisionMPI VisionMPI;
			tErrType error = VisionMPI.Start(videoSurfSameAsVisionFrame, false, displayRectBiggerThanVisionFrame);
			TS_ASSERT(error == kNoErr);
			TS_ASSERT( VisionMPI.Stop());
		}

		void testStartWithSameAsVisionRectVideoSurfTrueDispatchSynchronouslyNullRectShouldSucceed() {
			PRINT_TEST_NAME();
			VNVisionMPI VisionMPI;
			tErrType error = VisionMPI.Start(videoSurfSameAsVisionFrame, true, NULL);
			TS_ASSERT(error == kNoErr);
			TS_ASSERT( VisionMPI.Stop());
		}

		void testStartWithSameAsVisionRectVideoSurfTrueDispatchSynchronouslySmallerThanVisionFrameRectShouldSucceed() {
			PRINT_TEST_NAME();
			VNVisionMPI VisionMPI;
			tErrType error = VisionMPI.Start(videoSurfSameAsVisionFrame, true, displayRectSmallerThanVisionFrame);
			TS_ASSERT(error == kNoErr);
			TS_ASSERT( VisionMPI.Stop());
		}

		void testStartWithSameAsVisionRectVideoSurfTrueDispatchSynchronouslySameAsVisionFrameRectShouldSucceed() {
			PRINT_TEST_NAME();
			VNVisionMPI VisionMPI;
			tErrType error = VisionMPI.Start(videoSurfSameAsVisionFrame, true, displayRectSameAsVisionFrame);
			TS_ASSERT(error == kNoErr);
			TS_ASSERT( VisionMPI.Stop());
		}

		void testStartWithSameAsVisionRectVideoSurfTrueDispatchSynchronouslyBiggerThanVisionFrameRectShouldSucceed() {
			PRINT_TEST_NAME();
			VNVisionMPI VisionMPI;
			tErrType error = VisionMPI.Start(videoSurfSameAsVisionFrame, true, displayRectBiggerThanVisionFrame);
			TS_ASSERT(error == kNoErr);
			TS_ASSERT( VisionMPI.Stop());
		}

		void testUpdateAfterConstructionBeforeCallToStartShouldSucceed() {
			PRINT_TEST_NAME();
			VNVisionMPI VisionMPI;
			VisionMPI.Update();
		}

		void testUpdateAfterSuccessfullCallToThreadedStartShouldSucceed() {
			PRINT_TEST_NAME();
			VNVisionMPI VisionMPI;
			tErrType error = VisionMPI.Start(videoSurfSameAsVisionFrame, true, displayRectSameAsVisionFrame);
			TS_ASSERT(error == kNoErr);
			VisionMPI.Update();
			TS_ASSERT( VisionMPI.Stop());
		}

		void testUpdateAfterNonSuccessfullCallToStartShouldSucceed() {
			PRINT_TEST_NAME();
			VNVisionMPI VisionMPI;
			// Using (videoSurfSmallerThanVisionFrame, false, Null) arguments because this seems to fail to start
			tErrType error = VisionMPI.Start(videoSurfSmallerThanVisionFrame, false, NULL);
			TS_ASSERT(error != kNoErr);
			VisionMPI.Update();
		}

		void testUpdateAfterSuccessfullCallToPauseShouldSucceed() {
			PRINT_TEST_NAME();
			VNVisionMPI VisionMPI;
			tErrType error = VisionMPI.Start(videoSurfSameAsVisionFrame, true, displayRectSameAsVisionFrame);
			TS_ASSERT(error == kNoErr);
			TS_ASSERT(VisionMPI.Pause());
			VisionMPI.Update();
			TS_ASSERT( VisionMPI.Stop());
		}

		void testUpdateAfterSuccessfullCallToStopShouldSucceed() {
			PRINT_TEST_NAME();
			VNVisionMPI VisionMPI;
			tErrType error = VisionMPI.Start(videoSurfSameAsVisionFrame, true, displayRectSameAsVisionFrame);
			TS_ASSERT(error == kNoErr);
			TS_ASSERT(VisionMPI.Stop());
			VisionMPI.Update();
		}

		void testUpdateAfterSuccessfullCallToResumeShouldSucceed() {
			PRINT_TEST_NAME();
			VNVisionMPI VisionMPI;
			tErrType error = VisionMPI.Start(videoSurfSameAsVisionFrame, true, displayRectSameAsVisionFrame);
			TS_ASSERT(error == kNoErr);
			TS_ASSERT(VisionMPI.Resume());
			VisionMPI.Update();
			TS_ASSERT(VisionMPI.Stop());
		}

		void testStopAfterConstructionBeforeCallToStartShouldSucceed() {
			PRINT_TEST_NAME();
			VNVisionMPI VisionMPI;
			TS_ASSERT(!VisionMPI.Stop());
		}

		void testStopAfterSuccessfullCallToStartShouldSucceed() {
			PRINT_TEST_NAME();
			VNVisionMPI VisionMPI;
			tErrType error = VisionMPI.Start(videoSurfSameAsVisionFrame, true, displayRectSameAsVisionFrame);
			TS_ASSERT(error == kNoErr);
			TS_ASSERT(VisionMPI.Stop());
		}

		void testStopAfterNonSuccessfullCallToStartShouldSucceed() {
			PRINT_TEST_NAME();
			VNVisionMPI VisionMPI;
			// Using (videoSurfSmallerThanVisionFrame, false, Null) arguments because this seems to fail to start
			tErrType error = VisionMPI.Start(videoSurfSmallerThanVisionFrame, false, NULL);
			TS_ASSERT(error != kNoErr);
			TS_ASSERT(!VisionMPI.Stop());
		}

		void testStopAfterSuccessfullCallToResumeShouldSucceed() {
			PRINT_TEST_NAME();
			VNVisionMPI VisionMPI;
			tErrType error = VisionMPI.Start(videoSurfSameAsVisionFrame, true, displayRectSameAsVisionFrame);
			TS_ASSERT(error == kNoErr);
			TS_ASSERT(VisionMPI.Pause());
			TS_ASSERT(VisionMPI.Resume());
			TS_ASSERT(VisionMPI.Stop());
		}

		void testPauseAfterConstructionBeforeCallToStartShouldSucceed() {
			PRINT_TEST_NAME();
			VNVisionMPI VisionMPI;
			TS_ASSERT(VisionMPI.Pause());
		}

		void testPauseAfterSuccessfullCallToStartShouldSucceed() {
			PRINT_TEST_NAME();
			VNVisionMPI VisionMPI;
			tErrType error = VisionMPI.Start(videoSurfSameAsVisionFrame, true, displayRectSameAsVisionFrame);
			TS_ASSERT(error == kNoErr);
			TS_ASSERT(VisionMPI.Pause());
			TS_ASSERT(VisionMPI.Stop());
		}

		void testPauseAfterNonSuccessfullCallToStartShouldSucceed() {
			PRINT_TEST_NAME();
			VNVisionMPI VisionMPI;
			// Using (videoSurfSmallerThanVisionFrame, false, Null) arguments because this seems to fail to start
			tErrType error = VisionMPI.Start(videoSurfSmallerThanVisionFrame, false, NULL);
			TS_ASSERT(error != kNoErr);
			TS_ASSERT(VisionMPI.Pause());
		}

		void testPauseAfterSuccessfullCallToResumeShouldSucceed() {
			PRINT_TEST_NAME();
			VNVisionMPI VisionMPI;
			tErrType error = VisionMPI.Start(videoSurfSameAsVisionFrame, true, displayRectSameAsVisionFrame);
			TS_ASSERT(error == kNoErr);
			TS_ASSERT(VisionMPI.Resume());
			TS_ASSERT(VisionMPI.Pause());
			TS_ASSERT(VisionMPI.Stop());
		}

		void testPauseAfterSuccessfullCallToStopShouldSucceed() {
			PRINT_TEST_NAME();
			VNVisionMPI VisionMPI;
			tErrType error = VisionMPI.Start(videoSurfSameAsVisionFrame, true, displayRectSameAsVisionFrame);
			TS_ASSERT(error == kNoErr);
			TS_ASSERT(VisionMPI.Stop());
			TS_ASSERT(VisionMPI.Pause());
		}

		void testResumeAfterConstructionBeforeCallToStartShouldSucceed() {
			PRINT_TEST_NAME();
			VNVisionMPI VisionMPI;
			TS_ASSERT(VisionMPI.Resume());
			VisionMPI.Stop();
		}

		void testResumeAfterSuccessfullCallToStartShouldSucceed() {
			PRINT_TEST_NAME();
			VNVisionMPI VisionMPI;
			tErrType error = VisionMPI.Start(videoSurfSameAsVisionFrame, true, displayRectSameAsVisionFrame);
			TS_ASSERT(error == kNoErr);
			TS_ASSERT(VisionMPI.Resume());
			TS_ASSERT(VisionMPI.Stop());
		}

		void testResumeAfterNonSuccessfullCallToStartShouldSucceed() {
			PRINT_TEST_NAME();
			VNVisionMPI VisionMPI;
			// Using (videoSurfSmallerThanVisionFrame, false, Null) arguments because this seems to fail to start
			tErrType error = VisionMPI.Start(videoSurfSmallerThanVisionFrame, false, NULL);
			TS_ASSERT(error != kNoErr);
			TS_ASSERT(VisionMPI.Resume());
			VisionMPI.Stop();
		}

		void testResumeAfterSuccessfullCallToPauseShouldSucceed() {
			PRINT_TEST_NAME();
			VNVisionMPI VisionMPI;
			tErrType error = VisionMPI.Start(videoSurfSameAsVisionFrame, true, displayRectSameAsVisionFrame);
			TS_ASSERT(error == kNoErr);
			TS_ASSERT(VisionMPI.Pause());
			TS_ASSERT(VisionMPI.Resume());
			TS_ASSERT(VisionMPI.Stop());
		}

		void testResumeAfterSuccessfullCallToStopShouldSucceed() {
			PRINT_TEST_NAME();
			VNVisionMPI VisionMPI;
			tErrType error = VisionMPI.Start(videoSurfSameAsVisionFrame, true, displayRectSameAsVisionFrame);
			TS_ASSERT(error == kNoErr);
			TS_ASSERT(VisionMPI.Stop());
			TS_ASSERT(VisionMPI.Resume());
			VisionMPI.Stop();
		}

		void testIsRunningAfterConstructionBeforeCallToStartShouldSucceed() {
			PRINT_TEST_NAME();
			VNVisionMPI VisionMPI;
			TS_ASSERT(!VisionMPI.IsRunning());
		}

		void testIsRunningAfterSuccessfullCallToStartShouldSucceed() {
			PRINT_TEST_NAME();
			VNVisionMPI VisionMPI;
			tErrType error = VisionMPI.Start(videoSurfSameAsVisionFrame, true, displayRectSameAsVisionFrame);
			TS_ASSERT(error == kNoErr);
			TS_ASSERT(VisionMPI.IsRunning());
			TS_ASSERT(VisionMPI.Stop());
		}

		void testIsRunningAfterNonSuccessfullCallToStartShouldSucceed() {
			PRINT_TEST_NAME();
			VNVisionMPI VisionMPI;
			// Using (videoSurfSmallerThanVisionFrame, false, Null) arguments because this seems to fail to start
			tErrType error = VisionMPI.Start(videoSurfSmallerThanVisionFrame, false, NULL);
			TS_ASSERT(error != kNoErr);
			TS_ASSERT(!VisionMPI.IsRunning());
		}

		void testIsRunningAfterSuccessfullCallToPauseShouldSucceed() {
			PRINT_TEST_NAME();
			VNVisionMPI VisionMPI;
			tErrType error = VisionMPI.Start(videoSurfSameAsVisionFrame, true, displayRectSameAsVisionFrame);
			TS_ASSERT(error == kNoErr);
			TS_ASSERT(VisionMPI.Pause());
			TS_ASSERT(!VisionMPI.IsRunning());
			TS_ASSERT(VisionMPI.Stop());
		}

		void testIsRunningAfterSuccessfullCallToStopShouldSucceed() {
			PRINT_TEST_NAME();
			VNVisionMPI VisionMPI;
			tErrType error = VisionMPI.Start(videoSurfSameAsVisionFrame, true, displayRectSameAsVisionFrame);
			TS_ASSERT(error == kNoErr);
			TS_ASSERT(VisionMPI.Stop());
			TS_ASSERT(!VisionMPI.IsRunning());
		}

		void testIsRunningAfterSuccessfullCallToResumeShouldSucceed() {
			PRINT_TEST_NAME();
			VNVisionMPI VisionMPI;
			tErrType error = VisionMPI.Start(videoSurfSameAsVisionFrame, true, displayRectSameAsVisionFrame);
			TS_ASSERT(error == kNoErr);
			TS_ASSERT(VisionMPI.Resume());
			TS_ASSERT(VisionMPI.IsRunning());
			TS_ASSERT(VisionMPI.Stop());
		}

		void testGetFrameProcessingRateAfterConstructionShouldSucceed() {
			PRINT_TEST_NAME();
			VNVisionMPI VisionMPI;
			TS_ASSERT_EQUALS(0.03125f, VisionMPI.GetFrameProcessingRate());
		}

		void testGetFrameProcessingRateAfterSetFrameProcessingRateWithRateSmallerThanZeroShouldSucceed() {
			PRINT_TEST_NAME();
			VNVisionMPI VisionMPI;
			VisionMPI.SetFrameProcessingRate(-1.f);
			TS_ASSERT_EQUALS(-1.f, VisionMPI.GetFrameProcessingRate());
		}

		void testGetFrameProcessingRateAfterSetFrameProcessingRateWithZeroShouldSucceed() {
			PRINT_TEST_NAME();
			VNVisionMPI VisionMPI;
			VisionMPI.SetFrameProcessingRate(0.f);
			TS_ASSERT_EQUALS(0.f, VisionMPI.GetFrameProcessingRate());
		}

		void testGetFrameProcessingRateAfterSetFrameProcessingRateWithSmallerEqToOneAndBiggerThanZeroShouldSucceed() {
			PRINT_TEST_NAME();
			VNVisionMPI VisionMPI;
			VisionMPI.SetFrameProcessingRate(0.025f);
			TS_ASSERT_EQUALS(0.025f, VisionMPI.GetFrameProcessingRate());
		}

		void testGetFrameProcessingRateAfterSetFrameProcessingRateWithBiggerThanOnehouldSucceed() {
			PRINT_TEST_NAME();
			VNVisionMPI VisionMPI;
			VisionMPI.SetFrameProcessingRate(1.2f);
			TS_ASSERT_EQUALS(1.2f, VisionMPI.GetFrameProcessingRate());
		}

		void testSetFrameProcessingRateWithRateSmallerThanZeroShouldSucceed() {
			PRINT_TEST_NAME();
			VNVisionMPI VisionMPI;
			VisionMPI.SetFrameProcessingRate(-1.f);
			TS_ASSERT_EQUALS(-1.f, VisionMPI.GetFrameProcessingRate());
		}

		void testSetFrameProcessingRateWithZeroShouldSucceed() {
			PRINT_TEST_NAME();
			VNVisionMPI VisionMPI;
			VisionMPI.SetFrameProcessingRate(0.f);
			TS_ASSERT_EQUALS(0.f, VisionMPI.GetFrameProcessingRate());
		}

		void testSetFrameProcessingRateWithSmallerEqToOneAndBiggerThanZeroShouldSucceed() {
			PRINT_TEST_NAME();
			VNVisionMPI VisionMPI;
			VisionMPI.SetFrameProcessingRate(0.025f);
			TS_ASSERT_EQUALS(0.025f, VisionMPI.GetFrameProcessingRate());
		}

		void testSetFrameProcessingRateWithBiggerThanOnehouldSucceed() {
			PRINT_TEST_NAME();
			VNVisionMPI VisionMPI;
			VisionMPI.SetFrameProcessingRate(1.2f);
			TS_ASSERT_EQUALS(1.2f, VisionMPI.GetFrameProcessingRate());
		}

	private:
		void init() {
			Algorithm = new LF::Vision::VNVirtualTouch(0.15);
		
			int kCaptureWidth = GetScreenWidth();
			int kCaptureHeight = GetScreenHeight();
			
			LeapFrog::Brio::CDisplayMPI displayMPI;
			const LeapFrog::Brio::tDisplayScreenStats* myStats = displayMPI.GetScreenStats(0);
			int visionFrameWidth = myStats->width;
			int visionFrameHeight = myStats->height;

			float visionFrameAspectRatio = (float)visionFrameWidth/(float)visionFrameHeight;
			float captureAspectRatio = (float)kCaptureWidth/(float)kCaptureHeight;

			int targetWidth = 0;
			int targetHeight = 0;
			if (captureAspectRatio > visionFrameAspectRatio) {
				targetWidth = visionFrameHeight * captureAspectRatio;
				targetHeight = visionFrameHeight;
			} else {
				targetWidth = visionFrameWidth;
				targetHeight = visionFrameWidth / captureAspectRatio;
			}

			LeapFrog::Brio::tDisplayHandle displayHandle = displayMPI.CreateHandle(
				targetHeight, targetWidth, kPixelFormatRGB888, NULL);
				
			// Display rectangle (size = kCaptureWidth/2, kCaptureHeight/2) in center on screen
			SetRectDimension(displayRectSmallerThanVisionFrame, 
				ONE_FOURTHS * kCaptureWidth, (1 - ONE_FOURTHS) * kCaptureWidth, 
				ONE_FOURTHS * kCaptureHeight, (1 - ONE_FOURTHS) * kCaptureHeight);
			
			// Display rectangle same as vision frame (size = kCaptureWidth, kCaptureHeight)
			SetRectDimension(displayRectSameAsVisionFrame, 0, 0, kCaptureWidth, kCaptureHeight);

			// Display rectangle bigger than vision frame (size = 1.5 * kCaptureWidth, 1.5 * kCaptureHeight)
			SetRectDimension(displayRectBiggerThanVisionFrame, 0, 0, (1 + ONE_HALF) * kCaptureWidth, (1 + ONE_HALF) * kCaptureHeight);

			// VideoSurf rectangle (size = kCaptureWidth/2, kCaptureHeight/2)
			SetVideoSurfDimension(videoSurfSmallerThanVisionFrame, ONE_HALF * kCaptureWidth, ONE_HALF * kCaptureHeight, displayMPI, displayHandle);

			// VideoSurf rectangle (size = kCaptureWidth, kCaptureHeight) 
			SetVideoSurfDimension(videoSurfSameAsVisionFrame, kCaptureWidth, kCaptureHeight, displayMPI, displayHandle);

			// VideoSurf rectangle (size = 1.5 * kCaptureWidth, 1.5 * kCaptureHeight)
			SetVideoSurfDimension(videoSurfBiggerThanVisionFrame, (1 + ONE_HALF) * kCaptureWidth, (1 + ONE_HALF) * kCaptureHeight, displayMPI, displayHandle);

		}

		void cleanup() {
			delete Algorithm;
			
			delete displayRectSmallerThanVisionFrame;
			delete displayRectSameAsVisionFrame;
			delete displayRectBiggerThanVisionFrame;
			
			delete videoSurfSmallerThanVisionFrame;
			delete videoSurfSameAsVisionFrame;
			delete videoSurfBiggerThanVisionFrame;
		}

		void SetRectDimension (tRect*& Rect, int left, int top, int width, int height) {
			Rect = new tRect;
			Rect->left = left;
			Rect->right = left + width;
			Rect->top = top;
			Rect->bottom = top + height;
		}

		void SetVideoSurfDimension (tVideoSurf*& VideoSurf, int width, int height, const CDisplayMPI& displayMPI, const tDisplayHandle& displayHandle) {
			VideoSurf = new tVideoSurf;
			VideoSurf->width = width;
			VideoSurf->height = height;
			VideoSurf->pitch = displayMPI.GetPitch(displayHandle);
			VideoSurf->buffer = displayMPI.GetBuffer(displayHandle);
			VideoSurf->format = displayMPI.GetPixelFormat(displayHandle);
		}

	private:
		VNAlgorithm* Algorithm;
		tRect* displayRectSmallerThanVisionFrame;
		tRect* displayRectSameAsVisionFrame;
		tRect* displayRectBiggerThanVisionFrame;
		tVideoSurf* videoSurfSmallerThanVisionFrame;
		tVideoSurf* videoSurfSameAsVisionFrame;
		tVideoSurf* videoSurfBiggerThanVisionFrame;
};
