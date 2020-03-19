#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <iostream>

#include "Particle.h"

GLFWwindow* window;

GLuint ssb = -1;		// Shader Storage Buffer
GLuint vao = -1;		// Vertex Array Buffer
GLuint ppvao = -1;		// Post Processing VAO
GLuint depthMap = -1;
GLuint depthMapFBO = -1;

GLuint shaderProgram = -1;
GLuint computeProgram = -1;
GLuint postprocessProgram = -1;

static const std::string vertexShader("shaders/vs.glsl");
static const std::string geometryShader("shaders/gs.glsl");
static const std::string fragmentShader("shaders/fs.glsl");

static const std::string ppVertexShader("shaders/ppvs.glsl");
static const std::string ppFragmentShader("shaders/ppfs.glsl");

static const std::string computeShader("shaders/cs.glsl");

void fb_size_callback(GLFWwindow* window, int width, int height);
// static void MouseCallback(GLFWwindow* window, int Button, int Action, int Mode);
void inputCallback(GLFWwindow* window);
void drawDepthMap(double time);
void postProcessing();

int windowWidth = 1280;
int windowHeight = 720;

GLuint particleNumber = 20000;
int particleRadius = 3;
float particleSize = 0.01f;

float deltaTime = 0.0f;
float tmpDeltaTime = 0.0f;

bool Imgui = true;
bool contour = true;
bool spacePressed = false;

float cameraFOV = 45.0f;
float nearPlane = 1.0f;
float farPlane = 1000.0f;
glm::vec4 camPos;

void initOpenGL() {

	// Init glfw
	
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(windowWidth, windowHeight, "SPH Simulation", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return;
	}
	glfwMakeContextCurrent(window);
	// glfwSetMouseButtonCallback(window, MouseCallback);

	// Init glad

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return;
	}
	
	// Init viewport

	glViewport(0, 0, windowWidth, windowHeight);
	glfwSetFramebufferSizeCallback(window, fb_size_callback);

	std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
	std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

	
	// Initialize particles

	initParticles(ssb, particleNumber, particleRadius);
	initImGUI(window);
	initVAO(vao, ppvao, ssb);
	
	shaderProgram = initShader(shaderProgram, vertexShader.c_str(), GL_VERTEX_SHADER);
	shaderProgram = initShader(shaderProgram, geometryShader.c_str(), GL_GEOMETRY_SHADER);
	shaderProgram = initShader(shaderProgram, fragmentShader.c_str(), GL_FRAGMENT_SHADER);
	computeProgram = initShader(computeProgram, computeShader.c_str(), GL_COMPUTE_SHADER);
	postprocessProgram = initShader(postprocessProgram, ppVertexShader.c_str(), GL_VERTEX_SHADER);
	postprocessProgram = initShader(postprocessProgram, ppFragmentShader.c_str(), GL_FRAGMENT_SHADER);
		
	glLinkProgram(shaderProgram);
	glLinkProgram(computeProgram);
	glLinkProgram(postprocessProgram);

	glUseProgram(computeProgram);
	int maxParticles_loc = glGetUniformLocation(computeProgram, "maxParticles");
	if (maxParticles_loc != -1)
	{
		glUniform1ui(maxParticles_loc, particleNumber);
	}

	glUseProgram(shaderProgram);
	int quadLength_loc = glGetUniformLocation(shaderProgram, "quadLength");
	if (quadLength_loc != -1)
	{
		glUniform1f(quadLength_loc, particleSize);
	}
	int P_loc = glGetUniformLocation(shaderProgram, "projMatrix");
	if (P_loc != -1)
	{
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glm::mat4 P = glm::perspective(cameraFOV, (float)width/(float)height, nearPlane, farPlane);
		glUniformMatrix4fv(P_loc, 1, false, glm::value_ptr(P));
	}

	glUseProgram(0);

	camPos = glm::vec4(15, 10, 15, 1);

	glEnable(GL_DEPTH_TEST);
	initDepthMap(window, depthMap, depthMapFBO);

	// Set clear color
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

	return;
}

void draw_gui()
{
	glfwPollEvents();
	ImGui_ImplGlfwGL3_NewFrame();

	ImGui::Begin("Configuration", &Imgui, ImGuiWindowFlags_MenuBar);

	ImGui::TextUnformatted("Use space to pause / resume");

	ImGui::Checkbox("contour", &contour);

	ImGui::End();

	ImGui::Render();
}

