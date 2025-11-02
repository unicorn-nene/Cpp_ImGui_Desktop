#include <algorithm>
#include <array>
#include <chrono>
#include <compare>
#include <cstdint>
#include <fstream>
#include <map>
#include <string_view>
#include <vector>

#include <fmt/format.h>
#include <imgui.h>
#include <implot.h>

#include "Calender.hpp"

void Calender::Draw(std::string_view label, bool *open)
{
    ImGui::SetNextWindowPos(s_mainWindowPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(s_mainWindowSize, ImGuiCond_Always);

    ImGui::Begin(label.data(), open, s_mainWindowFlags);

    DrawDateCombo();
    ImGui::Separator();

    DrawCalender();
    ImGui::Separator();

    DrawMeetingList();

    if (addMeetingWindowOpen_)
        DrawAddMeetingWindow();

    ImGui::End();
}

void Calender::DrawDateCombo()
{
    //1.三个下拉框:分别选择 年/月/日
    ImGui::Text("Select a date:");

    // 绘制 日期 选择框
    ImGui::PushItemWidth(50);
    if (ImGui::BeginCombo("##day", std::to_string(selectedDay_).data()))
    {
        // 遍历1~31日
        for (std::int32_t day_idx = 1; day_idx <= 32; ++day_idx)
        {
            // 组合成完成日期
            const auto curr_date =
                std::chrono::year_month_day(std::chrono::year(selectedYear_),
                                            std::chrono::month(selectedMonth_),
                                            std::chrono::day(day_idx));
            // 检查日期是否合法
            if (!curr_date.ok())
                break;

            // 绘制可选项;如果用户选中某一天,就更新 selectedDay 和 selectedDate
            // 视觉效果:
            // 下拉框打开后会看到 1~31 的列表（实际只显示合法日期)
            // 当 day_idx == selectedDay_ 时,该选项会有高亮显示
            // 点击其他天数，高亮移动到新选择的那一天，同时更新 selectedDay_ 和 selectedDate_
            if (ImGui::Selectable(std::to_string(day_idx).data(),
                                  day_idx == selectedDay_))
            {
                selectedDay_ = day_idx;
                selectedDate_ = curr_date;
            }
        }
        ImGui::EndCombo();
    }

    ImGui::PopItemWidth();
    ImGui::SameLine(); // 在同一行继续绘制

    // 绘制 月份 下拉框
    ImGui::PushItemWidth(100);

    if (ImGui::BeginCombo("##month", s_monthNames[selectedMonth_ - 1].data()))
    {
        //遍历 1 ~ 12月
        for (std::int32_t month_idx = 1; month_idx <= 12; ++month_idx)
        {
            const auto curr_date =
                std::chrono::year_month_day(std::chrono::year(selectedYear_),
                                            std::chrono::month(month_idx),
                                            std::chrono::day(selectedDay_));
            if (!curr_date.ok())
                break;

            if (ImGui::Selectable(s_monthNames[selectedMonth_ - 1].data(),
                                  month_idx == selectedMonth_))
            {
                selectedMonth_ = month_idx;
                selectedDate_ = curr_date;
            }
        }

        ImGui::EndCombo();
    }

    ImGui::PopItemWidth();
    ImGui::SameLine();

    // 绘制 年份 下拉框
    ImGui::PushItemWidth(60);

    // 当前显示选中的年份
    if (ImGui::BeginCombo("##year", std::to_string(selectedYear_).data()))
    {
        for (std::int32_t year_idx = s_minYear; year_idx <= s_maxYear;
             ++year_idx)
        {
            const auto curr_date =
                std::chrono::year_month_day(std::chrono::year(year_idx),
                                            std::chrono::month(selectedMonth_),
                                            std::chrono::day(selectedDay_));

            if (!curr_date.ok())
                break;

            if (ImGui::Selectable(std::to_string(year_idx).data(),
                                  selectedYear_ == year_idx))
            {
                selectedYear_ = year_idx;
                selectedDate_ = curr_date;
            }
        }

        ImGui::EndCombo();
    }

    ImGui::PopItemWidth();

    // 添加会议按钮
    if (ImGui::Button("Add Meeting"))
        addMeetingWindowOpen_ = true;
}

void Calender::DrawCalender()
{
    //1.创建一个子窗口,用于显示整个日历
    const auto child_size = ImVec2(ImGui::GetContentRegionAvail().x, 400.0f);

    ImGui::BeginChild("###calender", child_size, false);

    //2.保存原来的字体大小,之后设置日历字体大小
    const auto original_font_size = ImGui::GetFontSize();
    ImGui::SetWindowFontScale(calenderFontSize_);

    //3.获取今天的日期
    const auto curr_time =
        std::chrono::floor<std::chrono::days>(std::chrono::system_clock::now());
    const auto todays_date = std::chrono::year_month_day(curr_time);

    const auto y = selectedYear_;

    //4.遍历12个月
    for (std::int32_t m = 1; m <= 12; ++m)
    {
        ImGui::Text("%s", fmt::format("{:.3}", s_monthNames[m - 1].data()));

        // 遍历 1 ~ 31 日期
        for (std::int32_t d = 1; d <= 31; ++d)
        {
            const auto iteration_date =
                std::chrono::year_month_day(std::chrono::year(y),
                                            std::chrono::month(m),
                                            std::chrono::day(d));

            if (!iteration_date.ok())
                break;

            ImGui::SameLine();

            // 根据不同条件显示不同颜色
            if (iteration_date == todays_date)
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%d", d);
            if (iteration_date == selectedDate_)
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "%d", d);
            if (meetings_.contains(iteration_date))
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "%d", d);
            else
                ImGui::Text("%d", d);

            // 处理鼠标点击事件
            // 用户点击某一天,更新 selectedDate, 并同步更新相关变量
            if (ImGui::IsItemClicked())
            {
                selectedDate_ = iteration_date;
                UpdateSelectedDateVariables();
            }
        }
    }

    ImGui::SetWindowFontScale(original_font_size);

    ImGui::EndChild();
}

