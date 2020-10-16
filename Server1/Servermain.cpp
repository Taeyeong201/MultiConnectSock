
#include <iostream>

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


	auto thread = std::thread([&] {
		for (int i = 1; i <= 500; i++) {
			if (i % 10 == 0) {
				str.clear();
			}
			str.append("asd");
			std::this_thread::sleep_for(std::chrono::milliseconds(33));
			std::cout << i << " send" << std::endl;
			char buffer[4096] = { 0, };
			std::copy(str.c_str(), str.c_str() + str.length(), buffer);

			test.write(buffer, str.length());

		}
	});

	thread.join();

	//bool tes1 = test._vec_socket[1]->is_open();
	//std::cout << tes1 << std::endl;

	//while (1) {
	//	std::this_thread::sleep_for(std::chrono::seconds(1));
	//}
	system("PAUSE");
	return 0;
}