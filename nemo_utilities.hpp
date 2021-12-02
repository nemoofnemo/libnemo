#pragma once
#include <list>
#include <memory>
#include <thread>
#include <chrono>
#include <mutex>
#include <iostream>
#include <map>

namespace nemo {

	static int nemp_random_key = 0;
	static char nemo_random_byte_array[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
			'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
			'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
			'_' };

	std::string get_random_string(int length);

	class IThreadPoolTask
	{
	public:
		virtual void run(void) = 0;
	};

	class ThreadPool final
	{
	private:
		enum class ThreadPoolStatus
		{
			STATUS_TYPE_NONE,
			STATUS_TYPE_HALT,
			STATUS_TYPE_PAUSE,
			STATUS_TYPE_RESUME
		};

		//instance
		static std::shared_ptr<ThreadPool> thread_pool_instance;
		//only worker thread, not include thread start with startNewThread()
		size_t threadCount = 0;
		//time in ms
		unsigned int interval = 0;
		//ThreadPool status
		ThreadPoolStatus status = ThreadPoolStatus::STATUS_TYPE_PAUSE;
		//mutex for task list
		std::mutex lock;
		std::list<std::shared_ptr<IThreadPoolTask>> taskList;

		ThreadPool();
		ThreadPool(const ThreadPool&) = delete;
		ThreadPool(ThreadPool&&) = delete;
		void operator=(const ThreadPool&) = delete;

		static void workerThread(void);

	public:

		//call halt() and wait until all the threads are exit.
		~ThreadPool();

		//init() must called before get instance
		static std::shared_ptr<ThreadPool>& init(size_t count = 2, unsigned long time = 10);

		//init() must called before get instance
		static std::shared_ptr<ThreadPool>& get(void);

		//discard all tasks, terminate all threads.after halt(), addTask() will not add new task to thread pool
		void halt(void);

		//pause all worker thread
		void pause(void);

		//resume from pause
		void resume(void);

		//start a new individual thread
		void startNewThread(std::shared_ptr<IThreadPoolTask> task);

		//add a new task to rear of list
		void addTask(std::shared_ptr<IThreadPoolTask> task);

		//clear taskList, remove all tasks
		void removeAll();

		//main thread loop
		void exec(void);

	};

	typedef void (*EventDispatcherCallback)(void*);

	class EventDispatcher final{
	private:
		static std::shared_ptr<EventDispatcher> event_dispatcher_instance;

		typedef std::list<EventDispatcherCallback> ListenerList;
		typedef std::list<EventDispatcherCallback>::iterator ListenerIterator;
		typedef std::map<std::string, ListenerList> EventDispatcherMap;
		typedef std::map<std::string, ListenerList>::iterator EventDispatcherMapIterator;
		EventDispatcherMap data;

		EventDispatcher();
		EventDispatcher(const EventDispatcher&) = delete;
		EventDispatcher(const EventDispatcher&&) = delete;
		EventDispatcher& operator=(const EventDispatcher&) = delete;
		~EventDispatcher();

	public:
		static std::shared_ptr<EventDispatcher> instance(void);
		void clear(void);
		void add_event(const std::string& event);
		void remove_event(const std::string& event);
		void add_listener(const std::string& event, EventDispatcherCallback cb);
		void remove_listener(const std::string& event);
		void trigger_event(const std::string& event, void* arg);
	};
};
