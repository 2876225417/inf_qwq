#!/bin/bash


BUILD_DIR="build"
BUILD_TYPE="Release"
CLAEN_BUILD=false
ENABLE_GPU="OFF"
ENABLE_EIGEN="OFF"
INSTALL_AFTER_BUILD=false

while [[ $# -gt 0 ]]; do
    case $1 in
        --debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        --clean)
            CLEAN_BUILD=true
            shift
            ;;
        --enable-gpu)
            ENABLE_GPU="ON"
            shift
            ;;
        --enable-eigen)
            ENABLE_EIGNE="ON"
            shift
            ;;
        --install)
            INSTALL_AFTER_BUILD=true
            shift
            ;;
        *)
            ehco "Unknown parameters: $1"
            exit 1
            ;;
    esac
done

if [ "$CLEAN_BUILD" = true]; then
    ehco "➤ Clean build dir..."
    rm -rf ${BUILD_DIR} && mkdir -p ${BUILD_DIR}
fi

if [ ! -d "${BUILD_DIR}" ]; then
    mkdir -p ${BUILD_DIR}
fi

echo "➤ Configure CMake (${BUILD_TYPE})..."
cmake -S . -B ${BUILD_DIR} \
      -DCMAKE_BUILD_TYPE=${BUILD_TYPE}
      -DEANBLE_GPU=${ENABLE_GPU}
      -DENABLE_EIGEN=${ENABLE_EIGEN}
      -DCMAKE_EXPORT_COMPILE_COMMONDS=ON

if [ $? -ne 0 ]; then
    ehco "❌ CMake Configure failed."
    exit 1
fi

echo "➤ Building"
cmake --build ${BUILD_DIR} -j${nproc}

if [ $? -ne 0 ]; then
    echo "❌ Building interrupted"
    exit 1
fi

if [ "$INSTALL_AFTER_BUILD" = true ]; then
    echo "➤ Installing..."
    sudo make --install ${BUILD_DIR}
fi

echo "➤ Building successfully. Generated files -> ${BUILD_DIR}/bin"

