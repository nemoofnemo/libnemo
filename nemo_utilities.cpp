#include "nemo_utilities.hpp"

using namespace std;
using namespace nemo;

std::shared_ptr<ThreadPool> nemo::ThreadPool::instance = nullptr;

nemo::ThreadPool::ThreadPool()
{
	
}

nemo::ThreadPool::~ThreadPool()
{
	ThreadPool::instance->halt();
	ThreadPool::instance.reset();
	cout << "nemo::ThreadPool::~ThreadPool : destructor called." << endl;
}

void nemo::ThreadPool::workerThread(void)
{
	auto& ptr = ThreadPool::instance;
	shared_ptr<IThreadPoolTask> task;

	cout << "nemo::ThreadPool::workerThread " 
		<< this_thread::get_id() 
		<< " running." 
		<< endl;

	for (;;) {
		if (ptr->status == ThreadPoolStatus::STATUS_TYPE_HALT)
			break;
		else if (ptr->status == ThreadPoolStatus::STATUS_TYPE_PAUSE) {
			this_thread::sleep_for(chrono::milliseconds(ptr->interval));
		}
		else if (ptr->status == ThreadPoolStatus::STATUS_TYPE_NONE) {
			if (ptr->taskList.size()) {
				if (ptr->lock.try_lock()) {
					if (ptr->taskList.size()) {
						task = *(ptr->taskList.begin());
					}
					ptr->lock.unlock();
					task->run();
				}
			}
			this_thread::sleep_for(chrono::milliseconds(ptr->interval));
		}
		else {
			cout << "nemo::ThreadPool::workerThread " 
				<< this_thread::get_id() 
				<< " invalid task type." 
				<< endl;
			break;
		}
	}

	ptr->lock.lock();
	ptr->threadCount--;
	ptr->lock.unlock();

	cout << "nemo::ThreadPool::workerThread " 
		<< this_thread::get_id() 
		<< " exit." 
		<< endl;
}

std::shared_ptr<ThreadPool>& nemo::ThreadPool::init(size_t count, unsigned long time)
{
	if (ThreadPool::instance) {
		ThreadPool::instance->halt();
	}

	ThreadPool::instance.reset(new ThreadPool());
	ThreadPool::instance->lock.lock();
	allocator<thread> alloc;
	thread* threads = alloc.allocate(count);
	alloc.construct(threads, workerThread);
	for (size_t i = 0; i < count; i++) {
		threads[i].detach();
	}
	delete[] threads;
	ThreadPool::instance->threadCount = count;
	ThreadPool::instance->interval = time;
	ThreadPool::instance->status = ThreadPoolStatus::STATUS_TYPE_NONE;
	ThreadPool::instance->lock.lock();

	cout << "nemo::ThreadPool::init success. count="
		<< count
		<< ", interval="
		<< time
		<< "ms."
		<< endl;

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
	cout << "nemo::ThreadPool::startNewThread "
		<< t.get_id()
		<< endl;
	t.detach();
}

void nemo::ThreadPool::addTask(std::shared_ptr<IThreadPoolTask> task)
{
	auto& ptr = ThreadPool::instance;
	ptr->lock.lock();
	if (ptr->status != ThreadPoolStatus::STATUS_TYPE_HALT) {
		ptr->taskList.push_back(task);
	}
	ptr->lock.unlock();
}

void nemo::ThreadPool::removeAll()
{
	auto& ptr = ThreadPool::instance;
	ptr->lock.lock();
	ptr->taskList.clear();
	ptr->lock.unlock();
}

void nemo::ThreadPool::halt(void)
{
	auto& ptr = ThreadPool::instance;
	ptr->lock.lock();
	ptr->status = ThreadPoolStatus::STATUS_TYPE_HALT;
	ptr->taskList.clear();
	ptr->lock.unlock();
	cout << "nemo::ThreadPool::halt" << endl;
}

void nemo::ThreadPool::pause(void)
{
	auto& ptr = ThreadPool::instance;
	ptr->lock.lock();
	ptr->status = ThreadPoolStatus::STATUS_TYPE_PAUSE;
	ptr->lock.unlock();
	cout << "nemo::ThreadPool::pause" << endl;
}

void nemo::ThreadPool::resume(void)
{
	auto& ptr = ThreadPool::instance;
	ptr->lock.lock();
	ptr->status = ThreadPoolStatus::STATUS_TYPE_NONE;
	ptr->lock.unlock();
	cout << "nemo::ThreadPool::resume" << endl;
}