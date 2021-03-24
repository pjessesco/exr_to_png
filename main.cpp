#include <vector>
#include <ostream>
#include <filesystem>
#include <string>
#include <iostream>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#define TINYEXR_IMPLEMENTATION
#include <tinyexr.h>

// https://github.com/Tom94/tev/blob/89fde044101eaadf43f2b44c9a84b54ffb240fc5/include/tev/Common.h#L159
inline float toSRGB(float linear, float gamma = 2.4f) {
    static const float a = 0.055f;
    if (linear <= 0.0031308f) {
        return 12.92f * linear;
    }
    else {
        float val = (1 + a) * pow(linear, 1 / gamma) - a;
        return std::min(val, 1.0f);
    }
}

std::vector<char> convert_data(const float* rgba, int width, int height, int target_channel_num) {

    static const auto stbiOStreamWrite = [](void* context, void* data, int size) {
        reinterpret_cast<std::ostream*>(context)->write(reinterpret_cast<char*>(data), size);
    };

    std::vector<float> floatData(width * height * target_channel_num);
    std::vector<char> result;
    result.resize(floatData.size());

    int src_idx = 0;
    for (size_t dst_idx = 0; dst_idx < width * height * target_channel_num; dst_idx++) {
        if(target_channel_num == 4){
            floatData[dst_idx] = toSRGB(rgba[src_idx]);
        }
        else if(target_channel_num == 3){
            // Skip if src_idx indicates alpha
            if(src_idx % 4 == 3){
                src_idx++;
            }
            floatData[dst_idx] = toSRGB(rgba[src_idx]);
        }
        src_idx++;
    }

    for (size_t i = 0; i < width * height * target_channel_num; i++) {
        result[i] = (char)(floatData[i]*255+0.5f);
    }

    return result;
}

int main(int argc, char** argv)
{
    if (argc < 3) {
        std::cout<<"Usage: exr_to_png.exe [exr_dir] [png_dir] [rgb|rgba])"<<std::endl;
        std::cout<<"    Pixel value [0.0, 1.0] in EXR is mapped to [0, 255] for LDR image."<<std::endl;
        std::cout<<"    WARNING : Not tested under the exr image without alpha channel."<<std::endl;

        exit(-1);
    }

    std::filesystem::path exr_dir = argv[1];
    std::filesystem::path png_dir = argv[2];
    std::string channel_mode = std::string(argv[3]);

    int target_channel_num = -1;

    if(!std::filesystem::exists(exr_dir)){
        std::cerr<<"Given exr_dir is not exist : "<<exr_dir<<std::endl;
        exit(-1);
    }

    if(channel_mode=="rgb" || channel_mode=="RGB"){
        target_channel_num = 3;
    }
    else if(channel_mode=="rgba" || channel_mode=="RGBA"){
        target_channel_num = 4;
    }
    else{
        std::cerr<<"Unexpected channel : "<<channel_mode<<std::endl;
        exit(-1);
    }

    for(const auto &p:std::filesystem::directory_iterator(exr_dir)){

        const std::filesystem::path exr_path = p.path();
        std::cout<<exr_path<<std::endl;

        if(exr_path.extension().string() != ".exr"){
            std::cout << " -- Skip unexpected extension file : " << exr_path.filename() << std::endl;
            continue;
        }

        const std::filesystem::path png_path = (png_dir / exr_path.stem()).string() + ".png";
        std::cout<<" >> "<<png_path<<std::endl;

        int width, height;
        float* rgba;
        const char* err;

        int ret = LoadEXR(&rgba, &width, &height, exr_path.string().c_str(), &err);
        if (ret != 0) {
            std::cerr<<"Error while load exr image : "<<std::string(err)<<std::endl;
            exit(-1);
        }
        std::vector<char> png_data = convert_data(rgba, width, height, target_channel_num);
        int success = stbi_write_png(png_path.string().c_str(), width, height, target_channel_num, static_cast<const void*>(png_data.data()), width * target_channel_num);
        if(success!=1){
            std::cerr<<"ERROR OCCURED"<<std::endl;
        }
        free(rgba);
    }

    return 0;
}