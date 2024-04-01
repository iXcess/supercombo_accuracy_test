
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>

#include "rknn_api.h"


#include "rknn_app_layout_cvt.h"
#include "rknn_app_dtype_cvt.h"
#include "rknn_app.h"


void check_ret(int ret, const char* msg){
    if (ret < 0){
        printf("ERROR: %s failed! ret=%d\n", msg, ret);
        exit(-1);
    }
}


void dump_tensor_attr(rknn_tensor_attr *attr)
{
    char dims_str[100];
    memset(dims_str, 0, sizeof(dims_str));
    for (int i = 0; i < attr->n_dims; i++) {
        sprintf(dims_str, "%s%d,", dims_str, attr->dims[i]);
    }

    printf("  index=%d, name=%s, n_dims=%d, dims=[%s], n_elems=%d, size=%d, w_stride=%d, size_with_stride=%d, fmt=%s, type=%s, qnt_type=%s, "
           "zp=%d, scale=%f\n",
           attr->index, attr->name, attr->n_dims, dims_str,
           attr->n_elems, attr->size, attr->w_stride, attr->size_with_stride, get_format_string(attr->fmt), get_type_string(attr->type),
           get_qnt_type_string(attr->qnt_type), attr->zp, attr->scale);
}


int init_rknn_app(rknn_app_context_t* rknn_app_ctx, const char* model_path, bool verbose_log){
    int ret = 0;
    rknn_app_ctx->m_path = (char *)model_path;
    rknn_app_ctx->verbose_log = verbose_log;
    ret = rknn_init(&(rknn_app_ctx->ctx), (void *)rknn_app_ctx->m_path, 0, rknn_app_ctx->init_flag, NULL);
    check_ret(ret, "rknn_init");

    ret = rknn_app_query_model_info(rknn_app_ctx);
    return ret;
}


