#pragma once

#include <memory>
#include <iostream>

#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>

class BaseSession {

};

class Session : public boost::enable_shared_from_this<Session> {
public:
	Session(
		boost::asio::ip::tcp::socket sock,
		//SessionControler* handle,
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
	int recv(char* buf, const std::size_t& size);

	bool isConnect;
	int objectID;

	//boost::asio::io_service::strand taskStrand;

private:
	boost::thread recvWork;

	boost::function<void(int)> disconnectSessionCall;
	boost::function<void()> bufferPopCall;
	//SessionControler* handle;

	static int objID_Creater;

	//void bufQueueChecker();
};

