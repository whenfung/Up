#version 330 core
out vec4 FragColor;

in vec3 TexCoords;  //3D纹理坐标的方向向量

uniform samplerCube skybox; //激活立方体贴图的纹理采样器
uniform float bili;

void main()
{    
	//将3D纹理坐标的方向向量传给纹理采样器进行纹理渲染
    FragColor = texture(skybox, TexCoords);
}