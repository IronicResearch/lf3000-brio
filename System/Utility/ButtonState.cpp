#include <Utility.h>

LF_BEGIN_BRIO_NAMESPACE()

static tButtonData gButtonData;

tButtonData		GetButtonState(void)
{
	return gButtonData;
}

void SetButtonState(tButtonData button_data)
{
	gButtonData = button_data;
}

LF_END_BRIO_NAMESPACE()
