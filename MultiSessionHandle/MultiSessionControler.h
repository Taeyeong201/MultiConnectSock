#pragma once

#include <iostream>
#include <memory>
#include <queue>
#include <vector>
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>

class MultiSessionControler
{
	class Session {
	public:
		Session(boost::asio::ip::tcp::socket sock);
		~Session();

		void registerDisconnectCallBack(boost::function<void(int)> f);

		void init(MultiSessionControler* handle);

		void disconect();

		int send();
		int recv();

		bool operator==(const Session& rhs);
		bool operator==(const int& rhs);
		
		boost::asio::ip::tcp::socket _socket;

		std::thread worker;

		bool isConnect;
		int objectID;

	private:
		static int objID_Creater;
		boost::function<void(int)> disconnectSession;

		MultiSessionControler* handle;
	};

public:
	MultiSessionControler();
	~MultiSessionControler();

	int initServerMode(const int port, unsigned int maxBufSize);
	int initClient(const std::string& serverIP, const int port, unsigned int maxBufSize);

	void acceptor();
	void async_acceptor();

	void connect();

	int write(char* buf, unsigned int size);
	int read(char* buf, unsigned int size);

	void disconnectSession(int objID);

private:
	friend Session;

	std::thread _IO_Worker;
	std::shared_ptr<boost::asio::io_service::work> _lock_work;

	std::unique_ptr<boost::asio::ip::tcp::acceptor>	_acceptor;

	std::vector<std::unique_ptr<Session>> _vec_socket;
	boost::asio::ip::tcp::endpoint address;

	struct BufferPacket {
		unsigned int size;
		char* buf;
	};

	std::queue<std::shared_ptr<BufferPacket>> InBufferQueue;
	std::queue<std::shared_ptr<BufferPacket>> OutBufferQueue;

	boost::mutex sendMutex;
	boost::condition_variable sendConditionVar;
	boost::mutex recvMutex;
	boost::condition_variable recvConditionVar;


	static boost::asio::io_context IOCtx;
	static int SocketActivateCount;
};