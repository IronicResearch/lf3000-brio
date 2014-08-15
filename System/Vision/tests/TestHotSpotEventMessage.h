#include <cxxtest/TestSuite.h>
#include <UnitTestUtils.h>

#include <Vision/VNHotSpotEventMessage.h>
#include <Vision/VNRectHotSpot.h>
#include <Vision/VNVisionMPI.h>
#include <PowerTypes.h>

#include <vector>

using namespace LeapFrog::Brio;
using namespace LF::Vision;

static const int NUM_RECHOTSPOT = 3;

class TestHotSpotEventMessage : public CxxTest::TestSuite, TestSuiteBase {	
	public:
		TestHotSpotEventMessage() {
			init();
		}

		~TestHotSpotEventMessage() {
			cleanup();
		}

		void testConstructionWithVisionEventTypeAndOneValidHotSpotShouldSucceed() {
			PRINT_TEST_NAME();
			std::auto_ptr<VNRectHotSpot> rectHotSpot (new VNRectHotSpot);
			std::auto_ptr<VNHotSpotEventMessage> hotSpotEventMessage(new VNHotSpotEventMessage(kVNHotSpotTriggeredEvent, rectHotSpot.get()));
			TS_ASSERT(hotSpotEventMessage.get());
		}

		void testConstructionWithVisionEventTypeAndOneInvalidHotSpotShouldSucceed() {
			PRINT_TEST_NAME();
			VNRectHotSpot* rectHotSpot;
			std::auto_ptr<VNHotSpotEventMessage> hotSpotEventMessage(new VNHotSpotEventMessage(kVNHotSpotTriggeredEvent, rectHotSpot));
			TS_ASSERT(hotSpotEventMessage.get());
		}

		void testConstructionWithVisionEventTypeAndOneNullHotSpotShouldSucceed() {
			PRINT_TEST_NAME();
			std::auto_ptr<VNHotSpotEventMessage> hotSpotEventMessage(new VNHotSpotEventMessage(kVNHotSpotTriggeredEvent, NULL));
			TS_ASSERT(hotSpotEventMessage.get());
		}

		void testConstructionWithNonVisionEventTypeAndOneValidHotSpotShouldSucceed() {
			PRINT_TEST_NAME();
			std::auto_ptr<VNRectHotSpot> rectHotSpot (new VNRectHotSpot);
			std::auto_ptr<VNHotSpotEventMessage> hotSpotEventMessage(new VNHotSpotEventMessage(kPowerKeepAlive, rectHotSpot.get()));
			TS_ASSERT(hotSpotEventMessage.get());
		}

		void testConstructionWithNonVisionEventTypeAndOneInvalidHotSpotShouldSucceed() {
			PRINT_TEST_NAME();
			VNRectHotSpot* rectHotSpot;
			std::auto_ptr<VNHotSpotEventMessage> hotSpotEventMessage(new VNHotSpotEventMessage(kPowerKeepAlive, rectHotSpot));
			TS_ASSERT(hotSpotEventMessage.get());
		}

		void testConstructionWithNonVisionEventTypeAndOneNullHotSpotShouldSucceed() {
			PRINT_TEST_NAME();
			std::auto_ptr<VNHotSpotEventMessage> hotSpotEventMessage(new VNHotSpotEventMessage(kPowerKeepAlive, NULL));
			TS_ASSERT(hotSpotEventMessage.get());
		}

		void testConstructionWithVisionEventTypeAndVectorOfValidHotSpotShouldSucceed() {
			PRINT_TEST_NAME();
			std::auto_ptr<VNHotSpotEventMessage> hotSpotEventMessage(new VNHotSpotEventMessage(kVNHotSpotGroupTriggeredEvent, vecValidRectHotSpot));
			TS_ASSERT(hotSpotEventMessage.get());
		}

		void testConstructionWithVisionEventTypeAndVectorOfInvalidHotSpotShouldSucceed() {
			PRINT_TEST_NAME();
			std::auto_ptr<VNHotSpotEventMessage> hotSpotEventMessage(new VNHotSpotEventMessage(kVNHotSpotGroupTriggeredEvent, vecInvalidRectHotSpot));
			TS_ASSERT(hotSpotEventMessage.get());
		}

