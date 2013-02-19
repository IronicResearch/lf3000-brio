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
	/// \class CRemotePlayerMessage
	/// 
	/// This event message is posted when avahi detects a new LeapFrog device
	/// on the same network and subnet, or when avahi detects that a LeapFrog
	/// device has left the network. Because avahi allows for a very long
	/// timeout before reporting a device it has lost communication with as
	/// being removed from the network, the removed event may be unreliable.
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
	/// \class CWirelessStateMessage
	/// 
	/// This event message is posted when the wireless network adapter changes
	/// states.
public:
	CWirelessStateMessage( tWirelessState state );
	virtual U16	GetSizeInBytes() const;
	tWirelessState	GetState() const;
private:
	tWirelessState	mState;
};

LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_BUTTONTYPES_H

// eof
