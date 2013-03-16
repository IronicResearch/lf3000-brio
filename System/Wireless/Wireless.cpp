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
#include <ifaddrs.h>
#include <signal.h>
#include <stdio.h>

using namespace fi::w1;

LeapFrog::Brio::tWirelessState TranslateStateStr(LeapFrog::Brio::CString state);

LF_BEGIN_BRIO_NAMESPACE()

#define ADAPTER_NAME "wlan0"
#define AVAHI_ADAPTER_NAME "wlan0:avahi"
#define kWirelessStateEventPriority	0

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
	mConnection_->set_timeout(2000);
	
	mServer_ = new org::freedesktop::Avahi::Server(*mConnection_);
	DBus::Path service_browser_path = mServer_->ServiceBrowserNew( -1, 0, "_workstation._tcp", "local", 0 );
	mBrowser_ = new org::freedesktop::Avahi::ServiceBrowser(*mConnection_, service_browser_path, mServer_);
	mBrowser_->EnableSignals();
	mBrowser_->Start();
	
	
	pthread_create(&mDispatchTask_, NULL, DBusDispatcherTask, (void*)mDispatcher_);
	
	//TODO: This is to help ensure the dispatch thread is up and running before
	//      we try and operate on it. Really though, we need a better way to synchronize
	sleep(1);
	
	//Turn off Wireless via ConnMan. This puts us in a known state and should hopefully
	//keep ConnMan's muddy paws off the network card!
	bSavedConnManState_ = GetConnManWirelessPower();
	SetConnManWirelessPower(false);
}

//----------------------------------------------------------------------------
CWirelessModule::~CWirelessModule()
{
        //Clean up in case the app doesn't
        LeaveAdhocNetwork();
        SetWirelessPower(false);
        
	//Restore ConnMan to it's former state
	SetConnManWirelessPower(bSavedConnManState_);
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
	try
	{
		fi::w1::WpaSupplicant wpa_supplicant(*mConnection_);
		if(power)
		{
			std::map< std::string, ::DBus::Variant > if_props;
			
			DBus::Variant ifname;
			ifname.writer().append_string("wlan0");
			if_props["Ifname"] = ifname;
			
			DBus::Variant conf_file;
			conf_file.writer().append_string("/etc/wpa_supplicant.conf");
			if_props["ConfigFile"] = conf_file;
			
			wpa_supplicant.CreateInterface(if_props);
		}
		else
		{
			DBus::Path interface = GetWPAInterface();
			wpa_supplicant.RemoveInterface(interface);
		}
	}
	catch(DBus::Error& err)
	{
		if( !( strcmp( err.name(), "fi.w1.wpa_supplicant1.InterfaceExists") == 0 ) &&
		    !( strcmp( err.name(), "fi.w1.wpa_supplicant1.InterfaceUnknown") == 0 ) )
		{
			debug_.DebugOut(kDbgLvlImportant, "DBus error: %s\n", err.what());
		}
		    
	}
	return kNoErr;
}

Boolean CWirelessModule::GetWirelessPower()
{
	return (GetWPAInterface() != "");
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
	
	DBus::Variant freq;
	freq.writer().append_int32(2437);
	network_props["frequency"] = freq;
	
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
		group.writer().append_string("CCMP");
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
		if( !ToggleAvahiAutoIP(true) )
			return kJoinFailedErr;
		
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
	
	ToggleAvahiAutoIP(false);
	
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
		return TranslateStateStr(state);
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

tErrType CWirelessModule::GetLocalWirelessAddress(in_addr& address)
{
	ifaddrs* address_list;
	ifaddrs* current_address;
	tErrType ret = kNoAddressErr;
	
	if( getifaddrs(&address_list) )
	{
		debug_.DebugOut(kDbgLvlImportant, "Failed to get network adapter list: %s\n", strerror(errno));
		return kUnspecifiedErr;
	}
	
	for( current_address = address_list; current_address != NULL; current_address = current_address->ifa_next )
	{
		if( strcmp(current_address->ifa_name, ADAPTER_NAME) == 0 ||
		    strcmp(current_address->ifa_name, AVAHI_ADAPTER_NAME) == 0)
		{
			if(current_address->ifa_addr->sa_family != AF_INET)
			{
				continue;
			}
			else
			{
				sockaddr_in* ipv4_addr = (sockaddr_in*)current_address->ifa_addr;
				address = ipv4_addr->sin_addr;
				ret = kNoErr;
				break;
			}
		}
	}
	freeifaddrs(address_list);
	if(ret == kNoAddressErr)
		debug_.DebugOut(kDbgLvlImportant, "No Wireless IP address assigned.\n");
	return ret;
}

Boolean CWirelessModule::SetConnManWirelessPower(Boolean power)
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

Boolean CWirelessModule::GetConnManWirelessPower()
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

Boolean CWirelessModule::ToggleAvahiAutoIP(Boolean on)
{
	const char* daemon_arg;
	if(on)
		daemon_arg = "-D";
	else
		daemon_arg = "-k";
	
	//We need to vfork a process to become avahi-autoipd
	pid_t fork_pid;
	if( (fork_pid = vfork()) < 0 )
	{
		debug_.DebugOut(kDbgLvlImportant, "Failed to fork for avahi-autoipd: %s\n", strerror(errno));
		return false;
	}
	else if( fork_pid == 0 )
	{
		if( execl( "/usr/sbin/avahi-autoipd", "avahi-autoipd", daemon_arg, "wlan0", NULL ) < 0 )
		{
			//Dangerous! We failed to exec after vfork. print debug?
			perror("avahi-autoipd exec failed");
			_exit(0);
		}
	}
	return true;
}

void*	CWirelessModule::DBusDispatcherTask( void* arg )
{
	DBus::BusDispatcher* dispatcher = (DBus::BusDispatcher*)arg;
	dispatcher->enter();
	return NULL;
}

LF_END_BRIO_NAMESPACE()

void wpa_supplicant1::Interface::PropertiesChanged(const std::map< std::string, ::DBus::Variant >& properties)
{
	if( properties.count("State") )
	{
		LeapFrog::Brio::CString state = properties.at("State").reader().get_string();
		LeapFrog::Brio::tWirelessState new_state = TranslateStateStr(state);
		
		LeapFrog::Brio::CWirelessStateMessage msg(new_state);
		mEvent.PostEvent(msg, kWirelessStateEventPriority);
	}
}

LeapFrog::Brio::tWirelessState TranslateStateStr(LeapFrog::Brio::CString state)
{
	if( state == "disconnected" ||
	    state == "inactive" ||
	    state == "scanning" )
		return LeapFrog::Brio::kWirelessDisconnected;
	
	else if( state == "associated" ||
	         state == "completed" )
		return LeapFrog::Brio::kWirelessConnected;
		
	return LeapFrog::Brio::kWirelessConnecting;
}

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
