#!/bin/bash
set -e

usage()
{
    echo "Now support rk3562/rk3566/rk3568/rk3588/rk3576/rv1103/rv1106."
    echo "Usage as: $0 rk3562"
}

if [ $# == 0 ]; then
    usage
    exit 0
fi
echo "Build linux demo for "$1

LIB_ARCH=aarch64
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
elif [ $1 == "rv1103" ]; then
  TARGET_SOC="rv1103"
  LIB_ARCH="armhf-uclibc"
elif [ $1 == "rv1106" ]; then
  TARGET_SOC="rv1106"
  LIB_ARCH="armhf-uclibc"
else 
    echo "$1 not recognize."
    usage
    exit 1
fi

if [[ -z ${GCC_COMPILER} ]];then
    if [[ ${TARGET_SOC} = "rv1106"  || ${TARGET_SOC} = "rv1103" ]];then
        echo "Please set GCC_COMPILER for $TARGET_SOC"
        echo "such as export GCC_COMPILER=~/opt/arm-rockchip830-linux-uclibcgnueabihf/bin/arm-rockchip830-linux-uclibcgnueabihf"
        exit
    else
        GCC_COMPILER=aarch64-linux-gnu
    fi
fi
echo "$GCC_COMPILER"
export CC=${GCC_COMPILER}-gcc
export CXX=${GCC_COMPILER}-g++

if command -v ${CC} >/dev/null 2>&1; then
    :
else
    echo "${CC} is not available"
    echo "Please set GCC_COMPILER for $TARGET_SOC"
    echo "such as export GCC_COMPILER=~/opt/arm-rockchip830-linux-uclibcgnueabihf/bin/arm-rockchip830-linux-uclibcgnueabihf"
    exit
fi

ROOT_PWD=$( cd "$( dirname $0 )" && cd -P "$( dirname "$SOURCE" )" && pwd )

# build
BUILD_DIR=${ROOT_PWD}/build/build_linux_${ARCH}


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


if [[ ! -d "${BUILD_DIR}" ]]; then
  mkdir -p ${BUILD_DIR}
fi

cd ${BUILD_DIR}
cmake ../.. \
    -DLIB_ARCH=${LIB_ARCH} \
    -DCMAKE_C_COMPILER=${GCC_COMPILER}-gcc \
    -DCMAKE_CXX_COMPILER=${GCC_COMPILER}-g++ \
    -DRKNPU2=${RKNPU2}
make -j4
make install
cd -


