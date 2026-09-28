// SPDX-License-Identifier: GPL-3.0-only

#include <chrono>
#include <format>
#include <print>
#include <string>

#include "wx/wx.hh"

#include "wx/assets/star3000.hh"
#include "wx/assets/star3000_small.hh"
#include "wx/assets/starjr.hh"

namespace Wx
{

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
  fontData = SDL_IOFromConstMem(star3000_small_font, sizeof(star3000_small_font));
  m_SmallFont = TTF_OpenFontIO(fontData, true, m_Config.fontSize);

  SDL_SetRenderVSync(m_Renderer, true);

  m_Running = true;
}

Application::~Application()
{
  TTF_CloseFont(m_SmallFont);
  m_SmallFont = nullptr;

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
  m_Slides.push_back({ Product::RegionalObservations, 5.0f });
  m_Slides.push_back({ Product::CurrentConditions, 8.0f });

  m_Crawls.push_back({ "September Precipitation: 0.5 in", 4.0f, false });
  m_Crawls.push_back({ "orcanet: fast, reliable cable internet for the Tampa Bay area.", 16.0f, true });

  auto lastFrame = std::chrono::system_clock::now();

  while (m_Running)
  {
    const auto now = std::chrono::system_clock::now();
    float deltaSeconds = std::chrono::duration<float>(now - lastFrame).count();

    m_RenderTime = std::chrono::round<std::chrono::seconds>(std::chrono::current_zone()->to_local(now));

    ProcessEvents();
    Update(deltaSeconds);
    Render();

    lastFrame = now;
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
  m_SlideTimer += deltaSeconds;
  if (m_SlideTimer >= m_Slides[m_CurrentSlide].durationSeconds)
  {
    m_SlideTimer = 0;
    AdvanceSlide();
  }

  m_CrawlTimer += deltaSeconds;
  if (m_CrawlTimer >= m_Crawls[m_CurrentCrawl].durationSeconds)
  {
    m_CrawlTimer = 0;
    AdvanceCrawl();
  }
  if (m_Crawls[m_CurrentCrawl].scroll)
    m_CrawlScroll += 2.0f;
}

void Application::AdvanceSlide()
{
  if (m_CurrentSlide == m_Slides.size() - 1)
    m_CurrentSlide = 0;
  else
    m_CurrentSlide += 1;

  // m_SlideScroll = 0.0f;
}

void Application::AdvanceCrawl()
{
  if (m_CurrentCrawl == m_Crawls.size() - 1)
    m_CurrentCrawl = 0;
  else
    m_CurrentCrawl += 1;

  m_CrawlScroll = 0.0f;
}

void Application::Render()
{
  std::int32_t windowWidth, windowHeight;
  SDL_GetWindowSize(m_Window, &windowWidth, &windowHeight);
  SDL_Rect clipRect{40, 20, windowWidth - 80, windowHeight - 40};
  SDL_FRect lineRect{0, static_cast<float>(windowHeight - 100), static_cast<float>(windowWidth), 2};

  SDL_SetRenderDrawColor(m_Renderer, m_BackgroundColor.r, m_BackgroundColor.g, m_BackgroundColor.b, m_BackgroundColor.a);
  SDL_RenderClear(m_Renderer);
  SDL_SetRenderDrawColor(m_Renderer, m_TextColor.r, m_TextColor.g, m_TextColor.b, m_TextColor.a);
  SDL_RenderRect(m_Renderer, &lineRect);

  SDL_SetRenderClipRect(m_Renderer, &clipRect);

  {
    std::string message;
    switch (m_Slides[m_CurrentSlide].product)
    {
    case Product::CurrentConditions:
      message = "Current Conditions";
      break;
    case Product::RegionalObservations:
      message = "Regional Observations";
      break;
    case Product::Warning:
      message = "Warning";
      break;
    }
    RenderTextAt(message, TextType::Small, windowWidth / 2, clipRect.y, TextAlignment::Center);
  }

  {
    std::string message;
    switch (m_Slides[m_CurrentSlide].product)
    {
    case Product::CurrentConditions:
      message = R"(
Conditions at Tampa

Temp: 59°F   Wind Chill: 50°F
Humidity: 5%  Dewpoint: 20°F

September Precipitation: 0.5 in
)";
      break;
    case Product::RegionalObservations:
      message = "Regional Observations Message";
      break;
    case Product::Warning:
      message = m_WeatherData.warnings[0];
      break;
    }
    RenderTextAt(message, TextType::Normal, clipRect.x, clipRect.y + 30, TextAlignment::Left);
  }

  RenderTextAt(std::format("{:%a %b %d}", m_RenderTime), TextType::Small, clipRect.x, lineRect.y + 3, TextAlignment::Left);
  RenderTextAt(std::format("{:%H:%M:%S %p}", m_RenderTime), TextType::Small, clipRect.w, lineRect.y + 3, TextAlignment::Right);
  {
    float x = clipRect.x;
    if (m_Crawls[m_CurrentCrawl].scroll)
      x = windowWidth - m_CrawlScroll;
    RenderTextAt(m_Crawls[m_CurrentCrawl].text, TextType::Normal, x, lineRect.y + 28, TextAlignment::Left);
  }

  SDL_SetRenderClipRect(m_Renderer, nullptr);

  SDL_RenderPresent(m_Renderer);
}

SDL_Texture *Application::RasterizeText(const std::string &text, TextType type)
{
  TTF_Font *font;
  switch (type)
  {
  case TextType::Normal:
    font = m_Font;
    break;
  case TextType::Small:
    font = m_SmallFont;
    break;
  }
  SDL_Surface *surface = nullptr;
  SDL_Texture *texture = nullptr;

  surface = TTF_RenderText_Solid_Wrapped(font, text.c_str(), 0, m_TextColor, 0);
  texture = SDL_CreateTextureFromSurface(m_Renderer, surface);

  SDL_DestroySurface(surface);
  return texture;
}

void Application::RenderTextAt(const std::string &text, TextType type, float x, float y, TextAlignment alignment)
{
  SDL_Texture *texture = RasterizeText(text, type);

  float width, height;
  SDL_GetTextureSize(texture, &width, &height);

  switch (alignment)
  {
  case TextAlignment::Left:
    break;
  case TextAlignment::Center:
    x -= width / 2.0f;
    break;
  case TextAlignment::Right:
    x -= width;
    break;
  }

  SDL_FRect rect{x, y, width, height};
  SDL_RenderTexture(m_Renderer, texture, nullptr, &rect);
  SDL_DestroyTexture(texture);
}

} // namespace Wx
