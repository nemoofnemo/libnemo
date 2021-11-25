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
	for(int i = 0; i < 20; ++i)
		cout << get_random_string(6) << endl;

	return 0;
}