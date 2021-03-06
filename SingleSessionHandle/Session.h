#pragma once

#include "Channel.h"
//#include "VideoChannel.h"

class Session :
	public BaseSession,
	public boost::enable_shared_from_this<Session> {
public:
	Session(
		boost::asio::ip::tcp::socket& sock,
		boost::asio::io_context& ioc,
		Channel& channel);
	~Session();

	void start();

	boost::asio::ip::tcp::socket socket_;

	std::size_t writeHandle(const BufferPacket&, boost::system::error_code&) override;
	std::size_t readHandle(char*, const std::size_t&) override;
	void disconnect() override;

protected:
	boost::asio::io_context& ioc_;

	Channel& channel_;
};
