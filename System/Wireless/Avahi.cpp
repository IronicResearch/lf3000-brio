#include <avahi-server.h>
#include <avahi-servicebrowser.h>

#include <EventMPI.h>
#include <DebugMPI.h>
#include <WirelessTypes.h>

#include <arpa/inet.h>
#include <string.h>

namespace org {
namespace freedesktop {
namespace Avahi {

#define kPlayerChangeEventPriority	0

#define AVAHI_PROTO_INET 0
#define AVAHI_PROTO_UNSPEC -1

/* ---------------- Server -------------------------- */
Server::Server(DBus::Connection &connection) :
		DBus::ObjectProxy(connection, "/", "org.freedesktop.Avahi")
{}
		
void Server::StateChanged(const int32_t& state, const std::string& error)
{}

/* ---------------- ServiceBrowser ------------------ */

ServiceBrowser::ServiceBrowser(DBus::Connection &connection, 
                               DBus::Path path,
                               Server* server) :
			DBus::ObjectProxy(connection, path, "org.freedesktop.Avahi"),
			mServer(server),
			mDebug(LeapFrog::Brio::kGroupWireless)
{
	pthread_mutex_init( &mMapLock, NULL );
}

ServiceBrowser::~ServiceBrowser()
{
	mResolutionQueue.Kill();
	pthread_join( mResolveServiceThread, NULL);
	pthread_mutex_destroy( &mMapLock );
}

void ServiceBrowser::Start()
{
	pthread_create(&mResolveServiceThread, NULL, ResolveServiceThread, this);
}

void ServiceBrowser::ItemNew(const int32_t& interface,
		             const int32_t& protocol,
		             const std::string& name,
		             const std::string& type,
		             const std::string& domain,
		             const uint32_t& flags)
{
	mDebug.DebugOut(LeapFrog::Brio::kDbgLvlValuable, "Found new host: %s\n", name.c_str());
	
	char hostname[HOST_NAME_MAX];
	//If we can get a hostname, make sure it doesn't look like ours
	if( !gethostname(hostname, HOST_NAME_MAX) )
	{
		//If it looks to be the same host, don't bother resolving it
		if( strncmp( name.c_str(), hostname, strlen(hostname) ) == 0 )
		{
			mDebug.DebugOut(LeapFrog::Brio::kDbgLvlValuable, "Host was localhost, skipping\n");
			return;
		}
	}
	
	//Make sure it looks like a LeapFrog device
	if( name.compare(0, 8, "Explorer") != 0 )
	{
		mDebug.DebugOut(LeapFrog::Brio::kDbgLvlValuable, "Host was not an LF device, skipping\n");
		return;
	}

	Service* service = new Service;
	service->interface = interface;
	service->protocol = protocol;
	service->name = name;
	service->type = type;
	service->domain = domain;
	service->flags = flags;
	
	mResolutionQueue.Push(service);
}
		
void ServiceBrowser::ItemRemove(const int32_t& interface,
		                const int32_t& protocol,
		                const std::string& name,
		                const std::string& type,
		                const std::string& domain,
		                const uint32_t& flags)
{
	//If the name ends with a closing square bracket, it is most
	//likely a mac address, so strip it off.
	std::string hostname = name;
	int name_len = hostname.size();
	if( hostname[name_len - 1] == ']' )
		hostname = hostname.substr(0, name_len - 20);

	//Make sure we recognize the hostname
	mDebug.DebugOut(LeapFrog::Brio::kDbgLvlValuable, "Lost connection to host %s\n", hostname.c_str());
	pthread_mutex_lock( &mMapLock );
	if( mServiceLookup.count( hostname ) )
	{		
		//Fill out a player message
		LeapFrog::Brio::tRemotePlayer player(hostname, mServiceLookup[hostname]);
		LeapFrog::Brio::CRemotePlayerMessage msg(player, false);
		
		//Don't hold the lock during a PostEvent in case a message
		//handler calls GetPossiblePlayers()
		pthread_mutex_unlock( &mMapLock );
		
		//Post it
		mEvent.PostEvent(msg, kPlayerChangeEventPriority);
		
		//Remove the address from our lookup table
		pthread_mutex_lock( &mMapLock );
		mServiceLookup.erase(hostname);
	}
	pthread_mutex_unlock( &mMapLock );
}
		
void ServiceBrowser::Failure(const std::string& error)
{
	mDebug.DebugOut(LeapFrog::Brio::kDbgLvlCritical, 
			"Failed to browse services: %s\n", error.c_str());
}

/* Intentially blank */
void ServiceBrowser::AllForNow()
{}

void ServiceBrowser::CacheExhausted()
{}

void* ServiceBrowser::ResolveServiceThread(void* browser_arg)
{
	ServiceBrowser* browser = (ServiceBrowser*)browser_arg;
	
	int32_t interface_out;
	int32_t protocol_out;
	std::string name_out;
	std::string type_out;
	std::string domain_out;
	std::string host_out;
	int32_t aprotocol_out;
	std::string address_out;
	uint16_t port_out;
	std::vector< std::vector< uint8_t > > txt_out;
	uint32_t flags_out;
	
	while( browser->mResolutionQueue.Alive() )
	{
		Service* service = browser->mResolutionQueue.Pop();
		//Queue death or reentrancy may cause NULL return
		if( service != NULL )
		{
			try
			{
				browser->mServer->ResolveService(service->interface,
					                service->protocol,
					                service->name,
					                service->type,
					                service->domain,
					                AVAHI_PROTO_UNSPEC,
					                0,
					                interface_out,
					                protocol_out,
					                name_out,
					                type_out,
					                domain_out,
					                host_out,
					                aprotocol_out,
					                address_out,
					                port_out,
					                txt_out,
					                flags_out);
				
				browser->mDebug.DebugOut(LeapFrog::Brio::kDbgLvlValuable,
				                         "Resolved %s to %s\n",
				                         service->name.c_str(),
				                         address_out.c_str());
				
				//If the name ends with a closing square bracket, it is most
				//likely a mac address, so strip it off.
				int name_len = service->name.size();
				if( service->name[name_len - 1] == ']' )
					service->name = service->name.substr(0, name_len - 20);
				
				//Parse address into in_addr
				struct in_addr addr;
				int ret = inet_pton( AF_INET, address_out.c_str(), &(addr.s_addr) );
				if( ret )
				{
					//Update our parent service browser
					pthread_mutex_lock( &(browser->mMapLock) );
					browser->mServiceLookup[service->name] = addr;
					pthread_mutex_unlock( &(browser->mMapLock) );
		
					//Fill out a player message
					LeapFrog::Brio::tRemotePlayer player(service->name, addr);
					LeapFrog::Brio::CRemotePlayerMessage msg(player, true);
	
					//Post Message
					browser->mEvent.PostEvent(msg, kPlayerChangeEventPriority);
				}
				else
				{
					browser->mDebug.DebugOut(LeapFrog::Brio::kDbgLvlImportant, "Failed to parse inet address from address string\n");
				}
			}
			catch(DBus::Error& err)
			{
				browser->mDebug.DebugOut(LeapFrog::Brio::kDbgLvlImportant, "DBus error: %s\n", err.what());
			}
			delete service;
		}
	}
}

std::map<std::string, struct in_addr> ServiceBrowser::GetResolvedServices()
{
	std::map<std::string, struct in_addr> services;
	pthread_mutex_lock( &mMapLock );
	services = mServiceLookup;
	pthread_mutex_unlock( &mMapLock );
	return services;
}

} } } // End namespace
