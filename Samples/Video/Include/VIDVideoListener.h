#ifndef VIDEO_VIDEO_INCLUDE_VIDVIDEOLISTENER_H_
#define VIDEO_VIDEO_INCLUDE_VIDVIDEOLISTENER_H_

/******************************************************************************
* Copyright 2012(c) LeapFrog Enterprises, Inc.
* All Rights Reserved
*******************************************************************************
*
* \file VIDVideoListener.h
*
* VIDVideoListener is an event listener class that responds only to video
* completion events.  Upon a video completion event this class calls the
* delegate method inside of the main state class, VIDMainState, so the state
* object can handle what happens upon audio completion.
**
* author: 	leapfrog
*  			alindblad - 2/12/13 - created
*
******************************************************************************/

#include <EventListener.h>

class VIDMainState;

/*
 * \class VIDVideoListener
 * VIDVideoListener is derived off of the IEvenListener and overwrites
 * the Notify method to catch notifications.
 */
class VIDVideoListener : public LeapFrog::Brio::IEventListener
{
public:
    /*
     *  VIDVideoListener constructor
     *
     *  Creates a listener instance given an VIDMainState object.
     */
    VIDVideoListener(VIDMainState* state);

    /*
     *  VIDVideoListener destructor
     *
     *  releases all memory allocated
     */
virtual ~VIDVideoListener();

LeapFrog::Brio::tEventStatus Notify(const LeapFrog::Brio::IEventMessage &msgIn);

private:
    static const LeapFrog::Brio::tEventType event_types_[];  ///< used to indicate which events to listen to
    VIDMainState* state_;  ///< the state to which delegate calls are made

	/*
	 * Copy Constructor and operator =
	 *
	 *  Private and unimplemented to prevent copying
	 */
	VIDVideoListener(const VIDVideoListener&);
	VIDVideoListener& operator =(const VIDVideoListener&);
};

#endif  // VIDEO_VIDEO_INCLUDE_VIDVIDEOLISTENER_H_
