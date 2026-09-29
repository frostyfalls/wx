// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <chrono>

#include "Wx/Wx.hh"
#include "Wx/Renderer.hh"

namespace Wx
{

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

private:
  const Configuration &m_Config;
  Renderer m_Renderer;

  bool m_Running = true;
  bool m_ShowDateTime = true;

  std::chrono::time_point<std::chrono::system_clock> m_CurrentTime;

  Color m_BackgroundColor{53, 4, 121, 0};
  Color m_TextColor{238, 238, 238, 0};
};

} // namespace Wx
