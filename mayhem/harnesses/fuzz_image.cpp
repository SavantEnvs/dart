// DART's hand-written GUI texture decoders (dart/gui/detail/image_decoders.cpp), compiled
// into the harness as upstream's test_GuiImageDecoders does (the Filament GUI itself is
// not built). DECODER selects loadBmpImage / loadTgaImage / loadPpmImage; the input is
// written to a scratch file with the matching extension.
#include "tmpfile.hpp"

#include <dart/gui/detail/image_decoders.hpp>

#ifndef DECODER
#  error "define DECODER as Bmp, Tga or Ppm"
#endif
#define MAYHEM_CAT2(a, b) a##b
#define MAYHEM_CAT(a, b) MAYHEM_CAT2(a, b)

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
  mayhem::TmpFile f(data, size, SUFFIX);
  if (!f.ok())
    return 0;
  auto img = dart::gui::detail::MAYHEM_CAT(MAYHEM_CAT(load, DECODER), Image)(f.path());
  (void)img;
  return 0;
}