		void testConstructionWithVisionEventTypeAndVectorOfNullHotSpotShouldSucceed() {
			PRINT_TEST_NAME();
			std::auto_ptr<VNHotSpotEventMessage> hotSpotEventMessage(new VNHotSpotEventMessage(kVNHotSpotGroupTriggeredEvent, vecNullRectHotSpot));
			TS_ASSERT(hotSpotEventMessage.get());
		}

		void testConstructionWithNonVisionEventTypeAndVectorOfValidHotSpotShouldSucceed() {
			PRINT_TEST_NAME();
			std::auto_ptr<VNHotSpotEventMessage> hotSpotEventMessage(new VNHotSpotEventMessage(kPowerKeepAlive, vecValidRectHotSpot));
			TS_ASSERT(hotSpotEventMessage.get());
		}

		void testConstructionWithNonVisionEventTypeAndVectorOfInvalidHotSpotShouldSucceed() {
			PRINT_TEST_NAME();
			std::auto_ptr<VNHotSpotEventMessage> hotSpotEventMessage(new VNHotSpotEventMessage(kPowerKeepAlive, vecInvalidRectHotSpot));
			TS_ASSERT(hotSpotEventMessage.get());
		}

		void testConstructionWithNonVisionEventTypeAndVectorOfNullHotSpotShouldSucceed() {
			PRINT_TEST_NAME();
			std::auto_ptr<VNHotSpotEventMessage> hotSpotEventMessage(new VNHotSpotEventMessage(kPowerKeepAlive, vecNullRectHotSpot));
			TS_ASSERT(hotSpotEventMessage.get());
		}

		void testGetHotSpotAfterConstructionWithVisionEventTypeAndOneValidHotSpotShouldSucceed() {
			PRINT_TEST_NAME();
			std::auto_ptr<VNRectHotSpot> rectHotSpot (new VNRectHotSpot);
			TS_ASSERT(rectHotSpot.get());
			VNHotSpotEventMessage hotSpotEventMessage(kVNHotSpotTriggeredEvent, rectHotSpot.get());
			TS_ASSERT(rectHotSpot.get() == hotSpotEventMessage.GetHotSpot());
		}

		void testGetHotSpotAfterConstructionWithVisionEventTypeAndOneInvalidHotSpotShouldSucceed() {
			PRINT_TEST_NAME();
			VNRectHotSpot* rectHotSpot;
			VNHotSpotEventMessage hotSpotEventMessage(kVNHotSpotTriggeredEvent, rectHotSpot);
			TS_ASSERT(rectHotSpot == hotSpotEventMessage.GetHotSpot());
		}

		void testGetHotSpotAfterConstructionWithVisionEventTypeAndOneNullHotSpotShouldSucceed() {
			PRINT_TEST_NAME();
			VNHotSpotEventMessage hotSpotEventMessage(kVNHotSpotTriggeredEvent, NULL);
			TS_ASSERT(NULL == hotSpotEventMessage.GetHotSpot());
		}

		void testGetHotSpotAfterConstructionWithNonVisionEventTypeAndOneValidHotSpotShouldSucceed() {
			PRINT_TEST_NAME();
			std::auto_ptr<VNRectHotSpot> rectHotSpot (new VNRectHotSpot);
			TS_ASSERT(rectHotSpot.get());
			VNHotSpotEventMessage hotSpotEventMessage(kPowerKeepAlive, rectHotSpot.get());
			TS_ASSERT(rectHotSpot.get() == hotSpotEventMessage.GetHotSpot());
		}

		void testGetHotSpotAfterConstructionWithNonVisionEventTypeAndOneInvalidHotSpotShouldSucceed() {
			PRINT_TEST_NAME();
			VNRectHotSpot* rectHotSpot;
			VNHotSpotEventMessage hotSpotEventMessage(kPowerKeepAlive, rectHotSpot);
			TS_ASSERT(rectHotSpot == hotSpotEventMessage.GetHotSpot());
		}

		void testGetHotSpotAfterConstructionWithNonVisionEventTypeAndOneNullHotSpotShouldSucceed() {
			PRINT_TEST_NAME();
			VNHotSpotEventMessage hotSpotEventMessage(kPowerKeepAlive, NULL);
			TS_ASSERT(NULL == hotSpotEventMessage.GetHotSpot());
		}

		void testGetHotSpotAfterConstructionWithVisionEventTypeAndVectorOfValidHotSpotShouldSucceed() {
			PRINT_TEST_NAME();
			VNHotSpotEventMessage hotSpotEventMessage(kVNHotSpotGroupTriggeredEvent, vecValidRectHotSpot);
			ASSERT_VECRECTHOTSPOT_EQUALS(vecValidRectHotSpot, hotSpotEventMessage.GetHotSpots());
		}

