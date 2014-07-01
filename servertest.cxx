#include <iostream>
#include <fstream>
#include <thread>
#include <mutex>
#include <atomic>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include "net.h"
#include "protocolheader.h"

inline unsigned long long current_time()
{
	struct timeval tv;
	gettimeofday(&tv, nullptr);
	unsigned long long l = tv.tv_sec * 1000000 + tv.tv_usec;
	return l;
}

unsigned long long count[10];
unsigned long long count_prev[10];
unsigned long long total(0);

bool loop = true;

int threadnum;
int connectnum;
int pollermax;

struct thread_wrap
{
	int id;
	session_manager * m;
	poller* p;

	thread_wrap(int id)
		: id(id)
	{
		char buf[8] = {};
		sprintf(buf, "%d", id);
		m = new session_manager(buf);
		p = new poller();
		p->create(pollermax);
		launch_server(p, m, "127.0.0.1", 10000+id);
	}

	void operator()()
	{ 
		std::cout << "thread " << id << " start..." << std::endl;
		while (loop)
		{
			p->poll(0);
			while (m->process_protocol())
				++count[id-1];
		}

		p->close();
		delete p;
		delete m;

		std::cout << "thread " << id << " term" << std::endl;
	}
};

int main(int argc, char* argv[])
{
	if (argc != 3)
	{
		std::cout << "usage: ./server threadnum connectnum" << std::endl;
		return 0;
	}

	threadnum = atoi(argv[1]);
	connectnum = atoi(argv[2]);
	pollermax = connectnum / threadnum;

	net_init();

	std::thread** threads = new std::thread*[threadnum]();
	for (int i = 0; i < threadnum; ++i)
	{
		threads[i] = new std::thread(thread_wrap(i+1));
	}

	while (loop)
	{
		total = 0;

		unsigned long long tm = current_time();
		sleep(10);
		unsigned long long diff_tm = (current_time() - tm) / 1000000;

		for (int i = 0; i < threadnum; ++i)
		{
			unsigned long long diff = count[i] - count_prev[i];
			total += diff;
			std::cout << "thread " << i+1 << " speed: " << diff / diff_tm << std::endl;
			count_prev[i] = count[i];
		}

		std::cout << "total speed:" << total / diff_tm << std::endl;

		std::fstream f;
		f.open("breakloop");
		if (f.is_open())
		{
			f.close();
			loop = false;
			std::cout << "break loop" << std::endl;
		}
	}

	for (int i = 0; i < threadnum; ++i)
	{
		threads[i]->join();
		delete threads[i];
		threads[i] = nullptr;
	}
	delete [] threads;
	threads = nullptr;

	net_term();

	return 0;
}
