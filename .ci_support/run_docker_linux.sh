#!/bin/sh

set -xe

cd /tmp
cmake -DCMAKE_INSTALL_PREFIX=${HOME}/.local \
      -DCMAKE_UNITY_BUILD=ON \
      -DCMAKE_CXX_FLAGS="-Wall -Wextra -Wpedantic -Wshadow -Wsuggest-override -Werror -D_GLIBCXX_ASSERTIONS --coverage" \
      -DSWIG_COMPILE_FLAGS="-O1 -Wno-unused-parameter -Wno-shadow" \
      -DUSE_SPHINX=ON -DSPHINX_FLAGS="-W -T -j4" \
      -B build /io
cd build
make install
make tests
ctest --output-on-failure --timeout 100 ${MAKEFLAGS}

# coverage
gcov `find lib/src/ -name "*.gcno"`
lcov --capture --directory lib --output-file coverage.info --include "*.cxx" -j 4
genhtml --output-directory coverage coverage.info
cp -v coverage.info coverage

UID_GID=$1
if test -n "${UID_GID}"
then
  sudo chown -R ${UID_GID} ~/.local/share/doc/*/html
  sudo cp -pr ~/.local/share/doc/*/html /io
fi
