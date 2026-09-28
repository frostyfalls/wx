#pragma once

#include <chrono>
#include <string>
#include <vector>

#include "Wx/Wx.hh"

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

namespace Wx
{

class Application
{
public:
  Application(const Configuration &config);
  ~Application();

  void Run();

private:
  bool Init();
  void ProcessEvents();
  void Update(float deltaSeconds);
  void Render();
  void AdvanceSlide();
  void AdvanceCrawl();
  void RenderText(const std::string &text, TextType type, float x, float y, TextAlignment alignment);

private:
  const Configuration &m_Config;
  bool m_Running = false;

  WeatherData m_WeatherData;

  SDL_Window *m_Window = nullptr;
  SDL_Renderer *m_Renderer = nullptr;
  TTF_Font *m_Font = nullptr;
  TTF_Font *m_SmallFont = nullptr;

  std::vector<Slide> m_Slides;
  std::size_t m_CurrentSlide = 0;
  float m_SlideTimer = 0.0f;
  // float m_SlideScroll = 0.0f;

  std::vector<Crawl> m_Crawls;
  std::size_t m_CurrentCrawl = 0;
  float m_CrawlTimer = 0.0f;
  float m_CrawlScroll = 0.0f;

  std::chrono::local_time<std::chrono::seconds> m_RenderTime;

  SDL_Color m_BackgroundColor{53, 4, 121, 0};
  SDL_Color m_TextColor{238, 238, 238, 0};
};

} // namespace Wx
