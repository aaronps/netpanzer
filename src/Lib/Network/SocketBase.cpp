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
#include <config.h>

#include <sstream>
#include <stdexcept>

#include "SocketHeaders.hpp"
#include "SocketBase.hpp"

namespace network
{

#ifdef USE_WINSOCK
class WinSockInit {
public:
    WinSockInit() {
        WSADATA wsaData;
        WORD wVers = MAKEWORD(2, 0);
        int rc = WSAStartup(wVers, &wsaData);
        if(rc != 0) {
	    fprintf(stderr, "Failed to initialize winsock: %d\n", rc);
	    exit(1);
        }
    }

    ~WinSockInit() {
	WSACleanup();
    }
};
WinSockInit _WinSockInit;
#endif

SocketBase::SocketBase()
{
}

SocketBase::~SocketBase()
{
    close();
}

void
SocketBase::create(bool tcp)
{
    if(tcp)
        sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    else
        sockfd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);

#ifdef USE_WINSOCK
    if(sockfd == INVALID_SOCKET) {
	std::stringstream msg;
	msg << "Couldn't create socket: " << WSAGetLastError();
	throw std::runtime_error(msg.str());
    }
#else
    if(sockfd < 0) {
        std::stringstream msg;
        msg << "Couldn't create socket: " << strerror(errno);
        throw std::runtime_error(msg.str());
    }
#endif
}

void
SocketBase::setNonBlocking()
{
#ifdef USE_WINSOCK
    unsigned long mode = 1;
    if(ioctlsocket(sockfd, FIONBIO, &mode) != 0) {
	std::stringstream msg;
	msg << "Couldn't set nonblocking mode: " << WSAGetLastError();
	throw std::runtime_error(msg.str());
    }
#else
    if(fcntl(sockfd, F_SETFL, O_NONBLOCK) < 0) {
        std::stringstream msg;
        msg << "Couldn't set socket to nonblocking mode: " << strerror(errno);
        throw std::runtime_error(msg.str());
    }
#endif
}

void
SocketBase::close()
{
#ifdef USE_WINSOCK
    closesocket(sockfd);
#else
    ::close(sockfd);
#endif
}

}

