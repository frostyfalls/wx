// SPDX-License-Identifier: GPL-3.0-only

#include "Wx/Application.hh"

int main()
{
  using namespace Wx;

  Configuration config{800, 600, "wx"};

  Application app(config);
  app.Run();
}
