// Hierarchical state machine

#pragma once

#include <mutex>
#include <unordered_map>

namespace HSM {

// Virtual information class to be extended to pass information around
struct HSMInfo {
	virtual ~HSMInfo() {}
};

// Virtual state class to be extended to create states
class HSMState {
public:
	HSMState();
	virtual ~HSMState();
	
	// Control if entering/exiting this state is currently allowed
	virtual bool can_enter(const HSMInfo *info) const;
	virtual bool can_exit(const HSMInfo *info) const;
	
	// Functions called on entering/exiting
	virtual void on_enter(HSMInfo *info);
	virtual void on_exit(HSMInfo *info);
	
	// Process an event
	// Return true to consume
	virtual bool process_event(HSMInfo *info);
	
	// Add a child state
	// Takes ownership of passed pointer
	void add_child_state(HSMState *state);
	
	// Set default state so when this state is entered (and no other child state is specified),
	// it will automatically transition into this default state
	void set_default_state(HSMState *state);
	
	HSMState *get_parent_state() const;
	HSMState *get_default_state() const;
	
protected:
	HSMState *parent_state;
	HSMState *default_state;
	
	// Map of all child/grandchild states to which direct child state must be taken to get there
	std::unordered_map<HSMState*, HSMState* const> child_states;
	
	friend class HSMachine;
};

// Base state machine class
class HSMachine: protected HSMState {
public:
	HSMachine();
	
	// Send an event to the active state and all parents until one consumes it
	// Returns false if the event was not consumed
	virtual bool process_event(HSMInfo *info = nullptr);
	
	// Transition to a new state
	// Returns false if the transition was not possible
	bool transition_to(HSMState *new_state, HSMInfo *info = nullptr);
	
	HSMState *get_current_state();
	
	// Allow addition of child states
	using HSMState::add_child_state;
	
protected:
	// Lock update of HSM to one thread
	std::recursive_mutex exec_mutex;
	
	HSMState *current_state;
};

}
