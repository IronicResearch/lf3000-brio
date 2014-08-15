#include <cxxtest/TestSuite.h>
#include <UnitTestUtils.h>
#include <UnitTestVisionUtils.h>

#include <Vision/VNRectHotSpot.h>
#include <Vision/VNPointTrigger.h>
#include <Vision/VNVisionMPI.h>
#include <Vision/VNVisionTypes.h>
#include <Vision/VNHotSpotEventMessage.h>

#include <EventMPI.h>
#include <DebugMPI.h>

#include <opencv2/opencv.hpp>

static const float ONE_FOURTHS = 0.25f;
static const float ONE_HALF = 0.50f;

const tEventType gHotSpotEventTypes[] = {
		LF::Vision::kVNHotSpotTriggeredEvent,
		LF::Vision::kVNHotSpotTriggerChangeEvent
	};

class TestRectHotSpot : public CxxTest::TestSuite, TestSuiteBase, UnitTestVisionUtils {	
	public:
		TestRectHotSpot() : debugMPI(LeapFrog::Brio::kGroupVision) {
			init();
		}

		~TestRectHotSpot() {
			cleanup();
		}

		void testConstructionWithNoArgumentShouldSucceed() {
			PRINT_TEST_NAME();
			std::auto_ptr<LF::Vision::VNRectHotSpot> rectHotSpot (new LF::Vision::VNRectHotSpot());
			TS_ASSERT(rectHotSpot.get());
		}

		void testConstructionWithAreaBiggerThanZeroFullOnScreenShouldSucceed() {
			PRINT_TEST_NAME();
			std::auto_ptr<LF::Vision::VNRectHotSpot> rectHotSpot (new LF::Vision::VNRectHotSpot(hotSpotRectBiggerThanZeroFullOnScreen));
			TS_ASSERT(rectHotSpot.get());
		}

		void testConstructionWithAreaBiggerThanZeroPartOnScreenShouldSucceed() {
			PRINT_TEST_NAME();
			std::auto_ptr<LF::Vision::VNRectHotSpot> rectHotSpot (new LF::Vision::VNRectHotSpot(hotSpotRectBiggerThanZeroPartOnScreen));
			TS_ASSERT(rectHotSpot.get());
		}

		void testConstructionWithAreaBiggerThanZeroFullOffScreenShouldSucceed() {
			PRINT_TEST_NAME();
			std::auto_ptr<LF::Vision::VNRectHotSpot> rectHotSpot (new LF::Vision::VNRectHotSpot(hotSpotRectBiggerThanZeroFullOffScreen));
			TS_ASSERT(rectHotSpot.get());
		}

		void testConstructionWithAreaEqualZeroShouldSucceed() {
			PRINT_TEST_NAME();
			std::auto_ptr<LF::Vision::VNRectHotSpot> rectHotSpot (new LF::Vision::VNRectHotSpot(hotSpotRectEqualZeroOnScreen));
			TS_ASSERT(rectHotSpot.get());
		}

		void testSetRectWithAreaBiggerThanZeroFullOnScreenShouldSucceed() {
			PRINT_TEST_NAME();
			LF::Vision::VNRectHotSpot rectHotSpot;
			rectHotSpot.SetRect(hotSpotRectBiggerThanZeroFullOnScreen);
			ASSERT_RECT_EQUALS(hotSpotRectBiggerThanZeroFullOnScreen, rectHotSpot.GetRect());
		}

		void testSetRectWithAreaBiggerThanZeroPartOnScreenShouldSucceed() {
			PRINT_TEST_NAME();
			LF::Vision::VNRectHotSpot rectHotSpot;
			rectHotSpot.SetRect(hotSpotRectBiggerThanZeroPartOnScreen);
			ASSERT_RECT_EQUALS(hotSpotRectBiggerThanZeroPartOnScreen, rectHotSpot.GetRect());
		}

