#include "nemo_utilities.h"

#ifdef _MSC_VER
void nemo::debug_log(const std::string& str) {
#ifndef _DEBUG
	return;
#else
	OutputDebugStringA(str.c_str());
	OutputDebugStringA("\n");
	std::cout << str << std::endl;
#endif
}
#else
void nemo::debug_log(const std::string& str) {
#ifndef _DEBUG
	return;
#else
	std::cerr << str << std::endl;
#endif
}
#endif // _MSC_VER

nemo::ThreadPool::ThreadPool(size_t count, unsigned long time)
{
	threadCount = count;
	interval = time;
	status = ThreadPoolStatus::STATUS_TYPE_EXEC;

	{
		std::lock_guard<std::mutex> lg(lock);
		std::allocator<std::thread> alloc;
		std::thread* threads = alloc.allocate(count);
		for (size_t i = 0; i < count; i++) {
			alloc.construct(threads + i, workerThread, this);
			threads[i].detach();
			alloc.destroy(threads + i);
			std::this_thread::sleep_for(std::chrono::milliseconds(time));
		}
		alloc.deallocate(threads, count * sizeof(std::thread));
	}

	std::cout << "nemo::ThreadPool::init success. count="
		<< count
		<< ", interval="
		<< time
		<< "ms."
		<< std::endl;
}

nemo::ThreadPool::~ThreadPool()
{
	halt();
	std::cout << "nemo::ThreadPool::~ThreadPool : destructor called." << std::endl;
}

void nemo::ThreadPool::workerThread(nemo::ThreadPool* ptr)
{
	std::shared_ptr<nemo::Task> task;

	std::cout << "nemo::ThreadPool::workerThread "
		<< std::this_thread::get_id()
		<< " running."
		<< std::endl;

	for (;;) {
		if (ptr->status == ThreadPoolStatus::STATUS_TYPE_HALT)
			break;
		else if (ptr->status == ThreadPoolStatus::STATUS_TYPE_PAUSE) {
			std::this_thread::sleep_for(std::chrono::milliseconds(ptr->interval));
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
			std::this_thread::sleep_for(std::chrono::milliseconds(ptr->interval));
		}
		else {
			std::cout << "nemo::ThreadPool::workerThread "
				<< std::this_thread::get_id()
				<< " invalid task type."
				<< std::endl;
			break;
		}
	}

	ptr->lock.lock();
	ptr->threadCount--;
	ptr->lock.unlock();

	std::cout << "nemo::ThreadPool::workerThread "
		<< std::this_thread::get_id()
		<< " exit."
		<< std::endl;
}

void nemo::ThreadPool::start_new_thread(std::shared_ptr<nemo::Task> task)
{
	if (!task)
		return;

	std::thread t([task]() {
		task->run();
		});
	std::cout << "nemo::ThreadPool::startNewThread "
		<< t.get_id()
		<< std::endl;
	t.detach();
}

void nemo::ThreadPool::add_task(std::shared_ptr<nemo::Task> task)
{
	if (!task)
		return;

	std::lock_guard<std::mutex> lg(lock);
	if (status != ThreadPoolStatus::STATUS_TYPE_HALT) {
		taskList.push_back(task);
	}
}

void nemo::ThreadPool::remove_all()
{
	std::lock_guard<std::mutex> lg(lock);
	taskList.clear();
}

void nemo::ThreadPool::exec(void)
{
	while (status != ThreadPoolStatus::STATUS_TYPE_HALT)
		std::this_thread::sleep_for(std::chrono::milliseconds(interval));
}

void nemo::ThreadPool::halt(void)
{
	{
		std::lock_guard<std::mutex> lg(lock);
		status = ThreadPoolStatus::STATUS_TYPE_HALT;
		taskList.clear();
	}

	while (threadCount > 0)
		std::this_thread::sleep_for(std::chrono::milliseconds(interval));

	std::cout << "nemo::ThreadPool::halt" << std::endl;
}

void nemo::ThreadPool::pause(void)
{
	lock.lock();
	status = ThreadPoolStatus::STATUS_TYPE_PAUSE;
	lock.unlock();
	std::cout << "nemo::ThreadPool::pause" << std::endl;
}

