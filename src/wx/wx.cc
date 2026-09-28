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
  fontData = SDL_IOFromConstMem(star3000_small_font, sizeof(star3000_small_font));
  m_SmallFont = TTF_OpenFontIO(fontData, true, m_Config.fontSize);

  SDL_SetRenderVSync(m_Renderer, true);

  m_Running = true;
}

Application::~Application()
{
  SDL_DestroyTexture(m_SlideTexture);
  m_SlideTexture = nullptr;

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
  auto lastFrame = std::chrono::system_clock::now();

  m_WeatherData.location = "Tampa";
  m_WeatherData.currentConditions = "Sunny";
  m_WeatherData.temperature = "81";
  m_WeatherData.humidity = "50";
  m_WeatherData.windChill = "30";
  m_WeatherData.warnings.push_back(R"(
Don't go outside there's a
hurricane lol. florida

with love, weather service
)");

  if (m_WeatherData.hasWarnings())
    AddSlide({ Product::Warning, 8.0f });
  AddSlide({ Product::RegionalObservations, 5.0f });
  AddSlide({ Product::CurrentConditions, 8.0f });

  // AddCrawl({ "Text crap poop", 2.0f, true });
  // AddCrawl({ "Item number 2", 4.0f, false });
  // AddCrawl({ "I LOVE TWC", 8.0f, false });
  // AddCrawl({ "Accurate, dependable forecasts - The Weather Channel", 9.0f, true });
  AddCrawl({ "September Precipitation: 0.5 in", 4.0f, false });
  AddCrawl({ "orcanet: fast, reliable cable internet for the Tampa Bay area.", 16.0f, true });

  m_Scrolling = true;

  m_CurrentSlide = 0;
  m_CurrentProduct = m_Slides[m_CurrentSlide].product;
  m_SlideDuration = m_Slides[m_CurrentSlide].durationSeconds;

  m_CurrentCrawl = 0;
  m_CrawlDuration = m_Crawls[m_CurrentCrawl].durationSeconds;
  m_Scrolling = m_Crawls[m_CurrentCrawl].scroll;
  m_ScrollOffset = 0.0f;
  m_CrawlText = m_Crawls[m_CurrentCrawl].text;
  while (m_Running)
  {
    const auto now = std::chrono::system_clock::now();
    const auto nowLocal = std::chrono::round<std::chrono::seconds>(std::chrono::current_zone()->to_local(now));
    float deltaSeconds = std::chrono::duration<float>(now - lastFrame).count();
    m_CurrentDateFmt = std::format("{:%a %b %d}", nowLocal);
    m_CurrentTimeFmt = std::format("{:%H:%M:%S %p}", nowLocal);
    lastFrame = now;

    ProcessEvents();
    Update(deltaSeconds);
    Render();
  }
}

void Application::AddSlide(const Slide &slide)
{
  m_Slides.push_back(slide);
}

void Application::AddCrawl(const Crawl &crawl)
{
  m_Crawls.push_back(crawl);
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
  m_ElapsedSecondsCrawl += deltaSeconds;
  if (m_ElapsedSeconds >= m_SlideDuration)
  {
    m_ElapsedSeconds = 0;
    AdvanceSlide();
  }
  if (m_ElapsedSecondsCrawl >= m_CrawlDuration)
  {
    m_ElapsedSecondsCrawl = 0;
    AdvanceCrawl();
  }
  if (m_Scrolling)
    m_ScrollOffset += 2.0f;
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

void Application::AdvanceCrawl()
{
  if (m_CurrentCrawl == m_Crawls.size() - 1)
    m_CurrentCrawl = 0;
  else
    m_CurrentCrawl += 1;

  m_Scrolling = m_Crawls[m_CurrentCrawl].scroll;
  m_CrawlDuration = m_Crawls[m_CurrentCrawl].durationSeconds;
  m_ScrollOffset = 0.0f;
  m_CrawlText = m_Crawls[m_CurrentCrawl].text;
}

void Application::Render()
{
  std::int32_t windowWidth, windowHeight;

  SDL_GetWindowSize(m_Window, &windowWidth, &windowHeight);
  SDL_Rect slideClip{40, 20, windowWidth - 80, windowHeight - 40};
  SDL_FRect lineRect{0, static_cast<float>(windowHeight - 100), static_cast<float>(windowWidth), 2};

  SDL_SetRenderDrawColor(m_Renderer, m_BackgroundColor.r, m_BackgroundColor.g, m_BackgroundColor.b, m_BackgroundColor.a);
  SDL_RenderClear(m_Renderer);

  SDL_SetRenderDrawColor(m_Renderer, m_TextColor.r, m_TextColor.g, m_TextColor.b, m_TextColor.a);
  SDL_RenderRect(m_Renderer, &lineRect);

  SDL_SetRenderClipRect(m_Renderer, &slideClip);

  {
    std::string message;
    switch (m_CurrentProduct)
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
    RenderTextAt(message, TextType::Small, windowWidth / 2, slideClip.y, TextAlignment::Center);
  }
  {
    std::string message;
    switch (m_CurrentProduct)
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
    RenderTextAt(message, TextType::Normal, slideClip.x, slideClip.y + 30, TextAlignment::Left);
  }

  RenderTextAt(m_CurrentDateFmt, TextType::Small, slideClip.x, lineRect.y + 3, TextAlignment::Left);
  RenderTextAt(m_CurrentTimeFmt, TextType::Small, slideClip.w, lineRect.y + 3, TextAlignment::Right);
  {
    float x = slideClip.x;
    if (m_Scrolling)
      x = windowWidth - m_ScrollOffset;
    RenderTextAt(m_CrawlText, TextType::Normal, x, lineRect.y + 28, TextAlignment::Left);
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
