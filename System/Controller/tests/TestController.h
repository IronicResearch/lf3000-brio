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
		if (type_ == kHWControllerDataChanged)
		{
			TS_ASSERT_EQUALS( size, sizeof(HWControllerEventMessage) );
			const HWControllerEventMessage& cmsg = reinterpret_cast<const HWControllerEventMessage&>(msg);
			const HWController* controller = cmsg.GetController();
			printf("kHWControllerDataChanged %p: %d\n", controller, controller ? controller->GetID() : 0);
			return kEventStatusOKConsumed;
		}
		if (type_ == kHWControllerModeChanged)
		{
			TS_ASSERT_EQUALS( size, sizeof(HWControllerEventMessage) );
			const HWControllerEventMessage& cmsg = reinterpret_cast<const HWControllerEventMessage&>(msg);
			const HWController* controller = cmsg.GetController();
			printf("kHWControllerModeChanged %p: %d\n", controller, controller ? controller->GetID() : 0);
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
		sleep(1);
	}
	
	//------------------------------------------------------------------------
	void testWasCreated( )
	{
		PRINT_TEST_NAME();
		
		TS_ASSERT( pControllerMPI_ != NULL );
		TS_ASSERT( pControllerMPI_->IsValid() == true );
	}
	
	//------------------------------------------------------------------------
	void testCoreMPI( )
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
	void testCaps( )
	{
		PRINT_TEST_NAME();
		
		U8 n = pControllerMPI_->GetNumberOfConnectedControllers();
		printf("%s: number of controllers = %d\n", __func__, n);

		std::vector<HWController*> controllers;
		std::vector<HWController*>::iterator it;

		pControllerMPI_->GetAllControllers(controllers);
		TS_ASSERT_EQUALS( n , controllers.size() );

		for (it = controllers.begin(); it != controllers.end(); it++) {
			HWController* controller = *(it);
			printf("%s: controller %p = %d\n", __func__, controller, controller->GetID());
		}
	}
};
