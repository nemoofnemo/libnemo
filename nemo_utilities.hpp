#pragma once
#include <list>
#include <memory>
#include <thread>
#include <chrono>
#include <mutex>
#include <iostream>

namespace nemo {
	enum ThreadPoolTaskType {
		TASK_TYPE_NONE,
		TASK_TYPE_HALT,
		TASK_TYPE_PAUSE,
		TASK_TYPE_RESUME
	};

	class IThreadPoolTask {
	public:
		enum ThreadPoolTaskType type = TASK_TYPE_NONE;
		virtual void run(void) = 0;
	};

	class ThreadPool final {
	private:
		//instance
		static std::shared_ptr<ThreadPool> instance;
		//only worker thread, not include thread start with startNewThread()
		size_t threadCount = 0;
		//time in ms
		unsigned int interval = 0;
		//mutex for task list
		std::mutex lock;
		std::list<std::shared_ptr<IThreadPoolTask>> taskList;

		ThreadPool();
		ThreadPool(const ThreadPool&) = delete;
		ThreadPool(ThreadPool&&) = delete;
		void operator=(const ThreadPool&) = delete;
		~ThreadPool();

		static void workerThread(void);

	public:
		//init() must called before get instance
		static std::shared_ptr<ThreadPool>& init(size_t count = 2, unsigned long time = 10);
		//init() must called before get instance
		static std::shared_ptr<ThreadPool>& get(void);
		//discard all tasks
		void halt(void);
		//pause all worker thread
		void pause(void);
		//resume from pause
		void resume(void);
		//start a new individual thread
		void startNewThread(std::shared_ptr<IThreadPoolTask> task);
		//add a new task to rear of list
		void addTask(std::shared_ptr<IThreadPoolTask> task);
		//void exec(void);
		
	};
};
