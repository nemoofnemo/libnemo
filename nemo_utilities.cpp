#include "nemo_utilities.h"

using namespace std;
using namespace nemo;

nemo::ThreadPool::ThreadPool(size_t count, unsigned long time)
{
	threadCount = count;
	interval = time;
	status = ThreadPoolStatus::STATUS_TYPE_EXEC;

	lock.lock();
	{
		allocator<thread> alloc;
		thread* threads = alloc.allocate(count);
		for (size_t i = 0; i < count; i++) {
			alloc.construct(threads + i, workerThread, this);
			threads[i].detach();
			alloc.destroy(threads + i);
			this_thread::sleep_for(chrono::milliseconds(time));
		}
		alloc.deallocate(threads, count * sizeof(thread));
	}
	
	lock.unlock();

	cout << "nemo::ThreadPool::init success. count="
		<< count
		<< ", interval="
		<< time
		<< "ms."
		<< endl;
}

nemo::ThreadPool::~ThreadPool()
{
	halt();
	cout << "nemo::ThreadPool::~ThreadPool : destructor called." << endl;
}

void nemo::ThreadPool::workerThread(nemo::ThreadPool* ptr)
{
	shared_ptr<nemo::Task> task;

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
		else if (ptr->status == ThreadPoolStatus::STATUS_TYPE_EXEC) {
			if (ptr->taskList.size()) {
				if (ptr->lock.try_lock()) {
					if (ptr->taskList.size()) {
						task = *(ptr->taskList.begin());
						ptr->taskList.pop_front();
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

void nemo::ThreadPool::startNewThread(std::shared_ptr<nemo::Task> task)
{
	thread t([task]() {
		task->run();
		});
	cout << "nemo::ThreadPool::startNewThread "
		<< t.get_id()
		<< endl;
	t.detach();
}

void nemo::ThreadPool::addTask(std::shared_ptr<nemo::Task> task)
{
	lock.lock();
	if (status != ThreadPoolStatus::STATUS_TYPE_HALT) {
		taskList.push_back(task);
	}
	lock.unlock();
}

void nemo::ThreadPool::removeAll()
{
	lock.lock();
	taskList.clear();
	lock.unlock();
}

void nemo::ThreadPool::exec(void)
{
	while (status != ThreadPoolStatus::STATUS_TYPE_HALT)
		this_thread::sleep_for(chrono::milliseconds(interval));
}

void nemo::ThreadPool::halt(void)
{
	lock.lock();
	status = ThreadPoolStatus::STATUS_TYPE_HALT;
	taskList.clear();
	lock.unlock();

	while (threadCount > 0)
		this_thread::sleep_for(chrono::milliseconds(interval * 2));

	cout << "nemo::ThreadPool::halt" << endl;
}

void nemo::ThreadPool::pause(void)
{
	lock.lock();
	status = ThreadPoolStatus::STATUS_TYPE_PAUSE;
	lock.unlock();
	cout << "nemo::ThreadPool::pause" << endl;
}

void nemo::ThreadPool::resume(void)
{
	lock.lock();
	status = ThreadPoolStatus::STATUS_TYPE_EXEC;
	lock.unlock();
	cout << "nemo::ThreadPool::resume" << endl;
}

std::string nemo::get_random_string(int length)
{
	std::string ret;

	if (length <= 0) {
		return ret;
	}

	auto time = std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::steady_clock::now());
	unsigned key = unsigned(time.time_since_epoch().count() % 86400) + nemp_random_key++;
	srand(key);

	for (int i = 0; i < length; ++i) {
		int random = rand();
		ret += nemo_random_byte_array[random % sizeof(nemo_random_byte_array)];
		srand(random);
	}

	return ret;
}

nemo::EventDispatcher::EventDispatcher()
{

}

nemo::EventDispatcher::~EventDispatcher()
{

}

void nemo::EventDispatcher::clear(void)
{
	data_lock.lock();
	data_map.clear();
	data_lock.unlock();
}

void nemo::EventDispatcher::add_event(const std::string& event)
{
	data_lock.lock();
	data_map[event];
	data_lock.unlock();
}

void nemo::EventDispatcher::remove_event(const std::string& event)
{
	data_lock.lock();
	auto it = data_map.find(event);
	if (it != data_map.end()) {
		data_map.erase(it);
	}
	data_lock.unlock();
}

void nemo::EventDispatcher::add_listener(const std::string& event, std::shared_ptr<nemo::Task> task)
{
	data_lock.lock();
	data_map[event].push_back(task);
	data_lock.unlock();
}

void nemo::EventDispatcher::remove_listener(std::shared_ptr<nemo::Task> task)
{
	data_lock.lock();
	
	data_lock.unlock();
}

void nemo::EventDispatcher::trigger_event(const std::string& event)
{
	data_lock.lock();
	auto target = data_map.find(event);
	if (target != data_map.end()) {
		auto& ls = target->second;
		auto it = ls.begin();
		while (it != ls.end()) {
			(*it)->run();
			it++;
		}
	}
	data_lock.unlock();
}

void nemo::EventDispatcher::debug_show(void)
{
	data_lock.lock();

	data_lock.unlock();
}
