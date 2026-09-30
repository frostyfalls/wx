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
  AddProduct(Product::CurrentConditions);
  AddLower(Lower::AdCrawl);
  AddLower(Lower::Location);
  AddLower(Lower::CurrentConditions);

  // XXX(frosty): Is this a hacky way to initialize kicking off the loops?
  m_CurrentProduct = m_Products.size() - 1;
  m_CurrentLower = m_Lowers.size() - 1;

  // TODO(frosty): Timed ad crawls that change when a certain time of day is hit
  m_CurrentAdCrawl = "Orcanet - Reliable, affordable internet for the Tampa Bay area";

  m_DrawOptions.shadow = true;
  m_DateTimeHeight = m_Renderer.MeasureText("Mon", TextType::Small).height;
}

void Application::Run()
{
  auto previousTime = std::chrono::system_clock::now();

  NextProduct();
  NextLower();
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

    else if (e.type == SDL_EVENT_KEY_DOWN && e.key.scancode == SDL_SCANCODE_S)
      m_DrawOptions.shadow = !m_DrawOptions.shadow;

    else if (e.type == SDL_EVENT_KEY_DOWN && e.key.scancode == SDL_SCANCODE_C)
      m_ShowDateTime = !m_ShowDateTime;

    else if (e.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED)
    {
      m_Renderer.Resize(e.window.data1, e.window.data2);

      m_ProductViewport.width = m_Renderer.Width() - m_ProductViewport.x * 2;
      m_ProductViewport.height = m_Renderer.Height() - m_ProductViewport.y - m_LowerViewport.height;

      m_LowerViewport.width = m_Renderer.Width() - m_LowerViewport.x * 2;
      m_LowerViewport.y = m_Renderer.Height() - m_LowerViewport.height;
    }
  }
}

void Application::OnUpdate(float deltaSeconds)
{
  m_ElapsedSecond += deltaSeconds;

  if (CurrentLower() != Lower::AdCrawl)
  {
    m_LowerTime += deltaSeconds;
    if (m_LowerTime >= 5)
      NextLower();
  }
  else
  {
    if (m_LowerOffset + m_Renderer.GetAsset(AssetId::Lower).width < 0)
      NextLower();
    else
      m_LowerOffset -= 2.0f;
  }

  if (m_ElapsedSecond >= 1.0f)
  {
    m_ElapsedSecond = 0.0f;
    const auto localTime = std::chrono::time_point_cast<std::chrono::seconds>
      (std::chrono::current_zone()->to_local(m_CurrentTime));
    m_Renderer.SetAsset(
      AssetId::Date,
      m_Renderer.RasterizeText(std::format("{:%a %b %d}", localTime), TextType::Small, m_TextColor)
    );
    std::string timeFmt = std::format("{:%I:%M:%S %p}", localTime);
    if (timeFmt.starts_with('0'))
      timeFmt.erase(0, 1);
    m_Renderer.SetAsset(
      AssetId::Time,
      m_Renderer.RasterizeText(timeFmt, TextType::Small, m_TextColor)
    );
  }
}

void Application::OnRender()
{
  m_Renderer.Clear(m_BackgroundColor);
  m_Renderer.DrawRect({0, m_LowerViewport.y, m_Renderer.Width(), 1}, m_TextColor);

  m_Renderer.SetViewport(m_ProductViewport);
  m_Renderer.DrawAsset(AssetId::Product, 0, 0, Alignment::Left, m_DrawOptions);
  m_Renderer.ClearViewport();

  m_Renderer.SetViewport(m_LowerViewport);
  float crawlY = 5;
  if (m_ShowDateTime)
  {
    crawlY -= 3;
    m_Renderer.DrawAsset(AssetId::Date, 0, crawlY, Alignment::Left, m_DrawOptions);
    m_Renderer.DrawAsset(AssetId::Time, m_LowerViewport.width, crawlY, Alignment::Right, m_DrawOptions);
    crawlY += m_DateTimeHeight + 5;
  }
  m_Renderer.DrawAsset(AssetId::Lower, m_LowerOffset, crawlY, Alignment::Left, m_DrawOptions);
  m_Renderer.ClearViewport();

  m_Renderer.Present();
}

void Application::NextProduct()
{
  if (m_Products.empty())
    return;

  m_CurrentProduct = (m_CurrentProduct + 1) % m_Products.size();

  std::string text;
  switch (CurrentProduct())
  {
  case Product::CurrentConditions:
    text = "Conditions at Tampa Bay\nFair / Windy\nTemp: 89°F   Wind Chill: 89°F";
    break;
  }
  m_Renderer.SetAsset(AssetId::Product, m_Renderer.RasterizeTextWrapped(text, TextType::Normal, m_TextColor, m_Renderer.Width()));
}

void Application::ClearProducts()
{
  m_Products.clear();
  m_CurrentProduct = 0;
}

void Application::NextLower()
{
  if (m_Lowers.empty())
    return;

  m_CurrentLower = (m_CurrentLower + 1) % m_Lowers.size();
  m_LowerOffset = 0;
  m_LowerTime = 0;

  std::string text;
  switch (CurrentLower())
  {
  case Lower::Location:
    text = "Conditions at Tampa Bay";
    break;
  case Lower::CurrentConditions:
    text = "Fair / Windy";
    break;
  case Lower::AdCrawl:
    text = m_CurrentAdCrawl;
    m_LowerOffset = m_Renderer.Width();
    break;
  }
  m_Renderer.SetAsset(AssetId::Lower, m_Renderer.RasterizeText(text, TextType::Normal, m_TextColor));
}

void Application::ClearLowers()
{
  m_Lowers.clear();
  m_CurrentLower = 0;
}

} // namespace Wx
