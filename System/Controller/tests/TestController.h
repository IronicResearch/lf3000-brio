// TestController.h

#include <cxxtest/TestSuite.h>
#include <boost/scoped_ptr.hpp>
#include <Hardware/HWController.h>
#include <Hardware/HWControllerMPI.h>
#include <Hardware/HWControllerEventMessage.h>
#include <EventMPI.h>
#include <EventListener.h>
#include <SystemErrors.h>
#include <SystemEvents.h>
#include <UnitTestUtils.h>

//For memcpy
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

#include "Utility.h"

using namespace LeapFrog::Brio;
using namespace LF::Hardware;

//============================================================================
// MyEventListener
//============================================================================
const tEventType kMyHandledTypes[] = { kHWAllControllerEvents };
					
//----------------------------------------------------------------------------
class MyControllerEventListener : public IEventListener
{
public:
	MyControllerEventListener( ) : IEventListener(kMyHandledTypes, ArrayCount(kMyHandledTypes)) {}
	
	virtual tEventStatus Notify( const IEventMessage &msg )
	{
		U16 size = msg.GetSizeInBytes();
		type_    = msg.GetEventType();

		// Only listening for HWControllerEventMessage types
		TS_ASSERT_EQUALS( size, sizeof(HWControllerEventMessage) );
		const HWControllerEventMessage& cmsg = reinterpret_cast<const HWControllerEventMessage&>(msg);
		const HWController* controller = cmsg.GetController();

		if (type_ == kHWControllerConnected)
		{
			printf("kHWControllerConnected %p: %d\n", controller, controller ? controller->GetID() : 0);
			return kEventStatusOKConsumed;
		}
		if (type_ == kHWControllerDisconnected)
		{
			printf("kHWControllerDisconnected %p: %d\n", controller, controller ? controller->GetID() : 0);
			return kEventStatusOKConsumed;
		}
		if (type_ == kHWControllerDataChanged)
		{
			printf("kHWControllerDataChanged %p: %d\n", controller, controller ? controller->GetID() : 0);
			return kEventStatusOKConsumed;
		}
		if (type_ == kHWControllerModeChanged)
		{
			printf("kHWControllerModeChanged %p: %d\n", controller, controller ? controller->GetID() : 0);
			return kEventStatusOKConsumed;
		}
		if (type_ == kHWControllerAnalogStickDataChanged)
		{
			printf("kHWControllerAnalogStickDataChanged %p: %d\n", controller, controller ? controller->GetID() : 0);
			return kEventStatusOKConsumed;
		}
		if (type_ == kHWControllerAccelerometerDataChanged)
		{
			printf("kHWControllerAccelerometerDataChanged %p: %d\n", controller, controller ? controller->GetID() : 0);
			return kEventStatusOKConsumed;
		}
		if (type_ == kHWControllerButtonStateChanged)
		{
			printf("kHWControllerButtonStateChanged %p: %d\n", controller, controller ? controller->GetID() : 0);
			return kEventStatusOKConsumed;
		}
		return kEventStatusOK;
	}

private:
	tEventType	type_;
};



//============================================================================
// TestController functions
//============================================================================
class TestController : public CxxTest::TestSuite, TestSuiteBase
{
private:
	HWControllerMPI*				pControllerMPI_;
	MyControllerEventListener		listener_;
public:
	//------------------------------------------------------------------------
	void setUp( )
	{
		pControllerMPI_ = new HWControllerMPI();
	}

	//------------------------------------------------------------------------
	void tearDown( )
	{
		delete pControllerMPI_;
	}
	
	//------------------------------------------------------------------------
	void XXXXtestWasCreated( )
	{
		PRINT_TEST_NAME();
		
		TS_ASSERT( pControllerMPI_ != NULL );
		TS_ASSERT( pControllerMPI_->IsValid() == true );
	}
	
	//------------------------------------------------------------------------
	void XXXXtestCoreMPI( )
	{
		PRINT_TEST_NAME();
		
		tVersion		version;
		const CString*	pName;
		const CURI*		pURI;
		
		tPlatformCaps test = kCapsAnalogController;
		TS_ASSERT(HasPlatformCapability(test));

		pName = pControllerMPI_->GetMPIName();
		TS_ASSERT_EQUALS( *pName, "HWControllerMPI" );
		version = pControllerMPI_->GetModuleVersion();
		TS_ASSERT_EQUALS( version, 3 );
		pName = pControllerMPI_->GetModuleName();
		TS_ASSERT_EQUALS( *pName, "HWController" );
		pURI = pControllerMPI_->GetModuleOrigin();
		TS_ASSERT_EQUALS( *pURI, "/LF/System/Controller" );
	}
	
	//------------------------------------------------------------------------
	void XXXXtestCaps( )
	{
		PRINT_TEST_NAME();

		sleep(1); // async events?
		
		U8 n = pControllerMPI_->GetNumberOfConnectedControllers();
		printf("%s: number of controllers = %d\n", __func__, n);

		std::vector<HWController*> controllers;
		std::vector<HWController*>::iterator it;

		pControllerMPI_->GetAllControllers(controllers);
		TS_ASSERT_EQUALS( n , controllers.size() );

		for (it = controllers.begin(); it != controllers.end(); it++) {
			HWController* controller = *(it);
			printf("%s: controller %p = %d, caps=%08x\n", __func__, controller, controller->GetID(), (unsigned int)controller->GetFunctionality());
		}

		sleep(1); // async events?
	}

	//------------------------------------------------------------------------
	void XXXXtestLEDs( )
	{
		PRINT_TEST_NAME();

		for (int i = 0; i < 5; i++) {
			sleep(1); // async events?
			if (pControllerMPI_->GetNumberOfConnectedControllers() > 1)
				break;
		}

		std::vector<HWController*> controllers;
		std::vector<HWController*>::iterator it;
		pControllerMPI_->GetAllControllers(controllers);

		for (it = controllers.begin(); it != controllers.end(); it++) {
			HWController* controller = *(it);
			U8 colors = controller->GetAvailableLEDColors();
			printf("%s: controller %p = %d, LED caps=%08x\n", __func__, controller, controller->GetID(), (unsigned int)colors);

			for (int i = 0; i < 8; i++) {
				HWControllerRGBLEDColor color = U8(1<<i);
				if (!(color & colors))
					continue;
				controller->SetLEDColor(color);
				TS_ASSERT_EQUALS( color , controller->GetLEDColor() );
				sleep(1);
				controller->SetLEDColor(kHWControllerLEDOff);
				TS_ASSERT_EQUALS( color , controller->GetLEDColor() );
				sleep(1);
			}
		}

		sleep(1); // async events?
	}

	//------------------------------------------------------------------------
	void testEvents( )
	{
		PRINT_TEST_NAME();

		pControllerMPI_->RegisterEventListener(&listener_);

		for (int i = 0; i < 5; i++) {
			sleep(1); // async events?
			if (pControllerMPI_->GetNumberOfConnectedControllers() > 1)
				break;
		}

		std::vector<HWController*> controllers;
		std::vector<HWController*>::iterator it;
		pControllerMPI_->GetAllControllers(controllers);

		for (int i = 0; i < 30; i++) {
			sleep(1);
		}

		pControllerMPI_->UnregisterEventListener(&listener_);
	}

};
