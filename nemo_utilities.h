#pragma once

#include <list>
#include <memory>
#include <thread>
#include <chrono>
#include <mutex>
#include <iostream>
#include <map>
#include <exception>
#include <cstdarg>
#include <cstring>

#ifdef _MSC_VER
#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#define new DBG_NEW
//call _CrtDumpMemoryLeaks() at exit of program.
#endif // _DEBUG
#include <Windows.h>
#endif

namespace nemo {

	void debug_log(const std::string& str);

	static constexpr size_t BYTE_ARRAY_ALIGN = sizeof(size_t);
	static int nemp_random_key = 0;
	static char nemo_random_byte_array[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
			'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
			'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z' };

	std::string get_random_string(int length);

	class ByteArray {
	private:
		uint8_t* m_ptr = nullptr;
		size_t m_size = 0;
		size_t m_cap = 0;

	public:
		ByteArray() noexcept;
		ByteArray(const void* data, size_t size);
		ByteArray(const ByteArray& arr);
		ByteArray(ByteArray&& arr) noexcept;
		virtual ~ByteArray() noexcept;

		ByteArray& operator+=(const ByteArray& right);
		ByteArray& operator=(const ByteArray& right);
		uint8_t& operator[](const size_t index);
		ByteArray& operator=(ByteArray&& right) noexcept;
		friend ByteArray operator+(const ByteArray& left, const ByteArray& right);

		size_t read_all(void* out, size_t buf_size);
		size_t read_all(ByteArray* arr);
		size_t read(void* out,size_t buf_size, size_t start, size_t end);
		size_t read(ByteArray* arr, size_t start, size_t end);
		size_t write(const void* in, size_t loc, size_t len);
		size_t write(const ByteArray* arr, size_t loc, size_t len);
		size_t append(const void* in, size_t len);
		size_t append(const ByteArray* arr, size_t len);
		size_t split(size_t start, size_t end);
		static ByteArray split(const ByteArray* arr, size_t start, size_t end);
		void clear(void) noexcept;
		size_t size(void) noexcept;

#ifdef _DEBUG
		void debug_show(void) {
			if (!m_ptr) {
				std::cout << this << "--> empty." << std::endl;
				return;
			}
			char* ptr = new char[m_size + 1];
			memcpy_s(ptr, m_size + 1, m_ptr, m_size);
			ptr[m_size] = '\0';
			std::cout << this << "--> m_size:" << m_size <<", m_cap: " 
				<< m_cap << ", content:\n\t" << ptr << std::endl;
			delete[] ptr;
		}
#endif
	};

	nemo::ByteArray operator+(const ByteArray& left, const ByteArray& right);

	class Task {
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
		void start_new_thread(std::shared_ptr<Task> task);

		//add a new task to rear of list
		void add_task(std::shared_ptr<Task> task);

		//clear taskList, remove all tasks
		void remove_all();

		//main thread loop
		void exec(void);

	};

	class EventDispatcher final {
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

		//remove all event and listener
		void clear(void);

		//add event
		void add_event(const std::string& event);

		//remove specific event and its listeners
		void remove_event(const std::string& event);

		//add a task to event.(callback to event)
		void add_listener(const std::string& event, std::shared_ptr<nemo::Task> task);

		//remove specific listener
		void remove_listener(const std::string& event, std::shared_ptr<nemo::Task> task);

		//trigger specific event's callback
		void trigger_event(const std::string& event);

		//for debug: display all data via cout.
		void debug_show(void);
	};
};
