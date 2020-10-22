#pragma once
#define WINVER 0x0A00
#define _WIN32_WINNT 0x0A00

#include <memory>
#include <iostream>
#include <queue>
#include <set>

#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>

#include "BufferPacket.h"

class BaseSession {
public:
	BaseSession();
	virtual std::size_t writeHandle(const BufferPacket&) = 0;
	virtual std::size_t readHandle(char*, const std::size_t&) = 0;
	virtual void disconnect() = 0;

	int objectID;
protected:
	static int objID_Creater;
};

class Channel {
public:
	void joinSession(boost::shared_ptr<BaseSession>);

	void registerDisConnectAllSession(boost::function<void()> f);

	void disconnectSession(boost::shared_ptr<BaseSession> objID);
	void closeAllSession();

	int write(char* buf, std::size_t size);
	int read(char* buf, const std::size_t& size);

	int countAliveSocket();

	std::set<boost::shared_ptr<BaseSession>> sessions_;

private:
	//TODO MultiSession
	std::queue<std::shared_ptr<BufferPacket>> inBufferQueue;

	boost::function<void()> disconnectAll;
	int SocketActivateCount;
};
