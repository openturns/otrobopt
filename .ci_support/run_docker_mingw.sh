#!/bin/sh

set -xe

cd /tmp

PREFIX=${PWD}/install
CXXFLAGS="-Wall -Wextra -D_GLIBCXX_ASSERTIONS" ${ARCH}-w64-mingw32-cmake \
  -DCMAKE_INSTALL_PREFIX=${PREFIX} \
  -DCMAKE_UNITY_BUILD=ON \
  -DPython_INCLUDE_DIR=${MINGW_PREFIX}/include/python${PYMAJMIN} \
  -DPython_LIBRARY=${MINGW_PREFIX}/lib/libpython${PYMAJMIN}.dll.a \
  -DPython_EXECUTABLE=/usr/bin/${ARCH}-w64-mingw32-python${PYMAJMIN}-bin \
  -B build /io
cd build
make install
${ARCH}-w64-mingw32-strip --strip-unneeded ${PREFIX}/bin/*.dll ${PREFIX}/Lib/site-packages/*/*.pyd
echo lib/test ${PREFIX}/Lib/site-packages/* | xargs -n 1 cp ${PREFIX}/bin/*.dll
OPENTURNS_NUM_THREADS=2 ctest -R pyinstallcheck --output-on-failure --timeout 1000 --schedule-random ${MAKEFLAGS}
make tests
OPENTURNS_NUM_THREADS=2 ctest -R cppcheck --output-on-failure --timeout 1000 --schedule-random ${MAKEFLAGS}
