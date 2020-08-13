#/bin/bash
set -ex

ROOTDIR="$(cd -P "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd )"

function buildAndTest() {
    if [[ $# -ne 3 ]]
    then
        echo "not correct number of args, doing nothing"
        return
    fi

    #NICE="nice -n20"
    NICE="schedtool -5 -e"

    ORIGINDIR=$(pwd)

    COMPILER=$1
    CXX_STANDARD=$2
    SANITIZER=$3
    CXXFLAGS=$4

    if [[ ${COMPILER:0:1} == "#" ]]
    then
        echo "commented out, doing nothing"
        return
    fi



    DIRNAME=${COMPILER}_cxx${CXX_STANDARD}_sanitizer${SANITIZER}_${CXXFLAGS}
    
    mkdir -p ${DIRNAME}
    rm -f ${DIRNAME}/CMakeCache.txt
    cd ${DIRNAME}

    CXX=$(which ${COMPILER}) cmake -G Ninja -DCMAKE_CXX_FLAGS=${CXXFLAGS} -DCMAKE_BUILD_TYPE=Debug -DRH_cxx_standard=${CXX_STANDARD} -DRH_sanitizer=${SANITIZER} ${ROOTDIR}
    ${NICE} cmake --build . -- -j 4
    ${NICE} ./rh -ns -ts=show
    ${NICE} ./rh -ns -ts=nanobench
    ${NICE} ./rh

    cd ${ORIGINDIR}
}

buildAndTest $*
