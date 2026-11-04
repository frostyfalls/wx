// SPDX-License-Identifier: GPL-3.0-only

#ifndef WX_HH
#define WX_HH

#include <string>

#include "Wx/Config.hh"

namespace Wx {

struct Configuration
{
    size_t width, height;
    const std::string title;
};

} // namespace Wx

#endif // WX_HH
