#ifndef WINDOW_HPP_
#define WINDOW_HPP_

#include <imgui.h>

#include <random>

#include "abcg.hpp"
#include "bombs.hpp"
#include "duck.hpp"

class Window : public abcg::OpenGLWindow {
 protected:
  void handleEvent(SDL_Event& event) override;
  void initializeGL() override;
  void paintGL() override;
  void paintUI() override;
  void resizeGL(int width, int height) override;
  void terminateGL() override;

 private:
  GLuint m_objectsProgram{};

  int m_viewportWidth{};
  int m_viewportHeight{};

  GameData m_gameData;

  Bombs m_bombs;
  Duck m_duck;
  
  abcg::ElapsedTimer m_restartWaitTimer;

  ImFont* m_font{};

  std::default_random_engine m_randomEngine;

  void checkCollisions();
  void checkWinCondition();

  void restart();
  void update();
};

#endif