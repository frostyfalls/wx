// SPDX-License-Identifier: GPL-3.0-only

#include <chrono>
#include <ranges>
#include <string>

#include <SDL3/SDL.h>

#include "Wx/Application.hh"

namespace Wx {

Application::Application(const Configuration &config)
    : m_Renderer(config)
{
    AddProduct(Product::CurrentConditions);
    AddProduct(Product::NWSForecast);
    AddProduct(Product::Bulletin);
    AddLower(Lower::Location);
    AddLower(Lower::CurrentConditions);
    AddLower(Lower::Crawl);

    AddCrawl(900, "Orcanet - Reliable, affordable cable internet and television for the Tampa Bay area... Visit us at orcanet.sh for a quick service installation today!");
    AddCrawl(935, "voidlinux.org - a small, adaptable Linux distribution fit for all needs");
    AddCrawl(937, "Need a robust private phone service for your small business? Reach out to Frosty's Home Lab at frostyshomelab.net to get your business running in no time...");

    // XXX(frosty): Is this a hacky way to initialize kicking off the loops?
    m_CurrentProduct = m_Products.size() - 1;
    m_CurrentLower = m_Lowers.size() - 1;

    m_Renderer.SetAsset(AssetId::Date, m_Renderer.RasterizeText("Tue Nov 3", TextType::Small, m_TextColor));
    m_Renderer.SetAsset(AssetId::Time, m_Renderer.RasterizeText("10:01:20 PM", TextType::Small, m_TextColor));

    m_DrawOptions.shadow = true;
}

void Application::Run()
{
    using Clock = std::chrono::steady_clock;
    auto previous = Clock::now();

    NextProduct();
    NextLower();
    while (!m_Quit) {
        const auto now = Clock::now();
        const std::chrono::duration<float> elapsed = now - previous;
        previous = now;
        const float deltaSeconds = elapsed.count();

        OnEvent();
        OnUpdate(deltaSeconds);
        OnRender();
    }
}

void Application::OnEvent()
{
    SDL_Event e;

    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_EVENT_QUIT) {
            m_Quit = true;
        } else if (e.type == SDL_EVENT_KEY_DOWN && e.key.scancode == SDL_SCANCODE_Q) {
            m_Quit = true;
        } else if (e.type == SDL_EVENT_KEY_DOWN && e.key.scancode == SDL_SCANCODE_F) {
            m_Renderer.ToggleFullscreen();
        } else if (e.type == SDL_EVENT_KEY_DOWN && e.key.scancode == SDL_SCANCODE_S) {
            m_DrawOptions.shadow = !m_DrawOptions.shadow;
        } else if (e.type == SDL_EVENT_KEY_DOWN && e.key.scancode == SDL_SCANCODE_N) {
            NextProduct();
        } else if (e.type == SDL_EVENT_KEY_DOWN && e.key.scancode == SDL_SCANCODE_G) {
            m_GreenMode = !m_GreenMode;
        } else if (e.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED) {
            m_Renderer.Resize(e.window.data1, e.window.data2);

            m_ProductViewport.width = m_Renderer.Width() - m_ProductViewport.x * 2;
            m_ProductViewport.height = m_Renderer.Height() - m_ProductViewport.y - m_LowerViewport.height;

            m_LowerViewport.width = m_Renderer.Width() - m_LowerViewport.x * 2;
            m_LowerViewport.y = m_Renderer.Height() - m_LowerViewport.height;
            RenderProduct();
        }
    }
}

void Application::OnUpdate(float deltaSeconds)
{
    bool isCrawl = CurrentLower() == Lower::Crawl;
    bool isBulletin = CurrentProduct() == Product::Bulletin;

    if (!isCrawl) {
        m_LowerTime += deltaSeconds;
        if (m_LowerTime >= 5) {
            NextLower();
        }
    } else {
        if (m_LowerOffset + m_Renderer.GetAsset(AssetId::Lower).width >= 0) {
            m_LowerOffset -= 100.0f * deltaSeconds;
        } else {
            NextLower();
        }
    }

    if (!isBulletin) {
        m_ProductTime += deltaSeconds;
        if (m_ProductTime >= 6) {
            NextProduct();
        }
    } else {
        if (m_ProductOffset + m_Renderer.GetAsset(AssetId::ProductBody).height >= 0) {
            m_ProductOffset -= 50.0f * deltaSeconds;
        } else {
            NextProduct();
        }
    }
}

