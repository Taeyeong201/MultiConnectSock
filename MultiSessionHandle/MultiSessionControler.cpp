#include "MultiSessionControler.h"

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

boost::asio::io_context MultiSessionControler::IOCtx;
int MultiSessionControler::SocketActivateCount = 0;

MultiSessionControler::MultiSessionControler()
	:
	_lock_work(std::make_shared<boost::asio::io_context::work>(IOCtx)),
	_vec_socket()
{
	_IO_Worker = std::thread([&]() {
		IOCtx.run();
	});

	_vec_socket.reserve(10);
}

MultiSessionControler::~MultiSessionControler()
{
	_lock_work.reset();
	IOCtx.stop();
	_IO_Worker.join();
}

int MultiSessionControler::initServerMode(const int port, unsigned int maxBufSize)
{
	address = boost::asio::ip::tcp::endpoint(
		boost::asio::ip::tcp::v4(), port);
	_acceptor = std::make_unique<boost::asio::ip::tcp::acceptor>(IOCtx, address);

	return 0;
}

int MultiSessionControler::initClient(const std::string& serverIP, const int port, unsigned int maxBufSize)
{
	address = boost::asio::ip::tcp::endpoint(
		boost::asio::ip::address::from_string(serverIP), port);
	return 0;
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
		_vec_socket.push_back(std::make_unique<Session>(std::move(sock)));
		_vec_socket.back()->init(this);
		_vec_socket.back()->registerDisconnectCallBack(
			boost::bind(&MultiSessionControler::disconnectSession, this, boost::placeholders::_1));
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
			_vec_socket.push_back(std::make_unique<Session>(std::move(socket)));
			_vec_socket.back()->init(this);
			_vec_socket.back()->registerDisconnectCallBack(
				boost::bind(&MultiSessionControler::disconnectSession, this, boost::placeholders::_1));

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
		_vec_socket.push_back(std::make_unique<Session>(std::move(sock)));
		_vec_socket.back()->init(this);
		_vec_socket.back()->registerDisconnectCallBack(
			boost::bind(&MultiSessionControler::disconnectSession, this, boost::placeholders::_1));
	}
	else {
		boostErrorHandler(__FUNCTION__, __LINE__, ec);
	}
}

int MultiSessionControler::write(char* buf, unsigned int size)
{
	if (SocketActivateCount < 1) return -1;
	std::shared_ptr<BufferPacket> tmp = std::make_shared<BufferPacket>();
	tmp->size = size;
	tmp->buf = new char[size];
	std::copy(tmp->buf, tmp->buf + size, buf);

	
	sendMutex.lock();
	OutBufferQueue.push(tmp);
	sendMutex.unlock();
	sendConditionVar.notify_all();
	
	return size;
}

int MultiSessionControler::read(char* buf, unsigned int size)
{
	boost::unique_lock<boost::mutex> lk(recvMutex);
	sendConditionVar.wait(lk, [&] {
		return !InBufferQueue.empty() || SocketActivateCount < 1;
	});
	if (SocketActivateCount < 1) return -1;

	std::shared_ptr<BufferPacket> tmp = InBufferQueue.front();
	InBufferQueue.pop();
	lk.unlock();

	std::copy(buf, buf + size, tmp->buf);


	return tmp->size;
}

void MultiSessionControler::disconnectSession(int objID)
{
	std::vector<std::unique_ptr<Session>>::iterator it = 
		std::find_if(
			_vec_socket.begin(), 
			_vec_socket.end(), 
			[&](const std::unique_ptr<Session>& session) { return session->objectID == objID; });

	if (it == _vec_socket.end()) {
		std::cout << "not found disconnectSeesion obj" << std::endl;
	}
	else {
		//int index = std::distance(_vec_socket.begin(), it);
		--SocketActivateCount;
		_vec_socket.erase(it);
		recvConditionVar.notify_all();
	}
}

int MultiSessionControler::Session::objID_Creater = 0;

MultiSessionControler::Session::Session(boost::asio::ip::tcp::socket sock)
	: _socket(std::move(sock)), objectID(objID_Creater++), isConnect(true)
{

}

MultiSessionControler::Session::~Session()
{
	_socket.close();
	std::cout << "Session Close obj_id=" << objectID << std::endl;
}

void MultiSessionControler::Session::registerDisconnectCallBack(boost::function<void(int)> f)
{
	disconnectSession = f;
}

void MultiSessionControler::Session::init(MultiSessionControler* handle)
{
	this->handle = handle;
}

void MultiSessionControler::Session::disconect()
{
	isConnect = false;
	disconnectSession(objectID);
}

int MultiSessionControler::Session::send()
{
	

	return 0;
}

int MultiSessionControler::Session::recv()
{

	return 0;
}

bool MultiSessionControler::Session::operator==(const Session& rhs)
{
	if(objectID == rhs.objectID)
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