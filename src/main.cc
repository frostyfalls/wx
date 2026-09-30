// SPDX-License-Identifier: GPL-3.0-only

#include "Wx/Application.hh"

int main()
{
  Wx::Application app({640, 480, "wx"});
  app.Run();
}
