// SPDX-License-Identifier: GPL-3.0-only

#include <chrono>
#include <format>
#include <print>
#include <string>

#include "wx/wx.hh"

#include "wx/assets/star3000.hh"
#include "wx/assets/starjr.hh"

namespace Wx
{

SDL_Color s_BackgroundColorNormal{53, 4, 121, 0};
SDL_Color s_BackgroundColorRegional{50, 50, 50, 0};

Application::Application(const Configuration &config)
  : m_Config(config)
{
  SDL_IOStream *fontData = nullptr;

  SDL_Init(SDL_INIT_VIDEO);
  TTF_Init();

  m_Window = SDL_CreateWindow(m_Config.title.c_str(), m_Config.width, m_Config.height, 0);
  m_Renderer = SDL_CreateRenderer(m_Window, nullptr);
  fontData = SDL_IOFromConstMem(star3000_font, sizeof(star3000_font));
  m_Font = TTF_OpenFontIO(fontData, true, m_Config.fontSize);

  SDL_SetRenderVSync(m_Renderer, true);

  m_Running = true;

  m_WeatherData.location = "Tampa";
  m_WeatherData.currentConditions = "Sunny";
}

Application::~Application()
{
  SDL_DestroyTexture(m_SlideTexture);

  TTF_CloseFont(m_Font);
  m_Font = nullptr;

  SDL_DestroyRenderer(m_Renderer);
  m_Renderer = nullptr;

  SDL_DestroyWindow(m_Window);
  m_Window = nullptr;

  TTF_Quit();
  SDL_Quit();
}

void Application::Run()
{
  auto lastFrame = std::chrono::steady_clock::now();

  while (m_Running)
  {
    auto now = std::chrono::steady_clock::now();
    float deltaSeconds = std::chrono::duration<float>(now - lastFrame).count();
    lastFrame = now;

    ProcessEvents();
    Update(deltaSeconds);
    Render();
  }
}

void Application::ProcessEvents()
{
  SDL_Event e;
  while (SDL_PollEvent(&e))
  {
    if (e.type == SDL_EVENT_QUIT)
      m_Running = false;
  }
}

void Application::Update(float deltaSeconds)
{
  m_ElapsedSeconds += deltaSeconds;
  if (m_ElapsedSeconds >= m_SlideDuration)
  {
    m_ElapsedSeconds = 0;
    AdvanceSlide();
  }

  if (m_Scrolling)
    m_ScrollOffset += 40.0f * deltaSeconds;
}

void Application::AdvanceSlide()
{
  if (m_CurrentSlide == m_Slides.size() - 1)
    m_CurrentSlide = 0;
  else
    m_CurrentSlide += 1;

  m_CurrentProduct = m_Slides[m_CurrentSlide].product;
  m_SlideDuration = m_Slides[m_CurrentSlide].durationSeconds;
}

void Application::Render()
{
  static std::size_t prevSlide = 0;

  if (!m_SlideTexture || m_CurrentSlide != prevSlide)
  {
    std::string message;
    switch (m_CurrentProduct)
    {
    case Product::CurrentConditions:
      message = "current conditions:";
      break;
    case Product::RegionalObservations:
      message = "regional observations:";
      break;
    case Product::Warning:
      message = "warning:";
      break;
    }
    m_SlideTexture = RasterizeText(message);
    SDL_GetTextureSize(m_SlideTexture, &m_SlideRect.w, &m_SlideRect.h);
  }

  SDL_SetRenderDrawColor(m_Renderer, m_BackgroundColor.r, m_BackgroundColor.g, m_BackgroundColor.b, m_BackgroundColor.a);
  SDL_RenderClear(m_Renderer);

  SDL_RenderTexture(m_Renderer, m_SlideTexture, nullptr, &m_SlideRect);

  SDL_RenderPresent(m_Renderer);

  prevSlide = m_CurrentSlide;
}

SDL_Texture *Application::RasterizeText(const std::string &text)
{
  SDL_Surface *surface = nullptr;
  SDL_Texture *texture = nullptr;

  surface = TTF_RenderText_Solid_Wrapped(m_Font, text.c_str(), 0, m_TextColor, 0);
  texture = SDL_CreateTextureFromSurface(m_Renderer, surface);

  SDL_DestroySurface(surface);
  return texture;
}

} // namespace Wx
