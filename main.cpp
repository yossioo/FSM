#pragma execution_character_set("utf-8")

#include <iostream>
#include <memory>
//#include <utility>
#include <vector>
#include <map>

#include "FSM.h"

using namespace functional_state_machine;

struct World
{
  size_t ticks{};
  
  bool did_stop = false;
  
  void resetTicks()
  {
    ticks = 0;
  }
};

int main()
{
  auto world = std::make_shared<World>();
  StateMachine state_machine;
  {
    auto red_state =
      State::createStateSharedPtr("Red", []() { std::cout << "RED" << std::endl; },
                                  [=]()
                                  {
                                    std::cout << "Turning Red light ON" << std::endl;
                                    world->did_stop = true;
                                  },
                                  [=]()
                                  {
                                    std::cout << "Turning Red light OFF" << std::endl;
                                    world->resetTicks();
                                  }
      );
    auto yellow_state =
      State::createStateSharedPtr("Yellow", []() { std::cout << "\tYELLOW" << std::endl; },
                                  []() { std::cout << "Turning Yellow light ON" << std::endl; },
                                  [=]()
                                  {
                                    std::cout << "Turning Yellow light OFF" << std::endl;
                                    world->resetTicks();
                                  }
      );
    auto green_state =
      State::createStateSharedPtr("Green", []() { std::cout << "\t\tGREEN" << std::endl; },
                                  []() { std::cout << "Turning Green light ON" << std::endl; },
                                  [=]()
                                  {
                                    std::cout << "Turning Green light OFF" << std::endl;
                                    world->resetTicks();
                                    world->did_stop = false;
                                  }
      );
    
    state_machine.addState(red_state);
    state_machine.addState(yellow_state);
    state_machine.addState(green_state);
    
    state_machine.addOnAnyTick([=]() { ++world->ticks; });
    state_machine.addOnAnyChange([](){ std::cout << "Something changed..." << std::endl;});
    state_machine.addOnTransitionConditionMet([=](const std::string &prev, const std::string &next,
                                                  const std::string &reason)
                                              {
                                                printf("Transition %s->%s because %s\n",
                                                       prev.c_str(),
                                                       next.c_str(),
                                                       reason.c_str());
                                              });
    red_state->transitions_.emplace_back(yellow_state,
                                         [=]()
                                         {
                                           return world->ticks == 4;
                                         },
                                         "4 ticks in red"
    );
    yellow_state->transitions_.emplace_back(green_state,
                                            [=]() { return world->did_stop; },
                                            "Allowed to drive"
    );
    yellow_state->transitions_.emplace_back(red_state,
                                            [=]() { return not world->did_stop and world->ticks >= 2; },
                                            "Stopping the traffic"
    );
    green_state->transitions_.emplace_back(yellow_state,
                                           [=]() { return world->ticks >= 5; },
                                           "prepare to stop"
    );
  }
  for (int i = 0; i < 25; ++i)
  {
    state_machine.spinOnce();
    if (i == 18)
    {
      state_machine.returnToPreviousState();
      std::cout << "\t\t==================== returnToPreviousState() " << std::endl;
    }
  }
  return 0;
}
