// DOF / World text motion files: FileInfoDof::loadFile and FileInfoWorld::loadFile.
// Byte 0 selects the parser (even: .dof against a fixed 3-DOF skeleton, odd: world
// recording); the rest of the input is the file content. A fresh skeleton is built per
// input so no state carries over.
#include "tmpfile.hpp"

#include <dart/dynamics/BodyNode.hpp>
#include <dart/dynamics/RevoluteJoint.hpp>
#include <dart/dynamics/Skeleton.hpp>
#include <dart/utils/FileInfoDof.hpp>
#include <dart/utils/FileInfoWorld.hpp>

using namespace dart::dynamics;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
  if (size < 1)
    return -1;
  const bool world = data[0] & 1;
  mayhem::TmpFile f(data + 1, size - 1, world ? ".txt" : ".dof");
  if (!f.ok())
    return 0;
  if (world) {
    dart::utils::FileInfoWorld info;
    info.loadFile(f.path().c_str());
  } else {
    SkeletonPtr skel = Skeleton::create("fuzz");
    BodyNode* parent = nullptr;
    for (int i = 0; i < 3; ++i)
      parent = skel->createJointAndBodyNodePair<RevoluteJoint>(parent).second;
    dart::utils::FileInfoDof info(skel.get());
    info.loadFile(f.path().c_str());
  }
  return 0;
}
