#!/bin/bash

set -e # Exit if something fails

SOURCE_DIR="$(pwd)"
BUILD_DIR="${SOURCE_DIR}/build"

if [[ $1 ]]; then
    VERSION="$1"
else
    VERSION="dev"
fi

CMAKE_OPTIONS="-DSTATIC_BUILD=ON -DCMAKE_BUILD_TYPE=Release"

mkdir -p "${BUILD_DIR}"
for PLATFORM in lin32 lin64 win32 win64;
do
    echo "Building for ${PLATFORM}"
    rm -r -f "${BUILD_DIR}/${PLATFORM}"
    mkdir -p "${BUILD_DIR}/${PLATFORM}"
    cd "${BUILD_DIR}/${PLATFORM}"

    if [[ "${PLATFORM}" == win* ]]; then
        PLATFORM_OPTIONS="${CMAKE_OPTIONS} -DGCC_ABI_WORKAROUND=ON"
    else
        PLATFORM_OPTIONS="${CMAKE_OPTIONS}"
    fi

    TOOLCHAIN_FILE="${SOURCE_DIR}/cmake/toolchain-${PLATFORM}.cmake"
    cmake ${SOURCE_DIR} ${PLATFORM_OPTIONS} -DCMAKE_TOOLCHAIN_FILE="${TOOLCHAIN_FILE}"
    make -j "$(nproc)"
done

cd "${BUILD_DIR}"
rm -f "srtextool-${VERSION}-lin.tar.gz srtextool-${VERSION}-win.zip"
echo "Packaging Linux binaries"
tar -czf "srtextool-${VERSION}-lin.tar.gz" "lin32/srtextool" "lin64/srtextool"
echo "Packaging Windows binaries"
zip "srtextool-${VERSION}-win.zip" "win32/srtextool.exe" "win64/srtextool.exe"

