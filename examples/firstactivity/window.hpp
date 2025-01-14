#ifndef WINDOW_HPP_
#define WINDOW_HPP_

#include <random>

#include "abcgOpenGL.hpp"

#include "bird.hpp"
#include "bamboo.hpp"
#include "gamedata.hpp"

class Window : public abcg::OpenGLWindow {
protected:
  void onEvent(SDL_Event const &event) override;
  void onCreate() override;
  void onUpdate() override;
  void onPaint() override;
  void onPaintUI() override;
  void onResize(glm::ivec2 const &size) override;
  void onDestroy() override;

private:
  glm::ivec2 m_viewportSize{};

  GLuint m_starsProgram{};
  GLuint m_objectsProgram{};

  GameData m_gameData;

  Bird m_bird;
  Bamboo m_bamboo;
  GLuint m_program;

  abcg::Timer m_restartWaitTimer;

  ImFont *m_font{};
  glm::mat4 m_projectionMatrix; 

  std::default_random_engine m_randomEngine;

  void restart();
  void checkCollisions();
  void checkWinCondition();
};

#endif