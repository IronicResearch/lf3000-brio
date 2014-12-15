#include "ALAudioListener.h"
#include <AudioTypes.h>
#include <typeinfo>
#include "ALMainState.h"
/*
 * AUDIO:
 * This is a sample audio listener. See ALMainState.cpp for example implementation
 * This warning is also in ALMainState Exit function:
 * There is a bug in Firmware 1.0.8. Setting the second argument (true) does nothing.
 * audio_mpi.StopAudio(mAudioId, true);
 * is the same as:
 * audio_mpi.StopAudio(mAudioId, false);
 *
 * What it's supposed to do:
 * If you stop the audio before it completes, it will call the listener as if the audio had completed
 * What actually happens:
 * You never get the call if the audio is stopped prematurely
 *
 * This issue has been fixed in firmware 1.1.40.
 * To get a callback if the listener is stopped prematurely, call:
 * audio_mpi.StopAudio(mAudioId, kStopAudioOptionsDoneMsg);
 */

const tEventType ALAudioListener::event_types_[] = {kAudioCompletedEvent};

ALAudioListener::ALAudioListener(ALMainState* state)
:   IEventListener(event_types_, 1),
    state_(state)
{
}

ALAudioListener::~ALAudioListener()
{
    /// do nothing
}

tEventStatus ALAudioListener::Notify(const IEventMessage &msgIn)
{
    try
    {
        // obtain the event type
        tEventType event_type = msgIn.GetEventType();

        // only respond if the type is an audio completion event
        if(event_type == kAudioCompletedEvent)
        {
            // cast the message into a CAudioEventMessage
            const CAudioEventMessage& audio_event_msg = dynamic_cast<const CAudioEventMessage&>(msgIn);
            tAudioMsgData audio_msg_data = audio_event_msg.audioMsgData;

            // call back to the state with the audio id of the just finished audio
            state_->HandleAudioDonePlaying(audio_msg_data.audioCompleted.audioID);
        }
    }

    // dynamic type casting can fail
    catch(const std::bad_cast& e)
    {
        // handle bad cast here
    }
    return kEventStatusOK;
}
