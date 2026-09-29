// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <map>

#include "Wx/Wx.hh"

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

namespace Wx
{

struct Rect
{
  std::int32_t x, y, width, height;
};

struct Color
{
  std::uint8_t r, g, b, a;
};

enum class AssetId
{
  Date,
  Time,
  Crawl,
  Slide,
};

struct Asset
{
  SDL_Texture *texture = nullptr;
  std::uint32_t width, height;
};

enum class TextType
{
  Normal,
  Small,
};

enum class TextAlignment
{
  Left,
  Center,
  Right,
};

class Renderer
{
public:
  Renderer(const Configuration &config);
  ~Renderer();

  void Present();

  void Clear(Color color);
  void DrawRect(Rect rect, Color color);

  Rect MeasureText(const std::string &text, TextType type);
  Asset RasterizeText(const std::string &text, TextType type, Color color);

  Asset &GetAsset(AssetId id) { return m_Assets[id]; }
  void SetAsset(AssetId id, Asset asset) { m_Assets[id] = asset; }
  void DrawAsset(AssetId, float x, float y, TextAlignment alignment);

  void SetViewport(const Rect &rect);
  void ClearViewport();

  std::int32_t Width() const { return m_Width; }
  std::int32_t Height() const { return m_Height; }
  void Resize(std::size_t width, std::size_t height) { m_Width = width, m_Height = height; }
  void SetFullscreen(bool enabled);

private:
  Configuration m_Config;
  std::int32_t m_Width = 0, m_Height = 0;
  std::map<AssetId, Asset> m_Assets;
  std::size_t m_FontSize = 32;

  SDL_Window *m_Window = nullptr;
  SDL_Renderer *m_Renderer = nullptr;
  TTF_Font *m_Font = nullptr;
  TTF_Font *m_SmallFont = nullptr;
};

} // namespace Wx
