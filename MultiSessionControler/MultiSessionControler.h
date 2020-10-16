#pragma once
//#define WINVER 0x0A00
//#define _WIN32_WINNT 0x0A00

#include <iostream>
#include <memory>
#include <queue>
#include <vector>
#include <atomic>
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/thread/scoped_thread.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>

class MultiSessionControler
{
	class Session {
	public:
		Session(
			boost::asio::ip::tcp::socket sock, 
			MultiSessionControler* handle,
			long maxbuf);
		~Session();

		void registerDisconnectCallBack(boost::function<void(int)> f);
		//void registerRecvCallBack(boost::function<void(int)> f);

		//void init(MultiSessionControler* handle);

		void disconect();

		bool operator==(const Session& rhs);
		bool operator==(const int& rhs);
		
		boost::asio::ip::tcp::socket _socket;

		int send();
		int recv(char* buf, std::size_t size);

		bool isConnect;
		int objectID;

		static std::atomic<int> sendCnt;
		boost::asio::io_service::strand taskStrand;

	private:
		boost::thread recvWork;

		boost::function<void(int)> disconnectSessionCall;
		MultiSessionControler* handle;

		static int objID_Creater;

		void bufQueueChecker();
	};

public:
	MultiSessionControler();
	~MultiSessionControler();

	int initServerMode(const int port, std::size_t maxBufSize);
	int initClient(const std::string& serverIP, const int port, std::size_t maxBufSize);

	void acceptor();
	void async_acceptor();

	void connect();

	int write(char* buf, std::size_t size);
	int read(char* buf, std::size_t size);

	void registerRecvCallback(boost::function<void(char*, std::size_t, char)> f);

	void disconnectSession(int objID);
private:
	void bufferQueueCheck();

private:
	friend Session;

	boost::thread_group _IO_Workers;
	boost::asio::io_service::strand recvStrand;
	boost::asio::io_service::strand bufferPopStrand;
	std::shared_ptr<boost::asio::io_service::work> _lock_work;

	std::unique_ptr<boost::asio::ip::tcp::acceptor>	_acceptor;

	std::vector<std::shared_ptr<Session>> _vec_socket;
	boost::asio::ip::tcp::endpoint address;

	struct BufferPacket {
		std::size_t size;
		char* buf;
		char whoami;
	};

	boost::function<void(char*, std::size_t, char)> recvCall;

	std::size_t maxBufferSize;
	char* sendBuffer;

	//std::queue<std::shared_ptr<BufferPacket>> InBufferQueue;
	std::queue<std::shared_ptr<BufferPacket>> OutBufferQueue;
	//std::queue<std::shared_ptr<BufferPacket>> InternalBufferQueue;

	boost::mutex sessionMutex;

	//boost::mutex sendMutex;
	//boost::condition_variable sendConditionVar;
	boost::mutex recvMutex;
	boost::condition_variable recvConditionVar;
	//boost::mutex internalMutex;
	//boost::condition_variable internalConditionVar;


	static boost::asio::io_context IOCtx;
	static int SocketActivateCount;
};