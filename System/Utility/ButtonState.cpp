#include <Utility.h>
#include <KernelMPI.h>
#include <EventMPI.h>

LF_BEGIN_BRIO_NAMESPACE()

tButtonData2 gButtonData;
tMutex gButtonDataMutex;
tDpadOrientation gDpadOrientation = kDpadPortrait;

tButtonData GetButtonState(void)
{
	tButtonData data = {gButtonData.buttonState, gButtonData.buttonTransition};
	return data;
}

tButtonData2 GetButtonState2(void)
{
	return gButtonData;
}

//Minh Saelock - this shouldn't be used anymore
void SetButtonState(tButtonData2 button_data)
{
	CKernelMPI kernel_mpi;
	kernel_mpi.LockMutex(gButtonDataMutex);
	gButtonData = button_data;
	kernel_mpi.UnlockMutex(gButtonDataMutex);
}

tDpadOrientation GetDpadOrientationState()
{
	return gDpadOrientation;
}

void RotateDpad(int rotation)
{
	U32 button_mask = kButtonUp | kButtonDown | kButtonRight | kButtonLeft;
	tButtonData2 old_dpad = gButtonData;
	gButtonData.buttonState &= ~button_mask;
	gButtonData.buttonTransition = 0;

	switch(rotation)
	{
	case 0:
		gButtonData.buttonState = old_dpad.buttonState;
		break;
	case 1:
		if(old_dpad.buttonState & kButtonUp)
			gButtonData.buttonState |= kButtonLeft;
		if(old_dpad.buttonState & kButtonDown)
			gButtonData.buttonState |= kButtonRight;
		if(old_dpad.buttonState & kButtonRight)
			gButtonData.buttonState |= kButtonUp;
		if(old_dpad.buttonState & kButtonLeft)
			gButtonData.buttonState |= kButtonDown;
		break;
	case 2:
		if(old_dpad.buttonState & kButtonUp)
			gButtonData.buttonState |= kButtonDown;
		if(old_dpad.buttonState & kButtonDown)
			gButtonData.buttonState |= kButtonUp;
		if(old_dpad.buttonState & kButtonRight)
			gButtonData.buttonState |= kButtonLeft;
		if(old_dpad.buttonState & kButtonLeft)
			gButtonData.buttonState |= kButtonRight;
		break;
	case 3:
		if(old_dpad.buttonState & kButtonUp)
			gButtonData.buttonState |= kButtonRight;
		if(old_dpad.buttonState & kButtonDown)
			gButtonData.buttonState |= kButtonLeft;
		if(old_dpad.buttonState & kButtonRight)
			gButtonData.buttonState |= kButtonDown;
		if(old_dpad.buttonState & kButtonLeft)
			gButtonData.buttonState |= kButtonUp;
		break;
	}
	gButtonData.buttonTransition = (old_dpad.buttonState ^ gButtonData.buttonState) & button_mask;
}

tErrType SetDpadOrientationState(tDpadOrientation dpad_orientation)
{
	if(gDpadOrientation == dpad_orientation)
		return kNoErr;

	CKernelMPI kernel_mpi;
	kernel_mpi.LockMutex(gButtonDataMutex);
	int rotations = 0;
	switch(gDpadOrientation)
	{
	case kDpadLandscape:
		switch(dpad_orientation)
		{
		case kDpadPortrait:
			rotations = 1;
			break;
		case kDpadLandscapeUpsideDown:
			rotations = 2;
			break;
		case kDpadPortraitUpsideDown:
			rotations = 3;
			break;
		}
		break;
	case kDpadPortrait:
		switch(dpad_orientation)
		{
		case kDpadLandscape:
			rotations = 3;
			break;
		case kDpadLandscapeUpsideDown:
			rotations = 1;
			break;
		case kDpadPortraitUpsideDown:
			rotations = 2;
			break;
		}
		break;
	case kDpadLandscapeUpsideDown:
		switch(dpad_orientation)
		{
		case kDpadLandscape:
			rotations = 2;
			break;
		case kDpadPortrait:
			rotations = 3;
			break;
		case kDpadPortraitUpsideDown:
			rotations = 1;
			break;
		}
		break;
	case kDpadPortraitUpsideDown:
		switch(dpad_orientation)
		{
		case kDpadLandscape:
			rotations = 1;
			break;
		case kDpadPortrait:
			rotations = 2;
			break;
		case kDpadLandscapeUpsideDown:
			rotations = 3;
			break;
		}
		break;
	}
	RotateDpad(rotations);
	gDpadOrientation = dpad_orientation;
	if(gButtonData.buttonTransition != 0) {
		U64 usec = kernel_mpi.GetElapsedTimeAsUSecs();
		U64 sec = usec / 1000000;
		usec %= 1000000;
		gButtonData.time.seconds      = sec;
		gButtonData.time.microSeconds = usec;
		CButtonMessage button_msg(gButtonData);
		CEventMPI event_mpi;
		event_mpi.PostEvent(button_msg, 0, 0);
	}
	kernel_mpi.UnlockMutex(gButtonDataMutex);
	return kNoErr;
}

LF_END_BRIO_NAMESPACE()
