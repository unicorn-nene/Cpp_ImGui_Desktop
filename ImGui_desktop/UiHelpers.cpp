#include <cstdint>
#include <exception>
#include <iostream>
#include <string_view>
#include <tuple>
#include <vector>

#include "lodepng.h"
#include <GLFW/glfw3.h>
#include <fmt/format.h>
#include <imgui.h>

#include "UiHelpers.hpp"

/**
 * @brief  从指定文件加载PNG图像并生成 OpenGL 纹理
 *
 * 此函数使用 loadpng 库 对 PNG 文件进行解码,然后通过OpenGL
 * 创建一个 2D 纹理对象,并将解码后的图像数据上传到GPU
 *
 * @param filename 图像文件路径(c 字符串)
 * @return std::tuple<GLuint, std::uint32_t, std::uint32_t>
 * - GLuint: 生成的 OpenGL 纹理 ID
 * - std::uint32_t : 图像宽度
 * - std::uint32_t : 图像高度
 *
 */
std::tuple<GLuint, std::uint32_t, std::uint32_t> loadTexture(
    const char *filename)
{
    // 使用lodepng 库解码 PNG 文件为 RGBA 数据
    std::vector<unsigned char> data;
    std::uint32_t width = 0U;
    std::uint32_t height = 0U;
    const auto error = lodepng::decode(data, width, height, filename);

    // 如果解码失败,抛出异常
    if (error)
        throw "Error loading image";

    // 生成并绑定一个新的 OpenGL 纹理对象
    GLuint texture = 0U;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // 设置纹理的包裹和过滤参数
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP); // s轴 -> X轴
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP); // T轴 -> Y轴
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //缩小
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //放大

    // 将 RGBA 数据上传至 GPU
    glTexImage2D(GL_TEXTURE_2D,    //
                 0,                // mipmap level
                 GL_RGBA,          // 内部格式
                 width,            // 图像尺寸宽度
                 height,           // 图像尺寸高度
                 0,                // 边框(必须为0)
                 GL_RGBA,          // 数据格式
                 GL_UNSIGNED_BYTE, // 颜色通道数据类型
                 &data[0]);        // 像素数据指针


    //返回纹理ID 与尺寸信息
    return std::make_tuple(texture, width, height);
}

/**
 * @brief 加载并在 ImGui 窗口中显示一张图片
 *
 * 此函数首先调用 loadTexture() 从文件中生成 OpenGL 纹理，
 * 然后使用 ImGui::Image() 将图像绘制在当前窗口中。
 * 若加载过程发生异常（如文件不存在或解码失败），则会输出错误并终止程序。
 *
 * @param image_filepath 图像文件路径(std::string_view)
 */
void LoadAndDisplayImage(std::string_view image_filepath)
{
    try
    {
        // 调用 loadTexture() 加载图像数据
        const auto &[myImageTexture, imageWidth, imageHeight] =
            loadTexture(image_filepath.data());

        // 将像素尺寸转换为 ImGui 可接受的向量类型
        const auto imageSize = ImVec2(static_cast<float>(imageWidth),
                                      static_cast<float>(imageHeight));

        // 在当前 ImGui 窗口绘制图像
        ImTextureID tex_id = (ImTextureID)(intptr_t)myImageTexture;
        ImGui::Image(tex_id, imageSize);
    }
    catch (const std::exception &e)
    {
        // 捕获标志异常
        std::cerr << e.what() << '\n';
        exit(0);
    }
}
