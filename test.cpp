#include <iostream>
using namespace std;

class Base {
private:
	int m_data;

public:
	Base() {
		m_data = 0;
		cout << "Base construct1 " << this << endl;
	}

	explicit Base(int data) {
		m_data = data;
		cout << "Base construct2 " << this << endl;
	}

	Base(const Base& b) {
		m_data = b.m_data;
		cout << "Base construct3 " << this << endl;
	}

	Base(Base&& b) noexcept {
		m_data = b.m_data;
		b.m_data = 0;
		cout << "Base move construct " << this << "target: " << &b << endl;
	}

	//Base(int data) {
	//	m_data = data;
	//	cout << "Base construct2" << endl;
	//}

	virtual ~Base() {
		cout << "Base destroy " << this << endl;
	}

	void func(void) {
		cout << "Base func " << m_data << endl;
	}

	virtual void test(void) {
		cout << "Base test" << endl;
	}

	Base& operator=(int data) {
		m_data = data;
		cout << "Base operator= int" << endl;
		return *this;
	}

	Base& operator=(const Base& b) {
		m_data = b.m_data;
		cout << "Base operator= const Base& b" << endl;
		return *this;
	}
};

class d1 : public Base {
private:
	int m_data;

public:
	d1() = delete;

	explicit d1(int data) :Base(data) {
		m_data = data;
		cout << "d1 construct1" << endl;
	}

	virtual ~d1() {
		cout << "d1 destroy" << endl;
	}

	void func(void) {
		cout << "d1 func " << m_data << endl;
	}

	virtual void test(void) {
		cout << "d1 test" << endl;
	}
};

Base baseTest(void) {
	Base b(5);
	return b;
}

int main(void) {
	Base base = baseTest();

	return 0;
}