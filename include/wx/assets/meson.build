assets_dir = meson.project_source_root() / 'assets'
tools_dir = meson.project_source_root() / 'tools'

bin2h = files(tools_dir / 'bin2h.py')

foreach font : [
  assets_dir / 'star3000.ttf',
  assets_dir / 'star3000_small.ttf',
  assets_dir / 'starjr.ttf',
]
  sources += custom_target(
    font.split('/')[-1],
    input: font,
    output: '@BASENAME@.hh',
    command: [
      bin2h,
      '@INPUT@',
      '@OUTPUT@',
      '@BASENAME@_font',
    ],
  )
endforeach
