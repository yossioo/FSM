//
// Created by yossi on 09/09/2021.
//

#pragma  once
#include <functional>
#include <utility>

using CallbackT = std::function<void()>;
using ConditionT = std::function<bool()>;

namespace functional_state_machine
{
class State
{
  explicit State(std::string name = "default",
                 CallbackT _in_state = []() {},
                 CallbackT _on_enter = []() {},
                 CallbackT _on_exit = []() {}) :
      name(
          std::move(name)), in_state(std::move(_in_state)), on_enter(std::move(_on_enter)), on_exit(std::move(_on_exit))
  {}

public:
  const std::string name = "no-name";
  const CallbackT in_state = []() {};
  const CallbackT on_enter = []() {};
  const CallbackT on_exit = []() {};
  
  typedef std::shared_ptr<State> SharedPtr;
  
  static SharedPtr createState(std::string name = "shared_state", CallbackT _in_state = nullptr,
                               CallbackT _on_enter = nullptr,
                               CallbackT _on_exit = nullptr)
  {
    makeEmpty(_in_state);
    makeEmpty(_on_enter);
    makeEmpty(_on_exit);
    auto state = new State{std::move(name), _in_state, _on_enter, _on_exit};
    SharedPtr state_ptr;
    state_ptr.reset(state);
    return state_ptr;
  }
  
  /// Not so sure about that, maybe think of better way to create empty state?
  static void makeEmpty(CallbackT &callback_t)
  {
    if (nullptr == callback_t)
    {
      callback_t = []() {};
    }
  }
  
  virtual ~State()
  {
    std::cout << "Destructing state: " << name << std::endl;
  }
};

struct Transition
{
  State::SharedPtr target_state_ptr;
  ConditionT condition;
  std::string reason;
  
  Transition(State::SharedPtr target_state_ptr, ConditionT condition, std::string reason)
      : target_state_ptr(std::move(target_state_ptr)), condition(std::move(condition)), reason(std::move(reason))
  {}
  
  virtual ~Transition()
  {
    printf("Destructing =>%s (%s)\n", target_state_ptr->name.c_str(), reason.c_str());
  }
};

class StateMachine
{
  std::vector<State::SharedPtr> states_;
  std::map<State::SharedPtr, std::vector<Transition>> transitions_;
  
  State::SharedPtr current_state;
  State::SharedPtr previous_state;
  State::SharedPtr next_state;
  
  CallbackT any_tick_callback = []() {};
  CallbackT any_change_callback = []() {};

public:
  void spinOnce()
  {
    if (next_state != current_state)
    {
      if (current_state)
      {
        current_state->on_exit();
      }
      next_state->on_enter();
      
      previous_state = current_state;
      current_state = next_state;
      
      any_change_callback();
    }
    
    if (current_state)
    {
      current_state->in_state();
    }
    for (const auto &transition: transitions_[current_state])
    {
      if (transition.condition())
      {
        next_state = transition.target_state_ptr;
      }
    }
    // Finally,
    any_tick_callback();
  }
  
  void addState(const State::SharedPtr &state)
  {
    states_.emplace_back(state);
    if (not next_state)
    {
      next_state = state;
    }
  }
  
  void addOnAnyTick(CallbackT callback)
  {
    any_tick_callback = std::move(callback);
  }
  
  void addOnAnyChange(CallbackT callback)
  {
    any_change_callback = std::move(callback);
  }
  
  void returnToPreviousState()
  {
    next_state = previous_state;
  }
  
  void addTransition(const State::SharedPtr &from_state, const Transition &transition)
  {
    transitions_[from_state].emplace_back(transition);
  }
};
}
