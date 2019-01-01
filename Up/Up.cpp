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
	Shader modelShader("model.vs", "model.fs");
	Shader lampShader("lamp.vs", "lamp.fs");

	// 背景着色器
	skyboxShader.use();
	skyboxShader.setInt("skybox", 0);

	// 光照阴影着色器
	shader.use();
	shader.setInt("diffuseTexture", 0);   //环境贴图
	shader.setInt("shadowMap", 1);        //阴影贴图

	modelShader.use();
	modelShader.setInt("material.diffuse", 0);

	Skybox   skybox;       //天空
	Floor    floor;        //地板
	DepthMap depthMap;     // 光源视图深度纹理，用作阴影
	Model nanosuit("resources/objects/nanosuit/nanosuit.obj");
	Model house("resources/objects/house/house.obj");
	Model dog("resources/objects/dog/12228_Dog_v1_L2.obj");
	Model balloon("resources/objects/balloon/11809_Hot_air_balloon_l2.obj");
	Model tree("resources/objects/tree/12150_Christmas_Tree_V2_L2.obj");

	// 所需纹理载入
	unsigned int woodTexture = loadTexture("resources/textures/wood.png");
	//所需天空图载入
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

		// 随时间变化
		//lightPos.x = sin(glfwGetTime()) * 3.0f;
		//lightPos.z = cos(glfwGetTime()) * 2.0f;
		//lightPos.y = 5.0 + cos(glfwGetTime()) * 1.0f;

		// 渲染
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //清除深度缓存和颜色缓存

		// 得到光源视图的深度信息，作为深度纹理贴图进行 阴影绘制。
		depthMap.renderMap(simpleDepthShader);  //生成深度纹理。
		
												
		//------------------------对shader做一些设置
		shader.use();
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		shader.setMat4("projection", projection);
		shader.setMat4("view", view);

		//给着色器传入照相机参数、光源参数、以及光空间参数
		shader.setVec3("viewPos", camera.Position);
		shader.setVec3("lightPos", lightPos);
		shader.setMat4("lightSpaceMatrix", depthMap.lightSpaceMatrix);
			
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, depthMap.textureID);

		floor.draw(shader);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, woodTexture);
		depthMap.renderScene(shader);      //渲染场景



		 // ----------------------------------------渲染OBJ文件
		modelShader.use();
		modelShader.setVec3("light.position", lightPos);
		modelShader.setVec3("viewPos", camera.Position);

		// light properties
		modelShader.setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
		modelShader.setVec3("light.diffuse", 0.5f, 0.5f, 0.5f);
		modelShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);

		// material properties
		modelShader.setVec3("material.specular", 0.5f, 0.5f, 0.5f);
		modelShader.setFloat("material.shininess", 32.0f);


		// view/projection transformations
		projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		view = camera.GetViewMatrix();
		modelShader.setMat4("projection", projection);
		modelShader.setMat4("view", view);

		// 渲染机器人
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-5.5f, -0.5f, 0.0f)); // translate it down so it's at the center of the scene
		model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));	// it's a bit too big for our scene, so scale it down
		//model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0, 0.0, 0.0));
		modelShader.setMat4("model", model);
		nanosuit.Draw(modelShader);

		// 渲染房子
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(20.0f, -0.5f, 0.0f)); // translate it down so it's at the center of the scene
		model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f));	// it's a bit too big for our scene, so scale it down
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0, 1.0, 0.0));
		modelShader.setMat4("model", model);
		house.Draw(modelShader);

		// 渲染小狗
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(1.0f, -0.5f, 11.0f)); // translate it down so it's at the center of the scene
		model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));	// it's a bit too big for our scene, so scale it down
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0, 0.0, 0.0));
		modelShader.setMat4("model", model);
		dog.Draw(modelShader);

		// 渲染圣诞树
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(20.0f, -0.5f, 24.0f)); // translate it down so it's at the center of the scene
		model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));	// it's a bit too big for our scene, so scale it down
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0, 0.0, 0.0));
		modelShader.setMat4("model", model);
		tree.Draw(modelShader);

		// 渲染热气球
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-30.0f, -0.5f, 0.0f)); // translate it down so it's at the center of the scene
		model = glm::scale(model, glm::vec3(0.02f, 0.02f, 0.02f));	// it's a bit too big for our scene, so scale it down
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0, 0.0, 0.0));
		modelShader.setMat4("model", model);
		balloon.Draw(modelShader);

		//---------------------------------将灯画出来
		// 将灯画出来
		lampShader.use();
		lampShader.setMat4("projection", projection);
		lampShader.setMat4("view", view);
		model = glm::mat4(1.0f);
		model = glm::translate(model, lightPos);
		model = glm::scale(model, glm::vec3(0.2f)); // a smaller cube
		lampShader.setMat4("model", model);
		Cube lamp;
		lamp.draw(lampShader, model);

		//---------------------------------渲染天空图
		skybox.draw(skyboxShader);         

		// 更新缓存和IO事件
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	//清除glfw初始化的东西
	glfwTerminate();
	return 0;
}
