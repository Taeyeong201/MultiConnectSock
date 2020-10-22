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

BaseSession::BaseSession()
	: objectID(objID_Creater++)
{
}

void Channel::joinSession(boost::shared_ptr<BaseSession> session)
{
	sessions_.insert(session);
}

void Channel::registerDisConnectAllSession(boost::function<void()> f)
{
	disconnectAll = f;
}

void Channel::closeAllSocket()
{
}

int Channel::write(char* buf, std::size_t size)
{
	BufferPacket buffer(buf, size);
	std::for_each(sessions_.begin(), sessions_.end(),
		boost::bind(&Session::writeHandle, boost::placeholders::_1, boost::ref(buffer)));
	return 0;
}

int Channel::read(char* buf, const std::size_t& size)
{
	return 0;
}

void Channel::disconnectSession(boost::shared_ptr<BaseSession> session)
{
	sessions_.erase(session);
}

Session::Session(
	boost::asio::ip::tcp::socket sock, 
	boost::asio::io_context& ioc, 
	Channel& channel, std::size_t maxbuf)
	: channel_(channel), socket_(std::move(sock))
{

}

Session::~Session()
{
	socket_.close();
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

void Session::disconect()
{
	channel_.disconnectSession(shared_from_this());
}

int Session::writeHandle(BufferPacket& buffer)
{
	BufferPacket tmp(buffer.buf_, buffer.size_);


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

int Session::recv(BufferPacket&)
{
	//boost::system::error_code ec;

	//int readSize = 0;

	//if (socket_.is_open()) {
	//	readSize = socket_.read_some(boost::asio::buffer(buf, size), ec);
	//	if (ec) {
	//		std::cout << "Session error ID : " << objectID << std::endl;
	//		//handle->InBufferQueue.front()->size = -ec.value();
	//		boostErrorHandler(__FUNCTION__, __LINE__, ec);

	//		disconect();
	//		return -1;
	//	}
	//}
	//else {
	//	disconect();
	//	return -1;
	//}

	//return readSize;

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

