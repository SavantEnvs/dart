// URDF: DartLoader::parseSkeletonString / parseWorldString (urdfdom model + DART's own
// URDF world parser and joint/shape translation). A document containing "<world" goes to
// the world parser, everything else to the skeleton parser.
#include <dart/common/Uri.hpp>
#include <dart/simulation/World.hpp>
#include <dart/utils/urdf/DartLoader.hpp>

#include <cstdint>
#include <stdexcept>
#include <string>

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
    const std::string xml(reinterpret_cast<const char*>(data), size);
    const dart::common::Uri base("file:///tmp/");
    dart::utils::DartLoader loader;
    if (xml.find("<world") != std::string::npos) {
      auto world = loader.parseWorldString(xml, base);
      (void)world;
    } else {
      auto skel = loader.parseSkeletonString(xml, base);
      (void)skel;
    }
  } catch (const std::runtime_error&) {
  } catch (const std::invalid_argument&) {
  } catch (const std::out_of_range&) {
  }
  return 0;
}
