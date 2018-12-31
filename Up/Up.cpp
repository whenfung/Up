#include <up/up.h>
#include <up/skybox.h>
#include <up/floor.h>
#include <up/cube.h>
#include <up/depthMap.h>

int main()
{
	GLFWwindow* window = initWindows();
	if (window == NULL) return -1;

	// 设置Opengl的全局配置，开启深度缓存
	glEnable(GL_DEPTH_TEST);

	// 初始化着色器
	Shader skyboxShader("skybox.vs", "skybox.fs");
	Shader simpleDepthShader("shadow_depth.vs", "shadow_depth.fs");
	Shader shader("shadow_map.vs", "shadow_map.fs");  

	// 背景着色器
	skyboxShader.use();
	skyboxShader.setInt("skybox", 0);

	// 光照阴影着色器
	shader.use();
	shader.setInt("diffuseTexture", 0);   //环境贴图
	shader.setInt("shadowMap", 1);        //阴影贴图

	Skybox   skybox;       //天空
	Floor    floor;        //地板
	DepthMap depthMap;     // 光源视图深度纹理，用作阴影

	// 载入所需纹理
	unsigned int woodTexture = loadTexture("resources/textures/sufei.jpg");
	skybox.loadCubemap();

	// 循环渲染
	while (!glfwWindowShouldClose(window))
	{
		// 计算每一帧的更新速度作出相应的调整
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		//监听按键并作出相应的响应
		processInput(window);

		// change light position over time
		lightPos.x = sin(glfwGetTime()) * 3.0f;
		lightPos.z = cos(glfwGetTime()) * 2.0f;
		lightPos.y = 5.0 + cos(glfwGetTime()) * 1.0f;

		// 渲染
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //清除深度缓存和颜色缓存

		// 得到光源视图的深度信息，作为深度纹理贴图进行 阴影绘制。
		depthMap.renderMap(simpleDepthShader);  //生成深度纹理

		// 像往常一样渲染场景，只不过多了个深度纹理 
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shader.use();
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		shader.setMat4("projection", projection);
		shader.setMat4("view", view);
		// 设置光源信息和相机信息
		shader.setVec3("viewPos", camera.Position);
		shader.setVec3("lightPos", lightPos);
		shader.setMat4("lightSpaceMatrix", depthMap.lightSpaceMatrix);
			
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, depthMap.textureID);
		floor.draw(shader);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, woodTexture);
		depthMap.renderScene(shader);      //渲染场景
		skybox.draw(skyboxShader);         //渲染天空图

		// 更新缓存和IO事件
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	//清除glfw初始化的东西
	glfwTerminate();
	return 0;
}
