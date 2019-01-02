class DepthMap
{
public:
	DepthMap();
	void renderMap(Shader & shader);
	void renderScene(const Shader & shader);
	~DepthMap();

	unsigned int depthMapFBO;     //帧缓存对象
	unsigned int textureID;        //深度缓存纹理
	const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
	glm::mat4 lightProjection, lightView;        //光源的透视矩阵，光源的观察矩阵
	glm::mat4 lightSpaceMatrix;                  //光空间变换矩阵
	float near_plane = 1.0f, far_plane = 30.0f;   //投影的最近到最远的距离
	Cube cube;
	glm::mat4 bodyModel;
	unsigned int textureHand;
	unsigned int textureBody;
	unsigned int textureLeg;
};

DepthMap::DepthMap()
{
	// 所需纹理载入
	textureHand = loadTexture("resources/textures/hand.jpeg");
	textureBody = loadTexture("resources/textures/body.jpg");
	textureLeg  = loadTexture("resources/textures/leg.jpg");

	// 创建帧深度对象缓存实现阴影
	//存在纹理中的所有这些深度值
	glGenFramebuffers(1, &depthMapFBO);  // 创建一个帧缓存对象
	glGenTextures(1, &textureID);         // 创建深度缓存纹理

	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glBindTexture(GL_TEXTURE_2D, textureID);

	// GL_DEPTH_COMPONENT 是深度纹理信息
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };  //纹理的边界设置为1.0，黑色 
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	
	// 绑定深度缓存纹理，也就是帧缓存对象的深度信息将存在 depthMap 上。
	// 当前的帧缓存存储了depthMap的纹理信息
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, textureID, 0);
	// 下面两句告诉Opengl我们不需要颜色数据，只要深度信息
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);  //解绑
}

inline void DepthMap::renderMap(Shader & shader)
{
	//投影方式
	//lightProjection = glm::perspective(glm::radians(45.0f), (GLfloat)SHADOW_WIDTH / (GLfloat)SHADOW_HEIGHT, near_plane, far_plane); // note that if you use a perspective projection matrix you'll have to change the light position as the current light position isn't enough to reflect the whole scene
	lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
	lightView = glm::lookAt(lightPos, glm::vec3(1.0f, 0.0f, 24.0), glm::vec3(0.0, 1.0, 0.0));  //光源看向场景中央，所以我们的这个角度就可以生成一个深度纹理
	lightSpaceMatrix = lightProjection * lightView;  //投影*观察
	// 从光的视图进行渲染画面
	shader.use();
	shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
	//一定要加glViewport，才能保证视口的阴影不会太小或者不完整
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);   //将光源视图的深度信息存到depthMapFBO中
	glClear(GL_DEPTH_BUFFER_BIT);                     //清除深度缓存

	renderScene(shader);                              //传入着色器进行渲染
	glBindFramebuffer(GL_FRAMEBUFFER, 0);             //对深度帧缓存对象进行解绑
	
	// 光视图的深度信息搞定后视口要切换到原来的样子，渲染实物
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

inline void DepthMap::renderScene(const Shader & shader)  //绘制机器人
{
	
	// 主干矩阵 （后面全部东西都是以这个矩阵层次建模）
	bodyModel = glm::mat4(1.0f);
	// 下面是判断机器人上热气球还是走路
	if(balloonMode) bodyModel = glm::translate(bodyModel, glm::vec3(camera.Position.x, camera.Position.y - 4.0, camera.Position.z));
	else bodyModel = glm::translate(bodyModel, glm::vec3(robot_x, 0.0f, robot_z));
	
	bodyModel = glm::rotate(bodyModel, glm::radians(robot_turn_front), glm::vec3(0.0, 1.0, 0.0));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureBody);
	// 身体
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0, 1.0, 0.0));
	model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.25f));
	model = bodyModel * model;
	cube.draw(shader, model);

	glBindTexture(GL_TEXTURE_2D, textureHand);
	// 头部
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0, 1.75, 0.0));
	model = glm::scale(model, glm::vec3(0.25f, 0.25f, 0.25f));
	model = bodyModel * model;
	cube.draw(shader, model);

	glBindTexture(GL_TEXTURE_2D, textureLeg);
	//右上肢
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.65, 1.00, 0.0));

	//----------------------右上肢转动
	model = glm::translate(model, glm::vec3(0.0, 0.5, 0.0));
	model = glm::rotate(model, glm::radians(robot_rotate), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::translate(model, glm::vec3(0.0, -0.5, 0.0));
	//-----------------------

	model = glm::scale(model, glm::vec3(0.15f, 0.5f, 0.15f));
	model = bodyModel * model;
	cube.draw(shader, model);

	// 左上肢
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-0.65, 1.00, 0.0));

	//----------------------左上肢转动
	model = glm::translate(model, glm::vec3(0.0, 0.5, 0.0));
	model = glm::rotate(model, glm::radians(-robot_rotate), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::translate(model, glm::vec3(0.0, -0.5, 0.0));
	//-----------------------


	model = glm::scale(model, glm::vec3(0.15f, 0.5f, 0.15f));
	model = bodyModel * model;
	cube.draw(shader, model);

	// 右下肢
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.3, 0.0, 0.0));

	//----------------------右下肢转动
	model = glm::translate(model, glm::vec3(0.0, 0.5, 0.0));
	model = glm::rotate(model, glm::radians(-robot_rotate), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::translate(model, glm::vec3(0.0, -0.5, 0.0));
	//-----------------------
	
	model = glm::scale(model, glm::vec3(0.15f, 0.5f, 0.15f));
	model = bodyModel * model;
	cube.draw(shader, model);

	// 左下肢
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-0.3, 0.0, 0.0));

	//----------------------左下肢转动
	model = glm::translate(model, glm::vec3(0.0, 0.5, 0.0));
	model = glm::rotate(model, glm::radians(robot_rotate), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::translate(model, glm::vec3(0.0, -0.5, 0.0));
	//-----------------------

	model = glm::scale(model, glm::vec3(0.15f, 0.5f, 0.15f));
	model = bodyModel * model;
	cube.draw(shader, model);

}

DepthMap::~DepthMap()
{
	glDeleteFramebuffers(1, &depthMapFBO);
	glDeleteTextures(1, &textureID);
}