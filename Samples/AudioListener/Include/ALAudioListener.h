#ifndef AUDIO_AUDIOLISTENER_INCLUDE_ALAUDIOLISTENER_H_
#define AUDIO_AUDIOLISTENER_INCLUDE_ALAUDIOLISTENER_H_

/******************************************************************************
* Copyright 2012(c) LeapFrog Enterprises, Inc.
* All Rights Reserved
*******************************************************************************
*
* \file ALAudioListener.h
*
* ALAudioListener is an event listener class that responds only to audio
* completion events.  Upon an audio completion event this class calls the
* delegate method inside of the main state class, ALMainState, so the state
* object can handle what happens upon audio completion.
**
* author: 	leapfrog
*  			alindblad - 10/24/12 - crated
*
******************************************************************************/

#include <EventListener.h>

class ALMainState;

/*
 * \class ALAudioListener
 * ALAudioListener is derived off of the IEvenListener and overwrites
 * the Notify method to catch notifications.
 */
class ALAudioListener : public LeapFrog::Brio::IEventListener
{
public:
    /*
     *  ALAudioListener constructor
     *
     *  Creates a listener instance given an ALMainState object.
     */
    ALAudioListener(ALMainState* state);

    /*
     *  ALAudioListener destructor
     *
     *  releases all memory allocated
     */
virtual ~ALAudioListener();

	LeapFrog::Brio::tEventStatus Notify(const LeapFrog::Brio::IEventMessage &msgIn);

private:
    static const LeapFrog::Brio::tEventType event_types_[];  ///< used to indicate which events to listen to
    ALMainState* state_;  ///< the state to which delegate calls are made

	/*
	 * Copy Constructor and operator =
	 *
	 *  Private and unimplemented to prevent copying
	 */
	ALAudioListener(const ALAudioListener&);
	ALAudioListener& operator =(const ALAudioListener&);
};

#endif  // AUDIO_AUDIOLISTENER_INCLUDE_ALAUDIOLISTENER_H_
