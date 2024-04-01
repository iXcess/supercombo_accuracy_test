# RKNN app demo

## 1.Introduction

Generate by `rknn.codegen` interface, this demo is mainly used to quickly verify the CAPI inference of RKNN.



## 2.Support statement

Currently, the following functions are supported:

- Obtain inference performance

- Obtain inference results and save them as npy files
  - Input needs to be specified, which can be an image or npy file. When the input image does not match the model's input shape, automatic resizing will be performed.
  
- If the dataset is specified during "**rknn.build**" and the dataset is valid, the first row of the dataset will be used as the output when generating the rknn app demo. The golden data will be generated through the "**rknn.inference**" interface of the Python API. When the demo is executed on the board, the cosine simularity of the inference results will be compared.

  **Notice:** Due to Python API read/resize image via “opencv”, while rknn app demo use "stbi", this may cause cosine simularity could lower than 0.9999



Feature List:

- Supports all RKNPU2 platforms
- Support quantized/float model
-  Support multi-input and multi-output models
- Use zero-copy CAPI for testing
- Supports image input. When the input image size does not match model's input size, use the stbi library for letterbox resize.
- Support npy input, currently only fp32 npy files are supported
- Supports cosine similarity for verifying inference results



## 2.Compilation

Specify the local rknpu2 repository path (available at https://github.com/airockchip/rknn-toolkit2/tree/master/rknpu2)

```
export RKNPU2={rknpu2_path}
```

If compiling the demo for Android system, please specify the ANDROID_NDK_PATH path

```
export ANDROID_NDK_PATH={android_ndk_path}
```



Compile

```
./build_android.sh {RKNPU_platform} or ./build_linux.sh {RKNPU_platform} 

e.g.
./build_android.sh rk3562
```



## 3.Push to device and execute

```shell
adb push ./install/rknn_app_demo_Linux /data/		# For Android dev-board, folder name is "rknn_app_demo_Android"
adb shell
cd /data/rknn_deploy_demo_Linux
export LD_LIBRARY_PATH=./lib/

#There are four ways to execute the command:
./run.sh

#obtain performance
./RKNN_deploy_demo ./model.rknn						# Here we assume the model name is "model.rknn"

#obtain average performace for N times
./RKNN_deploy_demo ./model.rknn "" 100

#Specify input (if "./data/outputs_golden" exists, it will calculate the cosine similarity of the inference results after inference)
./RKNN_deploy_demo ./model.rknn ./data/inputs/cat.jpg

#Specify input, and run it for 100 times (if "./data/outputs_golden" exists, it will calculate the cosine similarity of the inference results after inference)
./RKNN_deploy_demo ./model.rknn ./data/inputs/cat.jpg 100
```

- If the model has multiple inputs, let's suppose the inputs are named as in0.npy and in1.npy. The command to be followed is: **"./RKNN_deploy_demo ./model.rknn ./data/inputs/in0.npy#./data/inputs/in1.npy"**.




## 4.Demo result

After executing the demo, the following information can be seen:

- Information about the model's input and output tensors, including tensor names, dimensions, quantization types, layout, and other details. The specific meanings can be found in the user guide documentation at [https://github.com/airockchip/rknn-toolkit2/tree/master/doc. ↗](https://github.com/airockchip/rknn-toolkit2/tree/master/doc.)
- The time taken for rknn_run.
- (If a loop count is specified) the average time taken for n times inference.
- If input is specified, the cosine distance between the rknn inference result and the original model inference result will be printed.

Take https://github.com/airockchip/rknn-toolkit2/tree/master/rknn-toolkit2/examples/functions/codegen  as the example, cpp_gen_demo result is as followed:

```cpp
# set verbose_log True while init rknn_app_ctx
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
# set verbose_log False while init rknn_app_ctx
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

