// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <chrono>

#include "Wx/Renderer.hh"
#include "Wx/Wx.hh"

namespace Wx
{

enum class Product
{
  CurrentConditions,
};

enum class Lower
{
  Location,
  CurrentConditions,
  AdCrawl,
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
  Product &CurrentProduct() { return m_Products[m_CurrentProduct]; }
  void NextProduct();
  void ClearProducts();

  void AddLower(const Lower &product) { m_Lowers.push_back(product); }
  Lower &CurrentLower() { return m_Lowers[m_CurrentLower]; }
  void NextLower();
  void ClearLowers();

private:
  const Configuration &m_Config;
  Renderer m_Renderer;

  Color m_BackgroundColor{53, 4, 121, 0};
  Color m_TextColor{238, 238, 238, 0};
  Rect m_ProductViewport{40, 40, 0, 0};
  Rect m_LowerViewport{40, 0, 0, 100};

  std::vector<Product> m_Products;
  size_t m_CurrentProduct = 0;

  std::vector<Lower> m_Lowers;
  size_t m_CurrentLower = 0;
  float m_LowerTime = 0.0f;

  std::string m_CurrentAdCrawl;

  bool m_Running = true;
  bool m_ShowDateTime = true;
  bool m_Fullscreen = false;
  float m_ElapsedSecond = 1.0f;
  float m_LowerOffset = 0.0f;
  float m_DateTimeHeight = 0.0f;
  std::chrono::time_point<std::chrono::system_clock> m_CurrentTime;
  DrawOptions m_DrawOptions;
};

} // namespace Wx
