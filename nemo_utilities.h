#pragma once

#include <list>
#include <memory>
#include <thread>
#include <chrono>
#include <mutex>
#include <iostream>
#include <map>
#include <exception>

namespace nemo {

	static int nemp_random_key = 0;
	static char nemo_random_byte_array[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
			'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
			'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z' };

	std::string get_random_string(int length);

	//class RWLock {
	//private:
	//	std::mutex mtx_read;
	//	std::mutex mtx_write;
	//	std::atomic_int read_count = 0;

	//	RWLock(const RWLock&) = delete;
	//	RWLock(const RWLock&&) = delete;
	//	void operator=(const RWLock&) = delete;

	//public:
	//	explicit RWLock();
	//	virtual ~RWLock();

	//	void lock_read();
	//	void unlock_read();

	//	void lock_write();
	//	void unlock_write();
	//};

	class Task
	{
	public:
		virtual void run(void) = 0;
	};

	class ThreadPool final
	{
	private:
		enum class ThreadPoolStatus
		{
			STATUS_TYPE_EXEC,
			STATUS_TYPE_HALT,
			STATUS_TYPE_PAUSE,
		};

		//only worker thread, not include thread start with startNewThread()
		size_t threadCount = 0;
		//time in ms
		unsigned int interval = 0;
		//ThreadPool status
		ThreadPoolStatus status = ThreadPoolStatus::STATUS_TYPE_PAUSE;
		//mutex for task list
		std::mutex lock;
		std::list<std::shared_ptr<Task>> taskList;

		//ThreadPool() = delete;
		ThreadPool(const ThreadPool&) = delete;
		ThreadPool(ThreadPool&&) = delete;
		void operator=(const ThreadPool&) = delete;

		static void workerThread(nemo::ThreadPool* ptr);

	public:

		//count:thread count in thread pool, time in ms
		ThreadPool(size_t count = 2, unsigned long time = 10);

		//call halt() and wait until all the threads are exit.
		~ThreadPool();

		//discard all tasks, terminate all threads.after halt(), addTask() will not add new task to thread pool
		void halt(void);

		//pause all worker thread
		void pause(void);

		//resume from pause
		void resume(void);

		//start a new individual thread
		void startNewThread(std::shared_ptr<Task> task);

		//add a new task to rear of list
		void addTask(std::shared_ptr<Task> task);

		//clear taskList, remove all tasks
		void removeAll();

		//main thread loop
		void exec(void);

	};

	class EventDispatcher final{
	private:
		typedef std::list<std::shared_ptr<nemo::Task>> ListenerList;
		typedef std::list<std::shared_ptr<nemo::Task>>::iterator ListenerIterator;
		typedef std::map<std::string, ListenerList> EventDispatcherMap;
		typedef std::map<std::string, ListenerList>::iterator EventDispatcherMapIterator;
		EventDispatcherMap data_map;
		std::mutex data_lock;

		EventDispatcher(const EventDispatcher&) = delete;
		EventDispatcher(const EventDispatcher&&) = delete;
		EventDispatcher& operator=(const EventDispatcher&) = delete;

	public:
		EventDispatcher();
		~EventDispatcher();

		void clear(void);
		void add_event(const std::string& event);
		void remove_event(const std::string& event);
		void add_listener(const std::string& event, std::shared_ptr<nemo::Task> task);
		void remove_listener(const std::string& event, std::shared_ptr<nemo::Task> task);
		void trigger_event(const std::string& event);
		void debug_show(void);
	};
};
