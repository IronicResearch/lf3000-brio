
/*
 *	This file was automatically generated by dbusxx-xml2cpp; DO NOT EDIT!
 */

#ifndef __dbusxx__connman_manager_h__PROXY_MARSHAL_H
#define __dbusxx__connman_manager_h__PROXY_MARSHAL_H

#include <dbus-c++/dbus.h>
#include <cassert>

namespace net {
namespace connman {

class Manager_proxy
: public ::DBus::InterfaceProxy
{
public:

    Manager_proxy()
    : ::DBus::InterfaceProxy("net.connman.Manager")
    {
        
    }

public:

    /* properties exported by this interface */
public:
    void EnableSignals()
    {
    	connect_signal(Manager_proxy, PropertyChanged, _PropertyChanged_stub);
        connect_signal(Manager_proxy, TechnologyAdded, _TechnologyAdded_stub);
        connect_signal(Manager_proxy, TechnologyRemoved, _TechnologyRemoved_stub);
        connect_signal(Manager_proxy, ServicesAdded, _ServicesAdded_stub);
        connect_signal(Manager_proxy, ServicesRemoved, _ServicesRemoved_stub);
    }

    /* methods exported by this interface,
     * this functions will invoke the corresponding methods on the remote objects
     */
    std::vector< ::DBus::Struct< std::string, ::DBus::Variant > > GetProperties()
    {
        ::DBus::CallMessage call;
        call.member("GetProperties");
        ::DBus::Message ret = invoke_method (call);
        ::DBus::MessageIter ri = ret.reader();

        std::vector< ::DBus::Struct< std::string, ::DBus::Variant > > argout;
        ri >> argout;
        return argout;
    }

    void SetProperty(const std::string& argin0, const ::DBus::Variant& argin1)
    {
        ::DBus::CallMessage call;
        ::DBus::MessageIter wi = call.writer();

        wi << argin0;
        wi << argin1;
        call.member("SetProperty");
        ::DBus::Message ret = invoke_method (call);
    }

    std::vector< ::DBus::Struct< ::DBus::Path, std::map< std::string, ::DBus::Variant > > > GetTechnologies()
    {
        ::DBus::CallMessage call;
        call.member("GetTechnologies");
        ::DBus::Message ret = invoke_method (call);
        ::DBus::MessageIter ri = ret.reader();

        std::vector< ::DBus::Struct< ::DBus::Path, std::map< std::string, ::DBus::Variant > > > argout;
        ri >> argout;
        return argout;
    }

    void RemoveProvider(const ::DBus::Path& argin0)
    {
        ::DBus::CallMessage call;
        ::DBus::MessageIter wi = call.writer();

        wi << argin0;
        call.member("RemoveProvider");
        ::DBus::Message ret = invoke_method (call);
    }

    std::vector< std::vector< ::DBus::Struct< ::DBus::Path, std::map< std::string, ::DBus::Variant > > > > GetServices()
    {
        ::DBus::CallMessage call;
        call.member("GetServices");
        ::DBus::Message ret = invoke_method (call);
        ::DBus::MessageIter ri = ret.reader();

        std::vector< std::vector< ::DBus::Struct< ::DBus::Path, std::map< std::string, ::DBus::Variant > > > > argout;
        ri >> argout;
        return argout;
    }

    ::DBus::Path ConnectProvider(const std::vector< ::DBus::Struct< std::string, ::DBus::Variant > >& argin0)
    {
        ::DBus::CallMessage call;
        ::DBus::MessageIter wi = call.writer();

        wi << argin0;
        call.member("ConnectProvider");
        ::DBus::Message ret = invoke_method (call);
        ::DBus::MessageIter ri = ret.reader();

        ::DBus::Path argout;
        ri >> argout;
        return argout;
    }

    void RegisterAgent(const ::DBus::Path& argin0)
    {
        ::DBus::CallMessage call;
        ::DBus::MessageIter wi = call.writer();

        wi << argin0;
        call.member("RegisterAgent");
        ::DBus::Message ret = invoke_method (call);
    }

    void UnregisterAgent(const ::DBus::Path& argin0)
    {
        ::DBus::CallMessage call;
        ::DBus::MessageIter wi = call.writer();

        wi << argin0;
        call.member("UnregisterAgent");
        ::DBus::Message ret = invoke_method (call);
    }

    void RegisterCounter(const ::DBus::Path& argin0, const uint32_t& argin1, const uint32_t& argin2)
    {
        ::DBus::CallMessage call;
        ::DBus::MessageIter wi = call.writer();

        wi << argin0;
        wi << argin1;
        wi << argin2;
        call.member("RegisterCounter");
        ::DBus::Message ret = invoke_method (call);
    }

    void UnregisterCounter(const ::DBus::Path& argin0)
    {
        ::DBus::CallMessage call;
        ::DBus::MessageIter wi = call.writer();

        wi << argin0;
        call.member("UnregisterCounter");
        ::DBus::Message ret = invoke_method (call);
    }

    ::DBus::Path CreateSession(const std::vector< ::DBus::Struct< std::string, ::DBus::Variant > >& argin0, const ::DBus::Path& argin1)
    {
        ::DBus::CallMessage call;
        ::DBus::MessageIter wi = call.writer();

        wi << argin0;
        wi << argin1;
        call.member("CreateSession");
        ::DBus::Message ret = invoke_method (call);
        ::DBus::MessageIter ri = ret.reader();

        ::DBus::Path argout;
        ri >> argout;
        return argout;
    }

    void DestroySession(const ::DBus::Path& argin0)
    {
        ::DBus::CallMessage call;
        ::DBus::MessageIter wi = call.writer();

        wi << argin0;
        call.member("DestroySession");
        ::DBus::Message ret = invoke_method (call);
    }

    void ReleasePrivateNetwork(const ::DBus::Path& argin0)
    {
        ::DBus::CallMessage call;
        ::DBus::MessageIter wi = call.writer();

        wi << argin0;
        call.member("ReleasePrivateNetwork");
        ::DBus::Message ret = invoke_method (call);
    }


public:

    /* signal handlers for this interface
     */
    virtual void PropertyChanged(const std::string& argin0, const ::DBus::Variant& argin1) = 0;
    virtual void TechnologyAdded(const ::DBus::Path& argin0, const std::vector< ::DBus::Struct< std::string, ::DBus::Variant > >& argin1) = 0;
    virtual void TechnologyRemoved(const ::DBus::Path& argin0) = 0;
    virtual void ServicesAdded(const std::vector< ::DBus::Struct< ::DBus::Path, std::vector< ::DBus::Struct< std::string, ::DBus::Variant > > > >& argin0) = 0;
    virtual void ServicesRemoved(const std::vector< ::DBus::Path >& argin0) = 0;

private:

    /* unmarshalers (to unpack the DBus message before calling the actual signal handler)
     */
    void _PropertyChanged_stub(const ::DBus::SignalMessage &sig)
    {
        ::DBus::MessageIter ri = sig.reader();

        std::string arg0;
        ri >> arg0;
        ::DBus::Variant arg1;
        ri >> arg1;
        PropertyChanged(arg0, arg1);
    }
    void _TechnologyAdded_stub(const ::DBus::SignalMessage &sig)
    {
        ::DBus::MessageIter ri = sig.reader();

        ::DBus::Path arg0;
        ri >> arg0;
        std::vector< ::DBus::Struct< std::string, ::DBus::Variant > > arg1;
        ri >> arg1;
        TechnologyAdded(arg0, arg1);
    }
    void _TechnologyRemoved_stub(const ::DBus::SignalMessage &sig)
    {
        ::DBus::MessageIter ri = sig.reader();

        ::DBus::Path arg0;
        ri >> arg0;
        TechnologyRemoved(arg0);
    }
    void _ServicesAdded_stub(const ::DBus::SignalMessage &sig)
    {
        ::DBus::MessageIter ri = sig.reader();

        std::vector< ::DBus::Struct< ::DBus::Path, std::vector< ::DBus::Struct< std::string, ::DBus::Variant > > > > arg0;
        ri >> arg0;
        ServicesAdded(arg0);
    }
    void _ServicesRemoved_stub(const ::DBus::SignalMessage &sig)
    {
        ::DBus::MessageIter ri = sig.reader();

        std::vector< ::DBus::Path > arg0;
        ri >> arg0;
        ServicesRemoved(arg0);
    }
};

class Manager :
	public Manager_proxy,
	public DBus::ObjectProxy
{
public:
	Manager(DBus::Connection& conn) :
		DBus::ObjectProxy(conn, "/", "net.connman")
	{}
	
	void PropertyChanged(const std::string& argin0, const ::DBus::Variant& argin1)
	{}
	
	void TechnologyAdded(const ::DBus::Path& argin0, const std::vector< ::DBus::Struct< std::string, ::DBus::Variant > >& argin1)
	{}
	
	void TechnologyRemoved(const ::DBus::Path& argin0)
	{}
	
	void ServicesAdded(const std::vector< ::DBus::Struct< ::DBus::Path, std::vector< ::DBus::Struct< std::string, ::DBus::Variant > > > >& argin0)
	{}
	
	void ServicesRemoved(const std::vector< ::DBus::Path >& argin0)
	{}
};

typedef std::vector< ::DBus::Struct< ::DBus::Path, std::map< std::string, ::DBus::Variant > > > TechnologyArray;

} } 
#endif //__dbusxx__connman_manager_h__PROXY_MARSHAL_H