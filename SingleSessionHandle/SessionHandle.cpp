#include "SessionHandle.h"

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

boost::asio::io_context SessionHandle::IOCtx;

SessionHandle::SessionHandle()
	:
	_lock_work(std::make_shared<boost::asio::io_context::work>(IOCtx)),
	channel_(IOCtx)/*, videoChannel_(IOCtx)*/
{
	for (int i = 0; i < 4; i++)
		_IO_Workers.create_thread(boost::bind(&boost::asio::io_context::run, &IOCtx));
}

SessionHandle::~SessionHandle()
{
	_lock_work.reset();
	IOCtx.stop();
	_IO_Workers.join_all();
}

int SessionHandle::initServerMode(const unsigned short port)
{
	address = boost::asio::ip::tcp::endpoint(
		boost::asio::ip::tcp::v4(), port);
	_acceptor = std::make_unique<boost::asio::ip::tcp::acceptor>(IOCtx, address);

	return 0;
}

int SessionHandle::initClientMode(const std::string& serverIP, const unsigned short port)
{
	address = boost::asio::ip::tcp::endpoint(
		boost::asio::ip::address::from_string(serverIP), port);

	return 0;
}

void SessionHandle::initNodelayOpt(bool onoff)
{
	nodelay_ = onoff;
}

//void SessionHandle::initVideoChannel(bool onoff)
//{
//	isVideoChannel_ = onoff;
//}

void SessionHandle::acceptor()
{
	boost::asio::ip::tcp::socket socket(IOCtx);
	boost::system::error_code ec;

	_acceptor->accept(socket, ec);
	if (!ec) {
		std::cout << "Client Accept success" << std::endl;
		std::cout << "Local Endpoint: "
			<< socket.local_endpoint().address().to_string() << ", "
			<< socket.local_endpoint().port() << std::endl;
		std::cout << "Remote Endpoint: "
			<< socket.remote_endpoint().address().to_string() << ", "
			<< socket.remote_endpoint().port() << std::endl;

		socket.set_option(boost::asio::ip::tcp::no_delay(nodelay_));
		boost::shared_ptr<Session> session;
		//if (isVideoChannel_)
		//	session = boost::shared_ptr<Session>(new Session(socket, IOCtx, videoChannel_));
		//else
			session = boost::shared_ptr<Session>(new Session(socket, IOCtx, channel_));
		session->start();

	}
	else {
		boostErrorHandler(__FUNCTION__, __LINE__, ec);
	}
}

void SessionHandle::async_acceptor()
{
	_acceptor->async_accept([this](
		boost::system::error_code ec,
		boost::asio::ip::tcp::socket socket) {
			if (!ec) {
				std::cout << "Client Accept success" << std::endl;
				std::cout << "Local Endpoint: "
					<< socket.local_endpoint().address().to_string() << ", "
					<< socket.local_endpoint().port() << std::endl;
				std::cout << "Remote Endpoint: "
					<< socket.remote_endpoint().address().to_string() << ", "
					<< socket.remote_endpoint().port() << std::endl;

				socket.set_option(boost::asio::ip::tcp::no_delay(nodelay_));
				boost::shared_ptr<Session> session;
				//if (isVideoChannel_)
				//	session = boost::shared_ptr<Session>(new Session(socket, IOCtx, videoChannel_));
				//else
					session = boost::shared_ptr<Session>(new Session(socket, IOCtx, channel_));
				session->start();

				async_acceptor();
			}
			else {
				boostErrorHandler(__FUNCTION__, __LINE__, ec);
			}
		});
}

void SessionHandle::connect()
{
	boost::asio::ip::tcp::socket socket(IOCtx);
	boost::system::error_code ec;

	socket.connect(address, ec);
	if (!ec) {
		std::cout << "Server Connect success" << std::endl;
		std::cout << "Local Endpoint: "
			<< socket.local_endpoint().address().to_string() << ", "
			<< socket.local_endpoint().port() << std::endl;
		std::cout << "Remote Endpoint: "
			<< socket.remote_endpoint().address().to_string() << ", "
			<< socket.remote_endpoint().port() << std::endl;

		socket.set_option(boost::asio::ip::tcp::no_delay(nodelay_));
		boost::shared_ptr<Session> session(new Session(socket, IOCtx, channel_));
		session->start();
	}
	else {
		boostErrorHandler(__FUNCTION__, __LINE__, ec);
	}
}

void SessionHandle::close()
{
	channel_.closeAllSession();
}