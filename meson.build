# SPDX-License-Identifier: GPL-3.0-only

project(
  'wx',
  'cpp',
  version: '0.1.0',
  license: 'GPL-3.0-only',
  default_options: [
    'cpp_std=c++23',
    'warning_level=2',
  ],
)

configuration = configuration_data()
configuration.set_quoted('WX_VERSION', meson.project_version())

sdl3_dep = dependency('sdl3')
sdl3_ttf_dep = dependency('sdl3-ttf')

sources = []
dependencies = [sdl3_dep, sdl3_ttf_dep]
include = include_directories('include')

subdir('include')
subdir('src')
