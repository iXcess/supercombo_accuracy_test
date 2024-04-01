# RKNN app demo

## 1.简介

该 demo 由 rknn.codegen 接口生成，主要用于快速验证 RKNN 的 CAPI 推理。



## 2.支持情况

目前支持以下功能：

- 获取推理性能

- 获取推理结果，并保存为npy文件
  - 需要指定输入，可以是图片或npy文件。当输入为图片、且模型输入shape不匹配时，会自动做 resize 处理
  
- 若 rknn.build 时指定了 dataset、且dataset有效。则生成 rknn app demo时，会采用dataset的首行数据作为输出，通过 python api 的 rknn.inference 接口生成 golden 数据。板端执行demo时，会比对推理结果的余弦相似度

  **注意:** 由于 Python API 读取、缩放图片使用 opencv 库，而 rknn app demo 使用 stbi 读取、缩放图片，余弦相似度可能因此达不到0.9999



支持特性：

- 支持 RKNPU2 对应的所有平台
- 支持量化/浮点模型
- 支持多输入、多输出模型
- 默认使用零拷贝接口进行测试
- 支持图片输入，输入图片尺寸不匹配时，使用 stbi 库进行 letterbox resize
- 支持npy输入，目前只支持fp32的npy文件，npy数据shape应与原始模型一致。
- 支持校验推理结果的余弦相似度



## 2.编译

指定本地 rknpu2 仓库路径（可从https://github.com/airockchip/rknn-toolkit2/tree/master/rknpu2 获取）

```
export RKNPU2={rknpu2_path}
```

如果是编译安卓系统的demo，请指定  ANDROID_NDK_PATH 路径

```
export ANDROID_NDK_PATH={android_ndk_path}
```



编译

```
./build_android.sh {RKNPU_platform} 或 ./build_linux.sh {RKNPU_platform}
如
./build_android.sh rk3562
```



## 3.推送至板端并执行

```shell
adb push ./install/rknn_app_demo_Linux /data/			# 若板端为安卓系统，则目录应为 rknn_app_demo_Android
adb shell
cd /data/rknn_app_demo_Linux
export LD_LIBRARY_PATH=./lib/

执行指令有以下4种形式
执行自动生成的shell推理脚本
./run.sh

获取单次推理性能
./rknn_app_demo ./model.rknn							# 这里假设模型名字为 model.rknn，实际使用时请替换为自己 rknn 模型文件

获取多次推理性能
./rknn_app_demo ./model.rknn "" 100

指定输入(若 ./data/outputs_golden 存在，则推理后会计算推理结果的余弦相似度)
./rknn_app_demo ./model.rknn ./data/inputs/input0.jpg

指定输入、循环跑100次(若 ./data/outputs_golden 存在，则推理后会计算推理结果的余弦相似度)
./rknn_app_demo ./model.rknn ./data/inputs/input0.jpg 100
```

- **假设存在多输入 in0.npy, in1.npy，则推理指令为 ./RKNN_deploy_demo ./model.rknn ./data/inputs/in0.npy#./data/inputs/in1.npy**




## 4.打印信息与推理结果

执行demo后，可以看到以下信息

- 模型输入输出tensor的信息，包括tensor名、维度、量化类型、排布的等信息，具体含义可参考 https://github.com/airockchip/rknn-toolkit2/tree/master/doc 的 userguide 文档说明
- 模型推理时 rknn_run 的耗时
- (如果指定了循环跑次数) 循环跑n次的平均耗时
- 如果指定了输入，会打印 rknn 推理结果与原始模型推理结果的余弦距离

以 https://github.com/airockchip/rknn-toolkit2/tree/master/rknn-toolkit2/examples/functions/codegen 为例，demo的推理结果如下

```cpp
# 初始化模型将 verbose_log 设为 True
rknn_app_query_model_info: model input num: 1, output num: 1
INPUTS:
  [NORMAL]:  index=0, name=data, n_dims=4, dims=[1,224,224,3,], n_elems=150528, size=150528, w_stride=224, size_with_stride=150528, fmt=NHWC, type=INT8, qnt_type=AFFINE, zp=-13, scale=0.018317
  [NATIVE]:  index=0, name=data, n_dims=4, dims=[1,224,224,3,], n_elems=150528, size=150528, w_stride=224, size_with_stride=150528, fmt=NHWC, type=INT8, qnt_type=AFFINE, zp=-13, scale=0.018317
OUTPUTS:
  [NORMAL]:  index=0, name=prob, n_dims=4, dims=[1,1000,1,1,], n_elems=1000, size=2000, w_stride=0, size_with_stride=16000, fmt=NCHW, type=FP16, qnt_type=AFFINE, zp=0, scale=1.000000
  [NATIVE]:  index=0, name=prob, n_dims=5, dims=[1,125,1,1,8,], n_elems=1000, size=2000, w_stride=0, size_with_stride=2000, fmt=NC1HWC2, type=FP16, qnt_type=AFFINE, zp=0, scale=1.000000

LOAD INPUTS
  input[0] - IMG: data/inputs/dog_224x224.jpg

RUNNING RKNN
  loop[0] time: 5.474000 ms
Average time: 5.515000 ms

CHECK OUTPUT
  check all output to './data/outputs/runtime'
  with golden data in './data/outputs/golden'
  output[0] - prob:
    rknn_layout_convert: src->fmt=NC1HWC2(1,125,1,1,8,), dst->fmt=NCHW(1,1000,1,1,)
    rknn_dtype_convert: convert from FP16 to FP32
    cosine similarity: 0.998830
```

```cpp
# 初始化模型将 verbose_log 设为 False
LOAD INPUTS
  input[0] - IMG: data/inputs/dog_224x224.jpg

RUNNING RKNN
  loop[0] time: 4.927000 ms
Average time: 4.944000 ms

CHECK OUTPUT
  check all output to './data/outputs/runtime'
  with golden data in './data/outputs/golden'
  output[0] - prob:
    cosine similarity: 0.998830
```

