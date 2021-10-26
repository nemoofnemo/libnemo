#include "nemo_utilities.hpp"

using namespace std;
using namespace nemo;

class task : public IThreadPoolTask {
public:
	int m_num = 0;
	task(int num) {
		m_num = num;
	}
	void run(void) {
		cout << "task" << m_num << endl;
	}
};

int main(void) {
	ThreadPool::init();
	auto& tp = ThreadPool::get();
	for (int i = 0; i < 10; i++) {
		tp->addTask(make_shared<task>(i));
	}
	
	return 0;
}