// SPDX-License-Identifier: GPL-3.0-only

#include <print>
#include <string_view>

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "wx/wx.hh"

namespace
{

struct Configuration
{
  uint32_t width;
  uint32_t height;
};

class Application
{
public:
  Application(const Configuration &config);
  ~Application();

  void Run();
private:
  const Configuration &m_Config;
  bool m_Quit = true;

  SDL_Window *m_Window = nullptr;
  SDL_Renderer *m_Renderer = nullptr;
};

Application::Application(const Configuration &config)
  : m_Config(config)
{
  SDL_Init(SDL_INIT_VIDEO);
  TTF_Init();

  m_Window = SDL_CreateWindow("wx", m_Config.width, m_Config.height, 0);
  m_Renderer = SDL_CreateRenderer(m_Window, nullptr);

  SDL_SetRenderVSync(m_Renderer, true);

  m_Quit = false;
}

void Application::Run()
{
  SDL_FRect rect{40, 40, 100, 100};

  while (!m_Quit)
  {
    SDL_Event e;
    while (SDL_PollEvent(&e))
    {
      if (e.type == SDL_EVENT_QUIT)
        m_Quit = true;
    }

    SDL_SetRenderDrawColor(m_Renderer, 20, 20, 20, 0);
    SDL_RenderClear(m_Renderer);

    SDL_SetRenderDrawColor(m_Renderer, 221, 221, 221, 0);
    SDL_RenderFillRect(m_Renderer, &rect);

    SDL_RenderPresent(m_Renderer);

    rect.x += 2;
  }
}

Application::~Application()
{
  SDL_DestroyRenderer(m_Renderer);
  m_Renderer = nullptr;

  SDL_DestroyWindow(m_Window);
  m_Window = nullptr;

  TTF_Quit();
  SDL_Quit();
}

} // namespace

int main()
{
  std::println("wx version {}", Wx::version);

  Configuration config{800, 600};
  Application app(config);
  app.Run();
}
