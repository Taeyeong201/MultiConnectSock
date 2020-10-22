#include "SessionHandle.h"

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

boost::asio::io_context SessionHandleServer::IOCtx;

SessionHandleServer::SessionHandleServer()
	:
	recvStrand(IOCtx),
	bufferPopStrand(IOCtx),
	_lock_work(std::make_shared<boost::asio::io_context::work>(IOCtx)),
	_vec_socket(), sendBuffer(nullptr),
	SocketActivateCount(0), maxBufferSize(0)
{
	for (int i = 0; i < 7; i++)
		_IO_Workers.create_thread(boost::bind(&boost::asio::io_context::run, &IOCtx));

	//_vec_socket.reserve(10);
}

SessionHandleServer::~SessionHandleServer()
{
	_vec_socket.clear();
	_lock_work.reset();
	IOCtx.stop();
	_IO_Workers.join_all();

	delete[] sendBuffer;
}

int SessionHandleServer::initServerMode(const unsigned short port, std::size_t maxBufSize)
{
	address = boost::asio::ip::tcp::endpoint(
		boost::asio::ip::tcp::v4(), port);
	_acceptor = std::make_unique<boost::asio::ip::tcp::acceptor>(IOCtx, address);

	maxBufferSize = maxBufSize;
	sendBuffer = new char((unsigned int)maxBufferSize);
	return 0;
}

int SessionHandleServer::initClientMode(const std::string& serverIP, const unsigned short port, std::size_t maxBufSize)
{
	address = boost::asio::ip::tcp::endpoint(
		boost::asio::ip::address::from_string(serverIP), port);

	maxBufferSize = maxBufSize;
	sendBuffer = new char((unsigned int)maxBufferSize);

	return 0;
}

void SessionHandleServer::initNodelayOpt(bool onoff)
{
	nodelay = onoff;
}

void SessionHandleServer::acceptor()
{
	boost::asio::ip::tcp::socket sock(IOCtx);
	boost::system::error_code ec;

	_acceptor->accept(sock, ec);
	if (!ec) {
		std::cout << "Client Accept success" << std::endl;
		std::cout << "Local Endpoint: "
			<< sock.local_endpoint().address().to_string() << ", "
			<< sock.local_endpoint().port() << std::endl;
		std::cout << "Remote Endpoint: "
			<< sock.remote_endpoint().address().to_string() << ", "
			<< sock.remote_endpoint().port() << std::endl;

		SocketActivateCount++;
		//_vec_socket.insert(std::make_shared<Session>(std::move(sock), this, maxBufferSize));
		////_vec_socket.back()->init(this);
		//_vec_socket.back()->registerDisconnectCallBack(
		//	boost::bind(&SessionHandleServer::disconnectSession, this, boost::placeholders::_1));
		//_vec_socket.back()->registerBufPopCallBack(
		//	boost::bind(&SessionHandleServer::bufQueueChecker, this));
		//_vec_socket.back()->_socket.set_option(boost::asio::ip::tcp::no_delay(nodelay));
	}
	else {
		boostErrorHandler(__FUNCTION__, __LINE__, ec);
	}
}

void SessionHandleServer::async_acceptor()
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

			SocketActivateCount++;
			//_vec_socket.push_back(std::make_shared<Session>(std::move(socket), this, maxBufferSize));
			////_vec_socket.back()->init(this);
			//_vec_socket.back()->registerDisconnectCallBack(
			//	boost::bind(&SessionHandleServer::disconnectSession, this, boost::placeholders::_1));
			//_vec_socket.back()->registerBufPopCallBack(
			//	boost::bind(&SessionHandleServer::bufQueueChecker, this));
			//_vec_socket.back()->_socket.set_option(boost::asio::ip::tcp::no_delay(nodelay));

			std::cout << "Activate Socket = " << SocketActivateCount << std::endl;

			async_acceptor();
		}
		else {
			boostErrorHandler(__FUNCTION__, __LINE__, ec);
		}
	});
}

