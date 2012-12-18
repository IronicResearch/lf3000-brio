/*
 *
 *  D-Bus++ - C++ bindings for D-Bus
 *
 *  Copyright (C) 2005-2007  Paolo Durante <shackan@gmail.com>
 *
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */


#ifndef __DBUSXX_INTERFACE_H
#define __DBUSXX_INTERFACE_H

#include <string>
#include <map>
#include "api.h"
#include "util.h"
#include "types.h"
#include "message.h"

#include "interface-dispatcher.h"

namespace DBus
{

//todo: this should belong to to properties.h
struct DXXAPI PropertyData
{
  bool		read;
  bool		write;
  std::string	sig;
  Variant		value;
};

typedef std::map<std::string, PropertyData>	PropertyTable;

class IntrospectedInterface;

class ObjectAdaptor;
class InterfaceAdaptor;
class SignalMessage;

typedef std::map<std::string, InterfaceAdaptor *> InterfaceAdaptorTable;

class DXXAPI AdaptorBase
{
public:

  virtual const ObjectAdaptor *object() const = 0 ;

protected:

  InterfaceAdaptor *find_interface(const std::string &name);

  virtual ~AdaptorBase()
  {}

  virtual void _emit_signal(SignalMessage &) = 0;

  virtual void _send_message(const Message &) = 0;

  InterfaceAdaptorTable _interfaces;
};

/*
*/

class ObjectProxy;
class InterfaceProxy;
class CallMessage;

typedef std::map<std::string, InterfaceProxy *> InterfaceProxyTable;

class DXXAPI ProxyBase
{
public:

  virtual const ObjectProxy *object() const = 0 ;

protected:

  InterfaceProxy *find_interface(const std::string &name);

  virtual ~ProxyBase()
  {}

  virtual Message _invoke_method(CallMessage &) = 0;

  virtual void _invoke_method_async(CallMessage &, PendingCallSlot &) = 0;

  virtual bool _invoke_method_noreply(CallMessage &call) = 0;

  InterfaceProxyTable _interfaces;
};

class DXXAPI Interface
{
public:

  Interface(const std::string &name);

  virtual ~Interface();

  inline const std::string &name() const;

  static DBus::InterfaceDispatcher * override_interface_dispatcher(DBus::InterfaceDispatcher*);

private:

  std::string 	_name;
};

/*
*/

const std::string &Interface::name() const
{
  return _name;
}

/*
*/

typedef std::map< std::string, Slot<Message, const CallMessage &> > MethodTable;
typedef std::map< std::string, Slot<void, const CallMessage &> > AsyncTable;

class DXXAPI InterfaceAdaptor : public Interface, public virtual AdaptorBase
{
public:

  InterfaceAdaptor(const std::string &name);

  Message dispatch_method(const CallMessage &);

  // dispatch a method asynchronously. true return
  // value indicates if message was dispatched
  bool dispatch_method_async(const CallMessage &);

  void emit_signal(const SignalMessage &);

  void send_message(const Message &);

  Variant *get_property(const std::string &name);

  void set_property(const std::string &name, Variant &value);

  virtual IntrospectedInterface *introspect() const
  {
    return NULL;
  }

  static DBus::InterfaceDispatcher * override_dispatcher(DBus::InterfaceDispatcher * new_dispatcher)
  {
	  DBus::InterfaceDispatcher * old_dispatcher =  interface_dispatcher;
	  interface_dispatcher = new_dispatcher;
	  return old_dispatcher;
  }
protected:

  MethodTable	_methods;
  AsyncTable	_async_methods;
  PropertyTable	_properties;

  static DBus::InterfaceDispatcher *interface_dispatcher;
};

/*
*/

typedef std::map< std::string, Slot<void, const SignalMessage &> > SignalTable;

class DXXAPI InterfaceProxy : public Interface, public virtual ProxyBase
{
public:

  InterfaceProxy(const std::string &name);

  Message invoke_method(const CallMessage &);

  void invoke_method_async(const CallMessage &call, PendingCallSlot & slot);

  bool invoke_method_noreply(const CallMessage &call);

  bool dispatch_signal(const SignalMessage &);

  static DBus::InterfaceDispatcher * override_dispatcher(DBus::InterfaceDispatcher * new_dispatcher)
    {
  	  DBus::InterfaceDispatcher * old_dispatcher =  interface_dispatcher;
  	  interface_dispatcher = new_dispatcher;
  	  return old_dispatcher;
    }

  static InterfaceDispatcher *interface_dispatcher;
protected:
  SignalTable	_signals;
};

# define register_method(interface, method, callback) \
	InterfaceAdaptor::_methods[ #method ] = \
		new ::DBus::Callback< interface, ::DBus::Message, const ::DBus::CallMessage &>(this, & interface :: callback);

#define register_method_async(interface, method, callback) \
	InterfaceAdaptor::_async_methods[ #method ] = \
		new ::DBus::Callback< interface, void, const ::DBus::CallMessage &>(this, &interface :: callback);

# define bind_property(variable, type, can_read, can_write) \
	InterfaceAdaptor::_properties[ #variable ].read = can_read; \
	InterfaceAdaptor::_properties[ #variable ].write = can_write; \
	InterfaceAdaptor::_properties[ #variable ].sig = type; \
	variable.bind(InterfaceAdaptor::_properties[ #variable ]);

# define connect_signal(interface, signal, callback) \
	InterfaceProxy::_signals[ #signal ] = \
		new ::DBus::Callback< interface, void, const ::DBus::SignalMessage &>(this, & interface :: callback);

} /* namespace DBus */

#endif//__DBUSXX_INTERFACE_H
