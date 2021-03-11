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
    } else {
        float val = (1 + a) * pow(linear, 1 / gamma) - a;
        return std::min(val, 1.0f);
    }
}

std::vector<char> convert_data(const float* rgba, int width, int height) {

    static const auto stbiOStreamWrite = [](void* context, void* data, int size) {
        reinterpret_cast<std::ostream*>(context)->write(reinterpret_cast<char*>(data), size);
    };

    std::vector<float> floatData(width * height * 4);
    std::vector<char> result;
    result.resize(floatData.size());

    for (size_t i = 0; i < width * height * 4; i++) {
        floatData[i] = toSRGB(rgba[i]);
    }

    for (size_t i = 0; i < width * height * 4; i++) {
        result[i] = (char)(floatData[i]*255+0.5f);
    }

    return result;
}

int main(int argc, char** argv)
{
    if (argc < 3) {
        printf("Usage: exr_to_png.exe [exr_dir] [png_dir]\n");
        printf("    Pixel value [0.0, 1.0] in EXR is mapped to [0, 255] for LDR image.\n");
        exit(-1);
    }

    std::filesystem::path exr_dir = argv[1];
    std::filesystem::path png_dir = argv[2];

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
            printf("err: %s\n", err);
            return -1;
        }
        std::vector<char> png_data = convert_data(rgba, width, height);
        int success = stbi_write_png(png_path.string().c_str(), width, height, 4, static_cast<const void*>(png_data.data()), width * 4);
        if(success!=1){
            std::cout<<"ERROR OCCURED"<<std::endl;
        }
        free(rgba);
    }

    return 0;
}