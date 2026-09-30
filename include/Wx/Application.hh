// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <chrono>

#include "Wx/Wx.hh"
#include "Wx/Renderer.hh"

namespace Wx
{

enum class Product
{
  CurrentConditions,
};

class Application
{
public:
  Application(const Configuration &config);
  ~Application() = default;

  void Run();

  void AddProduct(const Product &product) { m_Products.push_back(product); }
  Product &CurrentProduct() { return m_Products[m_CurrentProduct]; }

private:
  void OnEvent();
  void OnUpdate(float deltaSeconds);
  void OnRender();

private:
  const Configuration &m_Config;
  Renderer m_Renderer;

  Color m_BackgroundColor{53, 4, 121, 0};
  Color m_TextColor{238, 238, 238, 0};
  Rect m_ProductViewport{40, 40, 0, 0};
  Rect m_CrawlViewport{40, 0, 0, 100};

  std::vector<Product> m_Products;
  size_t m_CurrentProduct;

  bool m_Running = true;
  bool m_ShowDateTime = true;
  bool m_Fullscreen = false;
  float m_ElapsedSecond = 1.0f;
  float m_CrawlOffset = 0.0f;
  float m_DateTimeHeight = 0.0f;
  std::chrono::time_point<std::chrono::system_clock> m_CurrentTime;
  DrawOptions m_DrawOptions;
};

} // namespace Wx
