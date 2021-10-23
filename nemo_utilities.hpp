#pragma once
#include <list>
#include <memory>
#include <thread>
#include <chrono>
#include <mutex>

namespace nemo {
	class IThreadPoolTask {
	public:
		virtual void run(void) = 0;
	};

	class ThreadPool final {
	private:
		//thread count
		int count = 0;
		//time in ms
		unsigned int interval = 0;
		std::list<std::shared_ptr<IThreadPoolTask>> taskList;

		ThreadPool() {}
		ThreadPool(const ThreadPool&) = delete;
		void operator=(const ThreadPool&) = delete;
		~ThreadPool();

	public:
		//instance
		static std::shared_ptr<ThreadPool> instance;
		static void init(int count = 2, unsigned int time = 10);

		void startNewThread(std::shared_ptr<IThreadPoolTask> task);
		void addTask(std::shared_ptr<IThreadPoolTask> task);
		void exec(void);
		void halt(void);
	};
};
