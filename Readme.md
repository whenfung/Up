## Up

利用Opengl实现一个3D场景，综合所学知识完成一个场景。

## 实验环境

1. 使用VS2017编辑器

2. 使用以下库，利用Cmake编译成X86的release模式

   - GLFW库：专门针对OpenGL的C语言库，它提供了一些渲染物体所需的最低限度的接口。

   - GLAD库：基于Core的3.3版本、C++语言（在线生成要保证生成加载器）

   - stb_image库：用来读取纹理图片

   - glm数学矩阵库：0.9.9版本（此时默认初始化为全0矩阵）

   - assimp库：3.3.1版本，用来读取obj文件

3. lib库链接

   glfw3.lib;opengl32.lib;user32.lib;gdi32.lib;shell32.lib;

## 实现效果