void SessionHandleServer::connect()
{
	boost::asio::ip::tcp::socket sock(IOCtx);
	boost::system::error_code ec;

	sock.connect(address, ec);
	if (!ec) {
		std::cout << "Server Connect success" << std::endl;
		std::cout << "Local Endpoint: "
			<< sock.local_endpoint().address().to_string() << ", "
			<< sock.local_endpoint().port() << std::endl;
		std::cout << "Remote Endpoint: "
			<< sock.remote_endpoint().address().to_string() << ", "
			<< sock.remote_endpoint().port() << std::endl;

		SocketActivateCount++;
		//_vec_socket.push_back(std::make_shared<Session>(std::move(sock), this, maxBufferSize));
		//_vec_socket.back()->registerDisconnectCallBack(
		//	boost::bind(&SessionHandleServer::disconnectSession, this, boost::placeholders::_1));
		//_vec_socket.back()->registerBufPopCallBack(
		//	boost::bind(&SessionHandleServer::bufQueueChecker, this));
		//_vec_socket.back()->_socket.set_option(boost::asio::ip::tcp::no_delay(nodelay));

		std::cout << "Activate Socket = " << SocketActivateCount << std::endl;
	}
	else {
		boostErrorHandler(__FUNCTION__, __LINE__, ec);
	}
}

int SessionHandleServer::write(char* buf, std::size_t& size)
{
	std::cerr << __FUNCTION__ << "@" << __LINE__ << "buffer write in" << std::endl;
	if (SocketActivateCount < 1) return -1;
	std::shared_ptr<BufferPacket> tmp = std::make_shared<BufferPacket>();
	tmp->size = size;
	tmp->dataBuf.reset(new char[size]);
	std::copy(buf, buf + size, tmp->dataBuf.get());

	tmp->cnt = SocketActivateCount;
	OutBufferQueue.push(tmp);

	int writeFlag = 0;

	//for (auto socket : _vec_socket) {
	//	if (socket->isConnect) {
	//		IOCtx.post(boost::asio::bind_executor(
	//			socket->taskStrand,
	//			boost::bind(&Session::send, socket.get())));
	//		writeFlag++;
	//	}
	//}
	//if (writeFlag < 0) {
	//	return -1;
	//}

	return size;
}

int SessionHandleServer::read(char* buf,const std::size_t& size)
{
	int status = -1;
	if (SocketActivateCount < 1) {
		std::cerr << __FUNCTION__ << "@" << __LINE__ << "not found active socket" << std::endl;
		return status;
	}
	else if (SocketActivateCount == 1) {
		auto socket = _vec_socket.begin()->get();
		status = socket->recv(buf, size);
	}
	else {
		std::cerr << __FUNCTION__ << "@" << __LINE__ << "have many activation sockets" << std::endl;
		//TODO:: 여러 소캣의 recv처리
		auto socket = _vec_socket.begin()->get();
		status = socket->recv(buf, size);
	}

	return status;
}

//void SessionHandleServer::registerRecvCallback(boost::function<void(char*, std::size_t, char)> f)
//{
//	recvCall = f;
//	//_IO_Workers.create_thread(boost::bind(&SessionHandleServer::readProc, this));
//}

void SessionHandleServer::disconnectSession(std::shared_ptr<Session> objID)
{
	boost::lock_guard<boost::mutex> lk(sessionMutex);
	--SocketActivateCount;
	_vec_socket.erase(objID);
	std::cout << "Activate Socket = " << SocketActivateCount << std::endl;
}

void SessionHandleServer::bufQueueChecker()
{
	if (--OutBufferQueue.front()->cnt == 0) {
		sessionMutex.lock();
		OutBufferQueue.pop();
		sessionMutex.unlock();

		std::cerr << __FUNCTION__ << "@" << __LINE__ << "Session OutQueue pop" << std::endl;
	}
}

void SessionHandleServer::closeAllSocket()
{
	boost::lock_guard<boost::mutex> lk(sessionMutex);
	std::for_each(_vec_socket.begin(), _vec_socket.end(),
		boost::bind(&SessionHandleServer::disconnectSession, this, boost::placeholders::_1));

	_vec_socket.clear();
}