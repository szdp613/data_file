#include <systemc.h>
#include <clkreset.h>
#include <Monitor.h>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>

using namespace std;

SC_MODULE(alexnet){
    sc_out <float> output_final[1000];
    float input[3][224][224];
    float weight_1st[3][11][11];
    float output_1st[64][27][27];
    float weight_2st[64][5][5];
    float output_2st[192][13][13];
    float weight_3st[192][3][3];
    float output_3st[384][13][13];
    float weight_4st[384][3][3];
    float output_4st[256][13][13];
    float weight_5st[256][3][3];
    float output_5st[256][13][13];
    float fc_6st[4096];
    float fc_7st[4096];
    float fc_8st[1000];

//first conv layer
void conv1(){
    int INPUT_CHANNELS = 3;
    int INPUT_HEIGHT = 224;
    int INPUT_WIDTH = 224;
    int OUTPUT_CHANNELS = 64;
    int OUTPUT_HEIGHT = 55;
    int OUTPUT_WIDTH = 55;
    int KERNEL_SIZE = 11;
    int ZERO_PADDING = 1;
    int STRIDE_SIZE = 4;
    int pool_size = 3;
    int pool_stride = 2;
    int pool_output_height = 27;
    int pool_output_width = 27;

    float output[OUTPUT_CHANNELS][OUTPUT_HEIGHT][OUTPUT_WIDTH];
    float bias[OUTPUT_CHANNELS];
    // Create convolution module
    //Convolution convolution("../data/convolution");

    // Load input data from file
    std::ifstream input_file("../data/dog.txt");
    if (input_file.is_open()) {
        float temp;
        for (int ic = 0; ic < INPUT_CHANNELS; ic++) {
            for (int ih = 0; ih < INPUT_HEIGHT; ih++) {
                for (int iw = 0; iw < INPUT_WIDTH; iw++) {
                    input_file >> temp;
                    input[ic][ih][iw] = temp;
                }
            }
        }
        input_file.close();
    } else {
        std::cout << "Failed to open input.txt" << std::endl;
        return ;
    }

    // Load weight data from file
    std::ifstream weight_file("../data/conv1_weight.txt");
    if (!weight_file.is_open()) {
        std::cout << "Failed to open weight.txt" << std::endl;
        return ;
    }

    // Load bias data from file
    std::ifstream bias_file("../data/conv1_bias.txt");
    if (bias_file.is_open()) {
        float temp;
        for (int oc = 0; oc < OUTPUT_CHANNELS; oc++) {
            bias_file >> temp;
            bias[oc] = temp;
        }
        bias_file.close();
    } else {
        std::cout << "Failed to open weight.txt" << std::endl;
        return ;
    }

    // Perform convolution
    for (int oc = 0; oc < OUTPUT_CHANNELS; oc++) {
        for (int oh = 0; oh < OUTPUT_HEIGHT; oh++) {
            for (int ow = 0; ow < OUTPUT_WIDTH; ow++) {
                float sum = 0;

                for (int ic = 0; ic < INPUT_CHANNELS; ic++) {
                    for (int kh = 0; kh < KERNEL_SIZE; kh++) {
                        for (int kw = 0; kw < KERNEL_SIZE; kw++) {
                            if(oh == 0 && ow == 0){
                                weight_file >> weight_1st[ic][kh][kw];
                            }
                            int ih = oh * STRIDE_SIZE + kh - ZERO_PADDING;
                            int iw = ow * STRIDE_SIZE + kw - ZERO_PADDING;

                            if (ih >= 0 && ih < INPUT_HEIGHT && iw >= 0 && iw < INPUT_WIDTH) {
                                sum += input[ic][ih][iw] * weight_1st[ic][kh][kw];
                            }
                        }
                    }
                }

               output[oc][oh][ow] = sum;
            }
        }
    }
    weight_file.close();

    // Add bias
    for (int oc = 0; oc < OUTPUT_CHANNELS; oc++) {
        for (int oh = 0; oh < OUTPUT_HEIGHT; oh++) {
            for (int ow = 0; ow < OUTPUT_WIDTH; ow++) {
                output[oc][oh][ow] += bias[oc];
            }
        }
    }

    // Apply ReLU
    for (int oc = 0; oc < OUTPUT_CHANNELS; oc++) {
        for (int oh = 0; oh < OUTPUT_HEIGHT; oh++) {
            for (int ow = 0; ow < OUTPUT_WIDTH; ow++) {
                if (output[oc][oh][ow] < 0) {
                    output[oc][oh][ow] = 0;
                }
            }
        }
    }

    // Max pooling
    
    for (int oc = 0; oc < OUTPUT_CHANNELS; oc++) {
        for (int oh = 0; oh < pool_output_height; oh++) {
            for (int ow = 0; ow < pool_output_width; ow++) {
                float max_value = output[oc][oh * pool_stride][ow * pool_stride];

                for (int kh = 0; kh < pool_size; kh++) {
                    for (int kw = 0; kw < pool_size; kw++) {
                        float value = output[oc][oh * pool_stride + kh][ow * pool_stride + kw];
                        if (value > max_value) {
                            max_value = value;
                        }
                    }
                }

                output_1st[oc][oh][ow] = max_value;
            }
        }
    }
    /*for (int oc = 0; oc < 64; oc++) {
        for (int oh = 0; oh < 27; oh++) {
            for (int ow = 0; ow < 27; ow++) {
                std::cout << "Output[" << oc << "][" << oh << "][" << ow << "] = " << output_1st[oc][oh][ow] << std::endl;
            }
        }
    }*/
}

//second conv layer
void conv2(){
    int INPUT_CHANNELS = 64;
    int INPUT_HEIGHT = 27;
    int INPUT_WIDTH = 27;
    int OUTPUT_CHANNELS = 192;
    int OUTPUT_HEIGHT = 27;
    int OUTPUT_WIDTH = 27;
    int KERNEL_SIZE = 5;
    int ZERO_PADDING = 2;
    int STRIDE_SIZE = 1;
    int pool_size = 3;
    int pool_stride = 2;
    int pool_output_height = 13;
    int pool_output_width = 13;

    float output[OUTPUT_CHANNELS][OUTPUT_HEIGHT][OUTPUT_WIDTH];
    float bias[OUTPUT_CHANNELS];
    // Create convolution module
    //Convolution convolution("../data/convolution");


    // Load weight data from file
    std::ifstream weight_file("../data/conv2_weight.txt");
    if (!weight_file.is_open()) {
        std::cout << "Failed to open conv2_weight.txt" << std::endl;
        return ;
    }

    // Load bias data from file
    std::ifstream bias_file("../data/conv2_bias.txt");
    if (bias_file.is_open()) {
        float temp;
        for (int oc = 0; oc < OUTPUT_CHANNELS; oc++) {
            bias_file >> temp;
            bias[oc] = temp;
        }
        bias_file.close();
    } else {
        std::cout << "Failed to open conv2_bias.txt" << std::endl;
        return ;
    }

    // Perform convolution
    for (int oc = 0; oc < OUTPUT_CHANNELS; oc++) {
        for (int oh = 0; oh < OUTPUT_HEIGHT; oh++) {
            for (int ow = 0; ow < OUTPUT_WIDTH; ow++) {
                float sum = 0;

                for (int ic = 0; ic < INPUT_CHANNELS; ic++) {
                    for (int kh = 0; kh < KERNEL_SIZE; kh++) {
                        for (int kw = 0; kw < KERNEL_SIZE; kw++) {
                            if(oh == 0 && ow == 0){
                                weight_file >> weight_2st[ic][kh][kw];
                            }
                            int ih = oh * STRIDE_SIZE + kh - ZERO_PADDING;
                            int iw = ow * STRIDE_SIZE + kw - ZERO_PADDING;

                            if (ih >= 0 && ih < INPUT_HEIGHT && iw >= 0 && iw < INPUT_WIDTH) {
                                sum += output_1st[ic][ih][iw] * weight_2st[ic][kh][kw];
                            }
                        }
                    }
                }

               output[oc][oh][ow] = sum;
            }
        }
    }
    weight_file.close();

    // Add bias
    for (int oc = 0; oc < OUTPUT_CHANNELS; oc++) {
        for (int oh = 0; oh < OUTPUT_HEIGHT; oh++) {
            for (int ow = 0; ow < OUTPUT_WIDTH; ow++) {
                output[oc][oh][ow] += bias[oc];
            }
        }
    }

    // Apply ReLU
    for (int oc = 0; oc < OUTPUT_CHANNELS; oc++) {
        for (int oh = 0; oh < OUTPUT_HEIGHT; oh++) {
            for (int ow = 0; ow < OUTPUT_WIDTH; ow++) {
                if (output[oc][oh][ow] < 0) {
                    output[oc][oh][ow] = 0;
                }
            }
        }
    }

    // Max pooling
    
    for (int oc = 0; oc < OUTPUT_CHANNELS; oc++) {
        for (int oh = 0; oh < pool_output_height; oh++) {
            for (int ow = 0; ow < pool_output_width; ow++) {
                float max_value = output[oc][oh * pool_stride][ow * pool_stride];

                for (int kh = 0; kh < pool_size; kh++) {
                    for (int kw = 0; kw < pool_size; kw++) {
                        float value = output[oc][oh * pool_stride + kh][ow * pool_stride + kw];
                        if (value > max_value) {
                            max_value = value;
                        }
                    }
                }

                output_2st[oc][oh][ow] = max_value;
            }
        }
    }

    /*for (int oc = 0; oc < 192; oc++) {
        for (int oh = 0; oh < 13; oh++) {
            for (int ow = 0; ow < 13; ow++) {
                std::cout << "Output[" << oc << "][" << oh << "][" << ow << "] = " << output_2st[oc][oh][ow] << std::endl;
            }
        }
    }*/
}

//third conv layer
void conv3(){
    int INPUT_CHANNELS = 192;
    int INPUT_HEIGHT = 13;
    int INPUT_WIDTH = 13;
    int OUTPUT_CHANNELS = 384;
    int OUTPUT_HEIGHT = 13;
    int OUTPUT_WIDTH = 13;
    int KERNEL_SIZE = 3;
    int ZERO_PADDING = 1;
    int STRIDE_SIZE = 1;
    

    float bias[OUTPUT_CHANNELS];
    // Create convolution module
    //Convolution convolution("../data/convolution");


    // Load weight data from file
    std::ifstream weight_file("../data/conv3_weight.txt");
    if (!weight_file.is_open()) {
        std::cout << "Failed to open conv3_weight.txt" << std::endl;
        return ;
    }

    // Load bias data from file
    std::ifstream bias_file("../data/conv3_bias.txt");
    if (bias_file.is_open()) {
        float temp;
        for (int oc = 0; oc < OUTPUT_CHANNELS; oc++) {
            bias_file >> temp;
            bias[oc] = temp;
        }
        bias_file.close();
    } else {
        std::cout << "Failed to open conv3_bias.txt" << std::endl;
        return ;
    }

    // Perform convolution
    for (int oc = 0; oc < OUTPUT_CHANNELS; oc++) {
        for (int oh = 0; oh < OUTPUT_HEIGHT; oh++) {
            for (int ow = 0; ow < OUTPUT_WIDTH; ow++) {
                float sum = 0;

                for (int ic = 0; ic < INPUT_CHANNELS; ic++) {
                    for (int kh = 0; kh < KERNEL_SIZE; kh++) {
                        for (int kw = 0; kw < KERNEL_SIZE; kw++) {
                            if(oh == 0 && ow == 0){
                                weight_file >> weight_3st[ic][kh][kw];
                            }
                            int ih = oh * STRIDE_SIZE + kh - ZERO_PADDING;
                            int iw = ow * STRIDE_SIZE + kw - ZERO_PADDING;

                            if (ih >= 0 && ih < INPUT_HEIGHT && iw >= 0 && iw < INPUT_WIDTH) {
                                sum += output_2st[ic][ih][iw] * weight_3st[ic][kh][kw];
                            }
                        }
                    }
                }

               output_3st[oc][oh][ow] = sum;
            }
        }
    }
    weight_file.close();

    // Add bias
    for (int oc = 0; oc < OUTPUT_CHANNELS; oc++) {
        for (int oh = 0; oh < OUTPUT_HEIGHT; oh++) {
            for (int ow = 0; ow < OUTPUT_WIDTH; ow++) {
                output_3st[oc][oh][ow] += bias[oc];
            }
        }
    }

    // Apply ReLU
    for (int oc = 0; oc < OUTPUT_CHANNELS; oc++) {
        for (int oh = 0; oh < OUTPUT_HEIGHT; oh++) {
            for (int ow = 0; ow < OUTPUT_WIDTH; ow++) {
                if (output_3st[oc][oh][ow] < 0) {
                    output_3st[oc][oh][ow] = 0;
                }
            }
        }
    }

    /*for (int oc = 0; oc < 384; oc++) {
        for (int oh = 0; oh < 13; oh++) {
            for (int ow = 0; ow < 13; ow++) {
                std::cout << "Output[" << oc << "][" << oh << "][" << ow << "] = " << output_3st[oc][oh][ow] << std::endl;
            }
        }
    }*/

}

//fourth conv layer
void conv4(){
    int INPUT_CHANNELS = 384;
    int INPUT_HEIGHT = 13;
    int INPUT_WIDTH = 13;
    int OUTPUT_CHANNELS = 256;
    int OUTPUT_HEIGHT = 13;
    int OUTPUT_WIDTH = 13;
    int KERNEL_SIZE = 3;
    int ZERO_PADDING = 1;
    int STRIDE_SIZE = 1;
    

    float bias[OUTPUT_CHANNELS];
    // Create convolution module
    //Convolution convolution("../data/convolution");


    // Load weight data from file
    std::ifstream weight_file("../data/conv4_weight.txt");
    if (!weight_file.is_open()) {
        std::cout << "Failed to open conv4_weight.txt" << std::endl;
        return ;
    }

    // Load bias data from file
    std::ifstream bias_file("../data/conv4_bias.txt");
    if (bias_file.is_open()) {
        float temp;
        for (int oc = 0; oc < OUTPUT_CHANNELS; oc++) {
            bias_file >> temp;
            bias[oc] = temp;
        }
        bias_file.close();
    } else {
        std::cout << "Failed to open conv4_bias.txt" << std::endl;
        return ;
    }

    // Perform convolution
    for (int oc = 0; oc < OUTPUT_CHANNELS; oc++) {
        for (int oh = 0; oh < OUTPUT_HEIGHT; oh++) {
            for (int ow = 0; ow < OUTPUT_WIDTH; ow++) {
                float sum = 0;

                for (int ic = 0; ic < INPUT_CHANNELS; ic++) {
                    for (int kh = 0; kh < KERNEL_SIZE; kh++) {
                        for (int kw = 0; kw < KERNEL_SIZE; kw++) {
                            if(oh == 0 && ow == 0){
                                weight_file >> weight_4st[ic][kh][kw];
                            }
                            int ih = oh * STRIDE_SIZE + kh - ZERO_PADDING;
                            int iw = ow * STRIDE_SIZE + kw - ZERO_PADDING;

                            if (ih >= 0 && ih < INPUT_HEIGHT && iw >= 0 && iw < INPUT_WIDTH) {
                                sum += output_3st[ic][ih][iw] * weight_4st[ic][kh][kw];
                            }
                        }
                    }
                }

               output_4st[oc][oh][ow] = sum;
            }
        }
    }
    weight_file.close();

    // Add bias
    for (int oc = 0; oc < OUTPUT_CHANNELS; oc++) {
        for (int oh = 0; oh < OUTPUT_HEIGHT; oh++) {
            for (int ow = 0; ow < OUTPUT_WIDTH; ow++) {
                output_4st[oc][oh][ow] += bias[oc];
            }
        }
    }

    // Apply ReLU
    for (int oc = 0; oc < OUTPUT_CHANNELS; oc++) {
        for (int oh = 0; oh < OUTPUT_HEIGHT; oh++) {
            for (int ow = 0; ow < OUTPUT_WIDTH; ow++) {
                if (output_4st[oc][oh][ow] < 0) {
                    output_4st[oc][oh][ow] = 0;
                }
            }
        }
    }

    /*for (int oc = 0; oc < 256; oc++) {
        for (int oh = 0; oh < 13; oh++) {
            for (int ow = 0; ow < 13; ow++) {
                std::cout << "Output[" << oc << "][" << oh << "][" << ow << "] = " << output_4st[oc][oh][ow] << std::endl;
            }
        }
    }*/

}

void conv5(){
    int INPUT_CHANNELS = 256;
    int INPUT_HEIGHT = 13;
    int INPUT_WIDTH = 13;
    int OUTPUT_CHANNELS = 256;
    int OUTPUT_HEIGHT = 13;
    int OUTPUT_WIDTH = 13;
    int KERNEL_SIZE = 3;
    int ZERO_PADDING = 1;
    int STRIDE_SIZE = 1;
    int pool_size = 3;
    int pool_stride = 2;
    int pool_output_height = 6;
    int pool_output_width = 6;

    float output[OUTPUT_CHANNELS][OUTPUT_HEIGHT][OUTPUT_WIDTH];
    float bias[OUTPUT_CHANNELS];
    // Create convolution module
    //Convolution convolution("../data/convolution");


    // Load weight data from file
    std::ifstream weight_file("../data/conv5_weight.txt");
    if (!weight_file.is_open()) {
        std::cout << "Failed to open conv5_weight.txt" << std::endl;
        return ;
    }

    // Load bias data from file
    std::ifstream bias_file("../data/conv5_bias.txt");
    if (bias_file.is_open()) {
        float temp;
        for (int oc = 0; oc < OUTPUT_CHANNELS; oc++) {
            bias_file >> temp;
            bias[oc] = temp;
        }
        bias_file.close();
    } else {
        std::cout << "Failed to open conv5_bias.txt" << std::endl;
        return ;
    }

    // Perform convolution
    for (int oc = 0; oc < OUTPUT_CHANNELS; oc++) {
        for (int oh = 0; oh < OUTPUT_HEIGHT; oh++) {
            for (int ow = 0; ow < OUTPUT_WIDTH; ow++) {
                float sum = 0;

                for (int ic = 0; ic < INPUT_CHANNELS; ic++) {
                    for (int kh = 0; kh < KERNEL_SIZE; kh++) {
                        for (int kw = 0; kw < KERNEL_SIZE; kw++) {
                            if(oh == 0 && ow == 0){
                                weight_file >> weight_5st[ic][kh][kw];
                            }
                            int ih = oh * STRIDE_SIZE + kh - ZERO_PADDING;
                            int iw = ow * STRIDE_SIZE + kw - ZERO_PADDING;

                            if (ih >= 0 && ih < INPUT_HEIGHT && iw >= 0 && iw < INPUT_WIDTH) {
                                sum += output_4st[ic][ih][iw] * weight_5st[ic][kh][kw];
                            }
                        }
                    }
                }

               output[oc][oh][ow] = sum;
            }
        }
    }
    weight_file.close();

    // Add bias
    for (int oc = 0; oc < OUTPUT_CHANNELS; oc++) {
        for (int oh = 0; oh < OUTPUT_HEIGHT; oh++) {
            for (int ow = 0; ow < OUTPUT_WIDTH; ow++) {
                output[oc][oh][ow] += bias[oc];
            }
        }
    }

    // Apply ReLU
    for (int oc = 0; oc < OUTPUT_CHANNELS; oc++) {
        for (int oh = 0; oh < OUTPUT_HEIGHT; oh++) {
            for (int ow = 0; ow < OUTPUT_WIDTH; ow++) {
                if (output[oc][oh][ow] < 0) {
                    output[oc][oh][ow] = 0;
                }
            }
        }
    }

    // Max pooling
    
    for (int oc = 0; oc < OUTPUT_CHANNELS; oc++) {
        for (int oh = 0; oh < pool_output_height; oh++) {
            for (int ow = 0; ow < pool_output_width; ow++) {
                float max_value = output[oc][oh * pool_stride][ow * pool_stride];

                for (int kh = 0; kh < pool_size; kh++) {
                    for (int kw = 0; kw < pool_size; kw++) {
                        float value = output[oc][oh * pool_stride + kh][ow * pool_stride + kw];
                        if (value > max_value) {
                            max_value = value;
                        }
                    }
                }

                output_5st[oc][oh][ow] = max_value;
            }
        }
    }

    /*for (int oc = 0; oc < 256; oc++) {
        for (int oh = 0; oh < 6; oh++) {
            for (int ow = 0; ow < 6; ow++) {
                std::cout << "Output[" << oc << "][" << oh << "][" << ow << "] = " << output_5st[oc][oh][ow] << std::endl;
            }
        }
    }*/
}

void fc6(){
    int INPUT_CHANNELS = 256;
    int INPUT_HEIGHT = 6;
    int INPUT_WIDTH = 6;
    float weight =0;
    float bias =0;


    // Load weight data from file
    std::ifstream weight_file("../data/fc6_weight.txt");
    if (!weight_file.is_open()) {
        std::cout << "Failed to open fc6_weight.txt" << std::endl;
        return ;
    }

    // Load bias data from file
    std::ifstream bias_file("../data/fc6_bias.txt");
    if (!bias_file.is_open()) {
        std::cout << "Failed to open fc6_bias.txt" << std::endl;
        return ;
    }

    for(int i = 0; i < 4096; i++){
        float sum = 0;
        for(int j = 0; j < 256; j++){
            for(int k = 0; k < 6; k++){
                for(int l = 0; l < 6; l++){
                    weight_file >> weight;
                    sum += output_5st[j][k][l] * weight;
                }
            }
        }
        bias_file >> bias;
        fc_6st[i] = sum + bias;
        //ReLU
        if(fc_6st[i] < 0){
            fc_6st[i] = 0;
        }
    }
    weight_file.close();
    bias_file.close();

    /*for (int oc = 0; oc < 4096; oc++) {
        std::cout << "Output[" << oc << "] = " << fc_6st[oc] << std::endl;
    }*/
}

void fc7(){
    float weight =0;
    float bias =0;


    // Load weight data from file
    std::ifstream weight_file("../data/fc7_weight.txt");
    if (!weight_file.is_open()) {
        std::cout << "Failed to open fc7_weight.txt" << std::endl;
        return ;
    }

    // Load bias data from file
    std::ifstream bias_file("../data/fc7_bias.txt");
    if (!bias_file.is_open()) {
        std::cout << "Failed to open fc7_bias.txt" << std::endl;
        return ;
    }

    for(int i = 0; i < 4096; i++){
        float sum = 0;
        for(int j = 0; j < 4096; j++){
            weight_file >> weight;
            sum += fc_6st[j] * weight;
        }
        bias_file >> bias;
        fc_7st[i] = sum + bias;
        //ReLU
        if(fc_7st[i] < 0){
            fc_7st[i] = 0;
        }
    }
    weight_file.close();
    bias_file.close();

    /*for (int oc = 0; oc < 4096; oc++) {
        std::cout << "Output[" << oc << "] = " << fc_7st[oc] << std::endl;
    }*/
}

void fc8(){
    float weight =0;
    float bias =0;


    // Load weight data from file
    std::ifstream weight_file("../data/fc8_weight.txt");
    if (!weight_file.is_open()) {
        std::cout << "Failed to open fc8_weight.txt" << std::endl;
        return ;
    }

    // Load bias data from file
    std::ifstream bias_file("../data/fc8_bias.txt");
    if (!bias_file.is_open()) {
        std::cout << "Failed to open fc8_bias.txt" << std::endl;
        return ;
    }

    for(int i = 0; i < 1000; i++){
        float sum = 0;
        for(int j = 0; j < 4096; j++){
            weight_file >> weight;
            sum += fc_7st[j] * weight;
        }
        bias_file >> bias;
        fc_8st[i] = sum + bias;
        //ReLU
        if(fc_8st[i] < 0){
            fc_8st[i] = 0;
        }
    }
    weight_file.close();
    bias_file.close();

    /*for (int oc = 0; oc < 1000; oc++) {
        std::cout << "Output[" << oc << "] = " << fc_8st[oc] << std::endl;
    }*/
    for(int i = 0; i < 1000; i++){
        output_final[i].write(fc_8st[i]);
    }
}



    SC_CTOR(alexnet) {
        SC_METHOD(conv1);
        SC_METHOD(conv2);
        SC_METHOD(conv3);
        SC_METHOD(conv4);
        SC_METHOD(conv5);
        SC_METHOD(fc6);
        SC_METHOD(fc7);
        SC_METHOD(fc8);
    }

};
int sc_main(int argc, char* argv[]) {
    // Create signals
    sc_signal<bool> reset, clock;
    sc_signal<float> output_final[1000];
    

    alexnet m_alexnet("alexnet");
    Reset m_Reset( "m_Reset", 10 );
    Clock m_clock( "m_clock", 5, 40 );
    Monitor m_Monitor( "m_Monitor" );
    m_Reset( reset );
	m_clock( clock );
    m_Monitor.rst( reset );
    for(int i = 0; i < 1000; i++){
        m_alexnet.output_final[i](output_final[i]);
        m_Monitor.fc_8st[i](output_final[i]);
    }



    // Run simulation
    sc_start(500, SC_NS );

    
    return 0;
}