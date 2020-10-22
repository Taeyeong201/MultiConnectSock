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

int Session::objID_Creater = 0;
//int Session::recvCnt = 0;

//Session::Session(boost::asio::ip::tcp::socket sock)
//	: _socket(std::move(sock)), task(IOCtx), objectID(objID_Creater++), isConnect(true)
//{
//	//recv_worker = std::thread([this] {
//	//	while (recv() == 0 && isConnect);
//	//});
//	//send_worker = std::thread([this] {
//	//	while (send() == 0 && isConnect);
//	//});
//}

Session::Session(
	boost::asio::ip::tcp::socket sock,
	//SessionControler* _handle,
	std::size_t maxbufsize)
	:
	_socket(std::move(sock)),
	//taskStrand(IOCtx),
	objectID(objID_Creater++),
	isConnect(true)/*, handle(_handle)*/
{
}

Session::~Session()
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

void Session::registerDisconnectCallBack(boost::function<void(int)> f)
{
	disconnectSessionCall = f;
}

void Session::registerBufPopCallBack(boost::function<void()> f)
{
	bufferPopCall = f;
}

void Session::disconect()
{
	isConnect = false;
	disconnectSessionCall(objectID);
}

int Session::send()
{
	//if (!isConnect) {
	//	std::cerr << __FUNCTION__ << "@" << __LINE__ << "Session send disconnect" << std::endl;

	//	return -1;
	//}

	//boost::system::error_code ec;
	//if (handle->OutBufferQueue.empty()) {
	//	std::cerr << __FUNCTION__ << "@" << __LINE__ << "Session send Queue Empty" << std::endl;
	//	return -1;
	//}
	//else {
	//	char* front = handle->OutBufferQueue.front()->dataBuf.get();
	//	int bufSize = handle->OutBufferQueue.front()->size;

	//	std::unique_ptr<char[]> tmp = std::make_unique<char[]>(bufSize);
	//	std::copy(front, front + bufSize, tmp.get());

	//	IOCtx.post(boost::asio::bind_executor(
	//		handle->bufferPopStrand, bufferPopCall));

	//	handle->sessionMutex.lock();
	//	std::cerr << __FUNCTION__ << "@" << __LINE__ << "Session send" << std::endl;
	//	std::size_t size = _socket.write_some(boost::asio::buffer(
	//		tmp.get(),
	//		bufSize), ec);
	//	handle->sessionMutex.unlock();

	//	if (!ec) {
	//		//_socket.shutdown(boost::asio::socket_base::shutdown_send);
	//		std::cerr << __FUNCTION__ << "@" << __LINE__ << "write Size : " << size << std::endl;

	//	}
	//	else {
	//		std::cerr << "Session error ID : " << objectID << std::endl;
	//		boostErrorHandler(__FUNCTION__, __LINE__, ec);
	//		IOCtx.post(boost::asio::bind_executor(
	//			handle->bufferPopStrand, boost::bind(&Session::disconect, this)));
	//		return -1;
	//	}
	//}





	return 0;
}

int Session::recv(char* buf, const std::size_t& size)
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

//void Session::bufQueueChecker()
//{
//	--handle->sendCnt;
//	if (handle->sendCnt == 0) {
//		handle->sessionMutex.lock();
//		handle->OutBufferQueue.pop();
//		handle->sessionMutex.unlock();
//
//		std::cerr << __FUNCTION__ << "@" << __LINE__ << "Session OutQueue pop" << std::endl;
//	}
//}

bool Session::operator==(const Session& rhs)
{
	if (objectID == rhs.objectID)
		return true;
	else {
		return false;
	}
}

bool Session::operator==(const int& rhs)
{
	if (objectID == rhs)
		return true;
	else {
		return false;
	}
}