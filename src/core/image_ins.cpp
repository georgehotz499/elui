#include "image_ins.h"
#include "log.h"
#include "string_helper.h"

extern "C" {
// LVGL库
#include "lvgl/lvgl.h"
// stb image库
#define STB_IMAGE_IMPLEMENTATION  // 必须定义一次以启用实现stb_image库
#include "core/stb_image.h"
// spng库
#include "spng.h"
}

#include <fstream>


namespace {
    // 兼容不同版本spng的常量定义（如果头文件未定义则手动补全）
    #ifndef SPNG_FMT_RGB
    #define SPNG_FMT_RGB 1  // RGB 24bit 格式
    #endif

    #ifndef SPNG_FMT_RGBA
    #define SPNG_FMT_RGBA 2 // RGBA 32bit 格式
    #endif

    #ifndef SPNG_DECODE_USE_ALPHA
    #define SPNG_DECODE_USE_ALPHA 0x01 // 启用Alpha通道解码
    #endif

    // 读取文件到内存缓冲区（保留fopen，通过宏关闭警告）
    static uint8_t* read_file(const char* filename, size_t* out_size) {
        FILE* f = fopen(filename, "rb");
        if (!f) {
            perror("fopen failed");
            return NULL;
        }

        // 获取文件大小
        fseek(f, 0, SEEK_END);
        *out_size = ftell(f);
        fseek(f, 0, SEEK_SET);

        // 分配内存并读取文件内容
        uint8_t* buf = (uint8_t*)malloc(*out_size);
        if (!buf) {
            perror("malloc failed");
            fclose(f);
            return NULL;
        }

        if (fread(buf, 1, *out_size, f) != *out_size) {
            perror("fread failed");
            free(buf);
            fclose(f);
            return NULL;
        }

        fclose(f);
        return buf;
    }

    // 定义解析格式枚举
    typedef enum {
        PNG_FORMAT_RGB_24BIT = 0,  // 24bit RGB格式，3通道
        PNG_FORMAT_RGBA_32BIT      // 32bit RGBA格式，4通道
    } PNGDecodeFormat;

    /**
     * 通用PNG解析函数：解析为24bit RGB或32bit RGBA格式
     * @param png_path: 输入参数，PNG图片路径
     * @param out_pixels: 输出参数，解析后的像素数据（需调用者手动free）
     * @param out_width: 输出参数，图片宽度
     * @param out_height: 输出参数，图片高度
     * @param out_channels: 输出参数，通道数（3=RGB，4=RGBA）
     * @param format: 输入参数，解析格式（PNG_FORMAT_RGB_24BIT/PNG_FORMAT_RGBA_32BIT）
     * @return 0=成功，-1=失败
     */
    static int parse_png_image(const char* png_path, uint8_t** out_pixels,
        uint32_t* out_width, uint32_t* out_height, int* out_channels,
        PNGDecodeFormat format) {
        // 入参合法性检查
        if (png_path == NULL || out_pixels == NULL || out_width == NULL ||
            out_height == NULL || out_channels == NULL) {
            fprintf(stderr, "invalid input parameters\n");
            return -1;
        }

        size_t file_size;
        uint8_t* file_buf = read_file(png_path, &file_size);
        if (!file_buf) return -1;

        // 初始化spng上下文
        spng_ctx* ctx = spng_ctx_new(0);
        if (!ctx) {
            free(file_buf);
            fprintf(stderr, "spng_ctx_new failed\n");
            return -1;
        }

        // 设置输入缓冲区
        if (spng_set_png_buffer(ctx, file_buf, file_size) != SPNG_OK) {
            fprintf(stderr, "spng_set_png_buffer failed\n");
            spng_ctx_free(ctx);
            free(file_buf);
            return -1;
        }

        // 获取PNG图片基础信息（宽、高）
        struct spng_ihdr ihdr;
        if (spng_get_ihdr(ctx, &ihdr) != SPNG_OK) {
            fprintf(stderr, "spng_get_ihdr failed\n");
            spng_ctx_free(ctx);
            free(file_buf);
            return -1;
        }
        *out_width = ihdr.width;
        *out_height = ihdr.height;

        // 根据解析格式设置通道数、解码格式、计算数据大小
        int channels;
        spng_format spng_fmt;
        size_t pixel_data_size;

        switch (format) {
        case PNG_FORMAT_RGB_24BIT:
            channels = 3;
            spng_fmt = SPNG_FMT_RGB8;
            pixel_data_size = ihdr.width * ihdr.height * channels;
            break;
        case PNG_FORMAT_RGBA_32BIT:
            channels = 4;
            spng_fmt = SPNG_FMT_RGBA8;
            pixel_data_size = ihdr.width * ihdr.height * channels;
            break;
        default:
            fprintf(stderr, "unsupported decode format\n");
            spng_ctx_free(ctx);
            free(file_buf);
            return -1;
        }
        *out_channels = channels;  // 设置输出通道数

        // 分配像素数据内存
        *out_pixels = (uint8_t*)malloc(pixel_data_size);
        if (!*out_pixels) {
            fprintf(stderr, "malloc for pixel data failed\n");
            spng_ctx_free(ctx);
            free(file_buf);
            return -1;
        }

        // 解码图片数据
        if (spng_decode_image(ctx, *out_pixels, pixel_data_size, spng_fmt, SPNG_DECODE_USE_ALPHA) != SPNG_OK) {
            fprintf(stderr, "spng_decode_image failed (format: %d)\n", format);
            free(*out_pixels);
            *out_pixels = NULL;
            spng_ctx_free(ctx);
            free(file_buf);
            return -1;
        }

        // 释放资源
        spng_ctx_free(ctx);
        free(file_buf);
        return 0;
    }

