#####
- This project is for personal learning and resume/portfolio purposes only.
- The source code was written/recreated by me as a learning exercise, based on course materials.
- It is not for commercial use, and the original content is not distributed.
- It serves as an example for learning C++, OpenGL programing.
- It only include cpp and GLSL source code.
- 
- 本项目仅供个人学习和简历展示使用  
- 源代码是我自己根据课程内容练习学习编写/复现完成  
- 不用于商业用途，也不对外传播原始内容
- 学习 c++ 编程与计算机图形学的示例
- 项目只上传了cpp代码和shader的GLSL代码
#####

OpenGL Real-Time Rendering Demo

Overview:
This project is a real-time 3D rendering demo built with C++17 and Modern OpenGL (Core Profile).
It implements a complete rendering pipeline, including window management, camera system, lighting and shadows, skybox, and model loading.
It serves as a personal portfolio project to demonstrate proficiency in computer graphics, rendering pipeline design, and modern OpenGL programming.

Rendering Architecture: Forward Rendering pipeline built with GLEW + GLFW + GLM. Includes multiple passes (shadow mapping + main rendering).

Window & Input System: Custom Window class manages OpenGL context and mouse/keyboard input, supporting free camera movement and rotation.

Camera System: FPS-style camera based on glm::lookAt, controlled via WASD + mouse movement
Lighting System: Supports Directional, Point, and Spot Lights — each managed via uniform buffer control in shaders

Shadow Mapping:
- Directional Light: standard 2D depth map shadows.
- Point/Spot Light: omnidirectional shadows using cube map depth textures
  
Mesh & Model Loading: Supports .obj model loading through Model and Mesh abstraction layers

Materials & Textures: Custom Material + Texture system with diffuse/specular maps and reflection control

Skybox: Implements cube map rendering for a realistic 360° environmental background

Animated Models: Models (e.g., helicopters) can rotate dynamically to simulate motion
Normal Calculation: Implements vertex smoothing via calcAverageNormals() for smooth shading

Technical Highlights:
 - Written in C++17 using OpenGL 4.6 Core Profile
 - Multi-shader management system (Scene + Shadow shaders)
 - Shadow rendering via Frame Buffer Objects (FBO)
 - All math handled with GLM (matrix, vector, quaternion operations)
 - Modular OOP architecture with dedicated classes for Shader, Camera, Light, Texture, and Model
 - Supports multiple dynamic light sources with shadows

[1] Shadow Map Pass

    ├── DirectionalShadowMapPass() → render depth map for directional lights
    
    └── OmniShadowMapPass() → render cube depth maps for point/spot lights

[2] Main Render Pass

    ├── Render skybox
    
    ├── Bind shadow maps to texture units
    
    └── Render all meshes/models with lighting and materials

Technology Stack
 - Language: C++17
 - Graphics API: OpenGL 4.x Core
 - Libraries Used:
 - GLEW – OpenGL extension loader
 - GLFW – window & input management
 - GLM – math library
 - stb_image – texture loading
 - Assimp – model importing


项目简介:
本项目是一个基于 C++17 + Modern OpenGL (Core Profile) 的实时三维渲染 Demo，完整实现了从窗口初始化、相机控制、光照与阴影系统到天空盒与模型加载的完整渲染流程。
支持 平行光阴影映射、点光/聚光全向阴影映射、PBR风格材质反射、以及 动态相机与模型动画

渲染架构 : 使用 "Forward Rendering" 管线，基于 GLEW + GLFW + GLM 实现；每帧执行多通道渲染（阴影 Pass + 主渲染 Pass）。

窗口与输入系统 : 封装 Window 类管理 OpenGL 上下文、输入事件（键鼠控制），支持相机自由移动与视角旋转。

相机系统 : 基于 glm::lookAt 实现第一人称相机，支持 WASD + 鼠标控制视角，实时更新 View 矩阵。

光照系统 : 实现三类光源：DirectionalLight / PointLight / SpotLight；每类光源拥有独立 Shader Uniform 管理。

阴影映射 (Shadow Mapping) : Directional Light: 深度贴图阴影; Point/Spot Light: 使用立方体深度贴图 (CubeMap Depth) 实现全向阴影。

Mesh & Model 加载 : 支持 .obj 模型加载 (Model 类)，并可通过 Mesh 类统一渲染接口。

材质与纹理 : 基于 Texture + Material 类实现漫反射 / 镜面反射控制，可加载 RGBA 纹理贴图。

天空盒 (Skybox) : 使用 CubeMap 实现 360° 环境贴图，提供逼真的背景环境光照。

动态模型动画 :  模型（如直升机）支持绕轴旋转动画，实现基础场景动态效果。

法线计算 : 实现顶点平均法线计算函数 calcAverageNormals()，支持光滑着色 (smooth shading)。

使用 现代 OpenGL Core Profile (GL 4.6) 编程接口；
 - 实现了 多 Shader 管理系统（主场景 + 阴影 Shader）；
 - 使用 Frame Buffer Object (FBO) 渲染阴影贴图；
 - 基于 glm 完成所有矩阵/向量数学运算；
 - 实现自定义相机类与光源类；
 - 支持 多光源组合照明 与 实时阴影更新；
 - 模块化 C++ 设计（独立类文件：Shader, Mesh, Camera, Light, Texture, Model 等）


