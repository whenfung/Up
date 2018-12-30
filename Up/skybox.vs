#version 330 core
layout (location = 0) in vec3 aPos;    //顶点属性为0用aPos

out vec3 TexCoords;       //这里的纹理坐标就是正方体的坐标，这就是天空图渲染的巧妙之处

uniform mat4 projection;  //透视矩阵，增加真实感
uniform mat4 view;        //观察矩阵，让天空图对应第一视角不变，营造天空很大的感觉

void main()
{
    TexCoords = aPos;      //天空图的纹理坐标等于实际坐标
    vec4 pos = projection * view * vec4(aPos, 1.0);  //天空图没有model矩阵，让天空图随着相机动
    gl_Position = pos.xyww;    
}  