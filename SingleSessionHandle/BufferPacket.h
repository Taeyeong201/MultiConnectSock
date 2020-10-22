#pragma once
#include <memory>
#include <atomic>

class BufferPacket
{
public:
	BufferPacket(char[], std::size_t);
	~BufferPacket();
	std::size_t size_;
	char* buf_;
};
