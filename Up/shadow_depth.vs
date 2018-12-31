#version 330 core
layout (location = 0) in vec3 aPos;  //顶点位置信息

uniform mat4 lightSpaceMatrix;       //光空间变化矩阵
uniform mat4 model;                  //模型的变换矩阵

void main()
{
    gl_Position = lightSpaceMatrix * model * vec4(aPos, 1.0);
}