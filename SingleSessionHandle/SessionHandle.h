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

	int write(char* buf, std::size_t& size);
	int read(char* buf,const std::size_t& size);

	//void registerRecvCallback(boost::function<void(char*, std::size_t, char)> f);
	void closeAllSocket();
private:
	void disconnectSession(std::shared_ptr<Session> objID);
	void bufQueueChecker();

private:
	friend Session;

	boost::thread_group _IO_Workers;
	boost::asio::io_service::strand recvStrand;
	boost::asio::io_service::strand bufferPopStrand;
	std::shared_ptr<boost::asio::io_service::work> _lock_work;

	std::unique_ptr<boost::asio::ip::tcp::acceptor>	_acceptor;

	std::set<std::shared_ptr<Session>> _vec_socket;
	boost::asio::ip::tcp::endpoint address;

	struct BufferPacket {
		std::size_t size;
		std::shared_ptr<char[]> dataBuf;
		std::atomic<int> cnt;
	};

	boost::function<void(char*, std::size_t, char)> recvCall;

	bool nodelay = false;

	std::size_t maxBufferSize;
	char* sendBuffer;

	std::queue<std::shared_ptr<BufferPacket>> OutBufferQueue;

	boost::mutex sessionMutex;

	int SocketActivateCount;
	static boost::asio::io_context IOCtx;
};