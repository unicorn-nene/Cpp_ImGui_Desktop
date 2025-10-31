###
- This project is for personal learning and resume/portfolio purposes only.
- The source code was written/recreated by me as a learning exercise, based on course materials.
- It is not for commercial use, and the original content is not distributed.
- It serves as an example for learning C++, ImGui GUI programming, and event handling.
- It is intended as a portfolio piece for C++ desktop application development
- only include cpp source code.

- 本项目仅供个人学习和简历展示使用  
- 源代码是我自己根据课程内容练习学习编写/复现  
- 不用于商业用途，也不对外传播原始内容
- 学习 C++ 和 ImGui GUI 编程与事件处理的示例
- 作为 C++ 桌面程序的作品集展示
- 只上传了src文件夹的 .cpp 文件, ImGui模板与cmake模板等没有上传.
###

ImGui Theme Editor & GUI Framework (C++ / OpenGL / GLFW / ImPlot)

Overview
This project is a lightweight and extensible C++ GUI framework built on top of Dear ImGui, OpenGL, and GLFW,
providing real-time UI rendering, custom theme management, and persistent user settings.
It features a fully interactive style editor that allows users to dynamically adjust ImGui appearance parameters
such as padding, rounding, spacing, and colors, and save/load them automatically via .ini configuration files.
The program demonstrates modular GUI design, OpenGL rendering cycle management, and persistence of UI preferences,
making it a work of modern C++ and ImGui integration.

Key Features
Customizable Theme System:
 - Real-time adjustment of ImGui styles (padding, rounding, spacing, etc.)
 - Full color editor for all ImGui elements
 - Theme persistence via .ini configuration
Robust Rendering Framework:
 - Built with GLFW + OpenGL 3
 - Frame cycle abstraction (start_cycle / end_cycle)
Modular C++ Design:
 - WindowBase encapsulates UI logic and configuration persistence
 - Desktop provides the application entry point and render control
ImPlot Integration:
 - Ready for visualizing real-time or statistical data
   
Technical Highlights:
  Written in Modern C++20, using <filesystem>, constexpr, and RAII
  Uses SimpleIni for lightweight configuration serialization
  Demonstrates clean separation of logic and rendering pipeline
  Can serve as a base for future C++ desktop applications or ImGui tools



项目概述
这是一个基于 Dear ImGui + OpenGL + GLFW 构建的轻量级 C++ 图形界面框架。
项目实现了一个完整的 主题样式编辑器，允许用户在运行时实时调整 ImGui 的界面参数（如边距、圆角、间距、颜色等），并通过 .ini 文件 自动保存/加载个性化主题。
该项目展示了现代 C++ 下的 模块化 GUI 设计、OpenGL 渲染循环控制 以及 UI 设置持久化机制.

主要特性
可自定义主题系统:
 - 实时修改 ImGui 样式参数（Padding / Rounding / Spacing 等）
 - 可视化颜色编辑器，支持所有控件配色
 - 自动保存与加载 .ini 配置
完善的渲染架构:
 - 使用 GLFW + OpenGL 3
 - 独立的渲染周期函数：start_cycle() / end_cycle()
模块化 C++ 设计:
 - WindowBase 负责主题管理与界面逻辑
 - Desktop 负责程序入口与渲染主循环
ImPlot 图表支持:
 - 适合展示实时数据或统计图形

技术亮点
 完全基于 现代 C++20（使用 constexpr、filesystem 等标准特性）
 使用 SimpleIni 实现轻量配置存储
 清晰分层：界面逻辑与渲染框架解耦
 可扩展为任意 桌面工具或 GUI 编辑器
