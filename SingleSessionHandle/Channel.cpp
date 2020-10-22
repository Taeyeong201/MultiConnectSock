#include "Channel.h"
int BaseSession::objID_Creater = 0;

BaseSession::BaseSession() : objectID(objID_Creater++) {}

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
	std::for_each(sessions_.begin(), sessions_.end(),
		boost::bind(&BaseSession::disconnect, boost::placeholders::_1));
}

int Channel::write(char* buf, std::size_t size)
{
	BufferPacket buffer(buf, size);
	std::for_each(sessions_.begin(), sessions_.end(),
		boost::bind(&BaseSession::writeHandle, boost::placeholders::_1, boost::ref(buffer)));

	return 0;
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

void Channel::disconnectSession(boost::shared_ptr<BaseSession> session)
{
	sessions_.erase(session);
}