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
	if (!ThreadPool::instance) {
		ThreadPool::instance.reset(new ThreadPool());
		
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
	auto& ptr = ThreadPool::instance;
	ptr->lock.lock();
	if (ptr->status != ThreadPoolTaskType::TASK_TYPE_HALT) {
		ptr->taskList.push_back(task);
	}
	ptr->lock.unlock();
}

void nemo::ThreadPool::halt(void)
{
	auto& ptr = ThreadPool::instance;
	ptr->lock.lock();
	ptr->status = ThreadPoolTaskType::TASK_TYPE_HALT;
	ptr->taskList.clear();
	for (size_t i = 0; i < ptr->threadCount; ++i) {
		auto task = make_shared<nemo::IThreadPoolTask>();
		task->type = ThreadPoolTaskType::TASK_TYPE_HALT;
		ptr->taskList.push_back(task);
	}
	ptr->lock.unlock();
}

void nemo::ThreadPool::pause(void)
{
	auto& ptr = ThreadPool::instance;
	ptr->lock.lock();
	ptr->status = ThreadPoolTaskType::TASK_TYPE_PAUSE;
	ptr->lock.unlock();
}

void nemo::ThreadPool::resume(void)
{
	auto& ptr = ThreadPool::instance;
	ptr->lock.lock();
	ptr->status = ThreadPoolTaskType::TASK_TYPE_NONE;
	ptr->lock.unlock();
}