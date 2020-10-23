#include "MultiSessionControler.h"

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

boost::asio::io_context MultiSessionControler::IOCtx;

MultiSessionControler::MultiSessionControler()
	:
	recvStrand(IOCtx),
	bufferPopStrand(IOCtx),
	_lock_work(std::make_shared<boost::asio::io_context::work>(IOCtx)),
	_vec_socket(), sendBuffer(nullptr),
	SocketActivateCount(0), maxBufferSize(0)
{
	for (int i = 0; i < 7; i++)
		_IO_Workers.create_thread(boost::bind(&boost::asio::io_context::run, &IOCtx));

	_vec_socket.reserve(10);
}

MultiSessionControler::~MultiSessionControler()
{
	_vec_socket.clear();
	_lock_work.reset();
	IOCtx.stop();
	_IO_Workers.join_all();

	delete[] sendBuffer;
}

int MultiSessionControler::initServerMode(const unsigned short port, std::size_t maxBufSize)
{
	address = boost::asio::ip::tcp::endpoint(
		boost::asio::ip::tcp::v4(), port);
	_acceptor = std::make_unique<boost::asio::ip::tcp::acceptor>(IOCtx, address);

	maxBufferSize = maxBufSize;
	sendBuffer = new char((unsigned int)maxBufferSize);
	return 0;
}

int MultiSessionControler::initClientMode(const std::string& serverIP, const unsigned short port, std::size_t maxBufSize)
{
	address = boost::asio::ip::tcp::endpoint(
		boost::asio::ip::address::from_string(serverIP), port);

	maxBufferSize = maxBufSize;
	sendBuffer = new char((unsigned int)maxBufferSize);

	return 0;
}

void MultiSessionControler::initNodelayOpt(bool onoff)
{
	nodelay = onoff;
}

void MultiSessionControler::acceptor()
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
		_vec_socket.push_back(std::make_shared<Session>(std::move(sock), this, maxBufferSize));
		//_vec_socket.back()->init(this);
		_vec_socket.back()->registerDisconnectCallBack(
			boost::bind(&MultiSessionControler::disconnectSession, this, boost::placeholders::_1));
		_vec_socket.back()->registerBufPopCallBack(
			boost::bind(&MultiSessionControler::bufQueueChecker, this));
		_vec_socket.back()->_socket.set_option(boost::asio::ip::tcp::no_delay(nodelay));
	}
	else {
		boostErrorHandler(__FUNCTION__, __LINE__, ec);
	}
}

void MultiSessionControler::async_acceptor()
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
			_vec_socket.push_back(std::make_shared<Session>(std::move(socket), this, maxBufferSize));
			//_vec_socket.back()->init(this);
			_vec_socket.back()->registerDisconnectCallBack(
				boost::bind(&MultiSessionControler::disconnectSession, this, boost::placeholders::_1));
			_vec_socket.back()->registerBufPopCallBack(
				boost::bind(&MultiSessionControler::bufQueueChecker, this));
			_vec_socket.back()->_socket.set_option(boost::asio::ip::tcp::no_delay(nodelay));

			std::cout << "Activate Socket = " << SocketActivateCount << std::endl;

			async_acceptor();
		}
		else {
			boostErrorHandler(__FUNCTION__, __LINE__, ec);
		}
	});
}

void MultiSessionControler::connect()
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
		_vec_socket.push_back(std::make_shared<Session>(std::move(sock), this, maxBufferSize));
		_vec_socket.back()->registerDisconnectCallBack(
			boost::bind(&MultiSessionControler::disconnectSession, this, boost::placeholders::_1));
		_vec_socket.back()->registerBufPopCallBack(
			boost::bind(&MultiSessionControler::bufQueueChecker, this));
		_vec_socket.back()->_socket.set_option(boost::asio::ip::tcp::no_delay(nodelay));

		std::cout << "Activate Socket = " << SocketActivateCount << std::endl;
	}
	else {
		boostErrorHandler(__FUNCTION__, __LINE__, ec);
	}
}

