
#include <cxxtest/TestSuite.h>
#include <UnitTestUtils.h>

#include <EventMPI.h>
#include <USBDeviceMPI.h>
#include <KernelMPI.h>
#include <CameraMPI.h>

#include <EventTypes.h>
#include <USBDeviceTypes.h>
#include <CameraTypes.h>

using namespace LeapFrog::Brio;

const tEventType HotPlugEvents[] = {kAllUSBDeviceEvents, kAllCameraEvents};

class TestHotplug : public CxxTest::TestSuite, TestSuiteBase
{
	private:
		CUSBDeviceMPI* pUSBDeviceMPI_;
		CCameraMPI* pCameraMPI_;
		CKernelMPI* pKernelMPI_;
	
		//Create an event listener to listen for camera plug/unplug events
		class CameraListener : public IEventListener
		{
			private:
				bool hotplug_occurred;
			public:
				CameraListener(): IEventListener(HotPlugEvents, ArrayCount(HotPlugEvents))
				{ hotplug_occurred = false; }
				
				tEventStatus Notify(const IEventMessage& msg)
				{
					printf("Event Detected\n");
					//Check for USBDeviceStateChange message (For Camera Inserts)
					if(msg.GetEventType() == kUSBDeviceStateChange)
					{
						const CUSBDeviceMessage& usb_msg = dynamic_cast<const CUSBDeviceMessage&>(msg);
						if(usb_msg.GetUSBDeviceState().USBDeviceState & kUSBDeviceHotPlug)
							hotplug_occurred = true;
					}
					//Check for Camera removal event
					if(msg.GetEventType() == kCameraRemovedEvent)
					{
						hotplug_occurred = true;
					}
					
					return kEventStatusOK;
				}

				void Reset()
				{
					hotplug_occurred = false;
				}

				bool HotPlugOccurred()
				{
					return hotplug_occurred;
				}
		};
		
	public:
		//------------------------------------------------------------------------
		void setUp( )
		{
			pCameraMPI_ = new CCameraMPI;
			pUSBDeviceMPI_ = new CUSBDeviceMPI;
			pKernelMPI_ = new CKernelMPI;
		}

		//------------------------------------------------------------------------
		void tearDown( )
		{
			delete pCameraMPI_;
			delete pUSBDeviceMPI_;
			delete pKernelMPI_;
		}
		//------------------------------------------------------------------------
		void testCameraInsert()
		{
			PRINT_TEST_NAME();
			
			CameraListener listener;
			pUSBDeviceMPI_->RegisterEventListener(&listener);
			
			if(!pCameraMPI_->IsValid())
			{
				printf("Please insert camera\n");
				
				while( !listener.HotPlugOccurred() )
				{
					pKernelMPI_->TaskSleep(100);
				}
				
				TS_ASSERT(pCameraMPI_->IsValid());
			}
			else
				TS_FAIL("MPI was deemed valid (Was camera inserted?)");
		}
		//------------------------------------------------------------------------
		void testCameraRemove()
		{
			PRINT_TEST_NAME();
			
			CameraListener listener;
			
			if ( pCameraMPI_->IsValid() )
			{
				printf("Please remove camera\n");
				pCameraMPI_->StartVideoCapture(NULL, &listener);
				while( !listener.HotPlugOccurred() )
				{
					pKernelMPI_->TaskSleep(100);
				}
				printf("Camera removal detected\n");
			}
			else
				TS_FAIL("MPI was deemed invalid");
		}
};