void Calender::DrawAddMeetingWindow()
{
    // 定义一个静态字符缓冲区,用来存放会议名称
    static char meeting_name_buffer[128] = "...";

    // 设置窗口大小和位置
    ImGui::SetNextWindowSize(s_meetingWindowSize);
    ImGui::SetNextWindowPos(s_meetingWindowPos);

    // 打开 "add meeting" 窗口
    ImGui::Begin("###addMeeting", &addMeetingWindowOpen_, s_meetingWindowFlags);

    // 显示提示文本,说明要添加会议的日期
    ImGui::Text("Add meeting to %d.%s.%d",
                selectedDay_,
                s_monthNames[selectedMonth_ - 1].data(),
                selectedYear_);
    // 输入框,用户输入会议名称
    ImGui::InputText("Meeting Name",
                     meeting_name_buffer,
                     sizeof(meeting_name_buffer));

    // "save" 按钮
    if (ImGui::Button("Save"))
    {
        meetings_[selectedDate_].push_back(Meeting{meeting_name_buffer});

        std::memset(meeting_name_buffer, 0, sizeof(meeting_name_buffer));

        addMeetingWindowOpen_ = false;
    }

    // "Cancel" 按钮
    ImGui::SameLine();
    if (ImGui::Button("Cancel"))
    {
        addMeetingWindowOpen_ = false;
    }

    ImGui::End();
}

