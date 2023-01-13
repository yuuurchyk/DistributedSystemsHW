ARG DEBIAN_FRONTEND=noninteractive
ARG UBUNTU_VERSION="22.04"

ARG ENABLE_CMAKE="false"
ARG CMAKE_VERSION_MAJOR="3"
ARG CMAKE_VERSION_MINOR="22"
ARG CMAKE_VERSION_PATCH="3"

ARG ENABLE_CLANG_FORMAT="false"
ARG CLANG_FORMAT_LLVM_VERSION_MAJOR="11"
ARG CLANG_FORMAT_LLVM_VERSION_MINOR="1"
ARG CLANG_FORMAT_LLVM_VERSION_PATCH="0"

ARG ENABLE_BOOST="false"
ARG BOOST_VERSION_MAJOR="1"
ARG BOOST_VERSION_MINOR="80"
ARG BOOST_VERSION_PATCH="0"

ARG ENABLE_TBB="false"
ARG TBB_VERSION_MAJOR="2021"
ARG TBB_VERSION_MINOR="6"
ARG TBB_VERSION_PATCH="0"

ARG ENABLE_NINJA="false"
ARG NINJA_VERSION_MAJOR="1"
ARG NINJA_VERSION_MINOR="11"
ARG NINJA_VERSION_PATCH="1"

ARG ENABLE_GTEST="false"
ARG GTEST_VERSION_MAJOR="1"
ARG GTEST_VERSION_MINOR="12"
ARG GTEST_VERSION_PATCH="1"

# TODO:
# * poco
# * cppcheck

FROM ubuntu:${UBUNTU_VERSION} AS impl_essentials

ARG DEBIAN_FRONTEND

RUN apt-get update
RUN apt-get install -y build-essential cmake wget unzip

# ----------------------
# cmake installation starts
# ----------------------
FROM impl_essentials AS cmake_true

ARG DEBIAN_FRONTEND
ARG CMAKE_VERSION_MAJOR
ARG CMAKE_VERSION_MINOR
ARG CMAKE_VERSION_PATCH
ARG CMAKE_VERSION="${CMAKE_VERSION_MAJOR}.${CMAKE_VERSION_MINOR}.${CMAKE_VERSION_PATCH}"
ARG CMAKE_ARCHIVE_BASENAME="cmake-${CMAKE_VERSION}"
ARG CMAKE_ARCHIVE_NAME="${CMAKE_ARCHIVE_BASENAME}.tar.gz"
ARG CMAKE_LINK="https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/${CMAKE_ARCHIVE_NAME}"

RUN apt-get install -y libssl-dev

RUN mkdir -p /tmp/cmake
RUN mkdir -p /install/cmake

WORKDIR /tmp/cmake
RUN wget ${CMAKE_LINK}
RUN tar -xf ./${CMAKE_ARCHIVE_NAME}
RUN rm -rf ./${CMAKE_ARCHIVE_NAME}
WORKDIR /tmp/cmake/${CMAKE_ARCHIVE_BASENAME}

RUN ./bootstrap --prefix=/install/cmake --parallel=$(nproc)
RUN make -j$(nproc)
RUN make install

FROM ubuntu:${UBUNTU_VERSION} AS cmake_false

RUN mkdir -p /install/cmake/bin

FROM cmake_${ENABLE_CMAKE} AS cmake
# ----------------------
# cmake installation ends
# ----------------------

# ----------------------
# clang-format installation starts
# ----------------------
FROM impl_essentials AS clang_format_true

ARG DEBIAN_FRONTEND
ARG CLANG_FORMAT_LLVM_VERSION_MAJOR
ARG CLANG_FORMAT_LLVM_VERSION_MINOR
ARG CLANG_FORMAT_LLVM_VERSION_PATCH
ARG CLANG_FORMAT_LLVM_VERSION="${CLANG_FORMAT_LLVM_VERSION_MAJOR}.${CLANG_FORMAT_LLVM_VERSION_MINOR}.${CLANG_FORMAT_LLVM_VERSION_PATCH}"
ARG LLVM_ARCHIVE_NAME="llvmorg-${CLANG_FORMAT_LLVM_VERSION}.zip"
ARG LLVM_LINK="https://github.com/llvm/llvm-project/archive/refs/tags/${LLVM_ARCHIVE_NAME}"

RUN apt-get install -y python3

RUN mkdir -p /tmp/llvm
RUN mkdir -p /install/clang_format/bin

