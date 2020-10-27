#define SINGLE 0
#if SINGLE
#include "SessionHandle.h"

int main() {

	SessionHandle sessionClient;
	sessionClient.initClientMode("192.168.0.201", 12345);
	sessionClient.connect();

	//std::shared_ptr<char> buf = std::make_shared<char>(4096);

	char buffer[4096] = { 0, };

	//memset(buf.get(), 0, 4096);

	//sessionClient.channel_.write(buf.get(), 100);

	//system("PAUSE");

	
	std::thread testes = std::thread([&] {
		for (int i = 0; i < 8000; i++) {
			memset(buffer, 0, 4096);
			int readSize = sessionClient.channel_.read(buffer, 4096);
			if (readSize > 0)
				std::cout << "(" << i << "/" << readSize << ") "
				<< std::string(buffer, readSize) << std::endl;
			else break;
		}
	});
	getchar();

	testes.join();

	sessionClient.close();


	return 0;
}

#else
#include "SessionHandle.h"

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

	SessionHandle client;
	SessionHandle client1;
	SessionHandle client2;
	client.initClientMode("192.168.0.201", 8090);
	client1.initClientMode("192.168.0.201", 8091);
	client2.initClientMode("192.168.0.201", 8092);
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
	//std::shared_ptr<bufferTest> buf = std::make_shared<bufferTest>(1920*1080);
	char* buf = new char[1920 * 1080 * 3 / 2];

	
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

		int readSize = client.channel_.read((char*)&qq, sizeof(test));
		std::cout << "read 1 size : " << readSize << std::endl;

		readSize = client.channel_.read(buf, qq.a);
		std::cout << "read 1 size : " << readSize << std::endl;
		if (readSize < 0) return;
		while (1) {
			int tt = 0;
			int rr = 0;

			tt = client.channel_.read((char*)&qq, sizeof(test));
			if (tt < 0) return;
			std::cout << "need size : " << qq.a << std::endl;
			while (1) {
				rr += client.channel_.read(buf, qq.a - rr);
				if (rr < 0) return;
				if (rr >= qq.a) {
					std::cout << "read size : " << rr << std::endl;
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
	delete[] buf;
	system("PAUSE");
	return 0;
}
#endif


//read async - 활용



//#include <boost/asio.hpp>
//#include <boost/smart_ptr/make_shared.hpp>
//#include <boost/thread.hpp>
//
//
//class Servant
//{
//	boost::asio::io_service m_ioservice;
//	boost::asio::io_service::work m_work;
//	boost::thread_group m_threads;
//
//public:
//	Servant() : m_ioservice(), m_work(m_ioservice), m_threads()
//	{
//		m_threads.create_thread(boost::bind(&boost::asio::io_service::run, &m_ioservice));
//		std::cout << "백그라운드 스케쥴러 시작" << std::endl;
//	};
//
//	~Servant()
//	{
//		m_ioservice.stop();
//		m_threads.join_all();
//	};
//
//
//	boost::unique_future<int> do_something(const std::string& str)
//	{
//		return schedule(boost::bind(&Servant::do_something_impl, this, str));
//	};
//
//private:
//	template <typename T>
//	boost::unique_future<int> schedule(T function)
//	{
//		typedef boost::packaged_task<int> task_t;
//		std::shared_ptr<task_t> task = std::make_shared<task_t>(function);
//
//		boost::unique_future<int> f = task->get_future();
//
//		m_ioservice.post(boost::bind(&task_t::operator(), task));
//		std::cout << "백그라운드 스케쥴러에 작업을 요청했다" << std::endl;
//
//		return f;
//	};
//
//
//	int do_something_impl(const std::string& str)
//	{
//		std::cout << "시간이 꽤 걸리는 작업 시작...." << std::endl;
//
//		boost::this_thread::sleep(boost::posix_time::milliseconds(3000));
//		std::cout << str << std::endl;
//		return 777;
//	};
//};
//
//
//int main()
//{
//	Servant s;
//	boost::unique_future<int> f = s.do_something("일감 1");
//
//	std::cout << "작업이 끝날 때까지 대기한다." << std::endl;
//
//	std::cout << f.get() << std::endl;
//
//	return 0;
//}