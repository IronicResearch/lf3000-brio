//============================================================================
// $Source: $
//
// Copyright (c) LeapFrog Enterprises, Inc.
//============================================================================
//
// File:
//		Wireless.cpp
//
// Description:
//		Implements the underlying Wireless Networking module.
//
//============================================================================

#include <SystemTypes.h>
#include <StringTypes.h>
#include <SystemErrors.h>

#include <KernelMPI.h>
#include <DebugMPI.h>
#include <EventListener.h>
#include <EventMessage.h>
#include <EventMPI.h>

#include <WirelessPriv.h>
#include <wpa_supplicant.h>
#include <wpa_supplicant_Interface.h>
#include <wpa_supplicant_Network.h>
#include <connman-manager.h>
#include <connman-tech.h>
#include <avahi-server.h>
#include <avahi-servicebrowser.h>

#include <unistd.h>
#include <string.h>

using namespace fi::w1;

LF_BEGIN_BRIO_NAMESPACE()

const CURI	kModuleURI	= "/LF/System/Wireless";

//============================================================================
// Informational functions
//============================================================================
//----------------------------------------------------------------------------
tVersion CWirelessModule::GetModuleVersion() const
{
	return kWirelessModuleVersion;
}

//----------------------------------------------------------------------------
const CString* CWirelessModule::GetModuleName() const
{
	return &kWirelessModuleName;
}

//----------------------------------------------------------------------------
const CURI* CWirelessModule::GetModuleOrigin() const
{
	return &kModuleURI;
}

//============================================================================
// Ctor & dtor
//============================================================================
//----------------------------------------------------------------------------
CWirelessModule::CWirelessModule() :
	debug_(kGroupWireless),
	bThreadRun_(true)
{
	DBus::_init_threading();
	mDispatcher_ = new DBus::BusDispatcher();
	DBus::default_dispatcher = mDispatcher_;
	mConnection_ = new DBus::Connection(DBus::Connection::SystemBus());
	mConnection_->set_timeout(30000);
	
	mServer_ = new org::freedesktop::Avahi::Server(*mConnection_);
	DBus::Path service_browser_path = mServer_->ServiceBrowserNew( -1, 0, "_workstation._tcp", "local", 0 );
	mBrowser_ = new org::freedesktop::Avahi::ServiceBrowser(*mConnection_, service_browser_path, mServer_);
	mBrowser_->EnableSignals();
	mBrowser_->Start();
	
	
	pthread_create(&mDispatchTask_, NULL, DBusDispatcherTask, (void*)mDispatcher_);
	
	//TODO: This is to help ensure the dispatch thread is up and running before
	//      we try and operate on it. Really though, we need a better way to synchronize
	sleep(1);
}

//----------------------------------------------------------------------------
CWirelessModule::~CWirelessModule()
{
	mDispatcher_->leave();
	pthread_join(mDispatchTask_, NULL);
	delete mConnection_;
	mBrowser_->Free();
	delete mBrowser_;
	delete mServer_;
}

//----------------------------------------------------------------------------
Boolean	CWirelessModule::IsValid() const
{
	return true;
}

DBus::Path CWirelessModule::GetWirelessTechnology()
{
	try
	{
		net::connman::Manager connman(*mConnection_);
		net::connman::TechnologyArray techs = connman.GetTechnologies();
		for( net::connman::TechnologyArray::iterator it = techs.begin(); it != techs.end(); it++ )
		{
			if( strcmp(it->_2["Name"].reader().get_string(), "WiFi") == 0 )
				return it->_1;
		}
	}
	catch(DBus::Error& err)
	{
		debug_.DebugOut(kDbgLvlImportant, "DBus error: %s\n", err.what());
	}
	return "";
}

