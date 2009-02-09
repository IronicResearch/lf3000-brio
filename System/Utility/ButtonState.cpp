#include <Utility.h>

LF_BEGIN_BRIO_NAMESPACE()

static tButtonData2 gButtonData;

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

LF_END_BRIO_NAMESPACE()
