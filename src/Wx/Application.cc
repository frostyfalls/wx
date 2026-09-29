// SPDX-License-Identifier: GPL-3.0-only

#include <chrono>
#include <format>
#include <print>
#include <string>

#include <SDL3/SDL.h>

#include "Wx/Application.hh"

namespace Wx
{

Application::Application(const Configuration &config)
  : m_Config(config)
  , m_Renderer(config)
{
}

void Application::Run()
{
  auto previousTime = std::chrono::system_clock::now();

  while (m_Running)
  {
    m_CurrentTime = std::chrono::system_clock::now();
    const float deltaSeconds = std::chrono::duration<float>
      (m_CurrentTime - previousTime).count();
    previousTime = m_CurrentTime;

    OnEvent();
    OnUpdate(deltaSeconds);
    OnRender();
  }
}

void Application::OnEvent()
{
  static bool fullscreen = false;
  SDL_Event e;

  while (SDL_PollEvent(&e))
  {
    if (e.type == SDL_EVENT_QUIT)
      m_Running = false;

    else if (e.type == SDL_EVENT_KEY_DOWN && e.key.scancode == SDL_SCANCODE_Q)
      m_Running = false;

    else if (e.type == SDL_EVENT_KEY_DOWN && e.key.scancode == SDL_SCANCODE_F)
      m_Renderer.SetFullscreen(fullscreen = !fullscreen);

    else if (e.type == SDL_EVENT_KEY_DOWN && e.key.scancode == SDL_SCANCODE_C)
      m_ShowDateTime = !m_ShowDateTime;

    else if (e.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED)
      m_Renderer.Resize(e.window.data1, e.window.data2);
  }
}

void Application::OnUpdate(float deltaSeconds)
{
  static float elapsedSecond = 1.0f;
  elapsedSecond += deltaSeconds;

  if (elapsedSecond >= 1.0f)
  {
    elapsedSecond -= 1.0f;
    const auto localTime = std::chrono::time_point_cast<std::chrono::seconds>
      (std::chrono::current_zone()->to_local(m_CurrentTime));
    m_Renderer.SetAsset(
      AssetId::Date,
      m_Renderer.RasterizeText(std::format("{:%a %b %d}", localTime), TextType::Small, m_TextColor)
    );
    m_Renderer.SetAsset(
      AssetId::Time,
      m_Renderer.RasterizeText(std::format("{:%I:%M:%S %p}", localTime), TextType::Small, m_TextColor)
    );
  }
}

void Application::OnRender()
{
  m_Renderer.Clear(m_BackgroundColor);
  if (m_ShowDateTime)
  {
    m_Renderer.DrawAsset(AssetId::Date, 20, 20, TextAlignment::Left);
    m_Renderer.DrawAsset(AssetId::Time, 20, 50, TextAlignment::Left);
  }
  m_Renderer.Present();
}

} // namespace Wx
