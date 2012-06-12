// TestPlatform.h

#include <cxxtest/TestSuite.h>
#include <SystemErrors.h>
#include <StringTypes.h>
#include <ButtonTypes.h>
#include <UnitTestUtils.h>
#include <Utility.h>

LF_USING_BRIO_NAMESPACE()

//============================================================================
// TestPlatform functions
//============================================================================
class TestPlatform : public CxxTest::TestSuite, TestSuiteBase
{
private:

public:
	//------------------------------------------------------------------------
	void setUp( )
	{
	}

	//------------------------------------------------------------------------
	void tearDown( )
	{
	}

	//------------------------------------------------------------------------
	void testPlatform( )
	{
		PRINT_TEST_NAME();

		printf("GetPlatformID:     0x%x\n", (unsigned int)GetPlatformID());
		printf("GetPlatformName:   %s\n", GetPlatformName().c_str());
		printf("GetPlatformFamily: %s\n", GetPlatformFamily().c_str());
		printf("\n");
		printf("HasPlatformCapability:\n");
		printf("kCapsTouchscreen:  %d\n", HasPlatformCapability(kCapsTouchscreen));
		printf("kCapsCamera:       %d\n", HasPlatformCapability(kCapsCamera));
		printf("kCapsAccelerometer:%d\n", HasPlatformCapability(kCapsAccelerometer));
		printf("kCapsMicrophone:   %d\n", HasPlatformCapability(kCapsMicrophone));
		printf("kCapsScreenLEX:    %d\n", HasPlatformCapability(kCapsScreenLEX));
		printf("kCapsScreenLPAD:   %d\n", HasPlatformCapability(kCapsScreenLPAD));
		printf("kCapsLF1000:       %d\n", HasPlatformCapability(kCapsLF1000));
		printf("kCapsLF2000:       %d\n", HasPlatformCapability(kCapsLF2000));
		printf("kCapsWifi:         %d\n", HasPlatformCapability(kCapsWifi));
		printf("kCapsCameraFront:  %d\n", HasPlatformCapability(kCapsCameraFront));
		printf("\n");
		printf("kCapsButtonMask:\n");
		printf("kButtonUp:         %d\n", HasPlatformCapability(kCapsButtonMask(kButtonUp)));
		printf("kButtonDown:       %d\n", HasPlatformCapability(kCapsButtonMask(kButtonDown)));
		printf("kButtonRight:      %d\n", HasPlatformCapability(kCapsButtonMask(kButtonRight)));
		printf("kButtonLeft:       %d\n", HasPlatformCapability(kCapsButtonMask(kButtonLeft)));
		printf("kButtonA:          %d\n", HasPlatformCapability(kCapsButtonMask(kButtonA)));
		printf("kButtonB:          %d\n", HasPlatformCapability(kCapsButtonMask(kButtonB)));
		printf("kButtonLeftShoulder:%d\n", HasPlatformCapability(kCapsButtonMask(kButtonLeftShoulder)));
		printf("kButtonRightShoulder:%d\n", HasPlatformCapability(kCapsButtonMask(kButtonRightShoulder)));
		printf("kButtonMenu:       %d\n", HasPlatformCapability(kCapsButtonMask(kButtonMenu)));
		printf("kButtonHint:       %d\n", HasPlatformCapability(kCapsButtonMask(kButtonHint)));
		printf("kButtonPause:      %d\n", HasPlatformCapability(kCapsButtonMask(kButtonPause)));
		printf("kButtonBrightness: %d\n", HasPlatformCapability(kCapsButtonMask(kButtonBrightness)));
		printf("kHeadphoneJackDetect:%d\n", HasPlatformCapability(kCapsButtonMask(kHeadphoneJackDetect)));
		printf("kCartridgeDetect:  %d\n", HasPlatformCapability(kCapsButtonMask(kCartridgeDetect)));
		printf("kButtonVolumeDown: %d\n", HasPlatformCapability(kCapsButtonMask(kButtonVolumeDown)));
		printf("kButtonVolumeUp:   %d\n", HasPlatformCapability(kCapsButtonMask(kButtonVolumeUp)));
		printf("kButtonEscape:     %d\n", HasPlatformCapability(kCapsButtonMask(kButtonEscape)));
		printf("\n");
	}
};

// EOF