DBus::Path CWirelessModule::GetWPAInterface()
{
	try
	{
		fi::w1::WpaSupplicant wpa_supplicant(*mConnection_);
		return wpa_supplicant.GetInterface("wlan0");
	}
	catch(DBus::Error& err)
	{
		debug_.DebugOut(kDbgLvlImportant, "DBus error: %s\n", err.what());
	}
	return "";
}

DBus::Path CWirelessModule::GetWPANetwork()
{
	try
	{
		DBus::Path interface_path = GetWPAInterface();
		if( interface_path != "" )
		{
			fi::w1::wpa_supplicant1::Interface interface(*mConnection_, interface_path);
			DBus::Path network = interface.CurrentNetwork();
			if(network == "/")
				return "";
			return network;
		}
	}
	catch(DBus::Error& err)
	{
		debug_.DebugOut(kDbgLvlImportant, "DBus error: %s\n", err.what());
	}
	return "";
}
	
tErrType CWirelessModule::SetWirelessPower(Boolean power)
{
	DBus::Path tech_path = GetWirelessTechnology();
		
	//Make sure we found a wifi tech
	if( tech_path == "" )
	{
		debug_.DebugOut( kDbgLvlImportant, "Failed to find a WiFi technology in ConnMan.\n" );
		return kNoWirelessErr;
	}
	
	try
	{
		net::connman::Technology tech(*mConnection_, tech_path);
		DBus::Variant powered;
		powered.writer().append_bool(power);
		tech.SetProperty("Powered", powered);
	}
	catch(DBus::Error& err)
	{
		debug_.DebugOut(kDbgLvlImportant, "DBus error: %s\n", err.what());
		return kUnspecifiedErr;
	}
	return kNoErr;
}

Boolean CWirelessModule::GetWirelessPower()
{
	DBus::Path tech_path = GetWirelessTechnology();
	bool ret = false;
	
	//Make sure we found a wifi tech
	if( tech_path == "" )
	{
		debug_.DebugOut( kDbgLvlImportant, "Failed to find a WiFi technology in ConnMan.\n" );
		return false;
	}
	
	try
	{
		net::connman::Technology tech(*mConnection_, tech_path);
		DBus::Variant powered = tech.GetProperties()["Powered"];
		ret = powered.reader().get_bool();
	}
	catch(DBus::Error& err)
	{
		debug_.DebugOut(kDbgLvlImportant, "DBus error: %s\n", err.what());
		ret = false;
	}
	return ret;
}
	
tErrType CWirelessModule::JoinAdhocNetwork( CString ssid, Boolean encrypted, CString password )
{
	//Try and get the interface path first
	DBus::Path interface_path = GetWPAInterface();
	if(interface_path == "")
		return kNoWirelessErr;

	std::map< std::string, ::DBus::Variant > network_props;
	
	DBus::Variant ssid_variant;
	ssid_variant.writer().append_string(ssid.c_str());
	network_props["ssid"] = ssid_variant;
	
	DBus::Variant mode;
	mode.writer().append_int32(1);
	network_props["mode"] = mode;
	
	DBus::Variant key_mgmt;
	
	if( encrypted )
	{
		key_mgmt.writer().append_string("WPA-NONE");
		
		DBus::Variant proto;
		proto.writer().append_string("WPA");
		network_props["proto"] = proto;
		
		DBus::Variant pairwise;
		pairwise.writer().append_string("NONE");
		network_props["pairwise"] = pairwise;
		
		DBus::Variant group;
		group.writer().append_string("TKIP");
		network_props["group"] = group;
		
		DBus::Variant psk;
		psk.writer().append_string(password.c_str());
		network_props["psk"] = psk;
	}
	else
	{
		key_mgmt.writer().append_string("NONE");
	}
	network_props["key_mgmt"] = key_mgmt;
	
	try
	{
		fi::w1::wpa_supplicant1::Interface interface(*mConnection_, interface_path);
		if(interface.CurrentNetwork() != "/")
			interface.Disconnect();
		DBus::Path new_network = interface.AddNetwork(network_props);
		interface.SelectNetwork( new_network );
	}
	catch(DBus::Error& err)
	{
		debug_.DebugOut(kDbgLvlImportant, "DBus error: %s\n", err.what());
		return kJoinFailedErr;
	}
	return kNoErr;
}

