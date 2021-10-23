#include "nemo_utilities.hpp"
#include <iostream>

using namespace std;

void func(void) {
	cout << "thread: " << this_thread::get_id() << " start" << endl;
	for (int i = 0; i < 10; i++) {
		cout << "thread: " << this_thread::get_id() << " count: " << i << endl;
		this_thread::sleep_for(chrono::milliseconds(500));
	}
}

int main(void) {
	cout << "main start" << endl;
	thread t1(func);
	thread t2(func);
	cout << "kkkk" << endl;
	t1.detach();
	cout << "ssss" << endl;
	t2.detach();
	cout << "end";
	this_thread::sleep_for(chrono::seconds(2));
	return 0;
}