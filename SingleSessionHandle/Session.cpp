#include "Session.h"

static std::mutex g_Mutex;
static void boostErrorHandler(const char* BeforeFucName, const int BeforeFucLine, const boost::system::error_code& ec)
{
	g_Mutex.lock();
	std::cerr << "{" << std::endl;
	std::cerr << "      Issue Func Name at : " << BeforeFucName << std::endl;
	std::cerr << "            Func Line at : " << BeforeFucLine << std::endl;
	std::cerr << "                 name    :  " << ec.category().name() << std::endl;
	std::cerr << "                 value   :  " << ec.value() << std::endl;
	std::cerr << "                 message :  " << ec.message() << std::endl;
	std::cerr << "}" << std::endl;
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
	channel_.disconnectSession(shared_from_this());
	//socket_.shutdown(boost::asio::socket_base::shutdown_both);
	//socket_.close();
}

std::size_t Session::writeHandle(const BufferPacket& buffer)
{
	boost::system::error_code ec;

	BufferPacket tmp(buffer.buf_, buffer.size_);

	std::size_t writeSize = socket_.write_some(boost::asio::buffer(
		tmp.buf_, buffer.size_), ec);

	if (!ec) {
		std::cerr << __FUNCTION__ << "@" << __LINE__ << "write Size : " << writeSize << std::endl;
	}
	else {
		std::cerr << "Session error ID : " << objectID << std::endl;
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
		std::cerr << __FUNCTION__ << "@" << __LINE__ << "read Size : " << readSize << std::endl;
	}
	else {
		std::cerr << "Session error ID : " << objectID << std::endl;
		boostErrorHandler(__FUNCTION__, __LINE__, ec);
		disconnect();
	}
	return readSize;
}