#include <iostream>
#include <string>

#include "HSM.hpp"

using namespace std;
using namespace HSM;

class State: public HSMState {
public:
	State(string name): name(name) {}
	
	~State() {
		cout << "destroy " << name << endl;
	}
	
	virtual bool can_enter(const HSMInfo *info) const {
		cout << "can_enter " << name << endl;
		return name != "e";
	}
	
	virtual bool can_exit(const HSMInfo *info) const {
		cout << "can_exit " << name << endl;
		return true;
	}
	
	virtual void on_enter(HSMInfo *info) {
		cout << "on_enter " << name << endl;
	}
	
	virtual void on_exit(HSMInfo *info) {
		cout << "on_exit " << name << endl;
	}
	
	virtual bool process_event(HSMInfo *info) {
		cout << "process_event " << name << endl;
		return name == "b";
	}
	
private:
	const string name;
};

int main() {
	HSMachine hsm;
	
	State *a = new State("a");
	State *b = new State("b");
	State *c = new State("c");
	State *d = new State("d");
	State *e = new State("e");
	
	hsm.add_child_state(a);
	hsm.add_child_state(e);
	a->add_child_state(b);
	b->add_child_state(c);
	b->add_child_state(d);
	b->set_default_state(d);
	
	cout << hsm.transition_to(d) << endl; // Enter from no state
	cout << "====" << endl;
	cout << hsm.transition_to(c) << endl; // Up and over to c
	cout << "====" << endl;
	cout << hsm.transition_to(b) << endl; // Enter default state of b
	cout << "====" << endl;
	cout << hsm.transition_to(b) << endl; // Up and back to default state of b
	cout << "====" << endl;
	cout << hsm.transition_to(e) << endl; // can_enter returns false for e; stay in d, return false
	cout << "====" << endl;
	
	cout << hsm.process_event() << endl;  // Event consumed by b
	
	// Pointers destroyed because they are owned by other states
}

/*

Example output

	can_enter a
	can_enter b
	can_enter d
	on_enter a
	on_enter b
	on_enter d
	1
	====
	can_exit d
	can_enter c
	on_exit d
	on_enter c
	1
	====
	can_exit c
	can_enter d
	on_exit c
	on_enter d
	1
	====
	can_exit d
	can_enter d
	on_exit d
	on_enter d
	1
	====
	can_exit d
	can_exit b
	can_exit a
	can_enter e
	0
	====
	process_event d
	process_event b
	1
	destroy e
	destroy a
	destroy b
	destroy d
	destroy c

*/
