/*
Copyright (C) 2004 by Matthias Braun <matze@braunis.de>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#ifndef __SOCKETBASE_HPP__
#define __SOCKETBASE_HPP__

#include <iostream>
#include <stdexcept>
#include "NetworkException.hpp"
#include "SocketHeaders.hpp"
#include "Address.hpp"
#include "Util/NoCopy.hpp"

namespace network
{

/** Base class for sockets, this is intended for internal use only */
class SocketBase : public NoCopy
{
public:
    const Address& getAddress() const
    {
        return addr;
    }

    bool isConnecting() { return _isConnecting; };
protected:
    friend class SocketSet;
    friend class SocketManager;

    virtual ~SocketBase();
    
    SocketBase(const Address &a, bool isTcp) throw(NetworkException);
    SocketBase(SOCKET fd, const Address &a) throw(NetworkException);
        
    virtual void onDataReady() = 0;
    virtual void onDisconected() {};
    virtual void onConnected() {};
    virtual void onSocketError() = 0;
    virtual void destroy() = 0;
    

    void setReuseAddr() throw(NetworkException);
    void doClose();
    
    void bindSocketTo(const Address& toaddr) throw(NetworkException);
    void bindSocket() throw(NetworkException) { bindSocketTo(addr); };
    void doListen() throw(NetworkException);
    void doConnect() throw(NetworkException);
    int  doSend(const void* data, size_t len) throw(NetworkException);
    int  doReceive(void* buffer, size_t len) throw(NetworkException);
    int  doSendTo(const Address& toaddr, const void* data, size_t len) throw(NetworkException);
    size_t  doReceiveFrom(Address& fromaddr, void* buffer, size_t len) throw(NetworkException);
    SOCKET doAccept(Address& fromaddr) throw(NetworkException);
    
private:
    void create(bool tcp) throw(NetworkException);
    void setNonBlocking() throw(NetworkException);
    
    void connectionFinished() 
    {
        _isConnecting = false;
        onConnected();
    };
    
    bool _isConnecting;
    SOCKET sockfd;
    Address addr;
    int lastError;
};

}

#endif

