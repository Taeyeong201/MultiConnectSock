#pragma once

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
	virtual int writeHandle(BufferPacket&) = 0;

	int objectID;
protected:
	static int objID_Creater;
};

class Channel {
public:
	void joinSession(boost::shared_ptr<BaseSession>);

	void registerDisConnectAllSession(boost::function<void()> f);

	void disconnectSession(boost::shared_ptr<BaseSession> objID);

	void closeAllSocket();

	int write(char* buf, std::size_t size);
	int read(char* buf, const std::size_t& size);

	std::set<boost::shared_ptr<BaseSession>> sessions_;
private:

private:
	std::queue<std::shared_ptr<BufferPacket>> inBufferQueue;

	boost::function<void()> disconnectAll;
	int SocketActivateCount;
};


class Session :
	public BaseSession,
	public boost::enable_shared_from_this<Session> {
public:
	Session(
		boost::asio::ip::tcp::socket sock,
		boost::asio::io_context& ioc,
		Channel& channel,
		std::size_t maxbuf);
	~Session();

	void disconect();

	boost::asio::ip::tcp::socket socket_;

	int writeHandle(BufferPacket&) override;
	int recv(BufferPacket&);

private:
	boost::asio::io_context _ioc;

	Channel& channel_;
};

