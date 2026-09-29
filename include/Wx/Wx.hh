// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "Wx/Config.hh"

namespace Wx
{

struct Configuration
{
  std::uint32_t width = 800;
  std::uint32_t height = 600;
  std::string title = "wx";
  std::size_t fontSize = 24;
};

enum class Product
{
  CurrentConditions,
  RegionalObservations,
  Warning,
  Forecast,
  TravelCities,
};

enum class TextType
{
  Normal,
  Small,
};

enum class TextAlignment
{
  Left,
  Center,
  Right,
};

struct WeatherData
{
  std::string location;
  std::string currentConditions;
  std::string temperature;
  std::string humidity;
  std::string windChill;
  std::vector<std::string> warnings{};

  bool hasWarnings() const { return !warnings.empty(); }
};

struct Slide
{
  Product product;
  float durationSeconds;
  bool scroll;
  const std::string text;
};

struct Crawl
{
  std::string text;
  float durationSeconds;
  bool scroll;
};

} // namespace Wx
