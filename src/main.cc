// SPDX-License-Identifier: GPL-3.0-only

#include "Wx/Application.hh"

int main()
{
  Wx::Configuration config{800, 600, "wx"};
  Wx::Application app(config);
  app.Run();
}