		void testGetHotSpotAfterConstructionWithVisionEventTypeAndVectorOfInvalidHotSpotShouldSucceed() {
			PRINT_TEST_NAME();
			VNHotSpotEventMessage hotSpotEventMessage(kVNHotSpotGroupTriggeredEvent, vecInvalidRectHotSpot);
			ASSERT_VECRECTHOTSPOT_EQUALS(vecInvalidRectHotSpot, hotSpotEventMessage.GetHotSpots());
		}

		void testGetHotSpotAfterConstructionWithVisionEventTypeAndVectorOfNullHotSpotShouldSucceed() {
			PRINT_TEST_NAME();
			VNHotSpotEventMessage hotSpotEventMessage(kVNHotSpotGroupTriggeredEvent, vecNullRectHotSpot);
			ASSERT_VECRECTHOTSPOT_EQUALS(vecNullRectHotSpot, hotSpotEventMessage.GetHotSpots());
		}

		void testGetHotSpotAfterConstructionWithNonVisionEventTypeAndVectorOfValidHotSpotShouldSucceed() {
			PRINT_TEST_NAME();
			VNHotSpotEventMessage hotSpotEventMessage(kPowerKeepAlive, vecValidRectHotSpot);
			ASSERT_VECRECTHOTSPOT_EQUALS(vecValidRectHotSpot, hotSpotEventMessage.GetHotSpots());
		}

		void testGetHotSpotAfterConstructionWithNonVisionEventTypeAndVectorOfInvalidHotSpotShouldSucceed() {
			PRINT_TEST_NAME();
			VNHotSpotEventMessage hotSpotEventMessage(kPowerKeepAlive, vecInvalidRectHotSpot);
			ASSERT_VECRECTHOTSPOT_EQUALS(vecInvalidRectHotSpot, hotSpotEventMessage.GetHotSpots());
		}

		void testGetHotSpotAfterConstructionWithNonVisionEventTypeAndVectorOfNullHotSpotShouldSucceed() {
			PRINT_TEST_NAME();
			VNHotSpotEventMessage hotSpotEventMessage(kPowerKeepAlive, vecNullRectHotSpot);
			ASSERT_VECRECTHOTSPOT_EQUALS(vecNullRectHotSpot, hotSpotEventMessage.GetHotSpots());
		}

		void testGetSizeInBytesAfterConstructionCreatingValidObjectShouldSucceed() {
			PRINT_TEST_NAME();
			std::auto_ptr<VNRectHotSpot> rectHotSpot (new VNRectHotSpot);
			VNHotSpotEventMessage hotSpotEventMessage(kVNHotSpotTriggeredEvent, rectHotSpot.get());
			TS_ASSERT(sizeof(VNHotSpotEventMessage) == hotSpotEventMessage.GetSizeInBytes());
		}

	private:
		void init() {
			for (int i = 0; i < NUM_RECHOTSPOT; i++) {
				const VNRectHotSpot* rectHotSpot = new VNRectHotSpot;
				vecValidRectHotSpot.push_back(rectHotSpot);
				
				const VNRectHotSpot* rectHotSpotInvalid;
				vecInvalidRectHotSpot.push_back(rectHotSpotInvalid);
				
				vecNullRectHotSpot.push_back(NULL);
			}
		}

		void cleanup() {
			for (int i = 0; i < NUM_RECHOTSPOT; i++) {
				delete vecValidRectHotSpot[i];
				delete vecInvalidRectHotSpot[i];
			}
			
			vecValidRectHotSpot.clear();
			vecInvalidRectHotSpot.clear();
			vecNullRectHotSpot.clear();
		}

		void ASSERT_VECRECTHOTSPOT_EQUALS(const std::vector<const VNHotSpot*>& vecHotSpot0, const std::vector<const VNHotSpot*>& vecHotSpot1) {
			for (int i = 0; i < NUM_RECHOTSPOT; i++) {
				TS_ASSERT(vecHotSpot0[i] == vecHotSpot1[i]);
			}
		}
		
	private:
		std::vector<const VNHotSpot*> vecValidRectHotSpot;
		std::vector<const VNHotSpot*> vecInvalidRectHotSpot;
		std::vector<const VNHotSpot*> vecNullRectHotSpot;
		
};
