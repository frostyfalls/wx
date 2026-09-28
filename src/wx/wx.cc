// SPDX-License-Identifier: GPL-3.0-only

#include <format>
#include <string>

#include "wx/wx.hh"

#include "wx/assets/star3000.hh"
#include "wx/assets/starjr.hh"

namespace Wx
{

Application::Application(const Configuration &config)
  : m_Config(config)
{
  SDL_IOStream *fontData = nullptr;

  SDL_Init(SDL_INIT_VIDEO);
  TTF_Init();

  m_Window = SDL_CreateWindow(m_Config.title.c_str(), m_Config.width, m_Config.height, 0);
  m_Renderer = SDL_CreateRenderer(m_Window, nullptr);
  fontData = SDL_IOFromConstMem(star3000_font, sizeof(star3000_font));
  m_Font = TTF_OpenFontIO(fontData, true, m_Config.fontSize);

  SDL_SetRenderVSync(m_Renderer, true);

  m_Quit = false;
}

void Application::Run()
{
  SDL_Color bgColor{53, 4, 121, 0};
  SDL_Color fgColor{238, 238, 238, 0};

  SDL_Surface *surface = nullptr;
  SDL_Texture *texture = nullptr;
  SDL_FRect textRect;

  const std::string message = std::format("wx version {}", VERSION);

  while (!m_Quit)
  {
    SDL_Event e;
    while (SDL_PollEvent(&e))
    {
      if (e.type == SDL_EVENT_QUIT)
        m_Quit = true;
    }

    SDL_SetRenderDrawColor(m_Renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
    SDL_RenderClear(m_Renderer);

    surface = TTF_RenderText_Solid(m_Font, message.c_str(), 0, fgColor);
    texture = SDL_CreateTextureFromSurface(m_Renderer, surface);
    textRect = {20, 20, static_cast<float>(surface->w), static_cast<float>(surface->h)};
    SDL_RenderTexture(m_Renderer, texture, nullptr, &textRect);

    SDL_RenderPresent(m_Renderer);
  }
}

Application::~Application()
{
  TTF_CloseFont(m_Font);
  m_Font = nullptr;

  SDL_DestroyRenderer(m_Renderer);
  m_Renderer = nullptr;

  SDL_DestroyWindow(m_Window);
  m_Window = nullptr;

  TTF_Quit();
  SDL_Quit();
}

} // namespace Wx
