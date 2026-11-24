// Scratch-file helper for harnesses whose public entry point only takes a path/URI.
// The file is created under $TMPDIR (fallback /tmp) with mkstemps, keeps the given
// suffix so extension-sniffing loaders engage, and is removed after each input.
#pragma once
#include <cstdint>
#include <cstdlib>
#include <string>
#include <unistd.h>

namespace mayhem {
class TmpFile
{
public:
  TmpFile(const uint8_t* data, size_t size, const char* suffix)
  {
    const char* dir = std::getenv("TMPDIR");
    if (dir == nullptr || *dir == '\0')
      dir = "/tmp";
    std::string tmpl = std::string(dir) + "/dartfuzz-XXXXXX" + suffix;
    int fd = mkstemps(&tmpl[0], static_cast<int>(std::string(suffix).size()));
    if (fd < 0)
      return;
    size_t off = 0;
    while (off < size) {
      ssize_t w = write(fd, data + off, size - off);
      if (w <= 0)
        break;
      off += static_cast<size_t>(w);
    }
    close(fd);
    if (off == size)
      mPath = tmpl;
    else
      unlink(tmpl.c_str());
  }
  ~TmpFile()
  {
    if (!mPath.empty())
      unlink(mPath.c_str());
  }
  bool ok() const { return !mPath.empty(); }
  const std::string& path() const { return mPath; }

private:
  std::string mPath;
};
} // namespace mayhem
