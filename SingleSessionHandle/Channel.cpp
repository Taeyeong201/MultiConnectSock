#include "Channel.h"
int BaseSession::objID_Creater = 0;

BaseSession::BaseSession() : objectID(objID_Creater++) {}

Channel::Channel(boost::asio::io_context& ioc)
	:ioc_(ioc)
{
}

void Channel::joinSession(boost::shared_ptr<BaseSession> session)
{
	sessions_.insert(session);
	SocketActivateCount++;
}

void Channel::registerDisConnectAllSession(boost::function<void()> f)
{
	disconnectAll = f;
}

void Channel::closeAllSession()
{
	for (auto it = sessions_.begin(); it != sessions_.end();) {
		it++->get()->disconnect();
	}
}

boost::system::error_code Channel::write(char* buf, std::size_t size)
{
	boost::system::error_code ec;
	if (sessions_.empty()) {
		checkSession();
		return ec;
	}
	BufferPacket buffer(buf, size);

	for (auto it = sessions_.begin(); it != sessions_.end();) {
		it++->get()->writeHandle(boost::ref(buffer), boost::ref(ec));
	}
	
	return ec;
}

int Channel::read(char* buf, const std::size_t& size)
{
	if (sessions_.empty())
		return -1;

	//TODO MultiSession
	return sessions_.begin()->get()->readHandle(buf, size);
}

int Channel::countAliveSocket()
{
	return SocketActivateCount;
}

void Channel::checkSession()
{
	ioc_.post([this]() {
		if (sessions_.empty()) {
			disconnectAll();
		}
	});
}

void Channel::disconnectSession(boost::shared_ptr<BaseSession> session)
{
	--SocketActivateCount;
	sessions_.erase(session);
	if(SocketActivateCount < 0)
		checkSession();
}