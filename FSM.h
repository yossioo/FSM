//
// Created by yossi on 09/09/2021.
//

#pragma  once
#include <functional>
#include <utility>

namespace functional_state_machine
{
class State;

class Transition;

using CallbackT = std::function<void()>;
using ConditionT = std::function<bool()>;
using TransitionConditionMetCallbackT = std::function<void(std::string, std::string, std::string)>;

inline bool oneTimeCondition(bool * flag)
{
  if (*flag)
  {
    *flag = false;
    return true;
  }
  return false;
}

class State
{
  explicit State(std::string name = "default",
                 CallbackT _in_state = []() {},
                 CallbackT _on_enter = []() {},
                 CallbackT _on_exit = []() {}) :
    name(
      std::move(name)), in_state(std::move(_in_state)), on_enter(std::move(_on_enter)), on_exit(std::move(_on_exit)) {}

public:
  const std::string name;
  const CallbackT in_state = []() {};
  const CallbackT on_enter = []() {};
  const CallbackT on_exit = []() {};
  
  std::vector<Transition> transitions_;
  
  typedef std::shared_ptr<State> SharedPtr;
  
  static SharedPtr createStateSharedPtr(std::string name = "shared_state", CallbackT _in_state = []() {},
                                        CallbackT _on_enter = []() {},
                                        CallbackT _on_exit = []() {})
  {
    auto state = new State{std::move(name), std::move(_in_state), std::move(_on_enter), std::move(_on_exit)};
    SharedPtr state_ptr;
    state_ptr.reset(state);
    return state_ptr;
  }
  
  ~State() = default;
};

class Transition
{
public:
  State::SharedPtr target_state_ptr;
  ConditionT condition;
  std::string reason;
  
  Transition(State::SharedPtr target_state_ptr, ConditionT condition, std::string reason)
    : target_state_ptr(std::move(target_state_ptr)), condition(std::move(condition)), reason(std::move(reason)) {}
  
  ~Transition() = default;
};

class StateMachine
{
  std::vector<State::SharedPtr> states_;
  std::vector<Transition> transition_from_any_state_;
  
  State::SharedPtr current_state_;
  State::SharedPtr previous_state_;
  State::SharedPtr next_state_;
  
  CallbackT any_tick_callback_;
  CallbackT any_change_callback_;
  TransitionConditionMetCallbackT transition_condition_met_callback_;
public:
  void spinOnce()
  {
    checkChangeToNext();
    
    current_state_->in_state();
    for (const auto &transition: current_state_->transitions_)
    {
      checkTransition(transition);
    }
    for (const auto &transition: transition_from_any_state_)
    {
      checkTransition(transition);
    }
    
    if (any_tick_callback_)
    {
      any_tick_callback_();
    }
  }
  
  void checkTransition(const Transition &transition)
  {
    if (transition.condition())
    {
      next_state_ = transition.target_state_ptr;
      if (transition_condition_met_callback_)
      {
        transition_condition_met_callback_(current_state_->name, next_state_->name, transition.reason);
      }
    }
  }
  
  void checkChangeToNext()
  {
    if (not isCurrentState(next_state_))
    {
      if (current_state_)
      {
        current_state_->on_exit();
      }
      if (any_change_callback_)
      {
        any_change_callback_();
      }
      previous_state_ = current_state_;
      current_state_ = next_state_;
      current_state_->on_enter();
    }
  }
  
  void addTransitionFromAnyState(const Transition& transition)
  {
    transition_from_any_state_.emplace_back(transition);
  }
  
  void addState(const State::SharedPtr &state)
  {
    states_.emplace_back(state);
    if (not next_state_)
    {
      next_state_ = state;
    }
  }
  
  void addOnAnyTick(CallbackT callback)
  {
    any_tick_callback_ = std::move(callback);
  }
  
  void addOnTransitionConditionMet(TransitionConditionMetCallbackT callback)
  {
    transition_condition_met_callback_ = std::move(callback);
  }
  
  void addOnAnyChange(CallbackT callback)
  {
    any_change_callback_ = std::move(callback);
  }
  
  void returnToPreviousState()
  {
    next_state_ = previous_state_;
  }
  
  [[nodiscard]] bool isCurrentState(const State::SharedPtr &other) const
  {
    return other == current_state_;
  }
  
  [[nodiscard]] bool isPreviousState(const State::SharedPtr &other) const
  {
    return other == previous_state_;
  }
};
}
