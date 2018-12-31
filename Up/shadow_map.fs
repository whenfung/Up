#version 330 core

out vec4 FragColor; //输出的颜色

in VS_OUT {
    vec3 FragPos;         // 世界坐标系上的位置
    vec3 Normal;          // 世界坐标系上的法向
    vec2 TexCoords;       // 纹理坐标
    vec4 FragPosLightSpace;  //光空间上的片元位置
} fs_in;

uniform sampler2D diffuseTexture;   //普通纹理
uniform sampler2D shadowMap;        //深度纹理

uniform vec3 lightPos;              //光源位置
uniform vec3 viewPos;               //眼睛位置

float ShadowCalculation(vec4 fragPosLightSpace)
{
    // 透视除法
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // 将（-1，1）的范围转化到（0，1）的范围
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // calculate bias (based on depth map resolution and slope)
    vec3 normal = normalize(fs_in.Normal);
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    // check whether current frag pos is in shadow
    // float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
    
    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if(projCoords.z > 1.0)
        shadow = 0.0;
        
    return shadow;
}

void main()
{           
    vec3 color = texture(diffuseTexture, fs_in.TexCoords).rgb;  //纹理颜色
    vec3 normal = normalize(fs_in.Normal);                      //单位化法向量
    vec3 lightColor = vec3(0.3);                                //光源颜色
    // ambient
    vec3 ambient = 0.3 * color;                                 //环境光
    // diffuse
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);        //漫反射
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * lightColor;
    // specular
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);     //镜面反射
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    vec3 specular = spec * lightColor;    
    // 计算阴影系数，将和前面的漫反射和镜面反射叠加形成真正的颜色
    float shadow = ShadowCalculation(fs_in.FragPosLightSpace);      //计算阴影系数                  
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;    
    
    FragColor = vec4(lighting, 1.0);  //输出颜色
}