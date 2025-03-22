#!/bin/bash

set -e

QT_INSTALL_PATH="/usr/lib/x86_64-linux-gnu/qt6"
X11_LIB_DIR="/usr/lib/x86_64-linux-gnu"
PG_LIB_DIR="/usr/lib/x86_64-linux-gnu"

BUILD_DIR="build"
PACKAGE_DIR="package"
 
rm -rf ${PACKAGE_DIR}
mkdir -p ${PACKAGE_DIR}/{bin,lib,3rdparty/onnxruntime,models}

echo "Copying executable file..."
cp -v ${BUILD_DIR}/bin/inf_qwq ${PACKAGE_DIR}/bin/

echo "Collecting dynamic dependencies..."
ldd ${BUILD_DIR}/bin/inf_qwq | awk '/=>/ {print $3}' | xargs -I{} cp -Lv {} ${PACKAGE_DIR}/lib/

echo "Collecting 3rdparty"
cp -rv 3rdparty/onnxruntime ${PACKAGE_DIR}/3rdparty



echo "Configuring Qt plugins..."
mkdir -p ${PACKAGE_DIR}/lib/qt6/plugins
cp -rv ${QT_INSTALL_PATH}/plugins/platforms ${PACKAGE_DIR}/lib/qt6/plugins
cp -rv ${QT_INSTALL_PATH}/plugins/sqldrivers ${PACKAGE_DIR}/lib/qt6/plugins

echo "Configuring X11 dependencies..."
cp -v ${X11_LIB_DIR}/libxcb.so.* ${PACKAGE_DIR}/lib/
cp -v ${X11_LIB_DIR}/libX11.so.6 ${PACKAGE_DIR}/lib/
cp -v ${X11_LIB_DIR}/libX11-xcb.so.1 ${PACKAGE_DIR}/lib/

echo "Configuring SQL dependencies..."
cp -v ${PG_LIB_DIR}/libpq.so.5 ${PACKAGE_DIR}/lib



