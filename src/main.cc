// SPDX-License-Identifier: GPL-3.0-only

#include <print>

#include "wx/Application.hh"

int main()
{
  std::println("wx version {}", Wx::VERSION);

  Wx::Configuration config;
  config.fontSize = 32;

  Wx::Application app(config);
  app.Run();
}
