#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture1;

void main()
{             
	//这里的TexCoords包括了透明度
    FragColor = texture(texture1, TexCoords);
}