int MultiSessionControler::write(char* buf, std::size_t& size)
{
	std::cout << __FUNCTION__ << "@" << __LINE__ << "buffer write in" << std::endl;
	if (SocketActivateCount < 1) return -1;
	std::shared_ptr<BufferPacket> tmp = std::make_shared<BufferPacket>();
	tmp->size = size;
	tmp->dataBuf.reset(new char[size]);
	std::copy(buf, buf + size, tmp->dataBuf.get());

	tmp->cnt = SocketActivateCount;
	OutBufferQueue.push(tmp);

	int writeFlag = 0;

	for (auto socket : _vec_socket) {
		if (socket->isConnect) {
			IOCtx.post(boost::asio::bind_executor(
				socket->taskStrand,
				boost::bind(&Session::send, socket.get())));
			writeFlag++;
		}
	}
	if (writeFlag < 0) {
		return -1;
	}

	return size;
}

int MultiSessionControler::read(char* buf,const std::size_t& size)
{
	int status = -1;
	if (SocketActivateCount < 1) {
		std::cout << __FUNCTION__ << "@" << __LINE__ << "not found active socket" << std::endl;
		return status;
	}
	else if (SocketActivateCount == 1) {
		auto socket = _vec_socket.front();
		status = socket->recv(buf, size);
	}
	else {
		std::cout << __FUNCTION__ << "@" << __LINE__ << "have many activation sockets" << std::endl;
		//TODO:: 여러 소캣의 recv처리
		auto socket = _vec_socket.front();
		status = socket->recv(buf, size);
	}

	return status;
}

//void MultiSessionControler::registerRecvCallback(boost::function<void(char*, std::size_t, char)> f)
//{
//	recvCall = f;
//	//_IO_Workers.create_thread(boost::bind(&MultiSessionControler::readProc, this));
//}

void MultiSessionControler::disconnectSession(int objID)
{
	boost::lock_guard<boost::mutex> lk(sessionMutex);
	std::vector<std::shared_ptr<Session>>::iterator it =
		std::find_if(
			_vec_socket.begin(),
			_vec_socket.end(),
			[&](const std::shared_ptr<Session>& session) { return session->objectID == objID; });

	if (it == _vec_socket.end()) {
		std::cout << "not found disconnectSeesion obj" << std::endl;
	}
	else {
		//int index = std::distance(_vec_socket.begin(), it);
		--SocketActivateCount;
		_vec_socket.erase(it);
		std::cout << "Activate Socket = " << SocketActivateCount << std::endl;
		//internalConditionVar.notify_all();
		//recvConditionVar.notify_all();
		//sendConditionVar.notify_all();
	}
}

void MultiSessionControler::bufQueueChecker()
{
	if (--OutBufferQueue.front()->cnt == 0) {
		sessionMutex.lock();
		OutBufferQueue.pop();
		sessionMutex.unlock();

		std::cout << __FUNCTION__ << "@" << __LINE__ << "Session OutQueue pop" << std::endl;
	}
}

void MultiSessionControler::closeAllSocket()
{
	boost::lock_guard<boost::mutex> lk(sessionMutex);
	while (1) {
		std::vector<std::shared_ptr<Session>>::iterator it =
			std::find_if(
				_vec_socket.begin(),
				_vec_socket.end(),
				[&](const std::shared_ptr<Session>& session) { return session->isConnect; });

		if (it == _vec_socket.end()) {
			std::cout << "end of session obj" << std::endl;
			break;
		}
		else {
			//int index = std::distance(_vec_socket.begin(), it);
			--SocketActivateCount;
			_vec_socket.erase(it);
			std::cout << "Activate Socket = " << SocketActivateCount << std::endl;
		}
	}
	_vec_socket.clear();
}

int MultiSessionControler::Session::objID_Creater = 0;
//int MultiSessionControler::Session::recvCnt = 0;

//MultiSessionControler::Session::Session(boost::asio::ip::tcp::socket sock)
//	: _socket(std::move(sock)), task(IOCtx), objectID(objID_Creater++), isConnect(true)
//{
//	//recv_worker = std::thread([this] {
//	//	while (recv() == 0 && isConnect);
//	//});
//	//send_worker = std::thread([this] {
//	//	while (send() == 0 && isConnect);
//	//});
//}

