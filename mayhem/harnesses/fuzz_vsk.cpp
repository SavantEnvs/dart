// VSK (Vicon skeleton XML): VskParser::readSkeleton(Uri) on a scratch .vsk file —
// parameters, segments, joints, markers.
#include "tmpfile.hpp"

#include <stdexcept>

#include <dart/common/Uri.hpp>
#include <dart/dynamics/Skeleton.hpp>
#include <dart/utils/VskParser.hpp>

// dart::common::ResourceRetriever::readAll / Resource::readAll report an unreadable or missing
// resource (e.g. a <include>/world file reference the input names) by throwing
// std::runtime_error — that is the API's error contract, so it is caught here. XmlHelpers'
// fixed-size vector parsing (parseFixedSizeXmlVector, toDouble/std::stod) rejects a malformed
// attribute by logging DART_ERROR "value [...] is not a valid double" and rethrowing whatever
// std::stod threw: std::invalid_argument for a non-number, std::out_of_range for a number that
// overflows a double (e.g. 1e99999). Upstream's tests/unit/utils/test_XmlHelpers.cpp expects a
// std::exception there, so both are caught. Every other exception (length_error, bad_alloc...)
// still escapes and is reported; ASan still reports real out-of-bounds accesses either way.
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
  try {
    mayhem::TmpFile f(data, size, ".vsk");
    if (!f.ok())
      return 0;
    auto skel = dart::utils::VskParser::readSkeleton(
        dart::common::Uri::createFromPath(f.path()));
    (void)skel;
  } catch (const std::runtime_error&) {
  } catch (const std::invalid_argument&) {
  } catch (const std::out_of_range&) {
  }
  return 0;
}
