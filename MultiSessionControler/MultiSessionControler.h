#pragma once
#define WINVER 0x0A00
#define _WIN32_WINNT 0x0A00

#include <iostream>
#include <memory>
#include <queue>
//#include <vector>
#include <set>
#include <atomic>
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/thread/scoped_thread.hpp>
#include <boost/enable_shared_from_this.hpp>
//#include <boost/bind.hpp>
#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>

class MultiSessionControler
{
	class Session /*: public boost::enable_shared_from_this<Session>*/ {
	public:
		Session(
			boost::asio::ip::tcp::socket sock,
			MultiSessionControler* handle,
			std::size_t maxbuf);
		~Session();

		void registerDisconnectCallBack(boost::function<void(int)> f);
		void registerBufPopCallBack(boost::function<void()> f);
		//void registerRecvCallBack(boost::function<void(int)> f);

		//void init(MultiSessionControler* handle);

		void disconect();

		bool operator==(const Session& rhs);
		bool operator==(const int& rhs);

		boost::asio::ip::tcp::socket _socket;

		int send();
		int recv(char* buf,const std::size_t& size);

		bool isConnect;
		int objectID;

		boost::asio::io_service::strand taskStrand;

	private:
		boost::thread recvWork;

		boost::function<void(int)> disconnectSessionCall;
		boost::function<void()> bufferPopCall;
		MultiSessionControler* handle;

		static int objID_Creater;

		//void bufQueueChecker();
	};

public:
	MultiSessionControler();
	~MultiSessionControler();

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
	void disconnectSession(int objID);
	void bufQueueChecker();

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
		std::shared_ptr<char[]> dataBuf;
		std::atomic<int> cnt;
	};

	boost::function<void(char*, std::size_t, char)> recvCall;

	bool nodelay = false;

	std::size_t maxBufferSize;
	char* sendBuffer;

	std::queue<std::shared_ptr<BufferPacket>> OutBufferQueue;
	//std::queue<std::shared_ptr<BufferPacket>> InBufferQueue;
	//std::queue<std::shared_ptr<BufferPacket>> InternalBufferQueue;

	boost::mutex sessionMutex;

	//boost::mutex sendMutex;
	//boost::condition_variable sendConditionVar;
	//boost::mutex recvMutex;
	//boost::condition_variable recvConditionVar;
	//boost::mutex internalMutex;
	//boost::condition_variable internalConditionVar;

	int SocketActivateCount;
	//std::atomic<int> sendCnt;
	static boost::asio::io_context IOCtx;
};