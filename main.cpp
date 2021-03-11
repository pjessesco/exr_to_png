#include <vector>
#include <ostream>
#include <filesystem>

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
        printf("Usage: exr2ldr input.exr output.png\n");
        printf("    Pixel value [0.0, 1.0] in EXR is mapped to [0, 255] for LDR image.\n");
        exit(-1);
    }

    int width, height;
    float* rgba;
    const char* err;

    int ret = LoadEXR(&rgba, &width, &height, argv[1], &err);
    if (ret != 0) {
        printf("err: %s\n", err);
        return -1;
    }

    std::vector<char> png_data = convert_data(rgba, width, height);

    int success = stbi_write_png(argv[2], width, height, 4, static_cast<const void*>(png_data.data()), width * 4);

    free(rgba);

    return 0;
}