int rknn_app_query_model_info(rknn_app_context_t* rknn_app_ctx){
    if (rknn_app_ctx->verbose_log==true) { printf("rknn_app_query_model_info: ");}
    int ret=0;

    rknn_input_output_num io_num;
    ret = rknn_query(rknn_app_ctx->ctx, RKNN_QUERY_IN_OUT_NUM, &io_num, sizeof(io_num));
    if (ret != RKNN_SUCC)
    {
        printf("rknn_query fail! ret=%d\n", ret);
        return -1;
    }
    if (rknn_app_ctx->verbose_log==true) { printf("model input num: %d, output num: %d\n", io_num.n_input, io_num.n_output); }
    rknn_app_ctx->n_input = io_num.n_input;
    rknn_app_ctx->n_output = io_num.n_output;

    rknn_app_ctx->in_attr = (rknn_tensor_attr*)malloc(sizeof(rknn_tensor_attr) * rknn_app_ctx->n_input);
    rknn_app_ctx->in_attr_native = (rknn_tensor_attr*)malloc(sizeof(rknn_tensor_attr) * rknn_app_ctx->n_input);
    rknn_app_ctx->input_mem = (rknn_tensor_mem**)malloc(sizeof(rknn_tensor_mem*) * rknn_app_ctx->n_input);
    
    rknn_app_ctx->out_attr = (rknn_tensor_attr*)malloc(sizeof(rknn_tensor_attr) * rknn_app_ctx->n_output);
    rknn_app_ctx->out_attr_native = (rknn_tensor_attr*)malloc(sizeof(rknn_tensor_attr) * rknn_app_ctx->n_output);
    rknn_app_ctx->output_mem = (rknn_tensor_mem**)malloc(sizeof(rknn_tensor_mem*) * rknn_app_ctx->n_output);


    if (rknn_app_ctx->verbose_log==true) { printf("INPUTS:\n"); }
    for (int i = 0; i < rknn_app_ctx->n_input; i++)
    {
        rknn_app_ctx->in_attr[i].index = i;
        ret = rknn_query(rknn_app_ctx->ctx, RKNN_QUERY_INPUT_ATTR, &(rknn_app_ctx->in_attr[i]), sizeof(rknn_tensor_attr));
        if (ret != RKNN_SUCC)
        {
            printf("rknn_query fail! ret=%d\n", ret);
            return -1;
        }
        if (rknn_app_ctx->verbose_log==true) {printf("  [NORMAL]:"); dump_tensor_attr(&rknn_app_ctx->in_attr[i]);}

        rknn_app_ctx->in_attr_native[i].index = i;
        ret = rknn_query(rknn_app_ctx->ctx, RKNN_QUERY_NATIVE_INPUT_ATTR, &(rknn_app_ctx->in_attr_native[i]), sizeof(rknn_tensor_attr));
        if (ret != RKNN_SUCC)
        {
            printf("rknn_query fail! ret=%d\n", ret);
            return -1;
        }
        if (rknn_app_ctx->verbose_log==true) { 
            printf("  [NATIVE]:");
            dump_tensor_attr(&rknn_app_ctx->in_attr_native[i]);
        }

    }

    if (rknn_app_ctx->verbose_log==true) { printf("OUTPUTS:\n");}
    for (int i = 0; i < rknn_app_ctx->n_output; i++)
    {
        rknn_app_ctx->out_attr[i].index = i;
        ret = rknn_query(rknn_app_ctx->ctx, RKNN_QUERY_OUTPUT_ATTR, &(rknn_app_ctx->out_attr[i]), sizeof(rknn_tensor_attr));
        if (ret != RKNN_SUCC)
        {
            printf("rknn_query fail! ret=%d\n", ret);
            return -1;
        }
        if (rknn_app_ctx->verbose_log==true) {printf("  [NORMAL]:");  dump_tensor_attr(&rknn_app_ctx->out_attr[i]); }

        rknn_app_ctx->out_attr_native[i].index = i;
        ret = rknn_query(rknn_app_ctx->ctx, RKNN_QUERY_NATIVE_OUTPUT_ATTR, &(rknn_app_ctx->out_attr_native[i]), sizeof(rknn_tensor_attr));
        if (ret != RKNN_SUCC)
        {
            printf("rknn_query fail! ret=%d\n", ret);
            return -1;
        }
        if (rknn_app_ctx->verbose_log==true) {printf("  [NATIVE]:");   dump_tensor_attr(&rknn_app_ctx->out_attr_native[i]);}
    }
    return 0;
}




int run_rknn_app(rknn_app_context_t* rknn_app_ctx, rknn_app_buffer* inputs, rknn_app_buffer* outputs){
    int ret = 0;

    for (int i=0; i<rknn_app_ctx->n_input; i++){
        // set io mem
        if (!inputs[i].has_set_rknn_io){
            if (inputs[i].dtype == RKNN_TENSOR_UINT8){
                rknn_app_ctx->in_attr_native[i].type = RKNN_TENSOR_UINT8;
                rknn_app_ctx->in_attr_native[i].pass_through = 0;
            } else {
                rknn_app_ctx->in_attr_native[i].pass_through = 1;            
            }
            ret = rknn_set_io_mem(rknn_app_ctx->ctx, rknn_app_ctx->input_mem[i], &(rknn_app_ctx->in_attr_native[i]));
            check_ret(ret, "rknn_set_io_mem");

            inputs[i].has_set_rknn_io = true;
        }
    }

    for (int i=0; i<rknn_app_ctx->n_output; i++){
        // set io mem
        if (!outputs[i].has_set_rknn_io){
            ret = rknn_set_io_mem(rknn_app_ctx->ctx, rknn_app_ctx->output_mem[i], &(rknn_app_ctx->out_attr_native[i]));
            check_ret(ret, "rknn_set_io_mem");

            outputs[i].has_set_rknn_io = true;
        }
    }

    ret = rknn_run(rknn_app_ctx->ctx, NULL);
    check_ret(ret, "rknn_run");
    return ret;
}


