#include "BufferPacket.h"

BufferPacket::BufferPacket(char* buf, std::size_t bufSize)
	: 
	buf_(std::make_shared<char[]>(bufSize, [](char* buf) {delete[] buf; })),
	size_(bufSize)
{
	std::copy(buf, buf + bufSize, buf_.get());
}

BufferPacket::BufferPacket(std::shared_ptr<char[]> buf, std::size_t bufSize)
	:
	size_(bufSize),
	buf_(buf)
{
}

BufferPacket::~BufferPacket()
{
}
