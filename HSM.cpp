#include "HSM.hpp"

#include <cassert>

namespace HSM {

// Default HSMState behavior
HSMState::HSMState(): parent_state(nullptr), default_state(nullptr) {}

HSMState::~HSMState() {
	// Destroy all children 😈
	for(const auto &child:child_states)
		if(child.first == child.second)
			delete child.first;
}

bool HSMState::can_enter(const HSMInfo *info) const {
	(void)info;
	return true;
}

bool HSMState::can_exit(const HSMInfo *info) const {
	(void)info;
	return true;
}

void HSMState::on_enter(HSMInfo *info) {
	(void)info;
}

void HSMState::on_exit(HSMInfo *info) {
	(void)info;
}

bool HSMState::process_event(HSMInfo *info) {
	(void)info;
	return false;
}

void HSMState::add_child_state(HSMState *state) {
	// Apply new state tree information upwards
	HSMState *target_state_ptr = this;
	HSMState *source_state_ptr = state;
	while(target_state_ptr) {
		for(const auto &new_state:state->child_states) {
			const auto emplace_ret = target_state_ptr->child_states.emplace(new_state.first, source_state_ptr);
			assert(emplace_ret.second); // Ensure unique
		}
		
		const auto emplace_ret = target_state_ptr->child_states.emplace(state, source_state_ptr);
		assert(emplace_ret.second); // Ensure unique
		
		source_state_ptr = target_state_ptr;
		target_state_ptr = target_state_ptr->parent_state;
	}
	
	// Tell new state about us
	state->parent_state = this;
}

void HSMState::set_default_state(HSMState *state) {
	assert(!default_state); // Don't already have a default state
	
	const auto state_it = child_states.find(state);
	assert(state_it != child_states.end()); // It is a child state
	assert(state_it->first == state_it->second); // It is a direct descendant
	
	default_state = state;
}

HSMState *HSMState::get_parent_state() const {
	return parent_state;
}

HSMState *HSMState::get_default_state() const {
	return default_state;
}

// HSM logic
HSMachine::HSMachine(): current_state(this) {}

bool HSMachine::process_event(HSMInfo *info) {
	std::lock_guard<std::recursive_mutex> lock(exec_mutex);
	
	// Deliver event info to current state and work up while they're returning false
	HSMState *exec_state = current_state;
	while(exec_state != this) {
		if(exec_state->process_event(info)) return true;
		exec_state = exec_state->parent_state;
	}
	
	return false;
}

bool HSMachine::transition_to(HSMState *new_state, HSMInfo *info) {
	std::lock_guard<std::recursive_mutex> lock(exec_mutex);
	
	// Exit if we're already in this state
	if(current_state == new_state)
		return true;
	
	// Determine if we can transition to this state
	{
		// Work upwards to the state that contains or is the new_state
		HSMState *test_state = current_state;
		while(test_state != new_state
			  && test_state->child_states.find(new_state) == test_state->child_states.end()) {
			if(!test_state->can_exit(info)) return false;
			test_state = test_state->parent_state;
		}
		
		// Work downwards into the state that is new_state
		// Do not try to enter if we came from a child state
		if(test_state != new_state)
			do {
				test_state = test_state->child_states.find(new_state)->second;
				if(!test_state->can_enter(info)) return false;
			} while(test_state != new_state);
		
		// Work down into default states
		while(test_state->default_state) {
			if(!test_state->default_state->can_enter(info)) return false;
			test_state = test_state->default_state;
		}
	}
	
	// Actually exit/enter states
	while(current_state != new_state
		  && current_state->child_states.find(new_state) == current_state->child_states.end()) {
		current_state->on_exit(info);
		current_state = current_state->parent_state;
	}
	
	if(current_state != new_state)
		do {
			current_state = current_state->child_states.find(new_state)->second;
			current_state->on_enter(info);
		} while(current_state != new_state);
	
	while(current_state->default_state) {
		current_state->default_state->on_enter(info);
		current_state = current_state->default_state;
	}
	
	return true;
}

HSMState *HSMachine::get_current_state() {
	std::lock_guard<std::recursive_mutex> lock(exec_mutex);
	return current_state;
}

}
