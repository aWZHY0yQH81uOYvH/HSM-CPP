// Hierarchical state machine

#pragma once

#include <mutex>
#include <unordered_map>
#include <utility>
#include <functional>

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
	virtual bool on_event(HSMInfo *info);
	
	// Add a child state
	// Takes ownership of passed pointer
	void add_child_state(HSMState *state, bool default_state = false);
	
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
	HSMachine(std::function<void(void)> transition_callback = []{});
	
	// Send an event to the active state and all parents until one consumes it
	// Returns false if the event was not consumed
	bool process_event(HSMInfo *info = nullptr);
	bool process_event(HSMInfo &info);
	template<typename InfoType, typename... Args> bool process_event(Args&&... args) {
		InfoType info(std::forward<Args>(args)...);
		return process_event(&info);
	}
	
	// Transition to a new state
	// Returns false if the transition was not possible
	bool transition_to(HSMState *new_state, HSMInfo *info = nullptr);
	bool transition_to(HSMState *new_state, HSMInfo &info);
	template<typename InfoType, typename... Args> bool transition_to(HSMState *new_state, Args&&... args) {
		InfoType info(std::forward<Args>(args)...);
		return transition_to(new_state, &info);
	}
	
	// Return if a transition is possible
	bool can_transition_to(HSMState *new_state, HSMInfo *info = nullptr);
	bool can_transition_to(HSMState *new_state, HSMInfo &info);
	template<typename InfoType, typename... Args> bool can_transition_to(HSMState *new_state, Args&&... args) {
		InfoType info(std::forward<Args>(args)...);
		return can_transition_to(new_state, &info);
	}
	
	// Returns true if current state is within the given state
	bool within(HSMState *query);
	
	// Same as above but don't try to lock the access mutex
	bool within_immediate(HSMState *query) const;
	
	HSMState *get_current_state();
	
	// Allow addition of child states
	using HSMState::add_child_state;
	
protected:
	// Lock update of HSM to one thread
	std::recursive_mutex exec_mutex;
	
	// Function called whenever a state transition is successful
	std::function<void(void)> transition_callback;
	
	HSMState *current_state;
};

}
