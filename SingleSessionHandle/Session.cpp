#include "Session.h"

static std::mutex g_Mutex;
static void boostErrorHandler(const char* BeforeFucName, const int BeforeFucLine, const boost::system::error_code& ec)
{
	g_Mutex.lock();
	std::cout << "{" << std::endl;
	std::cout << "      Issue Func Name at : " << BeforeFucName << std::endl;
	std::cout << "            Func Line at : " << BeforeFucLine << std::endl;
	std::cout << "                 name    :  " << ec.category().name() << std::endl;
	std::cout << "                 value   :  " << ec.value() << std::endl;
	std::cout << "                 message :  " << ec.message() << std::endl;
	std::cout << "}" << std::endl;
	g_Mutex.unlock();
}

Session::Session(
	boost::asio::ip::tcp::socket& sock,
	boost::asio::io_context& ioc,
	Channel& channel)
	: channel_(channel), socket_(std::move(sock)), ioc_(ioc)
{
}

Session::~Session()
{
	if (socket_.is_open())
		socket_.close();
	std::cout << "Session Close obj_id=" << objectID << std::endl;
}

void Session::start()
{
	channel_.joinSession(shared_from_this());
}

void Session::disconnect()
{
	if (socket_.is_open())
		socket_.close();
	channel_.disconnectSession(shared_from_this());
	//socket_.shutdown(boost::asio::socket_base::shutdown_both);
	//socket_.close();
}

std::size_t Session::writeHandle(const BufferPacket& buffer)
{
	boost::system::error_code ec;

	BufferPacket tmp(buffer.buf_, buffer.size_);
	//TODO Async
	//ioc_.post(boost::bind(&Session::do_write, this, tmp));
	std::size_t writeSize = socket_.write_some(boost::asio::buffer(
		tmp.buf_, buffer.size_), ec);

	if (!ec) {
		std::cout << __FUNCTION__ << "@" << __LINE__ << "write Size : " << writeSize << std::endl;
	}
	else {
		std::cout << "Session error ID : " << objectID << std::endl;
		boostErrorHandler(__FUNCTION__, __LINE__, ec);
		disconnect();
	}

	return writeSize;
}

std::size_t Session::readHandle(char* buf, const std::size_t& size)
{
	boost::system::error_code ec;

	std::size_t readSize = 
		socket_.read_some(boost::asio::buffer(buf, size), ec);

	if (!ec) {
		std::cout << __FUNCTION__ << "@" << __LINE__ << "read Size : " << readSize << std::endl;
	}
	else {
		std::cout << "Session error ID : " << objectID << std::endl;
		boostErrorHandler(__FUNCTION__, __LINE__, ec);
		disconnect();
	}
	return readSize;
}