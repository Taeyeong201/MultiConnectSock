#pragma once
#define WINVER 0x0A00
#define _WIN32_WINNT 0x0A00

#include <iostream>
#include <memory>
#include <queue>
#include <set>
#include <atomic>
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/thread/scoped_thread.hpp>
#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>

#include "Session.h"


class SessionHandleServer
{
public:
	SessionHandleServer();
	~SessionHandleServer();

	int initServerMode(const unsigned short port, std::size_t maxBufSize);
	int initClientMode(const std::string& serverIP, const unsigned short port, std::size_t maxBufSize);

	void initNodelayOpt(bool onoff);

	void acceptor();
	void async_acceptor();

	void connect();

	void close();

private:
	friend Session;

	boost::thread_group _IO_Workers;
	//boost::asio::io_service::strand recvStrand;
	//boost::asio::io_service::strand bufferPopStrand;
	std::shared_ptr<boost::asio::io_service::work> _lock_work;

	std::unique_ptr<boost::asio::ip::tcp::acceptor>	_acceptor;
	boost::asio::ip::tcp::endpoint address;

	bool nodelay = false;

	std::size_t maxBufferSize;

	Channel channel;

	static boost::asio::io_context IOCtx;
};