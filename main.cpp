#include "nemo_utilities.hpp"
#include <iostream>

using namespace std;

int main(void) {
	// Initialize two separate raw pointers.
// Note that they contain the same values.
	auto int1 = new int(1);
	auto int2 = new int(2);

	// Create two unrelated shared_ptrs.
	shared_ptr<int> p1(int1);
	shared_ptr<int> p2(int2);

	// Unrelated shared_ptrs are never equal.
	cout << "p1 < p2 = " << std::boolalpha << (p1 < p2) << endl;
	cout << "p1 == p2 = " << std::boolalpha << (p1 == p2) << endl;

	// Related shared_ptr instances are always equal.
	shared_ptr<int> p3(p2);
	cout << "p3 == p2 = " << std::boolalpha << (p3 == p2) << endl;
	return 0;
}