		void testSetRectWithAreaBiggerThanZeroFullOffScreenShouldSucceed() {
			PRINT_TEST_NAME();
			LF::Vision::VNRectHotSpot rectHotSpot;
			rectHotSpot.SetRect(hotSpotRectBiggerThanZeroFullOffScreen);
			ASSERT_RECT_EQUALS(hotSpotRectBiggerThanZeroFullOffScreen, rectHotSpot.GetRect());
		}

		void testSetRectWithAreaEqualZeroShouldSucceed() {
			PRINT_TEST_NAME();
			LF::Vision::VNRectHotSpot rectHotSpot;
			rectHotSpot.SetRect(hotSpotRectEqualZeroOffScreen);
			ASSERT_RECT_EQUALS(hotSpotRectEqualZeroOffScreen, rectHotSpot.GetRect());
		}

		void testGetRectAfterConstructionWithNoArgumentShouldSucceed() {
			PRINT_TEST_NAME();
			LF::Vision::VNRectHotSpot rectHotSpot;
			ASSERT_RECT_EQUALS(hotSpotZeroRect, rectHotSpot.GetRect());
		}

		void testGetRectAfterConstructionWithAreaBiggerThanZeroShouldSucceed() {
			PRINT_TEST_NAME();
			LF::Vision::VNRectHotSpot rectHotSpot (hotSpotRectBiggerThanZeroFullOnScreen);
			ASSERT_RECT_EQUALS(hotSpotRectBiggerThanZeroFullOnScreen, rectHotSpot.GetRect());
		}

		void testGetRectAfterConstructionWithAreaEqualZeroShouldSucceed() {
			PRINT_TEST_NAME();
			LF::Vision::VNRectHotSpot rectHotSpot (hotSpotRectEqualZeroOnScreen);
			ASSERT_RECT_EQUALS(hotSpotRectEqualZeroOnScreen, rectHotSpot.GetRect());
		}

		void testGetRectAfterSetRectWithAreaBiggerThanZeroShouldSucceed() {
			PRINT_TEST_NAME();
			LF::Vision::VNRectHotSpot rectHotSpot;
			rectHotSpot.SetRect(hotSpotRectBiggerThanZeroFullOnScreen);
			ASSERT_RECT_EQUALS(hotSpotRectBiggerThanZeroFullOnScreen, rectHotSpot.GetRect());
		}

		void testGetRectAfterSetRectWithAreaEqualZeroShouldSucceed() {
			PRINT_TEST_NAME();
			LF::Vision::VNRectHotSpot rectHotSpot;
			rectHotSpot.SetRect(hotSpotRectEqualZeroOnScreen);
			ASSERT_RECT_EQUALS(hotSpotRectEqualZeroOnScreen, rectHotSpot.GetRect());
		}

		void testTriggerWithNoTriggerSetShouldSucceed() {
			PRINT_TEST_NAME();
			LF::Vision::VNRectHotSpot rectHotSpot;

			cv::Mat input;
			rectHotSpot.Trigger(input);
			TS_ASSERT(!rectHotSpot.IsTriggered());
		}

		void testTriggerWithValidTriggerInputSize0ShouldSucceed() {
			PRINT_TEST_NAME();
			LF::Vision::VNRectHotSpot rectHotSpot;

			LF::Vision::VNPointTrigger* trigger = new LF::Vision::VNPointTrigger();
			TS_ASSERT(trigger);
			rectHotSpot.SetTrigger(trigger);

			cv::Mat input;
			rectHotSpot.Trigger(input);
			TS_ASSERT(!rectHotSpot.IsTriggered());
		}

		void testTriggerWithValidTriggerInputNotCV8UShouldSucceed() {
			PRINT_TEST_NAME();
			LF::Vision::VNRectHotSpot rectHotSpot;
			
			LF::Vision::VNPointTrigger* trigger = new LF::Vision::VNPointTrigger();
			TS_ASSERT(trigger);
			rectHotSpot.SetTrigger(trigger);

			cv::Mat input (48, 48, CV_32F);
			rectHotSpot.Trigger(input);
			TS_ASSERT(!rectHotSpot.IsTriggered());
		}

