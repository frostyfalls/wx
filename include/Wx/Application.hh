// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <map>
#include <chrono>
#include <string>
#include <vector>

#include "Wx/Wx.hh"
#include "Wx/Renderer.hh"

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

namespace Wx
{

enum class CrawlMode
{
  Static,
  Scrolling,
};

struct Crawl
{
  std::string text;
  CrawlMode mode;
};

class Application
{
public:
  Application(const Configuration &config);
  ~Application() = default;

  void Run();

  void AddCrawl(const Crawl &crawl);
  void AdvanceCrawl();

private:
  void ProcessEvents();
  void Update(float deltaSeconds);
  void Render();

  Crawl &CurrentCrawl() { return m_Crawls.at(m_CurrentCrawl); }

private:
  const Configuration &m_Config;
  Renderer m_Renderer;
  bool m_Running = false;

  std::vector<Crawl> m_Crawls;
  std::size_t m_CurrentCrawl = 0;
  float m_CrawlTimer = 0.0f;
  float m_CrawlScroll = 0.0f;

  std::int32_t m_CurrentCrawlWidth = 0;

  bool m_ShowDateTime = true;
  std::chrono::local_time<std::chrono::seconds> m_RenderTime;

  Color m_BackgroundColor{53, 4, 121, 0};
  Color m_TextColor{238, 238, 238, 0};
  Rect m_ClipRect{40, 20, 0, 0};
  Rect m_LineRect{0, 0, 0, 2};
};

} // namespace Wx
