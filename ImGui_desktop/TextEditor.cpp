#include <algorithm>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>

#include <fmt/format.h>
#include <imgui.h>
#include <implot.h>

#include "TextEditor.hpp"
namespace fs = std::filesystem;

void TextEditor::Draw(std::string_view label, bool *open)
{
    ImGui::SetNextWindowPos(s_mainWindowPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(s_mainWindowSize, ImGuiCond_Always);

    ImGui::Begin(label.data(),
                 open,
                 s_mainWindowFlags | ImGuiWindowFlags_MenuBar);

    SettingsMenuBar();
    DrawMenu();
    DrawContent();
    DrawInfo();

    ImGui::End();
}

void TextEditor::DrawMenu()
{
    const auto ctrl_pressed = ImGui::GetIO().KeyCtrl;
    const auto s_pressed = ImGui::IsKeyPressed(ImGuiKey_S);
    const auto l_pressed = ImGui::IsKeyPressed(ImGuiKey_L);

    if (ImGui::Button("Save") || (ctrl_pressed && s_pressed))
    {
        ImGui::OpenPopup("Load File");
    }

    ImGui::SameLine();

    if (ImGui::Button("Load") || (ctrl_pressed && l_pressed))
    {
        ImGui::OpenPopup("Save File");
    }

    ImGui::SameLine();

    if (ImGui::Button("Clear"))
    {
        std::memset(textBuffer_, 0, s_bufferSize);
    }

    DrawSavePopup();
    DrawLoadPopup();
}

void TextEditor::DrawSavePopup()
{
    static char saveFilenameBuffer[256] = "text.txt";
    const auto esc_pressed = ImGui::IsKeyPressed(ImGuiKey_Escape);
    ImGui::SetNextWindowSize(s_popUpSize);
    ImGui::SetNextWindowPos(s_popUpPos);

    if (ImGui::BeginPopupModal("Save File", nullptr, s_popUpFlags))
    {
        // 输入保存的文件名
        ImGui::InputText("Filename",
                         saveFilenameBuffer,
                         sizeof(saveFilenameBuffer));
        // 点击 "Save" 按钮时：保存文件，并更新当前文件名，关闭弹窗
        if (ImGui::Button("Save", s_popUpButtonSize))
        {
            SaveToFile(saveFilenameBuffer);
            currentFilename_ = saveFilenameBuffer;
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();
        // 点击 "Cancel" 或按下 ESC：关闭弹窗
        if (ImGui::Button("Cancel", s_popUpButtonSize) || (esc_pressed))
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void TextEditor::DrawLoadPopup()
{
    static char loadFilenameBuffer[256] = "text.txt";
    const auto esc_pressed = ImGui::IsKeyPressed(ImGuiKey_Escape);

    ImGui::SetNextWindowSize(s_popUpSize);
    ImGui::SetNextWindowPos(s_popUpPos);

    if (ImGui::BeginPopupModal("Load File", nullptr, s_popUpFlags))
    {
        // 输入要加载的文件名
        ImGui::InputText("Load",
                         loadFilenameBuffer,
                         sizeof(loadFilenameBuffer));
        // 点击 "Load" 按钮时：加载文件内容到缓冲区，更新当前文件名，关闭弹窗
        if (ImGui::Button("Load", s_popUpButtonSize))
        {
            LoadFromFile(loadFilenameBuffer);
            currentFilename_ = loadFilenameBuffer;
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();
        // 点击 "Cancel" 或按下 ESC：关闭弹窗
        if (ImGui::Button("Cancel", s_popUpButtonSize) || esc_pressed)
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}


void TextEditor::DrawContent()
{
    // 编辑区域大小
    static constexpr auto inputTextSize = ImVec2(1200.0f, 625.0f);
    // 行号区域大小（固定宽度 30，和输入区高度一致）
    static constexpr auto lineNumberSize = ImVec2(30.0f, inputTextSize.y);
    // 输入框标志：允许 Tab 输入，不允许水平滚动（文本自动换行）
    static constexpr auto inputTextFlags =
        ImGuiInputTextFlags_AllowTabInput |
        ImGuiInputTextFlags_NoHorizontalScroll;

    ImGui::BeginChild("LineNumbers", lineNumberSize);

    // 统计当前缓冲区的行数（换行符个数 + 1）
    const auto line_count =
        std::count(textBuffer_, textBuffer_ + s_bufferSize, '\n') + 1;
    for (auto i = 0; i < line_count; ++i)
        ImGui::Text("%d", i);

    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::InputTextMultiline("###inputField",
                              textBuffer_,
                              s_bufferSize,
                              inputTextSize,
                              inputTextFlags);
}

void TextEditor::DrawInfo()
{
    if (currentFilename_.size() == 0)
    {
        ImGui::Text("No File Opened!");
        return;
    }

    const auto file_extension = GetFileExtension(currentFilename_);
    ImGui::Text("Open file %s | File extension %s",
                currentFilename_,
                file_extension.data());
}


void TextEditor::SaveToFile(std::string_view filename)
{
    auto out = std::ofstream{filename.data()};

    if (out.is_open())
    {
        out << textBuffer_;
        out.close();
    }
}

void TextEditor::LoadFromFile(std::string_view filename)
{
    auto in = std::ifstream{filename.data()};

    if (in.is_open())
    {
        auto buffer = std::stringstream{};
        buffer << in.rdbuf();
        std::memcpy(textBuffer_, buffer.str().data(), s_bufferSize);

        in.close();
    }
}


std::string TextEditor::GetFileExtension(std::string_view filename)
{
    const auto file_path = fs::path{filename};

    return file_path.extension().string();
}
