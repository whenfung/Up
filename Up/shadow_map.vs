#version 330 core
layout (location = 0) in vec3 aPos;           //顶点位置信息
layout (location = 1) in vec3 aNormal;        //顶点法向
layout (location = 2) in vec2 aTexCoords;     //纹理坐标

out vec2 TexCoords;                           //传纹理坐标给片元着色器

out VS_OUT {                                  
    vec3 FragPos;                             //片元位置
    vec3 Normal;                              //片元法向
    vec2 TexCoords;                           //纹理坐标
    vec4 FragPosLightSpace;                   //片元位置在光空间的位置
} vs_out;

uniform mat4 projection;                      //透视矩阵
uniform mat4 view;                            //观察矩阵
uniform mat4 model;                           //模型矩阵
uniform mat4 lightSpaceMatrix;                //光空间的模视矩阵

void main()
{
    vs_out.FragPos = vec3(model * vec4(aPos, 1.0));               //世界坐标系上的位置
    vs_out.Normal = transpose(inverse(mat3(model))) * aNormal;    //世界坐标系上的法向
    vs_out.TexCoords = aTexCoords;                                //纹理坐标不用变
    vs_out.FragPosLightSpace = lightSpaceMatrix * vec4(vs_out.FragPos, 1.0); //世界坐标系变化到光空间系 
	
    gl_Position = projection * view * model * vec4(aPos, 1.0); //相机坐标系的位置
}