#include <iostream>

#define SINGLE 1
#if SINGLE
//#include "SingleSession.h"

#include "SessionHandle.h"

int main() {
	char test[100] = { 97, };
	memset(test, 97, 100);
	BufferPacket test11(test, 100);


	SessionHandle sessionServer;
	sessionServer.initServerMode(12345);
	sessionServer.async_acceptor();
	std::string str = "1";

	std::size_t s;
	char buf[100] = { 0, };

	//sessionServer.channel_.write(buf, 100);

	system("PAUSE");

	bool allOut = false;

	sessionServer.channel_.registerDisConnectAllSession([&]() {
		allOut = true;
	});

	auto thread = std::thread([&] {
		for (int i = 1; (i <= 2000) && !allOut; i++) {
			if (i % 10 == 0) {
				str.clear();
			}
			str.append("asd");
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			std::cout << i << " send" << std::endl;
			char buffer[4096] = { 0, };
			std::copy(str.c_str(), str.c_str() + str.length(), buffer);
			//s = ;
			sessionServer.channel_.write(buffer, str.length() + 1);
		}
		std::cout << "end thread" << std::endl;
	});
	std::cin.get();

	sessionServer.close();
	//thread.join();

	system("PAUSE");


	return 0;
}

#else

#include "MultiSessionControler.h"

int main() {
	MultiSessionControler test;
	test.initServerMode(12345, 123456);
	test.async_acceptor();
	//system("PAUSE");
	//getchar();
	for (int i = 1; i <= 3; i++) {
		std::this_thread::sleep_for(std::chrono::seconds(1));
		std::cout << i << " s" << std::endl;
	}

	std::string str = "1";

	std::size_t s;
	auto thread = std::thread([&] {
		for (int i = 1; i <= 2000; i++) {
			if (i % 10 == 0) {
				str.clear();
			}
			str.append("asd");
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			std::cout << i << " send" << std::endl;
			char buffer[4096] = { 0, };
			std::copy(str.c_str(), str.c_str() + str.length(), buffer);
			s = str.length();
			test.write(buffer, s);
		}
	});
	std::cin.get();
	test.closeAllSocket();
	thread.join();

	//bool tes1 = test._vec_socket[1]->is_open();
	//std::cout << tes1 << std::endl;

	//while (1) {
	//	std::this_thread::sleep_for(std::chrono::seconds(1));
	//}
	system("PAUSE");
	return 0;
}

#endif