// SPDX-License-Identifier: GPL-3.0-only

#include "Wx/Application.hh"

int main()
{
  Wx::Application app({800, 600, "wx"});
  app.Run();
}
