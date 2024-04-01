
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include "rknn_api.h"

#include "timer.h"
#include "rknn_app.h"
#include "data_utils.h"
#include "path_utils.h"


int load_input_data(rknn_app_context_t* rknn_app_ctx, const char* input_path, rknn_app_buffer* input_buffer){
    printf("\nLOAD INPUTS\n");
    int ret = 0;

    if (input_path == NULL){
        return -1;
    }

    std::vector<std::string> input_paths_split;
    input_paths_split = split(input_path, "#");
    if (input_paths_split.size() == 0){
        return -1;
    }

    if (rknn_app_ctx->n_input != input_paths_split.size()) {
        printf("ERROR: input number is not match, input number is %d, but input path is %s\n", rknn_app_ctx->n_input, input_path);
        return -1;
    }
    unsigned char* temp_data = NULL;
    rknn_tensor_type temp_data_type;
    for (int i=0; i<rknn_app_ctx->n_input; i++){
        if (strstr(input_paths_split[i].c_str(), ".npy")) {
            printf("  input[%d] - NPY: %s\n", i, input_paths_split[i].c_str());
            temp_data = (unsigned char*)load_npy(input_paths_split[i].c_str(), &(rknn_app_ctx->in_attr[i]), &temp_data_type);
        } 
        else {
            printf("  input[%d] - IMG: %s\n", i, input_paths_split[i].c_str());
            LETTER_BOX letter_box;
            temp_data = load_image_and_autoresize(input_paths_split[i].c_str(), &letter_box, &(rknn_app_ctx->in_attr[i]));
            temp_data_type = RKNN_TENSOR_UINT8;
        }
        if (!temp_data) {
            printf("ERROR: load input data failed\n");
            return -1;
        }

        ret = rknn_app_wrap_input_buffer(rknn_app_ctx, temp_data, temp_data_type, &input_buffer[i], i);
        free(temp_data);
    }

    return ret;
}


int loop_run(rknn_app_context_t* rknn_app_ctx, rknn_app_buffer* input_buffer, rknn_app_buffer* output_buffer, int loop_time){
    printf("\nRUNNING RKNN\n");
    int ret = 0;
    TIMER timer, timer_iner;
    timer.start();
    for (int i=0; i<loop_time; i++){
        timer_iner.start();
        ret = run_rknn_app(rknn_app_ctx, input_buffer, output_buffer);
        timer_iner.stop();
        printf("  loop[%d] time: %f ms\n", i, timer_iner.get_time());
    }
    timer.stop();
    printf("Average time: %f ms\n", timer.get_time() / loop_time);
    return ret;
}


int post_process_check_consine_similarity(rknn_app_context_t* rknn_app_ctx,
                                          rknn_app_buffer* output_buffer,
                                          const char* output_folder, 
                                          const char* golden_folder)
{
    int ret = 0;
    printf("\nCHECK OUTPUT\n");
    printf("  check all output to '%s'\n", output_folder);
    printf("  with golden data in '%s'\n", golden_folder);
    ret = folder_mkdirs(output_folder);

    float* temp_data = NULL;
    rknn_tensor_attr* attr = NULL;
    for (int idx=0; idx< rknn_app_ctx->n_output; idx++){
        attr = &(rknn_app_ctx->out_attr[idx]);
        printf("  output[%d] - %s:\n", idx, attr->name);
        char* golden_path = get_output_path(attr->name, golden_folder);
        char* output_path = get_output_path(attr->name, output_folder);
        temp_data = (float*)rknn_app_unwrap_output_buffer(rknn_app_ctx, &output_buffer[idx], RKNN_TENSOR_FLOAT32, idx);
        save_npy(output_path, temp_data, attr);
        free(temp_data);

        float cosine_similarity = compare_npy_cos_similarity(golden_path, output_path, attr->n_elems);
        printf("    cosine similarity: %f\n", cosine_similarity);
        free(golden_path);
        free(output_path);
    }
    return ret;
}


int main(int argc, char* argv[]){
    if (argc < 2 ){
        printf("Usage: ./rknn_app_demo model_path image_path repeat_times\n");
        printf("  Example: ./rknn_app_demo ./model.rknn ./test.npy 20\n");
        return -1;
    }
    int ret = 0;

    // init rknn_app
    rknn_app_context_t rknn_app_ctx;
    memset(&rknn_app_ctx, 0, sizeof(rknn_app_context_t));
    const char* model_path = argv[1];
    bool verbose_log = true;
    ret = init_rknn_app(&rknn_app_ctx, model_path, verbose_log);
    if (ret != 0){
        printf("init rknn app failed\n");
        return -1;
    }

    // init input output buffer
    rknn_app_buffer input_buffer[rknn_app_ctx.n_input];
    rknn_app_buffer output_buffer[rknn_app_ctx.n_output];
    ret = init_rknn_app_input_output_buffer(&rknn_app_ctx, input_buffer, output_buffer, false);
    if (ret != 0){
        printf("init input output buffer failed\n");
        return -1;
    }

    // load input data and wrap to input buffer
    const char* inputs_path = NULL;
    if (argc > 2){
        inputs_path = argv[2];
    }
    ret = load_input_data(&rknn_app_ctx, inputs_path, input_buffer);
    int input_given = ret;

    // inference
    int loop_time = 1;
    if (argc > 3){
        loop_time = atoi(argv[3]);
    }
    ret = loop_run(&rknn_app_ctx, input_buffer, output_buffer, loop_time);
    if (ret != 0){
        printf("run rknn app failed\n");
        return -1;
    }

    // save output result as npy
    if ((argc > 2) && (input_given>-1)){
        ret = post_process_check_consine_similarity(&rknn_app_ctx, output_buffer, "./data/outputs/runtime", "./data/outputs/golden");
    }
    else{
        printf("Inputs was not given, skip save output\n");
    }

    // release rknn
    release_rknn_app(&rknn_app_ctx);

    return 0;
}