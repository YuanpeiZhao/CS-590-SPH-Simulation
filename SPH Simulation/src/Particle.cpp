#include "Particle.h"

using namespace std;

void initParticles(GLuint &ssb, GLuint pNumber, int r) {

	Particle* p = new Particle[pNumber];
	
	for (int i = 0; i < pNumber; i++) {
		glm::vec3 pos = glm::vec3(float(rand()) / RAND_MAX * 2 * r - r, float(rand()) / RAND_MAX * 2 * r - r, float(rand()) / RAND_MAX * 2 * r - r);
		p[i].currPos = glm::vec4(pos, 1.f);
		p[i].prevPos = glm::vec4(pos, 1.f);
		p[i].vel = glm::vec4(0);
		p[i].acc = glm::vec4(0);
		p[i].para = glm::vec4(0);
		if (glm::distance(glm::vec3(0.0f), pos) <= r / 1.5f)
			p[i].matProp[0] = 0.0f;
		else p[i].matProp[0] = 1.0f;
	}

	glGenBuffers(1, &ssb);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssb);
	glBufferData(GL_SHADER_STORAGE_BUFFER, pNumber * sizeof(Particle), p, GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssb);

	delete p;
}

void initImGUI(GLFWwindow* window) {
	
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui_ImplGlfwGL3_Init(window, true);
	ImGui::StyleColorsClassic();
}

void initVAO(GLuint &vao, GLuint &ppvao, GLuint &ssb) {

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssb);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, ssb);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Particle), (GLvoid*)0);
	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (GLvoid*)(18 * sizeof(float)));
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Particle), (GLvoid*)(20 * sizeof(float)));
	glBindVertexArray(0);

	// Initialize post-processing buffer
	glGenVertexArrays(1, &ppvao);
	glBindVertexArray(ppvao);

	float verts[5 * 6] = {
	-1.0f, -1.0f, 0.0f,  0.0f,  0.0f,
	 1.0f, -1.0f, 0.0f,  1.0f,  0.0f,
	-1.0f,  1.0f, 0.0f,  0.0f,  1.0f,
	 1.0f,  1.0f, 0.0f,  1.0f,  1.0f,
	-1.0f,  1.0f, 0.0f,  0.0f,  1.0f,
	 1.0f, -1.0f, 0.0f,  1.0f,  0.0f
	};

	glGenBuffers(1, &ppvao);
	glBindBuffer(GL_ARRAY_BUFFER, ppvao);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (GLvoid*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (GLvoid*)(3 * sizeof(float)));
	glBindVertexArray(0);
}

static char* readShaderSource(const char* shaderFile)
{
	ifstream ifs(shaderFile, ios::in | ios::binary | ios::ate);
	if (ifs.is_open())
	{
		unsigned int filesize = static_cast<unsigned int>(ifs.tellg());
		ifs.seekg(0, ios::beg);
		char* bytes = new char[filesize + 1];
		memset(bytes, 0, filesize + 1);
		ifs.read(bytes, filesize);
		ifs.close();
		return bytes;
	}
	return NULL;
}

GLuint initShader(GLuint &program, const char* shaderFile, GLenum shaderType) {

	if(program == -1) program = glCreateProgram();
	GLchar* source = readShaderSource(shaderFile);

	GLuint shader = glCreateShader(shaderType);
	glShaderSource(shader, 1, (const GLchar**)&source, NULL);
	glCompileShader(shader);

	delete source;

	glAttachShader(program, shader); 

	return program;
}

void initDepthMap(GLFWwindow* window, GLuint &depthMap, GLuint &depthMapFBO)
{
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);

	glGenFramebuffers(1, &depthMapFBO);

	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}