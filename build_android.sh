#!/bin/bash

set -e

if [ -z ${ANDROID_NDK_PATH} ]
then
  ANDROID_NDK_PATH=~/opt/android-ndk-r17
fi

if [ -z ${RKNPU2} ]
then
  echo "RKNPU2 was not set."
  echo "Please set RKNPU2 via 'export RKNPU2={rknpu2_path}'" 
  echo "or put this folder under the rknpu2."
  RKNPU2=$(pwd | sed 's/\(rknpu2\).*/\1/g')

  STR_MUST_EXIST="rknpu2"
  if [[ $RKNPU2 != *$STR_MUST_EXIST* ]]
  then
    exit
  fi
fi


BUILD_TYPE=Release

usage()
{
    echo "Now support rk3562/rk3566/rk3568/rk3588/rk3576."
    echo "Usage as: $0 rk3562"
}

if [ $# == 0 ]; then
    usage
    exit 0
fi
echo "Build linux demo for "$1

if [ $1 == "rk3562" ]; then
  TARGET_SOC="rk356x"
elif [ $1 == "rk3566" ]; then
  TARGET_SOC="rk356x"
elif [ $1 == "rk3568" ]; then
  TARGET_SOC="rk356x"
elif [ $1 == "rk3588" ]; then
  TARGET_SOC="rk3588"
elif [ $1 == "rk3576" ]; then
  TARGET_SOC="rk3576"
else 
    echo "$1 not recognize."
    usage
    exit 1
fi
LIB_ARCH=arm64-v8a

ROOT_PWD=$( cd "$( dirname $0 )" && cd -P "$( dirname "$SOURCE" )" && pwd )

BUILD_DIR=${ROOT_PWD}/build/build_android_v8a

if [[ ! -d "${BUILD_DIR}" ]]; then
  mkdir -p ${BUILD_DIR}
fi

cd ${BUILD_DIR}
cmake ../.. \
        -DANDROID_TOOLCHAIN=clang \
        -DLIB_ARCH=${LIB_ARCH} \
       	-DCMAKE_SYSTEM_NAME=Android \
        -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_PATH/build/cmake/android.toolchain.cmake \
        -DANDROID_ABI="arm64-v8a" \
        -DANDROID_STL=c++_static \
        -DANDROID_PLATFORM=android-24 \
        -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
        -DRKNPU2=${RKNPU2} \
        -DCMAKE_C_FLAGS_DEBUG="-g -O1" \
        -DCMAKE_CXX_FLAGS_DEBUG="-g -O1"
make -j4
make install
cd ..

