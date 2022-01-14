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

int main(void) {
	test_event_dispatcher();
	return 0;
}