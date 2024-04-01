#ifndef _RK_DEPLOY_H
#define _RK_DEPLOY_H

#include "rknn_api.h"


typedef struct _rknn_app_buffer{
    void* virt_addr;
    int fd;
    int size;
    int offset;

    rknn_tensor_type dtype;

    bool has_create_rknn_mem;
    bool has_set_rknn_io;
} rknn_app_buffer;


typedef struct _rknn_app_context_t{
    char* m_path = nullptr;
    rknn_context ctx;
    // bool is_dyn_shape = false;

    int n_input;
    rknn_tensor_attr* in_attr = nullptr;
    rknn_tensor_attr* in_attr_native = nullptr;
    rknn_tensor_mem **input_mem;
    
    int n_output;
    rknn_tensor_attr* out_attr = nullptr;
    rknn_tensor_attr* out_attr_native = nullptr;
    rknn_tensor_mem **output_mem;

    bool verbose_log = false;

    // memory could be set outsider
    int init_flag = 0;
    rknn_input_range* dyn_range;

} rknn_app_context_t;


void dump_tensor_attr(rknn_tensor_attr *attr);

int init_rknn_app(rknn_app_context_t* rknn_app_ctx, const char* model_path, bool verbose_log);

int init_rknn_app_input_output_buffer(rknn_app_context_t* rknn_app_ctx, 
                                      rknn_app_buffer* inputs, 
                                      rknn_app_buffer* outputs, 
                                      bool from_fd);

int rknn_app_query_model_info(rknn_app_context_t* rknn_app_ctx);

int run_rknn_app(rknn_app_context_t* rknn_app_ctx, rknn_app_buffer* inputs, rknn_app_buffer* outputs);

int release_rknn_app(rknn_app_context_t* rknn_app_ctx);

int rknn_app_wrap_input_buffer(rknn_app_context_t* rknn_app_ctx, 
                               unsigned char* data, 
                               rknn_tensor_type data_type, 
                               rknn_app_buffer *buffer, 
                               int input_index);

unsigned char* rknn_app_unwrap_output_buffer(rknn_app_context_t* rknn_app_ctx, 
                                             rknn_app_buffer* buffer, 
                                             rknn_tensor_type dtype,
                                             int output_index);

#endif