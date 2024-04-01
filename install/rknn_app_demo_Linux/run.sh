ROOT_PWD=$( cd "$( dirname $0 )" && cd -P "$( dirname "$SOURCE" )" && pwd )
export LD_LIBRARY_PATH=${ROOT_PWD}/lib/
./rknn_app_demo supercombo.rknn ./data/inputs/in0.npy#./data/inputs/in1.npy#./data/inputs/in2.npy#./data/inputs/in3.npy#./data/inputs/in4.npy#./data/inputs/in5.npy#./data/inputs/in6.npy#./data/inputs/in7.npy#./data/inputs/in8.npy 10
