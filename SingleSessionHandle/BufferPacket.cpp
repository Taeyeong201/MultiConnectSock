#include "BufferPacket.h"

BufferPacket::BufferPacket(char buf[], std::size_t bufSize)
	:
	buf_(new char[bufSize]),
	size_(bufSize)
{
	std::copy(buf, buf + bufSize, buf_);
}

BufferPacket::~BufferPacket()
{
	delete[] buf_;
}