tErrType CWirelessModule::LeaveAdhocNetwork()
{
	//Try to get the interface path
	//If it doesn't exist, wifi is powered down, so we don't need to do anything
	DBus::Path interface_path = GetWPAInterface();
	if(interface_path == "")
		return kNoErr;
	
	try
	{
		//Just disconnect from whatever network right now until ConnMan knows about AdHoc
		fi::w1::wpa_supplicant1::Interface interface(*mConnection_, interface_path);
		if(interface.CurrentNetwork() != "/")
			interface.Disconnect();
		return kNoErr;
	}
	catch(DBus::Error& err)
	{
		debug_.DebugOut(kDbgLvlImportant, "DBus error: %s\n", err.what());
	}
	return kNoWirelessErr;
}

tErrType CWirelessModule::GetPossiblePlayers(PlayerList& players)
{
	std::map<std::string, struct in_addr> services = mBrowser_->GetResolvedServices();
	std::map<std::string, struct in_addr>::iterator it;
	
	players.clear();
	for( it = services.begin(); it != services.end(); it++ )
	{
		players.push_back(tRemotePlayer(it->first, it->second));
	}
	return kNoErr;
}
	
tWirelessState CWirelessModule::GetState()
{
	DBus::Path interface_path = GetWPAInterface();
	if(interface_path == "")
		return kWirelessOff;
	
	try
	{
		fi::w1::wpa_supplicant1::Interface interface(*mConnection_, interface_path);
		CString state = interface.State();
		if( state == "disconnected" ||
		    state == "inactive" ||
		    state == "scanning" )
			return kWirelessDisconnected;
		
		if( state == "associated" ||
		    state == "completed" )
		    	return kWirelessConnected;
		
		return kWirelessConnecting;
	}
	catch(DBus::Error& err)
	{
		debug_.DebugOut(kDbgLvlImportant, "DBus error: %s\n", err.what());
	}
	return kWirelessOff;
}

tWirelessMode CWirelessModule::GetMode()
{
	DBus::Path network_path = GetWPANetwork();
	if(network_path == "")
		return kWirelessNone;
	
	try
	{
		fi::w1::wpa_supplicant1::Network network(*mConnection_, network_path);
		const std::map< std::string, ::DBus::Variant > props = network.Properties();
		const DBus::Variant mode_variant = props.find("mode")->second;
		CString mode = mode_variant.reader().get_string();
		if(mode == "1")
			return kWirelessAdHoc;
		if(mode == "0")
			return kWirelessAccessPoint;
	}
	catch(DBus::Error& err)
	{
		debug_.DebugOut(kDbgLvlImportant, "DBus error: %s\n", err.what());
	}
	return kWirelessNone;
}

void*	CWirelessModule::DBusDispatcherTask( void* arg )
{
	DBus::BusDispatcher* dispatcher = (DBus::BusDispatcher*)arg;
	dispatcher->enter();
	return NULL;
}

LF_END_BRIO_NAMESPACE()

//============================================================================
// Instance management interface for the Module Manager
//============================================================================
#ifndef LF_MONOLITHIC_DEBUG
LF_USING_BRIO_NAMESPACE()

static CWirelessModule*	sinst = NULL;

extern "C"
{
	//------------------------------------------------------------------------
	ICoreModule* CreateInstance(tVersion version)
	{
		if( sinst == NULL )
			sinst = new CWirelessModule;
		return sinst;
	}
		
	//------------------------------------------------------------------------
	void DestroyInstance(ICoreModule* ptr)
	{
	//		assert(ptr == sinst);
		delete sinst;
		sinst = NULL;
	}
}
#endif	// LF_MONOLITHIC_DEBUG

// EOF