void nemo::ThreadPool::resume(void)
{
	lock.lock();
	status = ThreadPoolStatus::STATUS_TYPE_EXEC;
	lock.unlock();
	std::cout << "nemo::ThreadPool::resume" << std::endl;
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
	std::lock_guard<std::mutex> lg(data_lock);
	data_map.clear();
}

void nemo::EventDispatcher::add_event(const std::string& event)
{
	std::lock_guard<std::mutex> lg(data_lock);
	data_map[event];
}

void nemo::EventDispatcher::remove_event(const std::string& event)
{
	std::lock_guard<std::mutex> lg(data_lock);
	auto it = data_map.find(event);
	if (it != data_map.end()) {
		data_map.erase(it);
	}
}

void nemo::EventDispatcher::add_listener(const std::string& event, std::shared_ptr<nemo::Task> task)
{
	if (!task) {
		return;
	}

	std::lock_guard<std::mutex> lg(data_lock);
	data_map[event].push_back(task);
}

void nemo::EventDispatcher::remove_listener(const std::string& event, std::shared_ptr<nemo::Task> task)
{
	if (!task) {
		return;
	}

	std::lock_guard<std::mutex> lg(data_lock);
	
	auto target = data_map.find(event);
	if (target != data_map.end()) {
		auto& ls = target->second;
		auto it = ls.begin();

		while (it != ls.end()) {
			if (*it == task) {
				ls.erase(it);
				break;
			}
			it++;
		}
	}

}

void nemo::EventDispatcher::trigger_event(const std::string& event)
{
	std::lock_guard<std::mutex> lg(data_lock);

	auto target = data_map.find(event);
	if (target != data_map.end()) {
		auto& ls = target->second;
		auto it = ls.begin();
		while (it != ls.end()) {
			(*it)->run();
			it++;
		}
	}
}

void nemo::EventDispatcher::debug_show(void)
{
	std::cout << "----debug_show----" << std::endl;

	std::lock_guard<std::mutex> lg(data_lock);
	
	auto map_it = data_map.begin();
	while (map_it != data_map.end()) {
		std::cout << map_it->first << std::endl << '\t';
		auto ls_it = map_it->second.begin();
		while (ls_it != map_it->second.end()) {
			std::cout << *ls_it << ", ";
			ls_it++;
		}
		std::cout << std::endl;
		map_it++;
	}

	std::cout << "--debug_show end--" << std::endl;
}

nemo::ByteArray::ByteArray() noexcept
{
}

nemo::ByteArray::ByteArray(const void* data, size_t size)
{
	if (!data)
		throw std::invalid_argument("ByteArray: nullptr");
	if(size == 0)
		throw std::invalid_argument("ByteArray: invalid size");

	m_cap = (size % BYTE_ARRAY_ALIGN == 0)? 
		size : (size / BYTE_ARRAY_ALIGN + 1) * BYTE_ARRAY_ALIGN;
	m_ptr = (uint8_t*)malloc(m_cap);
	if (!m_ptr)
		throw;

	m_size = size;
	memcpy_s(m_ptr, m_cap, data, size);
}

nemo::ByteArray::ByteArray(const ByteArray& arr)
{
	if (!arr.m_ptr)
		return;

	m_cap = arr.m_cap;
	m_size = arr.m_size;
	m_ptr = (uint8_t*)malloc(m_cap);
	if (!m_ptr)
		throw;
	memcpy_s(m_ptr, m_cap, arr.m_ptr, arr.m_size);
}

nemo::ByteArray::ByteArray(ByteArray&& arr) noexcept
{
	if (!arr.m_ptr)
		return;

	m_cap = arr.m_cap;
	m_size = arr.m_size;
	m_ptr = arr.m_ptr;

	arr.m_ptr = nullptr;
	arr.m_cap = 0;
	arr.m_size = 0;
}

nemo::ByteArray::~ByteArray() noexcept
{
	if (m_ptr)
		free(m_ptr);
}

nemo::ByteArray& nemo::ByteArray::operator+=(const ByteArray& right)
{
	if (!right.m_ptr)
		return *this;

	if (m_size + right.m_size < m_size) {
		throw std::overflow_error("nemo::ByteArray::operator+");
		return *this;
	}

	if (m_cap >= m_size + right.m_size) {
		memcpy_s(m_ptr + m_size, m_cap - m_size, right.m_ptr, right.m_size);
		m_size += right.m_size;
	}
	else {
		size_t target = ((m_size + right.m_size) % BYTE_ARRAY_ALIGN == 0) ?
			m_size + right.m_size : ((m_size + right.m_size) / BYTE_ARRAY_ALIGN + 1) * BYTE_ARRAY_ALIGN;
		uint8_t* tmp = m_ptr;

		if ((m_ptr = (uint8_t*)realloc(m_ptr, target)) == nullptr) {
			free(tmp);
			throw;
		}

		memcpy_s(m_ptr + m_size, target - m_size, right.m_ptr, right.m_size);
		m_size += right.m_size;
		m_cap = target;
	}

	return *this;
}

