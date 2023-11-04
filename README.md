# Simple C++ Hierarchical State Machine

### Features

 * Ability to disallow entering or exiting states
 * Arbitrary information can be passed to state classes
 * Default states
 * Event processing
 * Thread safe

### Usage

 1. Create a main `HSMachine` class.
 2. Subclass `HSMState` to create your custom states, overriding the `on_*`, `can_*` and `process_event` functions if desired.
 3. Optionally extend `HSMInfo` to create structs of information that can be passed to your `HSMState`-derived classes whenever a state transition or event occurs.
 4. Allocate instances of your states, keeping pointers to them. Each pointer is an individual state, meaning you can have multiple states in the hierarchy that are represented by the same `HSMState`-derived class.
 5. Create your hierarchy using the `add_child_state` function. This will take ownership of the pointers, and all state objects should be freed when the main `HSMachine` is destroyed, assuming there are no orphaned states. `assert` statements should ensure your machine is well-formed.
 6. Optionally set default states using `set_default_state`.

See `demo.cpp`

### Behavior

When a state transition is requested, `can_enter` and `can_exit` will be called for all intermediate state transitions that would need to happen. If any of these calls return false, the operation is aborted. If not, the corresponding `on_enter` and `on_exit` functions are called in the same order. Asking to transition to the current state does nothing.

If the final state has any default state set, the path of default states are recursively followed. For any given state that has a default state, the machine should never come to rest at that given state.

Events are processed starting from the current state and work upwards. If any `process_event` function returns true, the event is consumed and no further parent states will receive it.