void render() {

	glBindVertexArray(vao);

	// Pass 1
	glUseProgram(computeProgram);
	int frameTimeDiff_loc = glGetUniformLocation(computeProgram, "frameTimeDiff");
	if (frameTimeDiff_loc != -1)
	{
		glUniform1f(frameTimeDiff_loc, deltaTime);
	}

	int pass_loc = glGetUniformLocation(computeProgram, "pass");
	if (pass_loc != -1)
	{
		glUniform1i(pass_loc, 1);
	}

	glDispatchCompute((particleNumber / WORK_GROUP_SIZE) + 1, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);

	// Pass 2
	glUseProgram(computeProgram);
	frameTimeDiff_loc = glGetUniformLocation(computeProgram, "frameTimeDiff");
	if (frameTimeDiff_loc != -1)
	{
		glUniform1f(frameTimeDiff_loc, deltaTime);
	}
	pass_loc = glGetUniformLocation(computeProgram, "pass");
	if (pass_loc != -1)
	{
		glUniform1i(pass_loc, 2);
	}
	glDispatchCompute((particleNumber / WORK_GROUP_SIZE) + 1, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);

	// Pass 3
	glUseProgram(computeProgram);
	frameTimeDiff_loc = glGetUniformLocation(computeProgram, "frameTimeDiff");
	if (frameTimeDiff_loc != -1)
	{
		glUniform1f(frameTimeDiff_loc, deltaTime);
	}
	pass_loc = glGetUniformLocation(computeProgram, "pass");
	if (pass_loc != -1)
	{
		glUniform1i(pass_loc, 3);
	}
	glDispatchCompute((particleNumber / WORK_GROUP_SIZE) + 1, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
	
	double time = glfwGetTime();

	drawDepthMap(time);

	// Render the scene

	glClearColor(0.6, 0.6, 0.6, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(shaderProgram);

	int V_loc = glGetUniformLocation(shaderProgram, "viewMatrix");
	if (V_loc != -1)
	{
		glm::mat4 V = glm::lookAt(glm::vec3(camPos), glm::vec3(0.0f, -5.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));	
		glUniformMatrix4fv(V_loc, 1, false, glm::value_ptr(V));
	}

	int camPos_loc = glGetUniformLocation(shaderProgram, "camPos");
	if (camPos_loc != -1)
	{
		glUniform4fv(camPos_loc, 1, &camPos[0]);
	}

	int time_loc = glGetUniformLocation(shaderProgram, "time");
	if (time_loc != -1)
	{
		glUniform1f(time_loc, static_cast<GLfloat>(time));
	}

	glDrawArrays(GL_POINTS, 0, particleNumber);

	draw_gui();

	glUseProgram(0);
	glBindVertexArray(0);

	postProcessing();
}

int main() {

	initOpenGL();

	while (!glfwWindowShouldClose(window))
	{
		inputCallback(window);

		render();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

void fb_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void inputCallback(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		if (!spacePressed)
		{
			spacePressed = true;
			if (deltaTime > 0.0f)
			{
				deltaTime = 0.0f;
				tmpDeltaTime = deltaTime;
			}
			else
			{
				deltaTime = 0.01f;
				tmpDeltaTime = deltaTime;
			}
		}
	}

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE)
	{
		spacePressed = false;
	}
}

void drawDepthMap(double time)
{
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);

	glUseProgram(shaderProgram);

	int V_loc = glGetUniformLocation(shaderProgram, "viewMatrix");
	if (V_loc != -1)
	{
		glm::mat4 V = glm::lookAt(glm::vec3(camPos), glm::vec3(0.0f, -5.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(V_loc, 1, false, glm::value_ptr(V));
	}

	int camPos_loc = glGetUniformLocation(shaderProgram, "camPos");
	if (camPos_loc != -1)
	{
		glUniform4fv(camPos_loc, 1, &camPos[0]);
	}

	int time_loc = glGetUniformLocation(shaderProgram, "time");
	if (time_loc != -1)
	{
		glUniform1f(time_loc, static_cast<GLfloat>(time));
	}

	glDrawArrays(GL_POINTS, 0, particleNumber);

	glUseProgram(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void postProcessing() {

	glUseProgram(postprocessProgram);

	if (contour) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthMap);

		int depthMap_loc = glGetUniformLocation(postprocessProgram, "depthMap");
		if (depthMap_loc != -1)
		{
			glUniform1i(depthMap_loc, 0);
		}
	}
	else {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);

		int depthMap_loc = glGetUniformLocation(postprocessProgram, "depthMap");
		if (depthMap_loc != -1)
		{
			glUniform1i(depthMap_loc, 0);
		}
	}

	glBindVertexArray(ppvao);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}