MultiSessionControler::Session::Session(
	boost::asio::ip::tcp::socket sock,
	MultiSessionControler* _handle,
	std::size_t maxbufsize)
	:
	_socket(std::move(sock)),
	taskStrand(IOCtx),
	objectID(objID_Creater++),
	isConnect(true), handle(_handle)
{
}

MultiSessionControler::Session::~Session()
{
	isConnect = false;
	_socket.close();
	//_socket.lowest_layer().release();

	//handle->internalConditionVar.notify_all();
	//if (recv_worker.joinable()) {
	//	recv_worker.join();
	//}

	//handle->sendConditionVar.notify_all();
	//if (send_worker.joinable()) {
	//	send_worker.join();
	//}

	std::cout << "Session Close obj_id=" << objectID << std::endl;
}

void MultiSessionControler::Session::registerDisconnectCallBack(boost::function<void(int)> f)
{
	disconnectSessionCall = f;
}

void MultiSessionControler::Session::registerBufPopCallBack(boost::function<void()> f)
{
	bufferPopCall = f;
}

void MultiSessionControler::Session::disconect()
{
	isConnect = false;
	disconnectSessionCall(objectID);
}

int MultiSessionControler::Session::send()
{
	if (!isConnect) {
		std::cout << __FUNCTION__ << "@" << __LINE__ << "Session send disconnect" << std::endl;

		return -1;
	}

	boost::system::error_code ec;
	if (handle->OutBufferQueue.empty()) {
		std::cout << __FUNCTION__ << "@" << __LINE__ << "Session send Queue Empty" << std::endl;
		return -1;
	}
	else {
		char* front = handle->OutBufferQueue.front()->dataBuf.get();
		int bufSize = handle->OutBufferQueue.front()->size;

		std::unique_ptr<char[]> tmp = std::make_unique<char[]>(bufSize);
		std::copy(front, front + bufSize, tmp.get());

		IOCtx.post(boost::asio::bind_executor(
			handle->bufferPopStrand, bufferPopCall));

		handle->sessionMutex.lock();
		std::cout << __FUNCTION__ << "@" << __LINE__ << "Session send" << std::endl;
		std::size_t size = _socket.write_some(boost::asio::buffer(
			tmp.get(),
			bufSize), ec);
		handle->sessionMutex.unlock();

		if (!ec) {
			//_socket.shutdown(boost::asio::socket_base::shutdown_send);
			std::cout << __FUNCTION__ << "@" << __LINE__ << "write Size : " << size << std::endl;

		}
		else {
			std::cout << "Session error ID : " << objectID << std::endl;
			boostErrorHandler(__FUNCTION__, __LINE__, ec);
			IOCtx.post(boost::asio::bind_executor(
				handle->bufferPopStrand, boost::bind(&Session::disconect, this)));
			return -1;
		}
	}





	return 0;
}

int MultiSessionControler::Session::recv(char* buf,const std::size_t& size)
{
	boost::system::error_code ec;

	int readSize = 0;

	if (_socket.is_open()) {
		readSize = _socket.read_some(boost::asio::buffer(buf, size), ec);
		if (ec) {
			std::cout << "Session error ID : " << objectID << std::endl;
			//handle->InBufferQueue.front()->size = -ec.value();
			boostErrorHandler(__FUNCTION__, __LINE__, ec);

			disconect();
			return -1;
		}
	}
	else {
		disconect();
		return -1;
	}

	return readSize;
}

//void MultiSessionControler::Session::bufQueueChecker()
//{
//	--handle->sendCnt;
//	if (handle->sendCnt == 0) {
//		handle->sessionMutex.lock();
//		handle->OutBufferQueue.pop();
//		handle->sessionMutex.unlock();
//
//		std::cout << __FUNCTION__ << "@" << __LINE__ << "Session OutQueue pop" << std::endl;
//	}
//}

bool MultiSessionControler::Session::operator==(const Session& rhs)
{
	if (objectID == rhs.objectID)
		return true;
	else {
		return false;
	}
}

bool MultiSessionControler::Session::operator==(const int& rhs)
{
	if (objectID == rhs)
		return true;
	else {
		return false;
	}
}