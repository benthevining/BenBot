# ======================================================================================
#
# ░▒▓███████▓▒░░▒▓████████▓▒░▒▓███████▓▒░       ░▒▓███████▓▒░ ░▒▓██████▓▒░▒▓████████▓▒░
# ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
# ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
# ░▒▓███████▓▒░░▒▓██████▓▒░ ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓███████▓▒░░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
# ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
# ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░      ░▒▓█▓▒░░▒▓█▓▒░▒▓█▓▒░░▒▓█▓▒░ ░▒▓█▓▒░
# ░▒▓███████▓▒░░▒▓████████▓▒░▒▓█▓▒░░▒▓█▓▒░      ░▒▓███████▓▒░ ░▒▓██████▓▒░  ░▒▓█▓▒░
#
# ======================================================================================

# This Dockerfile expects the context to be the repo root

FROM lichessbotdevs/lichess-bot:2026.3.27.1

ARG CMAKE_VERSION=4.2.3
ARG CMAKE_PRESET=clang

RUN --mount=type=bind,source=.,target=BenBot,rw \
  # install dependencies \
  apt-get update && \
  apt-get install -y --no-install-recommends git ninja-build clang libc++-dev libc++abi-dev build-essential wget && \
  # install CMake \
  wget "https://github.com/Kitware/CMake/releases/download/v$CMAKE_VERSION/cmake-$CMAKE_VERSION-linux-$(uname -m).sh" -q -O /tmp/cmake-install.sh && \
  chmod u+x /tmp/cmake-install.sh && \
  mkdir /opt/cmake-$CMAKE_VERSION && \
  /tmp/cmake-install.sh --skip-license --prefix=/opt/cmake-$CMAKE_VERSION && \
  rm /tmp/cmake-install.sh && \
  ln -s /opt/cmake-$CMAKE_VERSION/bin/* /usr/local/bin && \
  # configure & build \
  cd BenBot && \
  cmake --preset $CMAKE_PRESET -D BENBOT_DOCS=OFF -D BENBOT_TESTS=OFF && \
  cmake --build Builds/$CMAKE_PRESET -j4 --target ben_bot --config Release && \
  # install \
  cd .. && \
  cmake --install BenBot/Builds/$CMAKE_PRESET --prefix deploy --component ben_bot --config Release && \
  # clean up \
  apt-get remove -y git cmake ninja-build clang libc++-dev libc++abi-dev && \
  rm -rf /var/lib/apt/lists

COPY config/lichess/config.yml config/config.yml
