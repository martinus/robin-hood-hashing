#/bin/bash
set -ex

ROOTDIR="$(cd -P "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd )"

parallel -j12 --eta --halt now,fail=1 --shuf ${ROOTDIR}/src/scripts/build.sh <${ROOTDIR}/src/scripts/build_targets.ini
