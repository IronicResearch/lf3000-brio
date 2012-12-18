/*
 *
 *  D-Bus++ - C++ bindings for D-Bus
 *
 *  Copyright (C) 2012  Sojan James <sojan.james@gmail.com>
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

/*
 * The InterfaceDispatcher class is the path through which calls on the
 * Proxy or Adaptor object are called. Signals also pass through this
 * class.
 *
 * This class can be overridden to have a different behaviour. The calls
 * can be queued and made from a different thread context, the thread can belong
 * to a thread pool. One way would be to bind the call into a functor object,
 * queue it and call in a different context.
 *
 * I introduced this class as a precursor to using the boost::asio framework
 * to dispatch, without changing much of the core logic.
 *
 */
#ifndef INTERFACE_DISPATCHER_H_
#define INTERFACE_DISPATCHER_H_

#include "api.h"
#include "util.h"
#include "types.h"
#include "message.h"
#include "pendingcall.h"

namespace DBus
{

///////////////////////////


class DXXAPIPUBLIC InterfaceDispatcher
{
public:
	virtual bool dispatch_message(Slot<void, const DBus::CallMessage &> &slot, const DBus::CallMessage &msg)
	{
		//The default dispatcher just calls the slot in the context of the caller
		slot.call(msg);
		return true;
	}
	virtual bool dispatch_signal(Slot<void, const SignalMessage &> &slot, const SignalMessage &msg)
	{
		//The default dispatcher just calls the slot in the context of the caller
		slot.call(msg);
		return true;
	}

	virtual bool dispatch_pending_call(PendingCall *pcall)
	{
		//The default dispatcher just calls the slot in the context of the caller
		(pcall->slot()).call(pcall);
		return true;
	}

	virtual ~InterfaceDispatcher() {;}
};
} //namespace DBus

extern DBus::InterfaceDispatcher default_interface_dispatcher;

#endif /* INTERFACE_DISPATCHER_H_ */
