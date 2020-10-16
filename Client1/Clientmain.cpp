
#include "MultiSessionControler.h"

int main() {

	MultiSessionControler client;
	client.initClient("127.0.0.1", 12345, 1000*1000);
	client.connect();
	char t;
	char buf[4096] = { 0, };

	std::cin >> t;
	if (t == 't') {
		client.read(buf, 4096);
	}
	else if (t == 'x') {
		client.closeAllSocket();
	}
	
	//std::thread([&] {
	//	std::this_thread::sleep_for(std::chrono::milliseconds(10));

	//	for (int i = 1; i <= 150; i++) {
	//		//std::this_thread::sleep_for(std::chrono::milliseconds(1));
	//		std::cout << i << " read" << std::endl;
	//		memset(buf, 0, 4096);
	//		if (0 > client.read(buf, 4096)) {
	//			break;
	//		}
	//		std::cout << std::string(buf) << std::endl;

	//	}
	//}).join();


	//std::cout << std::string(buf) << std::endl;
	//}

	//while (1) {
	//	std::this_thread::sleep_for(std::chrono::seconds(1));
	//}
	//getchar();
	//delete[] buf;
	system("PAUSE");
	return 0;
}