#include <vector>
#include <cassert>

#include "HSM.hpp"

using namespace std;
using namespace HSM;

class State: public HSMState {
public:
	virtual void on_enter(HSMInfo *info) {
		assert(!entered);
		entered = true;
	}
	
	virtual void on_exit(HSMInfo *info) {
		assert(entered);
		entered = false;
	}
	
private:
	bool entered = false;
};

int main() {
	HSMachine hsm;
	
	vector<State*> states;
	
	// Build random state tree
	for(int x = 0; x < 10000; x++) {
		State *state = new State();
		
		unsigned int rnd = rand();
		rnd %= states.size()+1;
		
		if(rnd == states.size())
			hsm.add_child_state(state);
		
		else {
			states[rnd]->add_child_state(state);
			if(rnd % 10 == 0 && !states[rnd]->get_default_state())
				states[rnd]->set_default_state(state);
		}
		
		states.emplace_back(state);
	}
	
	// Perform random transitions
	for(int x = 0; x < 1000000; x++) {
		unsigned int rnd = rand();
		rnd %= states.size();
		assert(hsm.transition_to(states[rnd]));
	}
}
