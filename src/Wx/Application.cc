// SPDX-License-Identifier: GPL-3.0-only

#include <chrono>
#include <format>
#include <string>

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

  const std::string date = std::format("{:%a %b %d}", m_RenderTime);
  const std::string time = std::format("{:%I:%M:%S %p}", m_RenderTime);

  m_Renderer.SetAsset(AssetId::Date, m_Renderer.RasterizeText(date, TextType::Small, m_TextColor));
  m_Renderer.SetAsset(AssetId::Time, m_Renderer.RasterizeText(time, TextType::Small, m_TextColor));
  m_Renderer.SetAsset(AssetId::Crawl, m_Renderer.RasterizeText("This is an ad crawl you are viewing.", TextType::Normal, m_TextColor));

  while (m_Running)
  {
    const auto now = std::chrono::system_clock::now();
    float deltaSeconds = std::chrono::duration<float>(now - lastFrame).count();

    m_RenderTime = std::chrono::round<std::chrono::seconds>(std::chrono::current_zone()->to_local(now));
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
    {
      m_Running = false;
    }
    else if (e.type == SDL_EVENT_KEY_DOWN && e.key.scancode == SDL_SCANCODE_Q)
    {
      m_Running = false;
    }
    else if (e.type == SDL_EVENT_KEY_DOWN && e.key.scancode == SDL_SCANCODE_F)
    {
      fullscreen = !fullscreen;
      m_Renderer.SetFullscreen(fullscreen);
    }
    else if (e.type == SDL_EVENT_KEY_DOWN && e.key.scancode == SDL_SCANCODE_C)
    {
      m_ShowDateTime = !m_ShowDateTime;
    }
    else if (e.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED)
    {
      m_Renderer.Resize(e.window.data1, e.window.data2);

      m_ClipRect.width = static_cast<int>(m_Renderer.Width() - m_ClipRect.x * 2);
      m_ClipRect.height = static_cast<int>(m_Renderer.Height() - m_ClipRect.y * 2);

      m_LineRect.y = static_cast<float>(m_Renderer.Height() - 100);
      m_LineRect.width = static_cast<float>(m_Renderer.Width());

      m_Renderer.Present();
    }
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
  m_Renderer.Clear(m_BackgroundColor);
  m_Renderer.DrawRect(m_LineRect, m_TextColor);
  m_Renderer.Present();
}

} // namespace Wx