    // 兼容不同版本spng的常量定义
    #ifndef SPNG_COLOR_TYPE_GRAY
    #define SPNG_COLOR_TYPE_GRAY      0   // 灰度图 (1通道)
    #define SPNG_COLOR_TYPE_RGB       2   // RGB (3通道)
    #define SPNG_COLOR_TYPE_PALETTE   3   // 调色板 (映射为3通道)
    #define SPNG_COLOR_TYPE_GRAY_ALPHA 4  // 灰度+Alpha (2通道)
    #define SPNG_COLOR_TYPE_RGB_ALPHA 6   // RGBA (4通道)
    #endif

    /**
     * 仅获取PNG图片的宽、高、通道数（不解码像素数据）
     * @param png_path: PNG文件路径
     * @param out_width: 输出参数，图片宽度
     * @param out_height: 输出参数，图片高度
     * @param out_channels: 输出参数，通道数（1/2/3/4）
     * @return 0=成功，-1=失败
     */
    static int get_png_info(const char* png_path, uint32_t* out_width, uint32_t* out_height, int* out_channels) {
        // 入参合法性检查
        if (png_path == NULL || out_width == NULL || out_height == NULL || out_channels == NULL) {
            fprintf(stderr, "无效的输入参数\n");
            return -1;
        }

        // 打开PNG文件（仅读取，不加载完整内容）
        FILE* fp = fopen(png_path, "rb");
        if (!fp) {
            perror("打开PNG文件失败");
            return -1;
        }

        // 初始化spng上下文（使用文件流模式，更节省内存）
        spng_ctx* ctx = spng_ctx_new(0);
        if (!ctx) {
            fclose(fp);
            fprintf(stderr, "创建spng上下文失败\n");
            return -1;
        }

        // 将spng上下文绑定到文件流
        if (spng_set_png_file(ctx, fp) != SPNG_OK) {
            spng_ctx_free(ctx);
            fclose(fp);
            fprintf(stderr, "绑定PNG文件流失败\n");
            return -1;
        }

        // 1. 获取IHDR块（宽、高、颜色类型等基础信息）
        struct spng_ihdr ihdr;
        if (spng_get_ihdr(ctx, &ihdr) != SPNG_OK) {
            spng_ctx_free(ctx);
            fclose(fp);
            fprintf(stderr, "读取IHDR信息失败\n");
            return -1;
        }
        *out_width = ihdr.width;
        *out_height = ihdr.height;

        // 2. 根据color_type推导通道数
        switch (ihdr.color_type) {
        case SPNG_COLOR_TYPE_GRAY:
            *out_channels = 1;  // 灰度图，1通道
            break;
        case SPNG_COLOR_TYPE_RGB:
            *out_channels = 3;  // RGB，3通道
            break;
        case SPNG_COLOR_TYPE_PALETTE:
            *out_channels = 3;  // 调色板图，映射为RGB（3通道）
            break;
        case SPNG_COLOR_TYPE_GRAY_ALPHA:
            *out_channels = 2;  // 灰度+Alpha，2通道
            break;
        case SPNG_COLOR_TYPE_RGB_ALPHA:
            *out_channels = 4;  // RGBA，4通道
            break;
        default:
            fprintf(stderr, "不支持的PNG颜色类型: %d\n", ihdr.color_type);
            *out_channels = -1;
            spng_ctx_free(ctx);
            fclose(fp);
            return -1;
        }

        // 释放资源
        spng_ctx_free(ctx);
        fclose(fp);
        return 0;
    }

    typedef enum {
        kTypeNone,
        kTypeJpeg,
        kTypePng
    } ImageType;
    /**
     * @brief 获取图片的类型
     * @param image_name 带后缀的图片名称，图片类型通过后缀判断
     */
    static ImageType GetImageType(const std::string& image_name) {
        std::string::size_type found = image_name.find_last_of(".");
        if (image_name.npos == found) {
            LOGW("Found image name failed. %s", image_name.c_str());
            return kTypeNone;
        }
        std::string type = image_name.substr(found + 1);

        // 将后缀名转小写
        type = StringHelper::ToLowerCase(type);

        // JPEG格式
        if (type == "jpg" || type == "jpeg") {
            return kTypeJpeg;
        }
        else if (type == "png") {
            return kTypePng;
        }

        LOGW("Unsolved type:%s", type.c_str());
        return kTypeNone;
    }