WORKDIR /tmp/llvm
RUN wget ${LLVM_LINK}
RUN unzip ${LLVM_ARCHIVE_NAME}
RUN rm ${LLVM_ARCHIVE_NAME}
RUN mkdir build
RUN mkdir install
RUN cmake \
    -S llvm-project-llvmorg-${CLANG_FORMAT_LLVM_VERSION}/llvm -B build \
    -DCMAKE_INSTALL_PREFIX=/tmp/llvm/install -DCMAKE_BUILD_TYPE=Release \
    -DLLVM_BUILD_STATIC=ON -DLLVM_ENABLE_PROJECTS="clang" -DLLVM_ENABLE_RUNTIMES=""
RUN cmake --build build --target clang-format -- -j$(nproc)
RUN cp ./build/bin/clang-format /install/clang_format/bin

FROM ubuntu:${UBUNTU_VERSION} AS clang_format_false

RUN mkdir -p /install/clang_format/bin

FROM clang_format_${ENABLE_CLANG_FORMAT} AS clang_format
# ----------------------
# clang-format installation ends
# ----------------------

# ----------------------
# boost installation starts
# ----------------------
FROM impl_essentials AS boost_true

ARG DEBIAN_FRONTEND
ARG BOOST_VERSION_MAJOR
ARG BOOST_VERSION_MINOR
ARG BOOST_VERSION_PATCH
ARG BOOST_VERSION_UNDERSCORE="${BOOST_VERSION_MAJOR}_${BOOST_VERSION_MINOR}_${BOOST_VERSION_PATCH}"
ARG BOOST_VERSION_DOT="${BOOST_VERSION_MAJOR}.${BOOST_VERSION_MINOR}.${BOOST_VERSION_PATCH}"
ARG BOOST_ARCHIVE_BASENAME="boost_${BOOST_VERSION_UNDERSCORE}"
ARG BOOST_ARCHIVE_NAME="${BOOST_ARCHIVE_BASENAME}.tar.gz"
ARG BOOST_LINK="https://boostorg.jfrog.io/artifactory/main/release/${BOOST_VERSION_DOT}/source/${BOOST_ARCHIVE_NAME}"

RUN mkdir -p /tmp/boost
RUN mkdir -p /install/boost

WORKDIR /tmp/boost
RUN wget ${BOOST_LINK}
RUN tar -xf ./${BOOST_ARCHIVE_NAME}
RUN rm -rf ./${BOOST_ARCHIVE_NAME}
WORKDIR /tmp/boost/${BOOST_ARCHIVE_BASENAME}

RUN ./bootstrap.sh --prefix="/install/boost"
RUN ./b2 -j$(nproc) variant=release link=static,shared threading=multi runtime-link=shared
RUN ./b2 -j$(nproc) install

FROM ubuntu:${UBUNTU_VERSION} AS boost_false

RUN mkdir -p /install/boost/bin

FROM boost_${ENABLE_BOOST} AS boost
# ----------------------
# boost installation ends
# ----------------------

# ----------------------
# tbb installation starts
# ----------------------
FROM impl_essentials AS tbb_true

ARG DEBIAN_FRONTEND
ARG TBB_VERSION_MAJOR
ARG TBB_VERSION_MINOR
ARG TBB_VERSION_PATCH
ARG TBB_VERSION="${TBB_VERSION_MAJOR}.${TBB_VERSION_MINOR}.${TBB_VERSION_PATCH}"
ARG TBB_ARCHIVE_NAME="v${TBB_VERSION}.zip"
ARG TBB_LINK="https://github.com/oneapi-src/oneTBB/archive/refs/tags/${TBB_ARCHIVE_NAME}"
ARG TBB_EXTRACTED_FOLDER="oneTBB-${TBB_VERSION}"

RUN mkdir -p /tmp/tbb
RUN mkdir -p /install/tbb

WORKDIR /tmp/tbb
RUN wget ${TBB_LINK}
RUN unzip ${TBB_ARCHIVE_NAME}
RUN rm -rf ./${TBB_ARCHIVE_NAME}
WORKDIR /tmp/tbb/${TBB_EXTRACTED_FOLDER}

RUN cmake -S . -B build \
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/install/tbb -DTBB_TEST=OFF
RUN cmake --build build --target install -- -j$(nproc)

FROM ubuntu:${UBUNTU_VERSION} AS tbb_false

RUN mkdir -p /install/tbb/bin

FROM tbb_${ENABLE_TBB} AS tbb
# ----------------------
# tbb installation ends
# ----------------------

# ----------------------
# ninja installation starts
# ----------------------
FROM impl_essentials AS ninja_true

