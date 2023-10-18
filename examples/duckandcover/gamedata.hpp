#ifndef GAMEDATA_HPP_
#define GAMEDATA_HPP_

#include <bitset>

enum class Input { Right, Left, JumpUp, JumpDown };
enum class State { Playing, GameOver, Win, Ready, Go };

struct GameData {
  State m_state{State::Ready};
  std::bitset<4> m_input;  // [jumpup, jumpdown, left, right]
};

#endif