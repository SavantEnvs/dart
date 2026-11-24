#!/usr/bin/env bash
# mayhem/build.sh — sanitized DART libraries + libFuzzer harnesses, and a normal-flags build of
# upstream's parser integration tests (run by mayhem/test.sh). Re-runnable and offline: every
# dependency is an apt package in the image (DART_USE_SYSTEM_DEPENDENCIES=ON, no FetchContent).
set -euo pipefail
[ -n "${SOURCE_DATE_EPOCH:-}" ] || unset SOURCE_DATE_EPOCH

: "${SANITIZER_FLAGS=-fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer}"
: "${DEBUG_FLAGS:=-g -gdwarf-3}"
: "${CC:=clang}" ; : "${CXX:=clang++}" ; : "${LIB_FUZZING_ENGINE:=-fsanitize=fuzzer}"
: "${MAYHEM_JOBS:=$(nproc)}"
: "${COVERAGE_FLAGS=}"
export CC CXX

cd "$SRC"

COMMON_CMAKE=(
  -G Ninja
  -DBUILD_SHARED_LIBS=OFF
  -DDART_BUILD_GUI=OFF
  -DDART_BUILD_EXAMPLES=OFF
  -DDART_BUILD_DARTPY=OFF
  -DDART_USE_SYSTEM_DEPENDENCIES=ON
  -DDART_USE_SYSTEM_GOOGLETEST=ON
  -DDART_TREAT_WARNINGS_AS_ERRORS=OFF
  -DFETCHCONTENT_FULLY_DISCONNECTED=ON
)

# 1) Sanitized + SanCov-instrumented DART (dart, dart-io and its loaders), DWARF 3.
#    Debug build type with -O1 so asserts stay on (no -DNDEBUG) and DEBUG_FLAGS decides -g.
FUZZ_FLAGS="$SANITIZER_FLAGS -fsanitize=fuzzer-no-link $DEBUG_FLAGS"
cmake -S . -B build/fuzz "${COMMON_CMAKE[@]}" \
  -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=OFF \
  -DCMAKE_C_FLAGS_DEBUG="-O1" -DCMAKE_CXX_FLAGS_DEBUG="-O1" \
  -DCMAKE_C_FLAGS="$FUZZ_FLAGS" -DCMAKE_CXX_FLAGS="$FUZZ_FLAGS"
cmake --build build/fuzz -j"$MAYHEM_JOBS"

# 2) Harnesses: each linked with libFuzzer and with the standalone run-once driver.
mapfile -t DART_LIBS < <(find build/fuzz/lib -maxdepth 1 -name 'libdart*.a' | sort)
SYS_LIBS=(-lfcl -lccd -loctomap -loctomath -lassimp -ltinyxml2
  -lurdfdom_world -lurdfdom_model -lurdfdom_sensor -lurdfdom_model_state -lconsole_bridge
  -lpthread -ldl -lm)
INC=(-I"$SRC" -I"$SRC/build/fuzz" -I/usr/include/eigen3 -I"$SRC/mayhem/harnesses")
mkdir -p build/mayhem-obj
$CXX $SANITIZER_FLAGS $DEBUG_FLAGS -c mayhem/lsan_off.cc -o build/mayhem-obj/lsan_off.o
$CC $SANITIZER_FLAGS $DEBUG_FLAGS -c "$STANDALONE_FUZZ_MAIN" -o build/mayhem-obj/standalone_main.o

link_harness() { # <name> <sources/flags...>
  local name="$1"; shift
  $CXX -std=c++17 $SANITIZER_FLAGS $DEBUG_FLAGS $LIB_FUZZING_ENGINE "${INC[@]}" "$@" \
    build/mayhem-obj/lsan_off.o -Wl,--start-group "${DART_LIBS[@]}" -Wl,--end-group "${SYS_LIBS[@]}" \
    -o "$SRC/$name"
  $CXX -std=c++17 $SANITIZER_FLAGS $DEBUG_FLAGS "${INC[@]}" "$@" \
    build/mayhem-obj/lsan_off.o build/mayhem-obj/standalone_main.o \
    -Wl,--start-group "${DART_LIBS[@]}" -Wl,--end-group "${SYS_LIBS[@]}" \
    -o "$SRC/$name-standalone"
}

for h in skel urdf mjcf vsk c3d motion; do
  link_harness "fuzz_$h" mayhem/harnesses/fuzz_$h.cpp
done
# Image decoders: image_decoders.cpp is compiled into the harness with the fuzz flags (+SanCov),
# exactly as upstream's test_GuiImageDecoders does; only libpng/libjpeg are needed.
for d in Bmp:.bmp Tga:.tga Ppm:.ppm; do
  dec="${d%%:*}"; suf="${d#*:}"; lc="$(echo "$dec" | tr 'A-Z' 'a-z')"
  link_harness "fuzz_$lc" -fsanitize=fuzzer-no-link -DDECODER="$dec" -DSUFFIX="\"$suf\"" \
    mayhem/harnesses/fuzz_image.cpp dart/gui/detail/image_decoders.cpp -lpng -ljpeg
done

# 3) Upstream's parser integration tests with the project's NORMAL flags (independent tree).
cmake -S . -B build/tests "${COMMON_CMAKE[@]}" \
  -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON \
  -DCMAKE_C_FLAGS="$COVERAGE_FLAGS" -DCMAKE_CXX_FLAGS="$COVERAGE_FLAGS"
cmake --build build/tests -j"$MAYHEM_JOBS" --target \
  test_SkelParser test_MjcfParser test_VskParser test_DartLoader test_FileInfoWorld