ARG DEBIAN_FRONTEND
ARG NINJA_VERSION_MAJOR
ARG NINJA_VERSION_MINOR
ARG NINJA_VERSION_PATCH
ARG NINJA_VERSION="${NINJA_VERSION_MAJOR}.${NINJA_VERSION_MINOR}.${NINJA_VERSION_PATCH}"
ARG NINJA_ARCHIVE_BASENAME="v${NINJA_VERSION}"
ARG NINJA_ARCHIVE_NAME="${NINJA_ARCHIVE_BASENAME}.zip"
ARG NINJA_LINK="https://github.com/ninja-build/ninja/archive/refs/tags/${NINJA_ARCHIVE_NAME}"

RUN mkdir -p /tmp/ninja
RUN mkdir -p /install/ninja

WORKDIR /tmp/ninja
RUN wget ${NINJA_LINK}
RUN unzip ${NINJA_ARCHIVE_NAME}
RUN rm -rf ./${NINJA_ARCHIVE_NAME}
WORKDIR /tmp/ninja/ninja-${NINJA_VERSION}

RUN ls
RUN mkdir build
RUN cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="/install/ninja"
RUN cmake --build build --target install -- -j$(nproc)

FROM ubuntu:${UBUNTU_VERSION} AS ninja_false

RUN mkdir -p /install/ninja/bin

FROM ninja_${ENABLE_NINJA} AS ninja
# ----------------------
# ninja installation ends
# ----------------------

# ----------------------
# gtest installation starts
# ----------------------
FROM impl_essentials AS gtest_true

ARG DEBIAN_FRONTEND
ARG GTEST_VERSION_MAJOR
ARG GTEST_VERSION_MINOR
ARG GTEST_VERSION_PATCH
ARG GTEST_VERSION="${GTEST_VERSION_MAJOR}.${GTEST_VERSION_MINOR}.${GTEST_VERSION_PATCH}"
ARG GTEST_ARCHIVE_NAME="release-${GTEST_VERSION}.tar.gz"
ARG GTEST_FOLDER="googletest-release-${GTEST_VERSION}"
ARG GTEST_LINK="https://github.com/google/googletest/archive/refs/tags/${GTEST_ARCHIVE_NAME}"

RUN mkdir -p /tmp/gtest
RUN mkdir -p /install/gtest

WORKDIR /tmp/gtest
RUN wget ${GTEST_LINK}
RUN tar -xf ${GTEST_ARCHIVE_NAME}
RUN rm -rf ./${GTEST_ARCHIVE_NAME}
WORKDIR /tmp/gtest/${GTEST_FOLDER}

RUN cmake -S . -B build \
    -DCMAKE_BUILD_TYPE=Release -DBUILD_GMOCK=OFF \
    -DCMAKE_INSTALL_PREFIX=/install/gtest
RUN cmake --build build --target install -- -j$(nproc)

FROM ubuntu:${UBUNTU_VERSION} AS gtest_false

RUN mkdir -p /install/gtest/bin

FROM gtest_${ENABLE_GTEST} AS gtest
# ----------------------
# gtest installation ends
# ----------------------

FROM ubuntu:${UBUNTU_VERSION} AS final

ARG DEBIAN_FRONTEND

RUN apt-get update
RUN apt-get install -y build-essential rsync

COPY --from=cmake /install/cmake /install/cmake
COPY --from=clang_format /install/clang_format /install/clang_format
COPY --from=boost /install/boost /install/boost
COPY --from=ninja /install/ninja /install/ninja
COPY --from=tbb /install/tbb /install/tbb
COPY --from=gtest /install/gtest /install/gtest
# TODO: add other packages

ENV LD_LIBRARY_PATH=/lib:/usr/lib:/usr/local/lib
RUN rsync -a /install/cmake/* /usr/local
RUN rsync -a /install/clang_format/* /usr/local
RUN rsync -a /install/boost/* /usr/local
RUN rsync -a /install/ninja/* /usr/local
RUN rsync -a /install/tbb/* /usr/local
RUN rsync -a /install/gtest/* /usr/local
# TODO: add other packages

RUN rm -rf ./install

# ----------------------
# terminal stages start
# ----------------------
FROM final AS development

ARG DEBIAN_FRONTEND

RUN apt-get install -y python3 python3-pip git nano bash-completion gdb
RUN pip3 install cmake-format

RUN touch /root/.bashrc
RUN echo "source /etc/bash_completion" >> /root/.bashrc

FROM final AS deploy

COPY . /app

WORKDIR /app
RUN cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
RUN cmake --build build --target install

WORKDIR /
RUN rm -rf /app
# ----------------------
# terminal stages end
# ----------------------
