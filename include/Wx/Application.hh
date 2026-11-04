// SPDX-License-Identifier: GPL-3.0-only

#ifndef WX_APPLICATION_HH
#define WX_APPLICATION_HH

#include <chrono>

#include "Wx/Renderer.hh"
#include "Wx/Wx.hh"

namespace Wx {

enum class Product
{
    CurrentConditions = 0,
    NWSForecast,
    Bulletin,
};

enum class Lower
{
    Location = 0,
    CurrentConditions,
    Crawl,
};

class Application
{
public:
    Application(const Configuration &config);
    ~Application() = default;

    void Run();

private:
    void OnEvent();
    void OnUpdate(float deltaSeconds);
    void OnRender();

    void AddProduct(const Product &product) { m_Products.push_back(product); }
    Product &CurrentProduct() { return m_Products.at(m_CurrentProduct); }
    void NextProduct();
    void RenderProduct();
    void ClearProducts();

    void AddLower(const Lower &product) { m_Lowers.push_back(product); }
    Lower &CurrentLower() { return m_Lowers.at(m_CurrentLower); }
    void NextLower();
    void RenderLower();
    void ClearLowers();

    void AddCrawl(uint32_t when, const std::string &text) { m_Crawls[when] = text; };
    std::string CurrentCrawl();
    void ClearCrawls();

private:
    Renderer m_Renderer;
    DrawOptions m_DrawOptions;
    Color m_BackgroundColor{60, 8, 127, 0};
    Color m_TextColor{238, 238, 238, 0};
    Color m_BulletinColor{136, 52, 7, 0};
    Color m_GreenColor{0, 200, 0, 0};
    Rect m_ProductViewport{40, 30, 0, 0};
    Rect m_LowerViewport{40, 0, 0, 100};

    std::vector<Product> m_Products;
    size_t m_CurrentProduct = 0;
    float m_ProductOffset = 0.0f;
    float m_ProductTime = 0.0f;

    std::vector<Lower> m_Lowers;
    size_t m_CurrentLower = 0;
    float m_LowerOffset = 0.0f;
    float m_LowerTime = 0.0f;

    // TODO(frosty): Use a different type for the key to indicate its meaning
    std::map<uint32_t, std::string> m_Crawls;

    bool m_Quit = false;
    bool m_GreenMode = false;
};

} // namespace Wx

#endif // WX_APPLICATION_HH
