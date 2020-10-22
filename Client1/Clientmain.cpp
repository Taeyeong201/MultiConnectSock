#define SINGLE 1
#ifdef SINGLE
#include "SessionHandle.h"

int main() {

	SessionHandle sessionClient;
	sessionClient.initClientMode("127.0.0.1", 12345);
	sessionClient.connect();

	std::shared_ptr<char> buf = std::make_shared<char>(4096);
	memset(buf.get(), 0, 4096);

	//sessionClient.channel_.write(buf.get(), 100);

	system("PAUSE");

	
	std::thread([&] {
		for (int i = 0; i < 8000; i++) {
			memset(buf.get(), 0, 4096);
			int readSize = sessionClient.channel_.read(buf.get(), 4096);
			if (readSize > 0)
				std::cout << "(" << i << "/" << readSize << ") "
				<< std::string(buf.get(), readSize) << std::endl;
			else break;
		}
	}).join();



	sessionClient.close();
	system("PAUSE");

	return 0;
}

#else
#include "MultiSessionControler.h"

struct test {
	unsigned int a;
};

class bufferTest {
public:
	bufferTest(int size): buf(new char(size)) {

	}
	~bufferTest() {
		delete[] buf;
	}
	char* buf;
};

int main() {

	MultiSessionControler client;
	MultiSessionControler client1;
	MultiSessionControler client2;
	client.initClientMode("127.0.0.1", 8090, 1000*1000);
	client1.initClientMode("127.0.0.1", 8091, 1000*1000);
	client2.initClientMode("127.0.0.1", 8092, 1000*1000);
	//client.initClientMode("192.168.0.68", 12345, 1000*1000);
	client.connect();
	client1.connect();
	client2.connect();
	/* = std::make_shared<char[]>(100);*/


	test qq = { 0, };
	//std::cin >> t;
	//if (t == 't') {
	//	client.read(buf, 4096);
	//}
	//else if (t == 'x') {
	//	client.closeAllSocket();
	//}
	std::shared_ptr<bufferTest> buf = std::make_shared<bufferTest>(1920*1080);

	
	std::thread([&] {
		//std::this_thread::sleep_for(std::chrono::milliseconds(1));
		//int tt;
		//for (int i = 1; i <= 10000; i++) {
		//	//std::this_thread::sleep_for(std::chrono::milliseconds(1));
		//	std::cout << i << " read" << std::endl;
		//	memset(buf, 0, 4096);
		//	tt = client.read(buf, 4096);
		//	if (0 > tt) {
		//		break;
		//	}
		//	else if (tt == 0) {
		//		std::cout << "what the" << std::endl;
		//	}
		//	std::cout << std::string(buf) << std::endl;

		client.read(buf.get()->buf, 72);
		std::cout << "read 1 size : " << 72 << std::endl;

		client.read(buf.get()->buf, 534);
		std::cout << "read 1 size : " << 534 << std::endl;

		//}
		while (1) {
			int tt = 0;
			int rr = 0;

			tt = client.read((char*)&qq, sizeof(test));
			if (tt < 0) return;
			std::cout << "need size : " << qq.a << std::endl;
			while (1) {
				rr += client.read(buf.get()->buf, qq.a - rr);
				if (rr < 0) return;
				//std::cout << "read size : " << rr << std::endl;
				if (rr >= qq.a) {
					break;
				}
			}
		}


	}).join();


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
#endif