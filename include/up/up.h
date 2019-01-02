#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <up/shader.h>
#include <up/camera.h>
#include <up/model.h>

#include <iostream>

//-----------------------------------------------------------全局变量
// 设置窗口的高和宽
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// 创建相机并对象设置相机的默认参数，往后移动一点（也就是Z轴）
Camera camera(glm::vec3(0.0f, 0.0f, 7.0f));
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
bool firstMouse = true;

// 根据两个帧的渲染间隔作出相应的改变，保证画面流畅
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// 光源
glm::vec3 lightPos(3.4f, 9.6f, 16.9f);

//调节白天黑夜
float ratio = 1.0;

//机器人参数
float robot_x = 0.0;
float robot_z = 1.0;
float robot_step = 1;
float robot_rotate = 0.0;
float robot_turn_front = 0.0;

//-----------------------------------------------------------函数声明
//一些必须要用到的库的初始化
GLFWwindow* initWindows();

//监听window窗口的按键并作出响应
void processInput(GLFWwindow *window);
//监听窗口的大小变换作出视口的变化
void framebuffer_size_callback(GLFWwindow* window, int width, int height); 
//监听鼠标移动改变偏航角和俯仰角
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
//监听鼠标的滚动改变透视投影的视野
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
//载入图片并传到GPU，返回纹理对象的ID
unsigned int loadTexture(const char *path);


//-----------------------------------------------------------函数实现
inline GLFWwindow * initWindows()
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return NULL;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return NULL;
	}
	return window;
}

// 键盘监听函数
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)   //ESC退出
		glfwSetWindowShouldClose(window, true);

	//deltaTime是渲染间隔时间，用来调整速度
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)        //W是前进
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)        //S是后退
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)        //A是左移
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)        //D是右移
		camera.ProcessKeyboard(RIGHT, deltaTime);

	// 调节黑夜白天
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		if(ratio < 1.0)
			ratio += 0.05;
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		if (ratio > 0.2)
			ratio -= 0.05;

	// 机器人行走
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
		robot_turn_front = 0.0;
		robot_rotate += robot_step;
		robot_z += 0.01;
	}
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		robot_turn_front = 0.0;
		robot_rotate += robot_step;
		robot_z -= 0.01;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
		robot_turn_front = 90.0;
		robot_rotate += robot_step;
		robot_x += 0.01;
	}
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
		robot_turn_front = 90.0;
		robot_rotate += robot_step;
		robot_x -= 0.01;
	}
	if (robot_rotate > 30) robot_step = -robot_step;
	if (robot_rotate < -30) robot_step = -robot_step;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// 确保视口与新的窗口尺寸相匹配，注意不要让视口大于自己的屏幕
	glViewport(0, 0, width, height);
}

// 鼠标移动将触发这个函数
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)  //当鼠标第一次进入视口就会被捕捉
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX; //X方向的偏移量
	float yoffset = lastY - ypos; //Y方向的偏移量，由于屏幕坐标系有些不同

	lastX = xpos;  //更新当前位置
	lastY = ypos;  

	camera.ProcessMouseMovement(xoffset, yoffset);  //将偏移量传给相机
}

// 监听鼠标滚动
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

// 载入纹理，返回纹理对象ID
unsigned int loadTexture(char const * path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	//图片宽、高、以及颜色通道（颜色通道可以去掉图片的一些值）
	int width, height, nrComponents;
	//利用stb_image库将图片转成字节流
	unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;    
		if (nrComponents == 1)       //单色
			format = GL_RED;
		else if (nrComponents == 3)  //三色
			format = GL_RGB;
		else if (nrComponents == 4)  //加透明度
			format = GL_RGBA;

		//绑定纹理
		glBindTexture(GL_TEXTURE_2D, textureID);
		//2D纹理设置并传数据到GPU
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		//多级渐远纹理开启
		glGenerateMipmap(GL_TEXTURE_2D);

		//设置S方向和T方向
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		//为缩小和放大指定过滤模式
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}
	return textureID;
}

