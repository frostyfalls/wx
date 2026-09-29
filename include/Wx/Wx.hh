// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <cstdint>
#include <string>

#include "Wx/Config.hh"

namespace Wx
{

struct Configuration
{
  std::size_t width, height;
  const std::string title;
};

} // namespace Wx
