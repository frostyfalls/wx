// SPDX-License-Identifier: GPL-3.0-only

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
  void RenderMenu();

  void Clear(SDL_Color color);
  void DrawRect(SDL_FRect rect, SDL_Color color);
  void DrawText(const std::string &text, TextType type, float x, float y, TextAlignment alignment);

  void AdvanceCrawl();
  Crawl& CurrentCrawl() { return m_Crawls.at(m_CurrentCrawl); }
  void AddCrawl(const Crawl &crawl);

private:
  const Configuration &m_Config;
  bool m_Running = false;
  std::size_t m_Width, m_Height;

  SDL_Window *m_Window = nullptr;
  SDL_Renderer *m_Renderer = nullptr;
  TTF_Font *m_Font = nullptr;
  TTF_Font *m_SmallFont = nullptr;

  std::vector<Crawl> m_Crawls;
  std::size_t m_CurrentCrawl = 0;
  float m_CrawlTimer = 0.0f;
  float m_CrawlScroll = 0.0f;
  std::size_t m_FontSize = 32;

  std::int32_t m_CurrentCrawlWidth = 0;
  std::int32_t m_DateTimeHeight = 0;

  std::chrono::local_time<std::chrono::seconds> m_RenderTime;

  bool m_ShowDateTime = true;

  SDL_Color m_BackgroundColor{53, 4, 121, 0};
  SDL_Color m_TextColor{238, 238, 238, 0};
  SDL_Rect m_ClipRect{40, 20, 0, 0};
  SDL_FRect m_LineRect{0, 0, 0, 2};
};

} // namespace Wx