		void testIsTriggeredAfterConstructionShouldSucceed() {
			PRINT_TEST_NAME();
			LF::Vision::VNRectHotSpot rectHotSpot;
			TS_ASSERT(!rectHotSpot.IsTriggered());
		}

		void testIsTriggeredAfterCallToTriggerWithNoTriggeringEventShouldSucceed() {
			PRINT_TEST_NAME();
			LF::Vision::VNRectHotSpot rectHotSpot (hotSpotRectBiggerThanZeroFullOnScreen);

			LF::Vision::VNPointTrigger* trigger = new LF::Vision::VNPointTrigger();
			TS_ASSERT(trigger);
			rectHotSpot.SetTrigger(trigger);

			cv::Mat input (48, 48, CV_32F);
			rectHotSpot.Trigger(input);
			TS_ASSERT(!rectHotSpot.IsTriggered());
		}

		void testIsTriggeredAfterCallToTriggerWithTriggeringEventShouldSucceed() {
			PRINT_TEST_NAME();
			LF::Vision::VNRectHotSpot rectHotSpot (hotSpotTriggerTestRect);

			LF::Vision::VNPointTrigger* trigger = new LF::Vision::VNPointTrigger();
			TS_ASSERT(trigger);
			rectHotSpot.SetTrigger(trigger);

			cv::Mat input (48, 48, CV_32F);
			rectHotSpot.Trigger(input);
			TS_ASSERT(rectHotSpot.IsTriggered());
		}

		void testSetTriggerWithNullShouldSucceed() {
			PRINT_TEST_NAME();
			debugMPI.EnableThrowOnAssert();
			try {
				LF::Vision::VNRectHotSpot rectHotSpot;
				rectHotSpot.SetTrigger(NULL);
			}
			catch (LeapFrog::Brio::UnitTestAssertException e) {
			}
			debugMPI.DisableThrowOnAssert();
		}

		void testSetTriggerWithValidTriggerShouldSucceed() {
			PRINT_TEST_NAME();
			LF::Vision::VNRectHotSpot rectHotSpot;
			LF::Vision::VNPointTrigger* trigger = new LF::Vision::VNPointTrigger();
			TS_ASSERT(trigger);
			rectHotSpot.SetTrigger(trigger);
			TS_ASSERT(rectHotSpot.GetTrigger() == trigger);
		}

		void testSetTriggerWithInvalidTriggerShouldSucceed() {
			PRINT_TEST_NAME();
			debugMPI.EnableThrowOnAssert();
			try {
				LF::Vision::VNRectHotSpot rectHotSpot;
				LF::Vision::VNPointTrigger* trigger;
				rectHotSpot.SetTrigger(trigger);
			}
			catch (LeapFrog::Brio::UnitTestAssertException e) {
			}
			debugMPI.DisableThrowOnAssert();
		}

		void testGetTriggerAfterConstructionWithNoArgumentShouldSucceed() {
			PRINT_TEST_NAME();
			LF::Vision::VNRectHotSpot rectHotSpot;
			TS_ASSERT(rectHotSpot.GetTrigger() == NULL);
		}

		void testGetTriggerAfterConstructionWithRectArgumentShouldSucceed() {
			PRINT_TEST_NAME();
			LF::Vision::VNRectHotSpot rectHotSpot (hotSpotRectBiggerThanZeroFullOnScreen);
			TS_ASSERT(rectHotSpot.GetTrigger() == NULL);
		}

		void testGetTriggerAfterSetTriggerWithNullShouldSucceed() {
			PRINT_TEST_NAME();
			debugMPI.EnableThrowOnAssert();
			try {
				LF::Vision::VNRectHotSpot rectHotSpot;
				rectHotSpot.SetTrigger(NULL);
			}
			catch (LeapFrog::Brio::UnitTestAssertException e) {
			}
			debugMPI.DisableThrowOnAssert();
		}

