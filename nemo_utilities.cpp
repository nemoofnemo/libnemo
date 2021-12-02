#include "nemo_utilities.hpp"

using namespace std;
using namespace nemo;

std::shared_ptr<ThreadPool> nemo::ThreadPool::thread_pool_instance = nullptr;
std::shared_ptr<EventDispatcher> nemo::EventDispatcher::event_dispatcher_instance = nullptr;

nemo::ThreadPool::ThreadPool()
{
	
}

nemo::ThreadPool::~ThreadPool()
{
	ThreadPool::thread_pool_instance->halt();
	ThreadPool::thread_pool_instance.reset();
	cout << "nemo::ThreadPool::~ThreadPool : destructor called." << endl;
}

void nemo::ThreadPool::workerThread(void)
{
	auto& ptr = ThreadPool::thread_pool_instance;
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

std::shared_ptr<ThreadPool>& nemo::ThreadPool::init(size_t count, unsigned long time)
{
	if (ThreadPool::thread_pool_instance) {
		ThreadPool::thread_pool_instance->halt();
	}

	ThreadPool::thread_pool_instance.reset(new ThreadPool());
	ThreadPool::thread_pool_instance->lock.lock();
	allocator<thread> alloc;
	thread* threads = alloc.allocate(count);
	for (size_t i = 0; i < count; i++) {
		alloc.construct(threads + i, workerThread);
		threads[i].detach();
		alloc.destroy(threads + i);
		this_thread::sleep_for(chrono::milliseconds(time));
	}
	alloc.deallocate(threads, count * sizeof(thread));
	ThreadPool::thread_pool_instance->threadCount = count;
	ThreadPool::thread_pool_instance->interval = time;
	ThreadPool::thread_pool_instance->status = ThreadPoolStatus::STATUS_TYPE_NONE;
	ThreadPool::thread_pool_instance->lock.unlock();

	cout << "nemo::ThreadPool::init success. count="
		<< count
		<< ", interval="
		<< time
		<< "ms."
		<< endl;

	return ThreadPool::thread_pool_instance;
}

std::shared_ptr<ThreadPool>& nemo::ThreadPool::get(void)
{
	return ThreadPool::thread_pool_instance;
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
	auto& ptr = ThreadPool::thread_pool_instance;
	ptr->lock.lock();
	if (ptr->status != ThreadPoolStatus::STATUS_TYPE_HALT) {
		ptr->taskList.push_back(task);
	}
	ptr->lock.unlock();
}

void nemo::ThreadPool::removeAll()
{
	auto& ptr = ThreadPool::thread_pool_instance;
	ptr->lock.lock();
	ptr->taskList.clear();
	ptr->lock.unlock();
}

void nemo::ThreadPool::exec(void)
{
	auto& ptr = ThreadPool::thread_pool_instance;
	while (ptr->status != ThreadPoolStatus::STATUS_TYPE_HALT)
		this_thread::sleep_for(chrono::milliseconds(ptr->interval));
}

void nemo::ThreadPool::halt(void)
{
	auto& ptr = ThreadPool::thread_pool_instance;
	ptr->lock.lock();
	ptr->status = ThreadPoolStatus::STATUS_TYPE_HALT;
	ptr->taskList.clear();
	ptr->lock.unlock();

	while(ptr->threadCount > 0)
		this_thread::sleep_for(chrono::milliseconds(ptr->interval * 2));

	cout << "nemo::ThreadPool::halt" << endl;
}

void nemo::ThreadPool::pause(void)
{
	auto& ptr = ThreadPool::thread_pool_instance;
	ptr->lock.lock();
	ptr->status = ThreadPoolStatus::STATUS_TYPE_PAUSE;
	ptr->lock.unlock();
	cout << "nemo::ThreadPool::pause" << endl;
}

void nemo::ThreadPool::resume(void)
{
	auto& ptr = ThreadPool::thread_pool_instance;
	ptr->lock.lock();
	ptr->status = ThreadPoolStatus::STATUS_TYPE_NONE;
	ptr->lock.unlock();
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

std::shared_ptr<EventDispatcher> nemo::EventDispatcher::instance(void)
{
	if (!event_dispatcher_instance) {
		event_dispatcher_instance = std::make_shared< EventDispatcher>();
	}
	return event_dispatcher_instance;
}

void nemo::EventDispatcher::clear(void)
{
	data.clear();
}

void nemo::EventDispatcher::add_event(const std::string& event)
{
	data[event];
}

void nemo::EventDispatcher::remove_event(const std::string& event)
{
	auto it = data.find(event);
	if (it != data.end()) {
		data.erase(it);
	}
}

void nemo::EventDispatcher::add_listener(const std::string& event, EventDispatcherCallback cb)
{
	data[event].push_back(cb);
}

void nemo::EventDispatcher::trigger_event(const std::string& event, void* arg)
{
	auto target = data.find(event);
	if (target != data.end()) {
		auto& ls = target->second;
		auto it = ls.begin();
		while (it != ls.end()) {
			(*it)(arg);
			it++;
		}
	}
}
