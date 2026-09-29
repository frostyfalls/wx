// SPDX-License-Identifier: GPL-3.0-only

#include <chrono>
#include <format>
#include <optional>
#include <print>
#include <string>

#include "Wx/Application.hh"

#include "Wx/Assets/Star3000.hh"
#include "Wx/Assets/Star3000_Small.hh"
#include "Wx/Assets/StarJr.hh"

namespace Wx
{

Application::Application(const Configuration &config)
  : m_Config(config)
{
  Init();

  AddCrawl({ "orcanet: fast, reliable cable internet for the Tampa Bay area", CrawlMode::Scrolling });
  AddCrawl({ "Conditions at Tampa Bay", CrawlMode::Static });
  AddCrawl({ "Mostly Cloudy", CrawlMode::Static });
  AddCrawl({ "Temperature: 59°F", CrawlMode::Static });
  AddCrawl({ "September Precipitation: 0.5 in.", CrawlMode::Static });
  AddCrawl({ "Humidity: 50%  Dewpoint: 40°F", CrawlMode::Static });
  AddCrawl({ "Barometric Pressure: 30.02 in.", CrawlMode::Static });
  AddCrawl({ "Wind: SSE 9 mph", CrawlMode::Static });
  AddCrawl({ "Visibility: 9 mi. ceiling unlimited", CrawlMode::Static });
  AddCrawl({ "September precipitation: 4.94 in.", CrawlMode::Static });

  m_Running = true;

  TTF_GetStringSize(m_SmallFont, "Mon", 0, nullptr, &m_DateTimeHeight);
}

bool Application::Init()
{
  SDL_IOStream *fontData = nullptr;

  if (!SDL_Init(SDL_INIT_VIDEO))
    return false;

  if (!TTF_Init())
    return false;

  m_Window = SDL_CreateWindow(m_Config.title.c_str(), m_Config.width, m_Config.height, 0);
  if (!m_Window)
    return false;

  fontData = SDL_IOFromConstMem(g_Star3000, sizeof(g_Star3000));
  if (!fontData)
    return false;

  m_Font = TTF_OpenFontIO(fontData, true, m_FontSize);
  if (!m_Font)
    return false;

  fontData = SDL_IOFromConstMem(g_Star3000_Small, sizeof(g_Star3000_Small));
  if (!fontData)
    return false;

  m_SmallFont = TTF_OpenFontIO(fontData, true, m_FontSize);
  if (!m_SmallFont)
    return false;

  m_Renderer = SDL_CreateRenderer(m_Window, nullptr);
  if (!m_Renderer)
    return false;

  SDL_SetRenderVSync(m_Renderer, true);

  return true;
}

Application::~Application()
{
  if (m_SmallFont)
  {
    TTF_CloseFont(m_SmallFont);
    m_SmallFont = nullptr;
  }

  if (m_Font)
  {
    TTF_CloseFont(m_Font);
    m_Font = nullptr;
  }

  if (m_Renderer)
  {
    SDL_DestroyRenderer(m_Renderer);
    m_Renderer = nullptr;
  }

  if (m_Window)
  {
    SDL_DestroyWindow(m_Window);
    m_Window = nullptr;
  }

  TTF_Quit();
  SDL_Quit();
}

void Application::Run()
{
  auto lastFrame = std::chrono::system_clock::now();

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

void Application::ProcessEvents()
{
  SDL_Event e;
  while (SDL_PollEvent(&e))
  {
    if (e.type == SDL_EVENT_QUIT)
      m_Running = false;
    else if (e.type == SDL_EVENT_KEY_DOWN && e.key.scancode == SDL_SCANCODE_ESCAPE)
      m_Running = false;
    else if (e.type == SDL_EVENT_KEY_DOWN && e.key.scancode == SDL_SCANCODE_C)
      m_ShowDateTime = !m_ShowDateTime;
    else if (e.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED)
    {
      m_Width = e.window.data1;
      m_Height = e.window.data2;
      m_ClipRect.w = static_cast<int>(m_Width - m_ClipRect.x * 2);
      m_ClipRect.h = static_cast<int>(m_Height - m_ClipRect.y * 2);
      m_LineRect.y = static_cast<float>(m_Height - 100);
      m_LineRect.w = static_cast<float>(m_Width);
      SDL_RenderPresent(m_Renderer);
    }
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
  TTF_GetStringSize(m_Font, crawl.text.c_str(), 0, &m_CurrentCrawlWidth, nullptr);
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
      if (m_Width - m_CrawlScroll + m_CurrentCrawlWidth < 0)
        AdvanceCrawl();
      m_CrawlScroll += 2.0f;
    }
  }
}

void Application::Render()
{
  Clear(m_BackgroundColor);
  DrawRect(m_LineRect, m_TextColor);
  SDL_SetRenderClipRect(m_Renderer, &m_ClipRect);

  if (m_ShowDateTime)
  {
    const std::string date = std::format("{:%a %b %d}", m_RenderTime);
    const std::string time = std::format("{:%H:%M:%S %p}", m_RenderTime);
    DrawText(date, TextType::Small, m_ClipRect.x, m_LineRect.y + 4, TextAlignment::Left);
    DrawText(time, TextType::Small, m_ClipRect.x + m_ClipRect.w, m_LineRect.y + 4, TextAlignment::Right);
  }
  if (!m_Crawls.empty())
  {
    const Crawl &crawl = CurrentCrawl();
    float x = m_ClipRect.x;
    if (crawl.mode == CrawlMode::Scrolling)
      x = m_Width - m_CrawlScroll;
    float y = m_LineRect.y + 8;
    if (m_ShowDateTime)
      y += m_DateTimeHeight;
    DrawText(crawl.text, TextType::Normal, x, y, TextAlignment::Left);
  }

  SDL_SetRenderClipRect(m_Renderer, nullptr);
  SDL_RenderPresent(m_Renderer);
}

void Application::Clear(SDL_Color color)
{
  SDL_SetRenderDrawColor(m_Renderer, color.r, color.g, color.b, color.a);
  SDL_RenderClear(m_Renderer);
}

void Application::DrawRect(SDL_FRect rect, SDL_Color color)
{
  SDL_SetRenderDrawColor(m_Renderer, color.r, color.g, color.b, color.a);
  SDL_RenderRect(m_Renderer, &rect);
}

void Application::DrawText(const std::string &text, TextType type, float x, float y, TextAlignment alignment)
{
  TTF_Font *font = nullptr;
  SDL_Surface *surface = nullptr;
  SDL_Texture *texture = nullptr;
  SDL_FRect rect{x, y, 0, 0};

  switch (type)
  {
  case TextType::Normal:
    font = m_Font;
    break;
  case TextType::Small:
    font = m_SmallFont;
    break;
  }

  surface = TTF_RenderText_Solid_Wrapped(font, text.c_str(), 0, m_TextColor, 0);
  texture = SDL_CreateTextureFromSurface(m_Renderer, surface);

  rect.w = surface->w;
  rect.h = surface->h;
  SDL_DestroySurface(surface);

  switch (alignment)
  {
  case TextAlignment::Left:
    break;
  case TextAlignment::Center:
    rect.x -= rect.w / 2.0f;
    break;
  case TextAlignment::Right:
    rect.x -= rect.w;
    break;
  }

  SDL_SetTextureColorMod(texture, 0, 0, 0);
  SDL_SetTextureAlphaMod(texture, 128);
  rect.x += 3;
  rect.y += 3;
  SDL_RenderTexture(m_Renderer, texture, nullptr, &rect);

  SDL_SetTextureColorMod(texture, 255, 255, 255);
  SDL_SetTextureAlphaMod(texture, 255);
  rect.x -= 3;
  rect.y -= 3;
  SDL_RenderTexture(m_Renderer, texture, nullptr, &rect);
  SDL_DestroyTexture(texture);
}

} // namespace Wx
