// SPDX-License-Identifier: GPL-3.0-only

#include <cmath>
#include <iostream>
#include <print>

#include "Wx/Renderer.hh"

#include "Wx/Assets/Star3000.hh"
#include "Wx/Assets/Star3000_Small.hh"

namespace Wx {

Renderer::Renderer(const Configuration &config)
    : m_Config(config)
{
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();

    m_Window = SDL_CreateWindow(m_Config.title.c_str(), m_Config.width, m_Config.height, 0);
    m_Renderer = SDL_CreateRenderer(m_Window, nullptr);
    {
        SDL_IOStream *data = SDL_IOFromConstMem(g_Star3000, sizeof(g_Star3000));
        m_Font = TTF_OpenFontIO(data, true, m_FontSize);
    }
    {
        SDL_IOStream *data = SDL_IOFromConstMem(g_Star3000_Small, sizeof(g_Star3000_Small));
        m_SmallFont = TTF_OpenFontIO(data, true, m_FontSize);
    }

    Resize(m_Config.width, m_Config.height);
}

Renderer::~Renderer()
{
    if (m_Font) {
        TTF_CloseFont(m_Font);
        m_Font = nullptr;
    }
    if (m_SmallFont) {
        TTF_CloseFont(m_SmallFont);
        m_SmallFont = nullptr;
    }
    if (m_Renderer) {
        SDL_DestroyRenderer(m_Renderer);
        m_Renderer = nullptr;
    }
    if (m_Window) {
        SDL_DestroyWindow(m_Window);
        m_Window = nullptr;
    }
    TTF_Quit();
    SDL_Quit();
}

Rect Renderer::MeasureText(const std::string &text, TextType type)
{
    TTF_Font *font = nullptr;
    Rect r{0, 0, 0, 0};

    switch (type) {
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
    SDL_RenderPresent(m_Renderer);
}

void Renderer::SetFullscreen(bool enabled)
{
    if (enabled) {
        SDL_SetWindowFullscreen(m_Window, SDL_WINDOW_FULLSCREEN);
    } else {
        SDL_SetWindowFullscreen(m_Window, 0);
    }
}

void Renderer::ToggleFullscreen()
{
    m_Fullscreen = !m_Fullscreen;
    SetFullscreen(m_Fullscreen);
}

void Renderer::Clear(const Color &color)
{
    SDL_SetRenderDrawColor(m_Renderer, color.r, color.g, color.b, color.a);
    SDL_RenderClear(m_Renderer);
}

void Renderer::DrawRect(const Rect &rect, const Color &color)
{
    SDL_SetRenderDrawColor(m_Renderer, color.r, color.g, color.b, color.a);
    if (rect.width == 0 || rect.height == 0) {
        SDL_RenderFillRect(m_Renderer, nullptr);
    } else {
        SDL_FRect r{
            static_cast<float>(rect.x),
            static_cast<float>(rect.y),
            static_cast<float>(rect.width),
            static_cast<float>(rect.height),
        };
        SDL_RenderFillRect(m_Renderer, &r);
    }
}

Asset Renderer::RasterizeText(const std::string &text, TextType type, const Color &color)
{
    return RasterizeTextWrapped(text, type, color, 0);
}

Asset Renderer::RasterizeTextWrapped(const std::string &text, TextType type, const Color &color, size_t width)
{
    TTF_Font *font = nullptr;
    SDL_Surface *surface = nullptr;
    SDL_Texture *texture = nullptr;
    SDL_Color c{color.r, color.g, color.b, color.a};
    Asset asset;

    switch (type) {
    case TextType::Normal:
        font = m_Font;
        break;
    case TextType::Small:
        font = m_SmallFont;
        break;
    }

    surface = TTF_RenderText_Solid_Wrapped(font, text.c_str(), 0, c, width);
    texture = SDL_CreateTextureFromSurface(m_Renderer, surface);
    if (!texture) {
        return asset;
    }

    asset.texture = texture;
    asset.width = surface->w;
    asset.height = surface->h;

    SDL_DestroySurface(surface);
    return asset;
}

void Renderer::SetAsset(AssetId id, const Asset &asset)
{
    ClearAsset(id);
    m_Assets[id] = asset;
}

void Renderer::DrawAsset(AssetId id, float x, float y, Alignment alignment, const DrawOptions &options)
{
    Asset &asset = m_Assets[id];
    if (asset.texture == nullptr || asset.width == 0 || asset.height == 0) {
        return;
    }

    SDL_FRect rect{x, y, static_cast<float>(asset.width), static_cast<float>(asset.height)};
    switch (alignment) {
    case Alignment::Left:
        break;
    case Alignment::Center:
        rect.x -= rect.w / 2.0f;
        break;
    case Alignment::Right:
        rect.x -= rect.w;
        break;
    }
    rect.x = std::round(rect.x);
    rect.y = std::round(rect.y);
    rect.w = std::round(rect.w);
    rect.h = std::round(rect.h);

    if (options.shadow) {
        // TODO(frosty): Implement public ColorMod and AlphaMod functions
        Color c;
        SDL_GetTextureColorMod(asset.texture, &c.r, &c.g, &c.b);
        SDL_GetTextureAlphaMod(asset.texture, &c.a);

        SDL_SetTextureColorMod(asset.texture, 20, 20, 20);
        SDL_SetTextureAlphaMod(asset.texture, 255);
        rect.x -= 1;
        rect.y -= 1;
        SDL_RenderTexture(m_Renderer, asset.texture, nullptr, &rect);
        rect.x += 2;
        SDL_RenderTexture(m_Renderer, asset.texture, nullptr, &rect);
        rect.x -= 2;
        rect.y += 2;
        SDL_RenderTexture(m_Renderer, asset.texture, nullptr, &rect);
        rect.x += 1;
        rect.y -= 1;
        rect.x += m_ShadowOffset;
        rect.y += m_ShadowOffset;
        SDL_RenderTexture(m_Renderer, asset.texture, nullptr, &rect);

        SDL_SetTextureColorMod(asset.texture, c.r, c.g, c.b);
        SDL_SetTextureAlphaMod(asset.texture, c.a);
        rect.x -= m_ShadowOffset;
        rect.y -= m_ShadowOffset;
    }
    SDL_RenderTexture(m_Renderer, asset.texture, nullptr, &rect);
}

void Renderer::ClearAsset(AssetId id)
{
    Asset &asset = m_Assets[id];
    if (asset.texture != nullptr) {
        SDL_DestroyTexture(asset.texture);
        asset.texture = nullptr;
    }
    asset.width = 0;
    asset.height = 0;
}

Rect Renderer::GetViewport()
{
    SDL_Rect r;
    SDL_GetRenderViewport(m_Renderer, &r);
    return {r.x, r.y, r.w, r.h};
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
