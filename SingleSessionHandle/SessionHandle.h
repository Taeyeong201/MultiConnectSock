#pragma once

#include "Session.h"

class SessionHandle
{
public:
	SessionHandle();
	~SessionHandle();

	int initServerMode(const unsigned short port);
	int initClientMode(const std::string& serverIP, const unsigned short port);

	void initNodelayOpt(bool onoff);

	void acceptor();
	void async_acceptor();

	void connect();

	void close();

	Channel channel_;

private:
	boost::thread_group _IO_Workers;
	std::shared_ptr<boost::asio::io_service::work> _lock_work;

	std::unique_ptr<boost::asio::ip::tcp::acceptor>	_acceptor;
	boost::asio::ip::tcp::endpoint address;

	bool nodelay_ = false;

	static boost::asio::io_context IOCtx;
};