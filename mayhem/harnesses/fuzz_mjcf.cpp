// MJCF: MjcfParser::readWorld(Uri) — the public loader takes a URI, so the input is written to
// a scratch .xml file. Reaches dart/utils/mjcf/detail (bodies, geoms, joints, defaults,
// equality/weld attributes) and XmlHelpers fixed-size value parsing.
#include "tmpfile.hpp"

#include <stdexcept>

#include <dart/common/Uri.hpp>
#include <dart/simulation/World.hpp>
#include <dart/utils/mjcf/MjcfParser.hpp>

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
    mayhem::TmpFile f(data, size, ".xml");
    if (!f.ok())
      return 0;
    auto world = dart::utils::MjcfParser::readWorld(
        dart::common::Uri::createFromPath(f.path()));
    if (world)
      world->step();
  } catch (const std::runtime_error&) {
  } catch (const std::invalid_argument&) {
  } catch (const std::out_of_range&) {
  }
  return 0;
}
