#include "nemo_utilities.hpp"

using namespace std;
using namespace nemo;

std::shared_ptr<ThreadPool> nemo::ThreadPool::instance = nullptr;

nemo::ThreadPool::ThreadPool() {

}

nemo::ThreadPool::~ThreadPool()
{
	ThreadPool::instance->halt();
	ThreadPool::instance.reset();
}

void nemo::ThreadPool::workerThread(void)
{
	auto& ptr = ThreadPool::instance;
	shared_ptr<IThreadPoolTask> task;
	for (;;) {
		if (ptr->taskList.size()) {
			if (ptr->lock.try_lock()) {
				if (ptr->taskList.size()) {
					task = *(ptr->taskList.begin());
					ptr->lock.unlock();
					
				}
				else
					ptr->lock.unlock();
			}
		}
		this_thread::sleep_for(chrono::milliseconds(ptr->interval));
	}
}

std::shared_ptr<ThreadPool>& nemo::ThreadPool::init(size_t count, unsigned long time)
{
	if (ThreadPool::instance == nullptr) {
		ThreadPool::instance = make_shared<ThreadPool>();
		
		allocator<thread> alloc;
		thread* threads = alloc.allocate(count);
		alloc.construct(threads, workerThread);
		for (size_t i = 0; i < count; i++) {
			threads[i].detach();
		}
		delete[] threads;
		
		ThreadPool::instance->threadCount = count;
		ThreadPool::instance->interval = time;
	}
	return ThreadPool::instance;
}

std::shared_ptr<ThreadPool>& nemo::ThreadPool::get(void)
{
	return ThreadPool::instance;
}

void nemo::ThreadPool::startNewThread(std::shared_ptr<IThreadPoolTask> task)
{
	thread t([task]() {
		task->run();
		});
	t.detach();
}

void nemo::ThreadPool::addTask(std::shared_ptr<IThreadPoolTask> task)
{
}

void nemo::ThreadPool::halt(void)
{
	auto& ptr = ThreadPool::instance;
	
	for (size_t i = 0; i < ptr->threadCount; ++i) {
		ptr->lock.lock();

		ptr->lock.unlock();
	}
	
	ptr->threadCount = 0;
	ptr->interval = 0;
}

void nemo::ThreadPool::pause(void)
{
}

void nemo::ThreadPool::resume(void)
{
}