# syntax=docker/dockerfile:1.7
# Commit image for dartsim/dart (DART physics/robotics library): builds the sanitized
# dart + dart-io static libraries, the libFuzzer harnesses in mayhem/harnesses, and a
# separate normal-flags build of upstream's parser integration tests for mayhem/test.sh.
FROM ghcr.io/savantenvs/base:20261023

ARG COMMIT
ARG REPO_URL
ARG SOURCE_DATE_EPOCH
ARG SANITIZER_FLAGS
ARG COVERAGE_FLAGS=
ARG MAYHEM_JOBS=

# DART's build dependencies from apt (DART_USE_SYSTEM_DEPENDENCIES=ON, so nothing is fetched):
# Eigen, FCL/libccd/octomap (collision), assimp (meshes), tinyxml2 + urdfdom (model loaders),
# libpng/libjpeg (GUI image decoders compiled into the image harnesses), GoogleTest + google-benchmark (the test tree configures tests/benchmark unconditionally).
USER root
RUN [ -n "$SOURCE_DATE_EPOCH" ] && repro-sources-list.sh || true ; \
    apt-get update && apt-get install -yq --no-install-recommends \
      cmake ninja-build pkg-config \
      libeigen3-dev libfcl-dev libccd-dev liboctomap-dev libassimp-dev \
      libtinyxml2-dev liburdfdom-dev liburdfdom-headers-dev libconsole-bridge-dev \
      libpng-dev libjpeg-dev libgtest-dev libbenchmark-dev \
 && rm -rf /var/lib/apt/lists/*

USER mayhem
WORKDIR /mayhem
COPY --chown=2000:2000 . /mayhem

RUN if [ -x /opt/mayhem/ccache/prefill ]; then /opt/mayhem/ccache/prefill bash mayhem/build.sh; \
    else bash mayhem/build.sh; fi

RUN if [ -f mayhem/test.sh ]; then bash mayhem/test.sh; else echo "no mayhem/test.sh"; fi

LABEL org.opencontainers.image.source="${REPO_URL}" \
      org.opencontainers.image.revision="${COMMIT}" \
      rlenv.tier="commit" \
      rlenv.source_date_epoch="${SOURCE_DATE_EPOCH}" \
      rlenv.user="mayhem"

USER mayhem
WORKDIR /mayhem
