ARG DEBIAN_FRONTEND=noninteractive
ARG UBUNTU_VERSION="22.04"

ARG ENABLE_CMAKE="true"
ARG CMAKE_VERSION_MAJOR="3"
ARG CMAKE_VERSION_MINOR="22"
ARG CMAKE_VERSION_PATCH="3"

ARG ENABLE_CLANG_FORMAT="true"
ARG CLANG_FORMAT_LLVM_VERSION_MAJOR="11"
ARG CLANG_FORMAT_LLVM_VERSION_MINOR="1"
ARG CLANG_FORMAT_LLVM_VERSION_PATCH="0"

ARG ENABLE_BOOST="true"
ARG BOOST_VERSION_MAJOR="1"
ARG BOOST_VERSION_MINOR="80"
ARG BOOST_VERSION_PATCH="0"

ARG ENABLE_NINJA="true"
ARG NINJA_VERSION_MAJOR="1"
ARG NINJA_VERSION_MINOR="11"
ARG NINJA_VERSION_PATCH="1"

# ----------------------
# cmake installation starts
# ----------------------
FROM ubuntu:${UBUNTU_VERSION} as cmake_true

ARG DEBIAN_FRONTEND
ARG CMAKE_VERSION_MAJOR
ARG CMAKE_VERSION_MINOR
ARG CMAKE_VERSION_PATCH
ARG CMAKE_VERSION="${CMAKE_VERSION_MAJOR}.${CMAKE_VERSION_MINOR}.${CMAKE_VERSION_PATCH}"
ARG CMAKE_ARCHIVE_BASENAME="cmake-${CMAKE_VERSION}"
ARG CMAKE_ARCHIVE_NAME="${CMAKE_ARCHIVE_BASENAME}.tar.gz"
ARG CMAKE_LINK="https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/${CMAKE_ARCHIVE_NAME}"

RUN apt-get update
RUN apt-get install -y build-essential libssl-dev wget

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

RUN mkdir -p /install/cmake/

FROM cmake_${ENABLE_CMAKE} AS cmake
# ----------------------
# cmake installation ends
# ----------------------

# ----------------------
# clang-format installation starts
# ----------------------
FROM ubuntu:${UBUNTU_VERSION} AS clang_format_true

ARG DEBIAN_FRONTEND
ARG CLANG_FORMAT_LLVM_VERSION_MAJOR
ARG CLANG_FORMAT_LLVM_VERSION_MINOR
ARG CLANG_FORMAT_LLVM_VERSION_PATCH
ARG CLANG_FORMAT_LLVM_VERSION="${CLANG_FORMAT_LLVM_VERSION_MAJOR}.${CLANG_FORMAT_LLVM_VERSION_MINOR}.${CLANG_FORMAT_LLVM_VERSION_PATCH}"
ARG LLVM_ARCHIVE_NAME="llvmorg-${CLANG_FORMAT_LLVM_VERSION}.zip"
ARG LLVM_LINK="https://github.com/llvm/llvm-project/archive/refs/tags/${LLVM_ARCHIVE_NAME}"

RUN apt-get update
RUN apt-get install -y build-essential cmake python3 wget unzip

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

RUN mkdir -p /install/clang_format/

FROM clang_format_${ENABLE_CLANG_FORMAT} AS clang_format
# ----------------------
# clang-format installation ends
# ----------------------

# ----------------------
# boost installation starts
# ----------------------
FROM ubuntu:${UBUNTU_VERSION} AS boost_true

ARG DEBIAN_FRONTEND
ARG BOOST_VERSION_MAJOR
ARG BOOST_VERSION_MINOR
ARG BOOST_VERSION_PATCH
ARG BOOST_VERSION_UNDERSCORE="${BOOST_VERSION_MAJOR}_${BOOST_VERSION_MINOR}_${BOOST_VERSION_PATCH}"
ARG BOOST_VERSION_DOT="${BOOST_VERSION_MAJOR}.${BOOST_VERSION_MINOR}.${BOOST_VERSION_PATCH}"
ARG BOOST_ARCHIVE_BASENAME="boost_${BOOST_VERSION_UNDERSCORE}"
ARG BOOST_ARCHIVE_NAME="${BOOST_ARCHIVE_BASENAME}.tar.gz"
ARG BOOST_LINK="https://boostorg.jfrog.io/artifactory/main/release/${BOOST_VERSION_DOT}/source/${BOOST_ARCHIVE_NAME}"

RUN apt-get update
RUN apt-get install -y build-essential wget

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

RUN mkdir -p /install/boost

FROM boost_${ENABLE_BOOST} AS boost
# ----------------------
# boost installation ends
# ----------------------

# ----------------------
# ninja installation starts
# ----------------------
FROM ubuntu:${UBUNTU_VERSION} AS ninja_true

ARG DEBIAN_FRONTEND
ARG NINJA_VERSION_MAJOR
ARG NINJA_VERSION_MINOR
ARG NINJA_VERSION_PATCH
ARG NINJA_VERSION="${NINJA_VERSION_MAJOR}.${NINJA_VERSION_MINOR}.${NINJA_VERSION_PATCH}"
ARG NINJA_ARCHIVE_BASENAME="v${NINJA_VERSION}"
ARG NINJA_ARCHIVE_NAME="${NINJA_ARCHIVE_BASENAME}.zip"
ARG NINJA_LINK="https://github.com/ninja-build/ninja/archive/refs/tags/${NINJA_ARCHIVE_NAME}"

RUN apt-get update
RUN apt-get install -y build-essential cmake wget unzip

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

RUN mkdir -p /install/ninja

FROM ninja_${ENABLE_NINJA} AS ninja
# ----------------------
# ninja installation ends
# ----------------------

FROM ubuntu:${UBUNTU_VERSION} AS final

ARG DEBIAN_FRONTEND

RUN apt-get update
RUN apt-get install -y build-essential rsync

COPY --from=cmake /install/cmake /install/cmake
COPY --from=clang_format /install/clang_format /install/clang_format
COPY --from=boost /install/boost /install/boost
COPY --from=ninja /install/ninja /install/ninja
# TODO: add other packages

RUN rsync -a /install/cmake/* /usr/local
RUN rsync -a /install/clang_format/* /usr/local
RUN rsync -a /install/boost/* /usr/local
RUN rsync -a /install/ninja/* /usr/local
# TODO: add other packages

RUN rm -rf ./install

FROM final AS deploy

ARG DEBIAN_FRONTEND

RUN apt-get install -y python3 python3-pip
RUN pip3 install cmake-format
