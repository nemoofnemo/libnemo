#include "nemo_utilities.hpp"
#include <iostream>

using namespace std;

int main(void) {
	static shared_ptr<int> ptr;
	cout << boolalpha << (ptr == nullptr);
	return 0;
}