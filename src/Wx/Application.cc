// SPDX-License-Identifier: GPL-3.0-only

#include <chrono>
#include <format>
#include <string>

#include <SDL3/SDL.h>

#include "Wx/Application.hh"

namespace Wx
{

Application::Application(const Configuration &config)
  : m_Config(config)
  , m_Renderer(config)
{
  m_Running = true;
}

void Application::Run()
{
  auto lastFrame = std::chrono::system_clock::now();

  while (m_Running)
  {
    const auto now = std::chrono::system_clock::now();
    const float deltaSeconds = std::chrono::duration<float>(now - lastFrame).count();

    m_RenderTime = std::chrono::round<std::chrono::seconds>(
      std::chrono::current_zone()->to_local(now)
    );
    lastFrame = now;
    Update(deltaSeconds);

    ProcessEvents();
    Render();
  }
}

void Application::AddCrawl(const Crawl &crawl)
{
  bool empty = m_Crawls.empty();
  m_Crawls.push_back(crawl);
  if (empty)
    AdvanceCrawl();
}

void Application::AdvanceCrawl()
{
  m_CurrentCrawl = (m_CurrentCrawl + 1) % m_Crawls.size();
  m_CrawlTimer = 0;
  m_CrawlScroll = 0.0f;

  const Crawl &crawl = CurrentCrawl();
  Rect r = m_Renderer.MeasureText(crawl.text, TextType::Normal);
  m_CurrentCrawlWidth = r.width;
}

void Application::ProcessEvents()
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

void Application::Update(float deltaSeconds)
{
  if (!m_Crawls.empty())
  {
    const Crawl &crawl = CurrentCrawl();
    m_CrawlTimer += deltaSeconds;

    if (crawl.mode == CrawlMode::Static && m_CrawlTimer >= 5)
      AdvanceCrawl();

    else if (crawl.mode == CrawlMode::Scrolling)
    {
      if (m_Renderer.Width() - m_CrawlScroll + m_CurrentCrawlWidth < 0)
        AdvanceCrawl();

      m_CrawlScroll += 2.0f;
    }
  }
}

void Application::Render()
{
  std::int32_t crawlLine = 2;
  std::int32_t insetH = 40;
  std::int32_t insetV = 30;
  std::int32_t crawlHeight = m_Renderer.Height() - 100;
  Color slideColor{20, 20, 20, 0};
  Color crawlColor = m_BackgroundColor;

  m_Renderer.Clear(m_BackgroundColor);
  m_Renderer.DrawRect({0, crawlHeight, m_Renderer.Width(), crawlLine}, m_TextColor);

  m_Renderer.SetViewport({0, 0, m_Renderer.Width(), crawlHeight});
  m_Renderer.DrawRect({}, slideColor);
  m_Renderer.ClearViewport();

  m_Renderer.SetViewport({0, crawlHeight + crawlLine, m_Renderer.Width(), m_Renderer.Height() - crawlHeight - crawlLine});
  m_Renderer.DrawRect({}, crawlColor);
  m_Renderer.SetViewport({insetH, crawlHeight + crawlLine, m_Renderer.Width() - insetH * 2, m_Renderer.Height() - crawlHeight - crawlLine});
  if (m_ShowDateTime)
  {
    m_Renderer.DrawAsset(AssetId::Date, 0, 4, TextAlignment::Left);
    m_Renderer.DrawAsset(AssetId::Time, m_Renderer.Width() - insetH * 2, 4, TextAlignment::Right);
  }
  m_Renderer.ClearViewport();

  m_Renderer.Present();
}

} // namespace Wx