void Application::OnRender()
{
    bool isCrawl = CurrentLower() == Lower::Crawl;
    bool isBulletin = CurrentProduct() == Product::Bulletin;
    uint32_t lowerY = 3;
    Color color = m_GreenMode ? m_GreenColor : m_BackgroundColor;

    m_Renderer.Clear(color);
    if (isBulletin) {
        if (!m_GreenMode) {
            m_Renderer.DrawRect({0, 0, m_Renderer.Width(), m_ProductViewport.y + m_ProductViewport.height + 1}, m_BulletinColor);
        }
    } else {
        m_Renderer.DrawRect({0, m_LowerViewport.y, m_Renderer.Width(), 1}, m_TextColor);
    }

    m_Renderer.SetViewport(m_ProductViewport);
    m_Renderer.DrawAsset(AssetId::ProductTitle, m_ProductViewport.width / 2.0f, 0, Alignment::Center, m_DrawOptions);
    m_Renderer.DrawAsset(AssetId::ProductBody, 0, m_ProductOffset, Alignment::Left, m_DrawOptions);

    m_Renderer.SetViewport(m_LowerViewport);
    if (!isCrawl) {
        m_Renderer.DrawAsset(AssetId::Date, 0, lowerY, Alignment::Left, m_DrawOptions);
        m_Renderer.DrawAsset(AssetId::Time, m_LowerViewport.width, lowerY, Alignment::Right, m_DrawOptions);
        lowerY += 3 + m_Renderer.MeasureText("Mon", TextType::Small).height;
    }
    m_Renderer.DrawAsset(AssetId::Lower, m_LowerOffset, lowerY, Alignment::Left, m_DrawOptions);

    m_Renderer.ClearViewport();
    m_Renderer.Present();
}

void Application::NextProduct()
{
    if (m_Products.empty()) {
        return;
    }

    m_CurrentProduct = (m_CurrentProduct + 1) % m_Products.size();
    m_ProductOffset = 0.0f;
    m_ProductTime = 0.0f;

    RenderProduct();
}

void Application::RenderProduct()
{
    TextType titleType = TextType::Small;
    std::string productTitle, productBody;
    switch (CurrentProduct()) {
    case Product::CurrentConditions:
        productBody = "Conditions at Tampa Bay\nScattered Showers\nTemp: 89°F\nNovember Precipitation: 2.05 in";
        break;
    case Product::NWSForecast:
        productTitle = "Nat'l Weather Service Forecast";
        productBody = "Tonight...Drizzle and fog with a 90 percent chance of rain. High 70 to 75.";
        break;
    case Product::Bulletin:
        productBody = "No warnings currently";
        break;
    }
    if (productTitle.size() > 0) {
        m_Renderer.SetAsset(AssetId::ProductTitle, m_Renderer.RasterizeText(productTitle, titleType, m_TextColor));
        m_ProductOffset = m_Renderer.MeasureText(productTitle, titleType).height + m_ProductViewport.y;
    } else {
        m_Renderer.ClearAsset(AssetId::ProductTitle);
        if (CurrentProduct() == Product::Bulletin) {
            m_ProductOffset = m_ProductViewport.height + 20;
        } else {
            m_ProductOffset = 0;
        }
    }
    if (productBody.size() > 0) {
        m_Renderer.SetAsset(AssetId::ProductBody, m_Renderer.RasterizeTextWrapped(productBody, TextType::Normal, m_TextColor, m_ProductViewport.width));
    } else {
        m_Renderer.ClearAsset(AssetId::ProductBody);
    }
}

void Application::ClearProducts()
{
    m_Products.clear();
    m_CurrentProduct = 0;
}

void Application::NextLower()
{
    if (m_Lowers.empty()) {
        return;
    }

    m_CurrentLower = (m_CurrentLower + 1) % m_Lowers.size();
    m_LowerOffset = 0.0f;
    m_LowerTime = 0.0f;

    RenderLower();
}

void Application::RenderLower()
{
    std::string text;
    switch (CurrentLower()) {
    case Lower::Location:
        text = "Conditions at Tampa Bay";
        break;
    case Lower::CurrentConditions:
        text = "Fair / Windy";
        break;
    case Lower::Crawl:
        text = CurrentCrawl();
        m_LowerOffset = m_Renderer.Width() - m_LowerViewport.x;
        break;
    }
    m_Renderer.SetAsset(AssetId::Lower, m_Renderer.RasterizeText(text, TextType::Normal, m_TextColor));
}

void Application::ClearLowers()
{
    m_Lowers.clear();
    m_CurrentLower = 0;
}

std::string Application::CurrentCrawl()
{
    if (m_Crawls.size() == 0) {
        return "";
    }
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm local = *std::localtime(&t);
    uint32_t sinceMidnight = 60 * local.tm_hour + local.tm_min;
    std::string crawl;
    for (const auto &[key, value] : m_Crawls | std::views::reverse) {
        if (key <= sinceMidnight) {
            return value;
        }
    }
    return "";
}

void Application::ClearCrawls()
{
    m_Crawls.clear();
}

} // namespace Wx
