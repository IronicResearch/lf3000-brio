#ifndef LF_BRIO_WIRELESSTYPES_H
#define LF_BRIO_WIRELESSTYPES_H
//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		WirelessTypes.h
//
// Description:
//		Defines types for the Wireless Networking module. 
//
//==============================================================================

#include <EventMessage.h>
#include <SystemErrors.h>
#include <SystemEvents.h>
#include <SystemTypes.h>
#include <GroupEnumeration.h>

#include <netinet/in.h>
#include <vector>
LF_BEGIN_BRIO_NAMESPACE()


//==============================================================================	   
// Wireless events
//==============================================================================
#define WIRELESS_EVENTS					\
	(kRemotePlayerChanged)				\
	(kWirelessStateChanged)

BOOST_PP_SEQ_FOR_EACH_I(GEN_TYPE_VALUE, FirstEvent(kGroupWireless), WIRELESS_EVENTS)

const tEventType kAllWirelessEvents = AllEvents(kGroupWireless);


//==============================================================================	   
// Wireless errors
//==============================================================================
#define WIRELESS_ERRORS				\
	(kNoWirelessErr)			\
	(kJoinFailedErr)			\
	(kNoAddressErr)

BOOST_PP_SEQ_FOR_EACH_I(GEN_ERR_VALUE, FirstErr(kGroupWireless), WIRELESS_ERRORS)


//==============================================================================	   
// Wireless types
//==============================================================================
//------------------------------------------------------------------------------

class tRemotePlayer {
public:
	CString		hostname;
	in_addr 	address;
	tRemotePlayer(CString host, in_addr addr) :
		hostname(host), address(addr)
	{}
};

enum tWirelessState {
	kWirelessOff,
	kWirelessDisconnected,
	kWirelessConnecting,
	kWirelessConnected
};

enum tWirelessMode {
	kWirelessNone,
	kWirelessAdHoc,
	kWirelessAccessPoint
};

typedef std::vector<tRemotePlayer> PlayerList;

//------------------------------------------------------------------------------
class CRemotePlayerMessage : public IEventMessage {
public:
	CRemotePlayerMessage( const tRemotePlayer& player, Boolean added );
	~CRemotePlayerMessage();
	virtual U16	GetSizeInBytes() const;
	Boolean		PlayerAdded() const;
	Boolean		PlayerRemoved() const;
	tRemotePlayer	GetPlayer() const;
private:
	tRemotePlayer	mPlayer;
	Boolean		mAdded;
};

class CWirelessStateMessage : public IEventMessage {
public:
	CWirelessStateMessage( tWirelessState state, tWirelessMode mode );
	virtual U16	GetSizeInBytes() const;
	tWirelessState	GetState() const;
	tWirelessMode	GetMode() const;
private:
	tWirelessState	mState;
	tWirelessMode	mMode;
};

LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_BUTTONTYPES_H

// eof
