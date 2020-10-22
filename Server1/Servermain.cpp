
#include <iostream>

#define SINGLE 1
#ifdef SINGLE
//#include "SingleSession.h"

#include <set>
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/bind/bind.hpp>
#include <boost/asio.hpp>

class Test {
public:
	Test(int _a, int _b, int _c, char _d)
		: a(_a), b(_b), c(_c), d(_d) {

	}
	int a;
	int b;
	int c;
	char d;

	void showData() {
		std::cout << "a : " << a << " b : " << b << " c : " << c << std::endl;
	}
};

class TestTest {
public:
	void func1(std::shared_ptr<Test> pt) {
		pt->showData();
	}

};


int main() {

	std::set<std::shared_ptr<Test>> asd;

	std::shared_ptr<Test> a1 = std::make_shared<Test>(1, 2, 3, 4);
	std::shared_ptr<Test> a2 = std::make_shared<Test>(1, 3, 3, 4);
	std::shared_ptr<Test> a3 = std::make_shared<Test>(1, 4, 3, 4);
	std::shared_ptr<Test> a4 = std::make_shared<Test>(1, 5, 3, 4);

	asd.insert(a1);
	asd.insert(a2);
	asd.insert(a3);
	asd.insert(a4);
	asd.insert(std::make_shared<Test>(2, 2, 3, 4));
	//asd.insert(std::make_shared<Test>(3, 2, 3, 4));
	//asd.insert(std::make_shared<Test>(4, 2, 3, 4));
	//asd.insert(std::make_shared<Test>(5, 2, 3, 4));
	//asd.insert(std::make_shared<Test>(6, 2, 3, 4));
	//asd.insert(std::make_shared<Test>(7, 2, 3, 4));
	//asd.insert(std::make_shared<Test>(8, 2, 3, 4));

	asd.erase(a2);
	asd.erase(a3);
	asd.erase(a4);



	TestTest funn;
	std::for_each(asd.begin(), asd.end(), boost::bind(&TestTest::func1, funn, boost::placeholders::_1));




	return 0;
}




#else // SINGLE

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