		void testGetTriggerAfterSetTriggerWithValidTriggerShouldSucceed() {
			PRINT_TEST_NAME();
			LF::Vision::VNRectHotSpot rectHotSpot;
			LF::Vision::VNPointTrigger* trigger = new LF::Vision::VNPointTrigger();
			TS_ASSERT(trigger);
			rectHotSpot.SetTrigger(trigger);
			TS_ASSERT(rectHotSpot.GetTrigger() == trigger);
		}

		void testGetTriggerAfterSetTriggerWithInvalidTriggerShouldSucceed() {
			PRINT_TEST_NAME();
			debugMPI.EnableThrowOnAssert();
			try {
				LF::Vision::VNRectHotSpot rectHotSpot;
				LF::Vision::VNPointTrigger* trigger;
				rectHotSpot.SetTrigger(trigger);
			}
			catch (LeapFrog::Brio::UnitTestAssertException e) {
			}
			debugMPI.DisableThrowOnAssert();
		}

		void testSetTagWith0ShouldSucceed() {
			PRINT_TEST_NAME();
			LF::Vision::VNRectHotSpot rectHotSpot;
			rectHotSpot.SetTag(0);
			TS_ASSERT_EQUALS(0, rectHotSpot.GetTag());
		}

		void testSetTagWithMinus1ShouldSucceed() {
			PRINT_TEST_NAME();
			LF::Vision::VNRectHotSpot rectHotSpot;
			rectHotSpot.SetTag(-1);
			TS_ASSERT_EQUALS(ULONG_MAX, rectHotSpot.GetTag());
		}

		void testSetTagWith123ShouldSucceed() {
			PRINT_TEST_NAME();
			LF::Vision::VNRectHotSpot rectHotSpot;
			rectHotSpot.SetTag(123);
			TS_ASSERT_EQUALS(123, rectHotSpot.GetTag());
		}

		void testSetTagWithULONGMAXShouldSucceed() {
			PRINT_TEST_NAME();
			LF::Vision::VNRectHotSpot rectHotSpot;
			rectHotSpot.SetTag(ULONG_MAX);
			TS_ASSERT_EQUALS(ULONG_MAX, rectHotSpot.GetTag());
		}

		void testSetTagWithULONGMAXPlus1ShouldSucceed() {
			PRINT_TEST_NAME();
			LF::Vision::VNRectHotSpot rectHotSpot;
			rectHotSpot.SetTag(ULONG_MAX + 1);
			TS_ASSERT_EQUALS(0, rectHotSpot.GetTag());
		}

		void testGetTagAfter1stHotSpotShouldSucceed() {
			PRINT_TEST_NAME();
			LF::Vision::VNRectHotSpot rectHotSpot;
			TS_ASSERT_EQUALS(0, rectHotSpot.GetTag());
		}

		void testGetTagAfter5thHotSpotShouldSucceed() {
			PRINT_TEST_NAME();
			LF::Vision::VNRectHotSpot rectHotSpot0;
			LF::Vision::VNRectHotSpot rectHotSpot1;
			LF::Vision::VNRectHotSpot rectHotSpot2;
			LF::Vision::VNRectHotSpot rectHotSpot3;
			LF::Vision::VNRectHotSpot rectHotSpot4;
			TS_ASSERT_EQUALS(4, rectHotSpot4.GetTag());
		}

		void testGetTagAfter10thHotSpotShouldSucceed() {
			PRINT_TEST_NAME();
			LF::Vision::VNRectHotSpot rectHotSpot0;
			LF::Vision::VNRectHotSpot rectHotSpot1;
			LF::Vision::VNRectHotSpot rectHotSpot2;
			LF::Vision::VNRectHotSpot rectHotSpot3;
			LF::Vision::VNRectHotSpot rectHotSpot4;
			LF::Vision::VNRectHotSpot rectHotSpot5;
			LF::Vision::VNRectHotSpot rectHotSpot6;
			LF::Vision::VNRectHotSpot rectHotSpot7;
			LF::Vision::VNRectHotSpot rectHotSpot8;
			LF::Vision::VNRectHotSpot rectHotSpot9;
			TS_ASSERT_EQUALS(9, rectHotSpot9.GetTag());
		}

