
/*
 *	This file was automatically generated by dbusxx-xml2cpp; DO NOT EDIT!
 */

#ifndef __dbusxx__wpa_supplicant_Network_h__PROXY_MARSHAL_H
#define __dbusxx__wpa_supplicant_Network_h__PROXY_MARSHAL_H

#include <dbus-c++/dbus.h>
#include <cassert>

namespace fi {
namespace w1 {
namespace wpa_supplicant1 {

class Network_proxy
: public ::DBus::InterfaceProxy
{
public:

    Network_proxy()
    : ::DBus::InterfaceProxy("fi.w1.wpa_supplicant1.Network")
    {
        
    }

public:

    /* properties exported by this interface */
        const std::map< std::string, ::DBus::Variant > Properties() {
            ::DBus::CallMessage call ;
             call.member("Get"); call.interface("org.freedesktop.DBus.Properties");
            ::DBus::MessageIter wi = call.writer(); 
            const std::string interface_name = "fi.w1.wpa_supplicant1.Network";
            const std::string property_name  = "Properties";
            wi << interface_name;
            wi << property_name;
            ::DBus::Message ret = this->invoke_method (call);
            ::DBus::MessageIter ri = ret.reader ();
            ::DBus::Variant argout; 
            ri >> argout;
            return argout;
        };
        void Properties( const std::map< std::string, ::DBus::Variant > & input) {
            ::DBus::CallMessage call ;
             call.member("Set");  call.interface( "org.freedesktop.DBus.Properties");
            ::DBus::MessageIter wi = call.writer(); 
            ::DBus::Variant value;
            ::DBus::MessageIter vi = value.writer ();
            vi << input;
            const std::string interface_name = "fi.w1.wpa_supplicant1.Network";
            const std::string property_name  = "Properties";
            wi << interface_name;
            wi << property_name;
            wi << value;
            ::DBus::Message ret = this->invoke_method (call);
        };
        const bool Enabled() {
            ::DBus::CallMessage call ;
             call.member("Get"); call.interface("org.freedesktop.DBus.Properties");
            ::DBus::MessageIter wi = call.writer(); 
            const std::string interface_name = "fi.w1.wpa_supplicant1.Network";
            const std::string property_name  = "Enabled";
            wi << interface_name;
            wi << property_name;
            ::DBus::Message ret = this->invoke_method (call);
            ::DBus::MessageIter ri = ret.reader ();
            ::DBus::Variant argout; 
            ri >> argout;
            return argout;
        };
        void Enabled( const bool & input) {
            ::DBus::CallMessage call ;
             call.member("Set");  call.interface( "org.freedesktop.DBus.Properties");
            ::DBus::MessageIter wi = call.writer(); 
            ::DBus::Variant value;
            ::DBus::MessageIter vi = value.writer ();
            vi << input;
            const std::string interface_name = "fi.w1.wpa_supplicant1.Network";
            const std::string property_name  = "Enabled";
            wi << interface_name;
            wi << property_name;
            wi << value;
            ::DBus::Message ret = this->invoke_method (call);
        };
public:
    void EnableSignals()
    {
    	connect_signal(Network_proxy, PropertiesChanged, _PropertiesChanged_stub);
    }

    /* methods exported by this interface,
     * this functions will invoke the corresponding methods on the remote objects
     */

public:

    /* signal handlers for this interface
     */
    virtual void PropertiesChanged(const std::map< std::string, ::DBus::Variant >& properties) = 0;

private:

    /* unmarshalers (to unpack the DBus message before calling the actual signal handler)
     */
    void _PropertiesChanged_stub(const ::DBus::SignalMessage &sig)
    {
        ::DBus::MessageIter ri = sig.reader();

        std::map< std::string, ::DBus::Variant > properties;
        ri >> properties;
        PropertiesChanged(properties);
    }
};

class Network :
	public Network_proxy,
	public DBus::ObjectProxy
{
	public:
		Network(DBus::Connection &connection, DBus::Path path) :
			DBus::ObjectProxy(connection, path, "fi.w1.wpa_supplicant1")
		{}
		
		virtual void PropertiesChanged(const std::map< std::string, ::DBus::Variant >& properties)
		{}
};

} } } 
#endif //__dbusxx__wpa_supplicant_Network_h__PROXY_MARSHAL_H