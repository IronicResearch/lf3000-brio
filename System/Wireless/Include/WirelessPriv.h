#ifndef LF_BRIO_WIRELESSMODULEPRIV_H
#define LF_BRIO_WIRELESSMODULEPRIV_H
//==============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//==============================================================================
//
// File:
//		EventPriv.h
//
// Description:
//		Defines the interface for the private underlying Event module. 
//
//==============================================================================

#include <CoreModule.h>
#include <DebugMPI.h>
#include <KernelMPI.h>
#include <EventMPI.h>
#include <WirelessTypes.h>

#include <dbus-c++/dbus.h>
#include <avahi-server.h>
#include <avahi-servicebrowser.h>

LF_BEGIN_BRIO_NAMESPACE()

// Constants
const CString	kWirelessModuleName	= "Wireless";
const tVersion	kWirelessModuleVersion	= 2;

//==============================================================================
class CWirelessModule : public ICoreModule {
public:	
	// core functionality
	virtual Boolean			IsValid() const;
	virtual tVersion		GetModuleVersion() const;
	virtual const CString*		GetModuleName() const;	
	virtual const CURI*		GetModuleOrigin() const;
	
	/// Turn on/off Wireless networking adapter (in wpa_supplicant)
	VTABLE_EXPORT tErrType	SetWirelessPower(Boolean power);
	
	/// Get the on/off state of Wireless networking adapter. Returns false if no
	/// wireless adapter is found.
	VTABLE_EXPORT Boolean	GetWirelessPower();
	
	/// Connect to a given Ad-Hoc network.
	/// \param ssid The SSID of the AdHoc network to join.
	/// \param encrypted Whether the AdHoc network uses encryption or not.
	/// \param password If the network is encrypted, the passphrase to use, ignored when encrypted is false.
	VTABLE_EXPORT tErrType	JoinAdhocNetwork( CString ssid, Boolean encrypted, CString password );
	
	/// Disconnect from Ad-Hoc network. If connected to an Ad-Hoc network, disconnects
	/// from the network and attempts to revert to the state before joining the Ad-Hoc
	/// network. If not connected to an Ad-Hoc network, this function does nothing.
	VTABLE_EXPORT tErrType	LeaveAdhocNetwork();
	
	/// Get a list of nearby devices and their IP addresses. Note that this list may be out of date and
	/// some devices in the list may no longer be reachable. This list persists even when wireless is turned
	/// off so use with caution.
	VTABLE_EXPORT tErrType GetPossiblePlayers(PlayerList& players);
	
	/// Get the current state of the wireless card.
	VTABLE_EXPORT tWirelessState	GetState();
	
	/// Get the current mode of the wireless card. Returns kWirelessNone if state is not connecting
	/// or connected.
	VTABLE_EXPORT tWirelessMode	GetMode();
	
	/// Get the IP address of the wireless network interface on the localhost.
	VTABLE_EXPORT tErrType	GetLocalWirelessAddress(in_addr& address);
						
private:
	// Limit object creation to the Module Manager interface functions
	CWirelessModule();
	virtual ~CWirelessModule();
	friend LF_ADD_BRIO_NAMESPACE(ICoreModule*)
						::CreateInstance(LF_ADD_BRIO_NAMESPACE(tVersion));
	friend void	::DestroyInstance(LF_ADD_BRIO_NAMESPACE(ICoreModule*));
	
	//Internal utility functions
	DBus::Path GetWirelessTechnology();
	DBus::Path GetWPAInterface();
	DBus::Path GetWPANetwork();
	
	//ConnMan specific stuff
	Boolean SetConnManWirelessPower(Boolean power);
	Boolean GetConnManWirelessPower();
	
	//Start and stop Avahi Auto IP daemon
	Boolean ToggleAvahiAutoIP(Boolean on);
	

public:
	static void*	DBusDispatcherTask( void* arg );
	CDebugMPI	debug_;
	CKernelMPI	kernel_;
	CEventMPI	eventmgr_;
	
private:
	Boolean			bThreadRun_;
	pthread_t		mDispatchTask_;
	DBus::BusDispatcher*	mDispatcher_;
	DBus::Connection*	mConnection_;
	org::freedesktop::Avahi::Server* mServer_;
	org::freedesktop::Avahi::ServiceBrowser* mBrowser_;
	
	//Cache and state
	Boolean			bSavedConnManState_;

};

LF_END_BRIO_NAMESPACE()	
#endif // LF_BRIO_EVENTMGRMODULEPRIV_H

// eof
