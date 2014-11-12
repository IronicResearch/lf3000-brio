#include "VIDVideoListener.h"
#include "VideoTypes.h"
#include <typeinfo>
#include "VIDMainState.h"
/*
 * VIDEO:
 * This is a sample video listener. See VIDMainState.cpp for example implementation
 */

using LeapFrog::Brio::kVideoCompletedEvent;
using LeapFrog::Brio::tEventStatus;
using LeapFrog::Brio::CVideoEventMessage;

const LeapFrog::Brio::tEventType VIDVideoListener::event_types_[] = {kVideoCompletedEvent};

VIDVideoListener::VIDVideoListener(VIDMainState* state)
:   IEventListener(event_types_, 1),
    state_(state)
{
}

VIDVideoListener::~VIDVideoListener()
{
    /// do nothing
}

tEventStatus VIDVideoListener::Notify(const LeapFrog::Brio::IEventMessage &msgIn)
{
    try
    {
        // obtain the event type
        LeapFrog::Brio::tEventType event_type = msgIn.GetEventType();

        // only respond if the type is an audio completion event
        if(event_type == kVideoCompletedEvent)
        {
            // cast the message into a CAudioEventMessage
            const CVideoEventMessage& video_event_msg = dynamic_cast<const CVideoEventMessage&>(msgIn);
            LeapFrog::Brio::tVideoMsgData video_msg_data = video_event_msg.data_;

            // call back to the state with the video id of the just finished audio
            state_->HandleVideoDonePlaying(video_msg_data.hVideo);
        }
    }

    // dynamic type casting can fail
    catch(const std::bad_cast& e)
    {
        // handle bad cast here
    }
    return LeapFrog::Brio::kEventStatusOK;
}