int release_rknn_app(rknn_app_context_t* rknn_app_ctx){
    for (int i = 0; i < rknn_app_ctx->n_input; i++){
        rknn_destroy_mem(rknn_app_ctx->ctx, rknn_app_ctx->input_mem[i]);
    }

    for (int i = 0; i < rknn_app_ctx->n_output; i++){
        rknn_destroy_mem(rknn_app_ctx->ctx, rknn_app_ctx->output_mem[i]);
    }

    if (rknn_app_ctx->ctx>0){
        rknn_destroy(rknn_app_ctx->ctx);
    }

    free(rknn_app_ctx->in_attr);
    free(rknn_app_ctx->in_attr_native);
    free(rknn_app_ctx->input_mem);

    free(rknn_app_ctx->out_attr);
    free(rknn_app_ctx->out_attr_native);
    free(rknn_app_ctx->output_mem);

    return 0;
}


int rknn_app_wrap_input_buffer(rknn_app_context_t* rknn_app_ctx, unsigned char* data, rknn_tensor_type data_type, rknn_app_buffer *buffer, int input_index){
    int ret = 0;
    
    if (buffer->fd == 0){
        printf("ERROR: wrap_input_buffer failed! Please init buffer first\n");
        exit(-1);
    }

    rknn_tensor_attr* attr = &(rknn_app_ctx->in_attr[input_index]);
    rknn_tensor_attr* native_attr = &(rknn_app_ctx->in_attr_native[input_index]);

    if (data_type == RKNN_TENSOR_UINT8){
        if (native_attr->type != RKNN_TENSOR_INT8) {
            printf("ERROR: UINT8 data only available for INT8 input tensor. But input tensor type is %s\n", get_type_string(native_attr->type));
            exit(-1);
        }
        memcpy(buffer->virt_addr, data, native_attr->size_with_stride);
        buffer->dtype = RKNN_TENSOR_UINT8;
        return 0;
    }

    unsigned char* temp_data = (unsigned char*)malloc(attr->n_elems* get_type_size(attr->type));
    ret = rknn_app_dtype_convert((unsigned char*)data, 
                                data_type,
                                temp_data,
                                attr->type, 
                                attr->n_elems, 
                                attr->scale, 
                                attr->zp,
                                rknn_app_ctx->verbose_log);
    check_ret(ret, "rknn_app_dtype_convert");

    if (attr->fmt == RKNN_TENSOR_NHWC){
        rknn_tensor_attr temp_attr;
        temp_attr.fmt = RKNN_TENSOR_NCHW;
        temp_attr.n_dims = 4;
        temp_attr.dims[0] = attr->dims[0];
        temp_attr.dims[1] = attr->dims[3];
        temp_attr.dims[2] = attr->dims[1];
        temp_attr.dims[3] = attr->dims[2];

        ret = rknn_app_layout_convert((unsigned char*)temp_data, &temp_attr, (unsigned char*)buffer->virt_addr, native_attr, get_type_size(attr->type), rknn_app_ctx->verbose_log);
    }
    else{
        ret = rknn_app_layout_convert(temp_data, 
                                    attr,
                                    (unsigned char*)buffer->virt_addr,
                                    native_attr,
                                    get_type_size(attr->type),
                                    rknn_app_ctx->verbose_log);
    }

    check_ret(ret, "rknn_app_layout_convert");
    free(temp_data);

    return ret;
}

unsigned char* rknn_app_unwrap_output_buffer(rknn_app_context_t* rknn_app_ctx, rknn_app_buffer* buffer, rknn_tensor_type dtype, int output_index){
    int ret = 0;
    unsigned char* temp_data = (unsigned char*)malloc(rknn_app_ctx->out_attr_native[output_index].size_with_stride);
    ret = rknn_app_layout_convert((unsigned char*)buffer->virt_addr, 
                                &(rknn_app_ctx->out_attr_native[output_index]),
                                temp_data,
                                &(rknn_app_ctx->out_attr[output_index]),
                                get_type_size(rknn_app_ctx->out_attr[output_index].type),
                                rknn_app_ctx->verbose_log);
    check_ret(ret, "rknn_app_layout_convert");

    unsigned char* output_data = (unsigned char*)malloc(rknn_app_ctx->out_attr[output_index].n_elems* get_type_size(RKNN_TENSOR_FLOAT32));
    ret = rknn_app_dtype_convert(temp_data, 
                               rknn_app_ctx->out_attr_native[output_index].type,
                               output_data,
                               dtype, 
                               rknn_app_ctx->out_attr[output_index].n_elems, 
                               rknn_app_ctx->out_attr[output_index].scale, 
                               rknn_app_ctx->out_attr[output_index].zp,
                               rknn_app_ctx->verbose_log);
    check_ret(ret, "rknn_app_dtype_convert");
    free(temp_data);

    return output_data;
}


