#!/bin/sh

set -xe

cd /tmp
mkdir build && cd build

PREFIX=${PWD}/install
CXXFLAGS="-Wall -Wextra -D_GLIBCXX_ASSERTIONS" ${ARCH}-w64-mingw32-cmake \
  -DCMAKE_INSTALL_PREFIX=${PREFIX} \
  -DPYTHON_INCLUDE_DIR=${MINGW_PREFIX}/include/python${PYMAJMIN} \
  -DPYTHON_LIBRARY=${MINGW_PREFIX}/lib/libpython${PYMAJMIN}.dll.a \
  -DPYTHON_EXECUTABLE=/usr/bin/${ARCH}-w64-mingw32-python${PYMAJMIN}-bin \
  /io
make install
${ARCH}-w64-mingw32-strip --strip-unneeded ${PREFIX}/bin/*.dll ${PREFIX}/Lib/site-packages/*/*.pyd
echo lib/test ${PREFIX}/Lib/site-packages/* | xargs -n 1 cp ${PREFIX}/bin/*.dll
ctest -R pyinstallcheck --output-on-failure --timeout 100 --schedule-random ${MAKEFLAGS}
make tests
ctest -R cppcheck --output-on-failure --timeout 100 --schedule-random ${MAKEFLAGS}
