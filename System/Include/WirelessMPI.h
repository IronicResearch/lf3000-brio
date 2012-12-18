#ifndef LF_BRIO_WIRELESSMPI_H
#define LF_BRIO_WIRELESSMPI_H
//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		WirelessMPI.h
//
// Description:
//		Defines the Module Public Interface (MPI) for the Wireless networking module. 
//
//==============================================================================

#include <CoreMPI.h>
#include <EventListener.h>
#include <WirelessTypes.h>

LF_BEGIN_BRIO_NAMESPACE()


//==============================================================================
class CWirelessMPI : public ICoreMPI {
	/// \class CWirelessMPI
	///
	/// Class for managing wireless network connections for Brio applications.
	/// This class is aimed more towards P2P wireless gaming through ad-hoc
	/// networks than any kind of internet connectivity for the moment as
	/// internet gameplay is not planned for games as yet and games that need
	/// web browser functionality should be using Qt instead of Brio.
public:	
	// ICoreMPI functionality
	virtual	Boolean			IsValid() const;
	virtual const CString*	GetMPIName() const;		
	virtual tVersion		GetModuleVersion() const;
	virtual const CString*	GetModuleName() const;	
	virtual const CURI*		GetModuleOrigin() const;

	// class-specific functionality
	CWirelessMPI();
	virtual ~CWirelessMPI();

	/// Register event listener for CWirelessStateMessage and CRemotePlayerMessage
	tErrType	RegisterEventListener(const IEventListener *pListener);
	
	/// Unregister event listener
	tErrType	UnregisterEventListener(const IEventListener *pListener);
	
	/// Turn on/off Wireless networking adapter.
	tErrType	SetWirelessPower(Boolean power);
	
	/// Get the on/off state of Wireless networking adapter. Returns false if no
	/// wireless adapter is found.
	Boolean		GetWirelessPower();
	
	/// Connect to a given Ad-Hoc network.
	/// \param ssid The SSID of the AdHoc network to join.
	/// \param encrypted Whether the AdHoc network uses encryption or not.
	/// \param password If the network is encrypted, the passphrase to use, ignored when encrypted is false.
	tErrType	JoinAdhocNetwork( CString ssid, Boolean encrypted, CString password );
	
	/// Disconnect from Ad-Hoc network. If connected to an Ad-Hoc network, disconnects
	/// from the network and attempts to revert to the state before joining the Ad-Hoc
	/// network. If not connected to an Ad-Hoc network, this function does nothing.
	tErrType	LeaveAdhocNetwork();
	
	/// Get a list of nearby devices and their IP addresses. Note that this list may be out of date and
	/// some devices in the list may no longer be reachable. This list persists even when wireless is turned
	/// off so use with caution.
	tErrType GetPossiblePlayers(PlayerList& players);
	
	/// Get the current state of the wireless card.
	tWirelessState	GetState();
	
	/// Get the current mode of the wireless card. Returns kWirelessNone if state is not connecting
	/// or connected.
	tWirelessMode	GetMode();

private:
	class CWirelessModule*	pModule_;
};

LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_WIRELESSMPI_H

// eof
