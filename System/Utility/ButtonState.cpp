#include <Utility.h>

LF_BEGIN_BRIO_NAMESPACE()

static tButtonData2 gButtonData;
static tDpadOrientation gDpadOrientation = kDpadPortrait;

tButtonData GetButtonState(void)
{
	tButtonData data = {gButtonData.buttonState, gButtonData.buttonTransition};
	return data;
}

tButtonData2 GetButtonState2(void)
{
	return gButtonData;
}

void SetButtonState(tButtonData2 button_data)
{
	gButtonData = button_data;
}

tDpadOrientation GetDpadOrientationState()
{
	return gDpadOrientation;
}
tErrType SetDpadOrientationState(tDpadOrientation dpad_orientation)
{
	gDpadOrientation = dpad_orientation;
	return kNoErr;
}

LF_END_BRIO_NAMESPACE()
