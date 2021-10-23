#pragma once
#include <list>
#include <memory>

namespace nemo {
	class IThreadPoolTask {
	public:
		virtual void run(void) = 0;
	};

	class ThreadPool {
	private:
		int count;
		unsigned int interval;
	};
};