nemo::ByteArray& nemo::ByteArray::operator=(const ByteArray& right)
{
	if (m_ptr) {
		delete[] m_ptr;
	}
	
	if (right.m_ptr) {
		m_ptr = new uint8_t[right.m_cap];
		m_size = right.m_size;
		m_cap = right.m_cap;
		memcpy_s(m_ptr, m_cap, right.m_ptr, right.m_size);
	}
	else {
		m_ptr = nullptr;
		m_size = 0;
		m_cap = 0;
	}

	return *this;
}

uint8_t& nemo::ByteArray::operator[](const size_t index)
{
	if (index >= m_size)
		throw std::out_of_range("nemo::ByteArray::operator[]");

	return *(m_ptr + index);
}

nemo::ByteArray& nemo::ByteArray::operator=(ByteArray&& right) noexcept
{
	m_ptr = right.m_ptr;
	m_cap = right.m_cap;
	m_size = right.m_size;
	right.m_ptr = nullptr;
	right.m_cap = 0;
	right.m_size = 0;
	return *this;
}

nemo::ByteArray nemo::operator+(const ByteArray& left, const ByteArray& right)
{
	if (left.m_cap + right.m_cap < left.m_cap)
		throw std::out_of_range("nemo::operator+");

	uint8_t* tmp_ptr = (uint8_t*)malloc(left.m_cap + right.m_cap);
	if (!tmp_ptr)
		throw;

	nemo::ByteArray arr;
	arr.m_ptr = tmp_ptr;
	arr.m_size = left.m_size + right.m_size;
	arr.m_cap = left.m_cap + right.m_cap;

	return arr;
}

size_t nemo::ByteArray::read_all(void* out, size_t buf_size)
{
	return nemo::ByteArray::read(out, buf_size, 0, m_size);
}

size_t nemo::ByteArray::read_all(ByteArray* arr)
{
	if (!arr)
		throw std::invalid_argument("nemo::ByteArray::read_all: nullptr");

	return nemo::ByteArray::read(arr, 0, m_size);
}

size_t nemo::ByteArray::read(void* out, size_t buf_size, size_t start, size_t end)
{
	if(!out)
		throw std::invalid_argument("nemo::ByteArray::read: nullptr");
	if(end < start)
		throw std::invalid_argument("nemo::ByteArray::read");
	if (start >= m_size || end > m_size)
		throw std::out_of_range("nemo::ByteArray::read");
	if (buf_size == 0 || start == end)
		return 0;

	if (buf_size >= end - start) {
		memcpy_s(out, buf_size, m_ptr + start, end - start);
		return end - start;
	}
	else {
		memcpy_s(out, buf_size, m_ptr + start, buf_size);
		return buf_size;
	}
}

size_t nemo::ByteArray::read(ByteArray* arr, size_t start, size_t end)
{
	if (!arr)
		throw std::invalid_argument("nemo::ByteArray::read: nullptr");
	if (start > end)
		throw std::invalid_argument("nemo::ByteArray::read");
	if(start >= m_size || end > m_size)
		throw std::invalid_argument("nemo::ByteArray::read");
	if (start == end)
		return 0;

	if (arr->m_ptr)
		free(arr->m_ptr);

	if (m_ptr) {
		arr->m_size = end - start;
		arr->m_cap = (arr->m_size % BYTE_ARRAY_ALIGN == 0) ?
			arr->m_size : (arr->m_size / BYTE_ARRAY_ALIGN + 1) * BYTE_ARRAY_ALIGN;
		arr->m_ptr = (uint8_t*)malloc(arr->m_cap);
		memcpy_s(arr->m_ptr, arr->m_cap, m_ptr + start, arr->m_size);
		return arr->m_size;
	}
	else {
		arr->m_ptr = nullptr;
		arr->m_size = 0;
		arr->m_cap = 0;
		return 0;
	}
}