		void testGetTagAfterSetTagTo0ShouldSucceed() {
			PRINT_TEST_NAME();
			LF::Vision::VNRectHotSpot rectHotSpot;
			rectHotSpot.SetTag(0);
			TS_ASSERT_EQUALS(0, rectHotSpot.GetTag());
		}

		void testGetTagAfterSetTagToMinus1ShouldSucceed() {
			PRINT_TEST_NAME();
			LF::Vision::VNRectHotSpot rectHotSpot;
			rectHotSpot.SetTag(-1);
			TS_ASSERT_EQUALS(ULONG_MAX, rectHotSpot.GetTag());
		}

		void testGetTagAfterSetTagTo123ShouldSucceed() {
			PRINT_TEST_NAME();
			LF::Vision::VNRectHotSpot rectHotSpot;
			rectHotSpot.SetTag(123);
			TS_ASSERT_EQUALS(123, rectHotSpot.GetTag());
		}

		void testGetTagAfterSetTagToULONGMAXShouldSucceed() {
			PRINT_TEST_NAME();
			LF::Vision::VNRectHotSpot rectHotSpot;
			rectHotSpot.SetTag(ULONG_MAX);
			TS_ASSERT_EQUALS(ULONG_MAX, rectHotSpot.GetTag());
		}

		void testGetTagAfterSetTagToULONGMAXPlus1ShouldSucceed() {
			PRINT_TEST_NAME();
			LF::Vision::VNRectHotSpot rectHotSpot;
			rectHotSpot.SetTag(ULONG_MAX + 1);
			TS_ASSERT_EQUALS(0, rectHotSpot.GetTag());
		}

	private:
		void init() {
			LeapFrog::Brio::CEventMPI eventMPI;
			eventMPI.RegisterEventListener(&hotspotListener);

			int kCaptureWidth = GetScreenWidth();
			int kCaptureHeight = GetScreenHeight();

			// Rectangle HotSpot (size = kCaptureWidth/2, kCaptureHeight/2) in center on screen
			hotSpotRectBiggerThanZeroFullOnScreen.left = ONE_FOURTHS * kCaptureWidth;
			hotSpotRectBiggerThanZeroFullOnScreen.right = (1 - ONE_FOURTHS) * kCaptureWidth;
			hotSpotRectBiggerThanZeroFullOnScreen.top = ONE_FOURTHS * kCaptureHeight;
			hotSpotRectBiggerThanZeroFullOnScreen.bottom = (1 - ONE_FOURTHS) * kCaptureHeight;

			// Rectangle HotSpot (size = kCaptureWidth/2, kCaptureHeight/2) in partial top right on screen
			hotSpotRectBiggerThanZeroPartOnScreen.left = (1 - ONE_FOURTHS) * kCaptureWidth;
			hotSpotRectBiggerThanZeroPartOnScreen.right = (1 + ONE_FOURTHS) * kCaptureWidth;
			hotSpotRectBiggerThanZeroPartOnScreen.top = -ONE_FOURTHS * kCaptureHeight;
			hotSpotRectBiggerThanZeroPartOnScreen.bottom = ONE_FOURTHS * kCaptureHeight;

			// Rectangle HotSpot (size = kCaptureWidth/2, kCaptureHeight/2) in center left off screen
			hotSpotRectBiggerThanZeroFullOffScreen.left = (ONE_FOURTHS - 1) * kCaptureWidth;
			hotSpotRectBiggerThanZeroFullOffScreen.right = -ONE_FOURTHS * kCaptureWidth;
			hotSpotRectBiggerThanZeroFullOffScreen.top = ONE_FOURTHS * kCaptureHeight;
			hotSpotRectBiggerThanZeroFullOffScreen.bottom = (1 - ONE_FOURTHS) * kCaptureHeight;

			// Rectangle HotSpot (size = 0, 0) in center on screen
			hotSpotRectEqualZeroOnScreen.left = ONE_HALF * kCaptureWidth;
			hotSpotRectEqualZeroOnScreen.right = ONE_HALF * kCaptureWidth;
			hotSpotRectEqualZeroOnScreen.top = ONE_HALF * kCaptureHeight;
			hotSpotRectEqualZeroOnScreen.bottom = ONE_HALF * kCaptureHeight;

			// Rectangle HotSpot (size = 0, 0) in center right off screen
			hotSpotRectEqualZeroOffScreen.left = (1 + ONE_FOURTHS) * kCaptureWidth;
			hotSpotRectEqualZeroOffScreen.right = (1 + ONE_FOURTHS) * kCaptureWidth;
			hotSpotRectEqualZeroOffScreen.top = ONE_HALF * kCaptureHeight;
			hotSpotRectEqualZeroOffScreen.bottom = ONE_HALF * kCaptureHeight;

			// Rectangle HotSpot (size = 4000,4000; centered at -10000,-10000) in top left corner
			hotSpotTriggerTestRect.left = -12000;
			hotSpotTriggerTestRect.right = -8000;
			hotSpotTriggerTestRect.top = -12000;
			hotSpotTriggerTestRect.bottom = -8000;

			// Rectangle HotSpot (size = 0, 0) in top left corner
			hotSpotZeroRect.left = 0;
			hotSpotZeroRect.right = 0;
			hotSpotZeroRect.top = 0;
			hotSpotZeroRect.bottom = 0;
		}