int init_rknn_app_input_output_buffer(rknn_app_context_t *rknn_app_ctx, rknn_app_buffer *input_buffer, rknn_app_buffer *output_buffer, bool from_fd){
    int ret = 0;

    for (int idx=0; idx< rknn_app_ctx->n_input; idx++){
        if (from_fd == true){
            rknn_app_ctx->input_mem[idx] = rknn_create_mem_from_fd(rknn_app_ctx->ctx, 
                                                                input_buffer[idx].fd, 
                                                                input_buffer[idx].virt_addr, 
                                                                rknn_app_ctx->in_attr_native[idx].size_with_stride,
                                                                input_buffer[idx].offset);
        } else {
            rknn_app_ctx->input_mem[idx] = rknn_create_mem(rknn_app_ctx->ctx, 
                                                        rknn_app_ctx->in_attr_native[idx].size_with_stride);
            input_buffer[idx].fd = rknn_app_ctx->input_mem[idx]->fd;
            input_buffer[idx].virt_addr = rknn_app_ctx->input_mem[idx]->virt_addr;
            input_buffer[idx].size = rknn_app_ctx->in_attr_native[idx].size_with_stride;
            input_buffer[idx].offset = 0;
        }
        input_buffer[idx].has_create_rknn_mem = true;
        input_buffer[idx].has_set_rknn_io = false;

        // if (input_buffer[idx].has_set_rknn_io == false){
        //     rknn_app_ctx->in_attr_native[idx]->pass_through = 1;
        //     ret = rknn_set_io_mem(rknn_app_ctx->ctx, rknn_app_ctx->input_mem[idx], &(rknn_app_ctx->in_attr_native[idx]));
        //     check_ret(ret, "rknn_set_io_mem");
        //     input_buffer[idx].has_set_rknn_io = true;
        // }
    }

    for (int idx=0; idx< rknn_app_ctx->n_output; idx++){
        if (from_fd == true){
            rknn_app_ctx->output_mem[idx] = rknn_create_mem_from_fd(rknn_app_ctx->ctx, 
                                                                    output_buffer[idx].fd, 
                                                                    output_buffer[idx].virt_addr, 
                                                                    rknn_app_ctx->out_attr_native[idx].size_with_stride,
                                                                    output_buffer[idx].offset);
        } else {
            rknn_app_ctx->output_mem[idx] = rknn_create_mem(rknn_app_ctx->ctx, 
                                                            rknn_app_ctx->out_attr_native[idx].size_with_stride);
            output_buffer[idx].fd = rknn_app_ctx->output_mem[idx]->fd;
            output_buffer[idx].virt_addr = rknn_app_ctx->output_mem[idx]->virt_addr;
            output_buffer[idx].size = rknn_app_ctx->out_attr_native[idx].size_with_stride;
            output_buffer[idx].offset = 0;
        }
        output_buffer[idx].has_create_rknn_mem = true;
        output_buffer[idx].has_set_rknn_io = false;

        // if (output_buffer[idx].has_set_rknn_io == false){
        //     ret = rknn_set_io_mem(rknn_app_ctx->ctx, rknn_app_ctx->output_mem[idx], &(rknn_app_ctx->out_attr_native[idx]));
        //     check_ret(ret, "rknn_set_io_mem");
        //     output_buffer[idx].has_set_rknn_io = true;
        // }
    }
    return ret;
}