    /**
     * @brief RGB、RGBA转为BGR、BGRA（stb库解析的数据为RGB、RGBA）
     * @param data 图像数据
     * @param width 图像宽度
     * @param height 图像高度
     * @param channels 图像通道
     */
    static void RgbToBgr(uint8_t* data, int width, int height, int channels) {
        if (channels < 3) return;  // 至少需要 RGB 三通道

        // 遍历每个像素，交换 R 和 B 通道
        for (int i = 0; i < width * height; ++i) {
            int idx = i * channels;
            unsigned char temp = data[idx];       // 保存 R 通道
            data[idx] = data[idx + 2];            // B 通道移到 R 位置
            data[idx + 2] = temp;                 // 原 R 通道移到 B 位置
            // G 通道位置不变（idx + 1）
        }
    }


    /**
     * @brief 解析图片数据
     * @param image_path 文件路径
     * @return 返回图片数据指针，nullptr 解析图片失败
     */
    static ImageDes ReadImage(const std::string& image_path) {
        // 宽度
        int width{ 0 };
        // 高度
        int height{ 0 };
        // 通道数
        int channels{ 0 };

        // 判断是否为支持的格式
        ImageType type = GetImageType(image_path);
        if (kTypeNone == type) {
            return nullptr;
        }
        
        uint8_t* data{ nullptr };
        // 解析JPEG/JPG图片
        if (kTypeJpeg == type) {
            //通过 stbi_load 的最后一个参数，可以强制输出格式：
            //    0：自动推断（默认）
            //    3：强制 RGB 格式（丢弃 Alpha 通道）
            //    4：强制 RGBA 格式（补 Alpha 通道）
            data = stbi_load(image_path.c_str(), &width, &height, &channels, 0);
            // 获取图片失败，直接返回
            if (0 == width || 0 == height) {
                LOGW("Failed to load image:%s", image_path.c_str());
                return nullptr;
            }
        }
        else if (kTypePng == type) {
            // 获取PNG图片位深
            if (0 != get_png_info(image_path.c_str(), (uint32_t*)&width, (uint32_t*)&height, &channels)) {
                LOGI("Failed to get png color type:%s", image_path.c_str());
                return nullptr;
            }
            if (0 != parse_png_image(image_path.c_str(), &data, (uint32_t*)&width, (uint32_t*)&height, &channels,
                (3 == channels) ? PNG_FORMAT_RGB_24BIT : PNG_FORMAT_RGBA_32BIT)) {
                LOGI("Failed to parse png image:%s", image_path.c_str());
                return nullptr;
            }
        }
        // RGB、RGBA转为BGR、BGRA
        RgbToBgr(data, width, height, channels);
        
        lv_image_dsc_t* img_dsc = new lv_image_dsc_t;
        memset(img_dsc, 0, sizeof(lv_image_dsc_t));
        img_dsc->header.cf = (3 == channels) ? LV_COLOR_FORMAT_RGB888 : LV_COLOR_FORMAT_ARGB8888;
        img_dsc->header.magic = LV_IMAGE_HEADER_MAGIC;
        img_dsc->header.w = width;
        img_dsc->header.h = height;
        img_dsc->header.stride = width * channels;
        img_dsc->data_size = width * height * channels;
        img_dsc->data = data;
        
        return (ImageDes)img_dsc;
    }
}

ImageDes ImageIns::ParseImage(const std::string& image_path) {
    auto iter = m_cache.find(image_path); 
    if (iter != m_cache.end()) {
        return iter->second;
    }
    return ReadImage(image_path);
}

void ImageIns::DestroyImageDes(ImageDes des) {
    if (!des || m_image_cache.end() != m_image_cache.find(des)) {
        return;
    }
    lv_image_dsc_t* descript = static_cast<lv_image_dsc_t*>(des);
    // 释放图像数据
    free((void*)descript->data);
    // 释放lv_image_dsc_t结构体
    delete descript;
    des = nullptr;
}

void ImageIns::CacheImage(const std::string& image_path) {
    // 已缓存，则不再次缓存
    auto iter = m_cache.find(image_path);
    if (iter != m_cache.end()) {
        return;
    }
    auto image = ReadImage(image_path);
    if (image) {
        m_cache[image_path] = image;
        m_image_cache.insert(image);
    }
}

void ImageIns::RemoveCacheImage(const std::string& image_path) {
    auto iter = m_cache.find(image_path);
    if (iter != m_cache.end()) {
        lv_image_dsc_t* descript = static_cast<lv_image_dsc_t*>(iter->second);
        // 释放图像数据
        free((void*)descript->data);
        // 释放lv_image_dsc_t结构体
        delete descript;

        m_image_cache.erase(iter->second);
        m_cache.erase(image_path);
    }
}

int ImageIns::GetImageCount() {
    return m_cache.size();
}

std::map<std::string, ImageDes> ImageIns::m_cache;
std::set<ImageDes> ImageIns::m_image_cache;
