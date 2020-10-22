#pragma once
#include <memory>
#include <atomic>

class BufferPacket
{
public:
	BufferPacket(char*, std::size_t);
	BufferPacket(std::shared_ptr<char[]>, std::size_t);
	~BufferPacket();

	std::size_t size_;
	std::shared_ptr<char[]> buf_;
	std::atomic<int> cnt_;
};

