#include "nemo_utilities.h"

using namespace std;
using namespace nemo;

class cb : public nemo::Task {
private:
	string m_str;
public:
	cb(string str) {
		m_str = str;
	}
	void run(void) {
		cout << m_str << endl;
	}
};

void test_event_dispatcher(void) {
	EventDispatcher ed;
	std::shared_ptr<cb> c1 = std::make_shared<cb>("qqqq");
	std::shared_ptr<cb> c2 = std::make_shared<cb>("kkkk");
	std::shared_ptr<cb> c3 = std::make_shared<cb>("aaaa");

	ed.add_event("wasted");
	ed.add_listener("fuck", c1);
	ed.add_listener("fuck", c2);
	ed.add_listener("shit", c3);

	ed.debug_show();
	ed.trigger_event("wasted");
	ed.trigger_event("fuck");
	ed.trigger_event("shit");

	ed.remove_event("aaa");
	ed.remove_event("wasted");
	ed.remove_event("shit");
	ed.debug_show();

	ed.remove_listener("fuck", c1);
	ed.remove_listener("fuck", c1);
	ed.remove_listener("fuck", nullptr);
	ed.debug_show();

	ed.trigger_event("shit");
	ed.trigger_event("fuck");
}

void test_random_string(void) {
	for (int i = 0; i < 20; i++) {
		cout << get_random_string(8) << endl;
	}
}

void test_byte_array(void) {
	const char str[] = "0123456789";
	const char str2[] = "qwerty";
	char cont[100] = { 0 };

	ByteArray arr1;
	ByteArray arr2(str, sizeof(str));
	ByteArray arr3(arr2);

	cout << "init test" << endl;

	arr1.debug_show();
	arr2.debug_show();
	if (arr2.size() != sizeof(str)) {
		cout << "error0" << endl;
	}
	arr3.debug_show();
	cout << "arr3.size() : " << arr3.size() << endl;

	arr1 = arr3;
	arr1.debug_show();
	arr1.clear();
	arr1.debug_show();

	cout << "----read test----" << endl;

	auto ret = arr2.read_all(&arr1);
	arr1.debug_show();
	if (ret != arr2.size()) {
		cout << "error1" << endl;
	}

	ret = arr2.read(&arr1, 1, 5);
	if (ret != 4) {
		cout << "error2" << endl;
	}
	arr1.debug_show();

	ret = arr2.read(&arr1, 1, 6);
	if (ret != 5) {
		cout << "error2.1" << endl;
	}
	arr1.debug_show();

	ret = arr1.read_all(cont, 100);
	cont[ret] = '\0';
	if (ret != arr1.size()) {
		cout << "error3" << endl;
	}
	cout << "cont-->" << cont << endl;

	ret = arr2.read(cont, 100, 1, 5);
	cont[ret] = '\0';
	if (ret != 4) {
		cout << "error4" << endl;
	}
	cout << "cont: " << cont << endl;

	ret = arr2.read(cont, 2, 1, 5);
	cont[2] = '\0';
	if (ret != 2) {
		cout << "error5" << endl;
	}
	cout << "cont: " << cont << endl;
	
	cout << "----write test----" << endl;
	arr1.clear();

	ret = arr1.write(str, 0, 6);
	if (ret != 6) {
		cout << "error6" << endl;
	}
	arr1.debug_show();

	ret = arr1.write(&arr2, arr1.size(), 2);
	if (ret != 2) {
		cout << "error6.1" << endl;
	}
	arr1.debug_show();

	ret = arr1.write(&arr2, arr1.size(), 6);
	if (ret != 6) {
		cout << "error6.2" << endl;
	}
	arr1.debug_show();

	ret = arr1.write(&arr2, 2, 4);
	if (ret != 4) {
		cout << "error6.3" << endl;
	}
	arr1.debug_show();

	arr1.clear();
	arr1.append(str2, sizeof(str2) - 1);
	arr1.debug_show();
	arr1.append(&arr3, 4);
	arr1.debug_show();
	
	cout << "----split test----" << endl;
	arr1 = arr2;
	arr1.split(1, 6);
	arr1.debug_show();
	arr1.write(str2, 0, sizeof(str2) - 1);
	arr1 = nemo::ByteArray::split(&arr2, 2, 7);
	arr1.debug_show();
}
