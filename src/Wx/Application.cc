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
  AddProduct(Product::CurrentConditions);

  m_DrawOptions.shadow = true;
  m_DateTimeHeight = m_Renderer.MeasureText("Mon", TextType::Small).height;
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
  SDL_Event e;

  while (SDL_PollEvent(&e))
  {
    if (e.type == SDL_EVENT_QUIT)
      m_Running = false;

    else if (e.type == SDL_EVENT_KEY_DOWN && e.key.scancode == SDL_SCANCODE_Q)
      m_Running = false;

    else if (e.type == SDL_EVENT_KEY_DOWN && e.key.scancode == SDL_SCANCODE_F)
    {
      m_Fullscreen = !m_Fullscreen;
      m_Renderer.SetFullscreen(m_Fullscreen);
    }

    else if (e.type == SDL_EVENT_KEY_DOWN && e.key.scancode == SDL_SCANCODE_C)
      m_ShowDateTime = !m_ShowDateTime;

    else if (e.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED)
    {
      m_Renderer.Resize(e.window.data1, e.window.data2);

      m_ProductViewport.width = m_Renderer.Width() - m_ProductViewport.x * 2;
      m_ProductViewport.height = m_Renderer.Height() - m_ProductViewport.y - m_CrawlViewport.height;

      m_CrawlViewport.width = m_Renderer.Width() - m_CrawlViewport.x * 2;
      m_CrawlViewport.y = m_Renderer.Height() - m_CrawlViewport.height;
    }
  }
}

void Application::OnUpdate(float deltaSeconds)
{
  m_ElapsedSecond += deltaSeconds;

  if (m_Renderer.Width() - m_CrawlOffset + m_Renderer.GetAsset(AssetId::Crawl).width < 0)
    m_CrawlOffset = 0.0f;
  else
    m_CrawlOffset += 2.0f;

  if (m_ElapsedSecond >= 1.0f)
  {
    m_ElapsedSecond = 0.0f;
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
  if (m_Renderer.GetAsset(AssetId::Crawl).texture == nullptr)
    m_Renderer.SetAsset(AssetId::Crawl, m_Renderer.RasterizeText("Orcanet - Affordable, reliable internet for the Tampa Bay area", TextType::Normal, m_TextColor));

  if (m_Renderer.GetAsset(AssetId::Product).texture == nullptr)
    m_Renderer.SetAsset(AssetId::Product, m_Renderer.RasterizeTextWrapped("Conditions at Tampa Bay\nFair / Windy\nTemp: 89°F   Wind Chill: 89°F", TextType::Normal, m_TextColor, m_Renderer.Width()));

  m_Renderer.Clear(m_BackgroundColor);
  m_Renderer.DrawRect({0, m_CrawlViewport.y, m_Renderer.Width(), 1}, m_TextColor);

  m_Renderer.SetViewport(m_ProductViewport);
  m_Renderer.DrawAsset(AssetId::Product, 0, 0, Alignment::Left, m_DrawOptions);
  m_Renderer.ClearViewport();

  m_Renderer.SetViewport(m_CrawlViewport);
  float crawlY = 5;
  if (m_ShowDateTime)
  {
    crawlY -= 3;
    m_Renderer.DrawAsset(AssetId::Date, 0, crawlY, Alignment::Left, m_DrawOptions);
    m_Renderer.DrawAsset(AssetId::Time, m_CrawlViewport.width, crawlY, Alignment::Right, m_DrawOptions);
    crawlY += m_DateTimeHeight + 5;
  }
  m_Renderer.DrawAsset(AssetId::Crawl, m_Renderer.Width() - m_CrawlOffset, crawlY, Alignment::Left, m_DrawOptions);
  m_Renderer.ClearViewport();

  m_Renderer.Present();
}

} // namespace Wx
