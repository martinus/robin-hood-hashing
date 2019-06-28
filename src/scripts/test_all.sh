#/bin/bash
set -e

ROOTDIR="$(cd -P "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd )"

function build() {
    ORIGINDIR=`pwd`

    COMPILER=$1
    CXX_STANDARD=$2
    DIRNAME=${COMPILER}_cxx${CXX_STANDARD}
    
    rm -Rf ${DIRNAME}
    mkdir -p ${DIRNAME}
    cd ${DIRNAME}

    CXX=`which ${COMPILER}` cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DRH_cxx_standard=${CXX_STANDARD} ${ROOTDIR}
    cmake --build .
    ./rh

    cd ${ORIGINDIR}
}

build "g++-4.9" "11"
build "g++-4.9" "14"

build "g++" "11"
build "g++" "14"
build "g++" "17"

build "clang++" "11"
build "clang++" "14"
build "clang++" "17"
build "clang++" "2a"
