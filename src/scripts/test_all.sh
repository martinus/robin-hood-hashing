#/bin/bash
set -e

ROOTDIR="$(cd -P "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd )"

function build() {
    ORIGINDIR=$(pwd)

    COMPILER=$1
    CXX_STANDARD=$2
    SANITIZER=$3

    DIRNAME=${COMPILER}_cxx${CXX_STANDARD}_sanitizer${SANITIZER}
    
    rm -Rf ${DIRNAME}
    mkdir -p ${DIRNAME}
    cd ${DIRNAME}

    CXX=$(which ${COMPILER}) cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug -DRH_cxx_standard=${CXX_STANDARD} -DRH_sanitizer=${SANITIZER} ${ROOTDIR}
    cmake --build .

    ./rh -ns -ts=show
    ./rh

    cd ${ORIGINDIR}
}

build "g++-4.9" "11" "OFF"
build "g++-4.9" "14" "OFF"

build "g++-5" "11" "OFF"
build "g++-5" "14" "OFF"
build "g++-5" "17" "OFF"

build "g++-6" "11" "OFF"
build "g++-6" "14" "OFF"
build "g++-6" "17" "OFF"

build "g++-7" "11" "OFF"
build "g++-7" "14" "OFF"
build "g++-7" "17" "OFF"

build "g++" "11" "ON"
build "g++" "14" "ON"
build "g++" "17" "ON"
build "g++" "20" "ON"

build "clang++-6" "11" "OFF"
build "clang++-6" "14" "OFF"
build "clang++-6" "17" "OFF"
build "clang++-6" "20" "OFF"

build "clang++" "11" "ON"
build "clang++" "14" "ON"
build "clang++" "17" "ON"
build "clang++" "20" "ON"
