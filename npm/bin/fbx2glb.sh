#!/bin/bash
#
# fbx2glb.sh <input FBX> <output GLB>
#
# TODO: Pass command line switches through to binary.

set -e

BINDIR=`dirname $0`
BINDIR=`cd ${BINDIR} ; pwd`

SYSTEM=`uname -s`
FBX2GLTF="${BINDIR}/${SYSTEM}/FBX2glTF"

if [ ! -f "${FBX2GLTF}" ]; then
    echo "Unable to find 'FBX2glTF' binary: ${FBX2GLTF}"
    exit 1
fi

if [ "$#" != 2 ]; then
    echo "Usage: <fbx input> <glb output>"
    exit 1
fi

fullpath() {
  OLDPWD=$PWD
  cd "$(dirname "$1")"
  FULLPATH="$PWD/$(basename "$1")"
  cd "$OLDPWD"
  echo "$FULLPATH"
}

INFILE=$(fullpath $1)
OUTFILE=$(fullpath $(basename $2 ".glb"))

# construct a safe work dir
SCRIPT_BASE=`basename $0`
TEMP_DIR=`mktemp -d "/tmp/${SCRIPT_BASE}.XXXX"`
trap "rm -rf ${TEMP_DIR}" EXIT
cd ${TEMP_DIR}

# some hard-coded defaults for now
"${FBX2GLTF}" --binary --flip-v --input "${INFILE}" --output "${OUTFILE}"
