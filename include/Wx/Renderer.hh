// SPDX-License-Identifier: GPL-3.0-only

#ifndef WX_RENDERER_HH
#define WX_RENDERER_HH

#include <map>
#include <string>

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "Wx/Wx.hh"

namespace Wx {

enum class AssetId
{
    Date = 0,
    Time,
    Lower,
    ProductTitle,
    ProductBody,
};

enum class TextType
{
    Normal = 0,
    Small,
};

enum class Alignment
{
    Left = 0,
    Center,
    Right,
};

struct Rect
{
    int32_t x = 0, y = 0, width = 0, height = 0;
};

struct Color
{
    uint8_t r = 0, g = 0, b = 0, a = 0;
};

struct Asset
{
    uint32_t width = 0, height = 0;
    SDL_Texture *texture = nullptr;
};

struct DrawOptions
{
    bool shadow = false;
};

class Renderer
{
public:
    Renderer(const Configuration &config);
    ~Renderer();

    void Present();

    void Clear(const Color &color);
    void DrawRect(const Rect &rect, const Color &color);

    Rect MeasureText(const std::string &text, TextType type);
    Asset RasterizeText(const std::string &text, TextType type, const Color &color);
    Asset RasterizeTextWrapped(const std::string &text, TextType type, const Color &color, size_t width);

    Asset &GetAsset(AssetId id) { return m_Assets[id]; }
    void SetAsset(AssetId id, const Asset &asset);
    void DrawAsset(AssetId id, float x, float y, Alignment alignment, const DrawOptions &options);
    void ClearAsset(AssetId id);

    Rect GetViewport();
    void SetViewport(const Rect &rect);
    void ClearViewport();

    int32_t Width() const { return m_Width; }
    int32_t Height() const { return m_Height; }
    void Resize(int32_t width, int32_t height) { m_Width = width, m_Height = height; }
    void SetFullscreen(bool enabled);
    void ToggleFullscreen();

private:
    Configuration m_Config;
    int32_t m_Width = 0, m_Height = 0;
    std::map<AssetId, Asset> m_Assets;
    const size_t m_FontSize = 32;
    const uint8_t m_ShadowOffset = 2;
    bool m_Fullscreen = false;

    SDL_Window *m_Window = nullptr;
    SDL_Renderer *m_Renderer = nullptr;
    TTF_Font *m_Font = nullptr;
    TTF_Font *m_SmallFont = nullptr;
};

} // namespace Wx

#endif // WX_RENDERER_HH