		void cleanup() {
			LeapFrog::Brio::CEventMPI eventMPI;
			eventMPI.UnregisterEventListener(&hotspotListener);
		}

		void ASSERT_RECT_EQUALS(LeapFrog::Brio::tRect rect0, LeapFrog::Brio::tRect rect1) {
			TS_ASSERT_EQUALS(rect0.left, rect1.left);
			TS_ASSERT_EQUALS(rect0.right, rect1.right);
			TS_ASSERT_EQUALS(rect0.top, rect1.top);
			TS_ASSERT_EQUALS(rect0.bottom, rect1.bottom);
		}
		
		class HotSpotListener : public IEventListener {
			public:
				HotSpotListener() : IEventListener(gHotSpotEventTypes, ArrayCount(gHotSpotEventTypes)) {
				}
				
				tEventStatus Notify(const IEventMessage& msg) {
					const LF::Vision::VNHotSpotEventMessage& hsMessage = dynamic_cast<const LF::Vision::VNHotSpotEventMessage&>(msg);
					const LF::Vision::VNHotSpot* hs = hsMessage.GetHotSpot();

					if (hs->IsTriggered()) {
						tEventType type = msg.GetEventType();
						if (type == LF::Vision::kVNHotSpotTriggeredEvent)
							TS_TRACE("Event is triggered -> LF::Vision::kVNHotSpotTriggeredEvent");
						else if (type == LF::Vision::kVNHotSpotTriggerChangeEvent)
							TS_TRACE("Event is triggered -> LF::Vision::kVNHotSpotTriggerChangeEvent");
					}
					return kEventStatusOK;
				}
		};		
		HotSpotListener hotspotListener;

private:
		LeapFrog::Brio::tRect hotSpotRectBiggerThanZeroFullOnScreen;
		LeapFrog::Brio::tRect hotSpotRectBiggerThanZeroPartOnScreen;
		LeapFrog::Brio::tRect hotSpotRectBiggerThanZeroFullOffScreen;
		LeapFrog::Brio::tRect hotSpotRectEqualZeroOnScreen;
		LeapFrog::Brio::tRect hotSpotRectEqualZeroOffScreen;
		LeapFrog::Brio::tRect hotSpotTriggerTestRect;
		LeapFrog::Brio::tRect hotSpotZeroRect;
		
		LeapFrog::Brio::CDebugMPI debugMPI;
};
