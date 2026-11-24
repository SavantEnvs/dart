// C3D motion capture binary: FileInfoC3D::loadFile -> loadC3DFile (header, parameter
// section, frame data, DEC float conversion), on a scratch .c3d file.
#include "tmpfile.hpp"

#include <dart/utils/FileInfoC3D.hpp>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
  mayhem::TmpFile f(data, size, ".c3d");
  if (!f.ok())
    return 0;
  dart::utils::FileInfoC3D info;
  info.loadFile(f.path().c_str());
  return 0;
}
