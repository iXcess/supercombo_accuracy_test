ROOT_PWD=$( cd "$( dirname $0 )" && cd -P "$( dirname "$SOURCE" )" && pwd )
export LD_LIBRARY_PATH=${ROOT_PWD}/lib/
./rknn_app_demo supercombo.rknn
