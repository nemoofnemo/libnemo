#include "nemo_utilities.hpp"
#include <iostream>

using namespace std;

struct ab {
	int a = 10;
	~ab() {
		cout << "~ab" << endl;
	}
};

struct cd {
	ab a;
	~cd() {
		cout << "~cd" << endl;
 	}
};

void func(void) {
	
}

int main(void) {
	auto ptr1 = make_shared<cd>();
	shared_ptr<ab> ptr2(ptr1, &ptr1->a);
	return 0;
}