void Calender::DrawMeetingList()
{
    // 如果没有会议,直接提示
    if (!meetings_.size())
    {
        ImGui::Text("No meeting at all.");
        return;
    }

    // 显示当前选中日期的信息
    ImGui::Text("Meeting on %d.%s,%d: ",
                selectedDay_,
                s_monthNames[selectedMonth_ - 1].data(),
                selectedYear_);

    // 如果当前日期没有会议 -> 提示
    if (!meetings_.contains(selectedDate_))
    {
        ImGui::Text("No meetings for this day.");
        return;
    }

    // 遍历当前日期的会议
    for (const auto &meeting : meetings_[selectedDate_])
    {
        // 显示会议名称(带小圆点的条目)
        ImGui::BulletText("%s", meeting.name_.data());

        // 用户点击该会议条目 -> 删除会议
        if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
        {
            std::erase(meetings_[selectedDate_], meeting);
            if (meetings_[selectedDate_].size() == 0)
                meetings_.erase(selectedDate_);

            return;
        }
    }
}


void Calender::SaveMeetingsToFile(std::string_view filename)
{
    auto out = std::ofstream(filename.data(), std::ios::binary);

    if (!out || !out.is_open())
        return;

    // 写入 会议数量
    const auto meetings_count = meetings_.size();
    out.write(reinterpret_cast<const char *>(&meetings_count),
              sizeof(meetings_count));

    //遍历每个日期和对应会议列表
    for (const auto &[date, meeting_vec] : meetings_)
    {
        // 写入/序列化 日期
        out.write(reinterpret_cast<const char *>(&date), sizeof(date));

        // 写入/序列化该日期下的会议数量
        const auto meetings_count_on_that_date = meeting_vec.size();
        out.write(reinterpret_cast<const char *>(&meetings_count_on_that_date),
                  sizeof(meetings_count_on_that_date));

        // 写入/序列化每个会议对象
        for (const auto &meeting : meeting_vec)
        {
            meeting.Serialize(out);
        }

        out.close();
    }
}

void Calender::LoadMeetingsFromFile(std::string_view filename)
{
    auto in = std::ifstream(filename.data(), std::ios::binary);

    if (!in || !in.is_open())
        return;

    // 读取会议总数
    auto num_meetings = std::size_t{0};
    in.read(reinterpret_cast<char *>(&num_meetings), sizeof(num_meetings));

    // 循环读取每个日期以及对应的会议
    for (std::size_t i = 0; i < num_meetings; ++i)
    {
        // 读取 当前日期
        auto date = std::chrono::year_month_day{};
        in.read(reinterpret_cast<char *>(&date), sizeof(date));

        // 读取当前日期的 会议数量
        auto num_meeting_in_that_date = std::size_t{0};
        in.read(reinterpret_cast<char *>(&num_meeting_in_that_date),
                sizeof(num_meeting_in_that_date));

        for (std::size_t i = 0; i < num_meeting_in_that_date; ++i)
        {
            auto meeting = Meeting::Deserialize(in);
            meetings_[date].push_back(meeting);
        }
    }

    in.close();
}

void Calender::Meeting::Serialize(std::ofstream &out) const
{
    // 序列化 会议名长度
    const auto name_length = name_.size();
    out.write(reinterpret_cast<const char *>(&name_length),
              sizeof(name_length));

    // 写入会议内容
    out.write(name_.data(), static_cast<std::streamsize>(name_.size()));
}

Calender::Meeting Calender::Meeting::Deserialize(std::ifstream &in)
{
    auto meeting = Meeting{};
    auto name_length = std::size_t{0};

    in.read(reinterpret_cast<char *>(&name_length), sizeof(name_length));

    meeting.name_.resize(name_length);
    in.read(meeting.name_.data(),
            static_cast<std::streamsize>(meeting.name_.size()));

    return meeting;
}

void Calender::UpdateSelectedDateVariables()
{
    // std::chrono::day/month/day 本质是 unsigned int类型,但是只提供了一个
    // 显式转换成 unsigned int 的显式转换操作.
    // 必须先转换成 unsigned int 再进行其他转换操作
    selectedDay_ =
        static_cast<int>(selectedDate_.day().operator unsigned int());
    selectedMonth_ =
        static_cast<int>(selectedDate_.month().operator unsigned int());
    selectedYear_ = static_cast<int>(selectedDate_.year());
}
