#!/bin/bash

if test "$#" -lt 1
then
  echo -e "1. linux\n2. mingw\n\n> "
  read choice
else
  choice="$1"
fi

case $choice in
  "1" | "linux")
    docker pull openturns/archlinux-module
    docker run --rm -e MAKEFLAGS -e OPENTURNS_NUM_THREADS=2 -v `pwd`:/io openturns/archlinux-module /io/.ci_support/run_docker_linux.sh `id -u`:`id -g`
    ;;
  "2" | "mingw")
    docker pull openturns/archlinux-module-mingw
    docker run --rm -e MAKEFLAGS -e OPENTURNS_NUM_THREADS=2 -v `pwd`:/io openturns/archlinux-module-mingw /io/.ci_support/run_docker_mingw.sh
    ;;
  *)
    echo "sorry?"
    exit 1
    ;;
esac
