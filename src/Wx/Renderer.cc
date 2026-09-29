// SPDX-License-Identifier: GPL-3.0-only

#include <print>

#include "Wx/Renderer.hh"

#include "Wx/Assets/Star3000.hh"
#include "Wx/Assets/Star3000_Small.hh"

namespace Wx
{

Renderer::Renderer(const Configuration &config)
  : m_Config(config)
{
  SDL_IOStream *fontData = nullptr;

  SDL_Init(SDL_INIT_VIDEO);
  TTF_Init();

  m_Window = SDL_CreateWindow(m_Config.title.c_str(), m_Config.width, m_Config.height, 0);

  fontData = SDL_IOFromConstMem(g_Star3000, sizeof(g_Star3000));
  m_Font = TTF_OpenFontIO(fontData, true, m_FontSize);

  fontData = SDL_IOFromConstMem(g_Star3000_Small, sizeof(g_Star3000_Small));
  m_SmallFont = TTF_OpenFontIO(fontData, true, m_FontSize);

  m_Renderer = SDL_CreateRenderer(m_Window, nullptr);

  SDL_SetRenderVSync(m_Renderer, true);
}

Renderer::~Renderer()
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

Rect Renderer::MeasureText(const std::string &text, TextType type)
{
  std::println("MeasureText();");

  TTF_Font *font = nullptr;
  Rect r{0, 0, 0, 0};

  switch (type)
  {
    case TextType::Normal:
      font = m_Font;
      break;
    case TextType::Small:
      font = m_SmallFont;
      break;
  }

  TTF_GetStringSize(font, text.c_str(), 0, &r.width, &r.height);
  return r;
}

void Renderer::Present()
{
  std::println("Present();");

  SDL_RenderPresent(m_Renderer);
}

void Renderer::SetFullscreen(bool enabled)
{
  std::println("SetFullscreen();");

  if (enabled)
    SDL_SetWindowFullscreen(m_Window, SDL_WINDOW_FULLSCREEN);
  else
    SDL_SetWindowFullscreen(m_Window, 0);
}

void Renderer::Clear(Color color)
{
  std::println("Clear();");

  SDL_SetRenderDrawColor(m_Renderer, color.r, color.g, color.b, color.a);
  SDL_RenderClear(m_Renderer);
}

void Renderer::DrawRect(Rect rect, Color color)
{
  std::println("DrawRect();");

  SDL_SetRenderDrawColor(m_Renderer, color.r, color.g, color.b, color.a);
  if (rect.width == 0 || rect.height == 0)
  {
    SDL_RenderFillRect(m_Renderer, nullptr);
  }
  else
  {
    SDL_FRect r{
      static_cast<float>(rect.x),
      static_cast<float>(rect.y),
      static_cast<float>(rect.width),
      static_cast<float>(rect.height),
    };
    SDL_RenderFillRect(m_Renderer, &r);
  }
}

Asset Renderer::RasterizeText(const std::string &text, TextType type, Color color)
{
  std::println("RasterizeText();");

  TTF_Font *font = nullptr;
  SDL_Surface *surface = nullptr;
  SDL_Texture *texture = nullptr;
  SDL_Color c{color.r, color.g, color.b, color.a};
  Asset asset;

  switch (type)
  {
  case TextType::Normal:
    font = m_Font;
    break;
  case TextType::Small:
    font = m_SmallFont;
    break;
  }

  surface = TTF_RenderText_Solid_Wrapped(font, text.c_str(), 0, c, 0);
  texture = SDL_CreateTextureFromSurface(m_Renderer, surface);

  asset.texture = texture;
  asset.width = surface->w;
  asset.height = surface->h;

  SDL_DestroySurface(surface);
  return asset;
}

void Renderer::DrawAsset(AssetId id, float x, float y, TextAlignment alignment)
{
  std::println("DrawAsset();");

  Asset asset = m_Assets[id];

  SDL_FRect rect{x, y, static_cast<float>(asset.width), static_cast<float>(asset.height)};

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

  SDL_RenderTexture(m_Renderer, asset.texture, nullptr, &rect);
}

void Renderer::SetViewport(const Rect &rect)
{
  SDL_Rect r{rect.x, rect.y, rect.width, rect.height};
  SDL_SetRenderViewport(m_Renderer, &r);
}

void Renderer::ClearViewport()
{
  SDL_SetRenderViewport(m_Renderer, nullptr);
}

} // namespace Wx