size_t nemo::ByteArray::write(const void* in, size_t loc, size_t len)
{
	if (!in)
		throw std::invalid_argument("nemo::ByteArray::write: nullptr");
	if (loc > m_size)
		throw std::out_of_range("nemo::ByteArray::write");
	if (len == 0)
		return 0;

	if (loc + len <= m_cap) {
		memcpy_s(m_ptr + loc, m_cap - loc, in, len);
		if (loc + len > m_size)
			m_size = loc + len;
	}
	else {
		uint8_t* tmp = m_ptr;
		size_t target = ((loc + len) % nemo::BYTE_ARRAY_ALIGN == 0) ?
			loc + len : ((loc + len) / nemo::BYTE_ARRAY_ALIGN + 1) * nemo::BYTE_ARRAY_ALIGN;
		if ((m_ptr = (uint8_t*)realloc(m_ptr, target)) == nullptr) {
			free(tmp);
			throw;
		}
		memcpy_s(m_ptr + loc, target - loc, in, len);
		m_size = loc + len;
		m_cap = target;
	}

	return len;
}

size_t nemo::ByteArray::write(const ByteArray* arr, size_t loc, size_t len)
{
	if (!arr)
		throw std::invalid_argument("nemo::ByteArray::read: nullptr");
	if (loc > m_size)
		throw std::invalid_argument("nemo::ByteArray::write");
	if (len == 0)
		return 0;

	size_t len2 = (len >= arr->m_size) ?
		arr->m_size : len;

	if (loc + len2 <= m_cap) {
		memcpy_s(m_ptr + loc, m_cap - loc, arr->m_ptr, len2);
		if (loc + len2 > m_size)
			m_size = loc + len2;
	}
	else {
		uint8_t* tmp = m_ptr;
		size_t target = ((loc + len2) % nemo::BYTE_ARRAY_ALIGN == 0) ?
			loc + len2 : ((loc + len2) / nemo::BYTE_ARRAY_ALIGN + 1) * nemo::BYTE_ARRAY_ALIGN;
		if ((m_ptr = (uint8_t*)realloc(m_ptr, target)) == nullptr) {
			free(tmp);
			throw;
		}
		memcpy_s(m_ptr + loc, target - loc, arr->m_ptr, len2);
		m_size = loc + len2;
		m_cap = target;
	}

	return len2;
}

size_t nemo::ByteArray::append(const void* in, size_t len)
{
	return nemo::ByteArray::write(in, m_size, len);
}

size_t nemo::ByteArray::append(const ByteArray* arr, size_t len)
{
	return nemo::ByteArray::write(arr, m_size, len);
}

size_t nemo::ByteArray::split(size_t start, size_t end)
{
	if (start > end)
		throw std::invalid_argument("nemo::ByteArray::split");
	if (start >= m_size || end > m_size)
		throw std::invalid_argument("nemo::ByteArray::split");
	if (start == end)
		return 0;

	size_t target = (end - start) % nemo::BYTE_ARRAY_ALIGN == 0 ?
		end - start : ((end - start) / nemo::BYTE_ARRAY_ALIGN + 1) * nemo::BYTE_ARRAY_ALIGN;
	auto tmp_ptr = malloc(target);
	
	if (!tmp_ptr)
		throw;

	memcpy_s(tmp_ptr, target, m_ptr + start, end - start);
	free(m_ptr);
	m_ptr = (uint8_t*)tmp_ptr;
	m_size = end - start;
	m_cap = target;

	return m_size;
}

nemo::ByteArray nemo::ByteArray::split(const ByteArray* arr, size_t start, size_t end)
{
	if (!arr)
		throw std::invalid_argument("nemo::ByteArray::split");
	if(start > end)
		throw std::invalid_argument("nemo::ByteArray::split");
	if(start >= arr->m_size || end > arr->m_size)
		throw std::invalid_argument("nemo::ByteArray::split");
	if (start == end)
		return nemo::ByteArray();

	return nemo::ByteArray(arr->m_ptr + start, end - start);
}

void nemo::ByteArray::clear(void) noexcept{
	if (m_ptr) {
		m_cap = 0;
		m_size = 0;
		free(m_ptr);
		m_ptr = nullptr;
	}
}

size_t nemo::ByteArray::size(void) noexcept {
	return m_size;
}