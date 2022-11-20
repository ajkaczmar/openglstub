/* Based on spherical harmonics computation example by Andreas Mantler (ands)*/


#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include "glad/glad.h"
#include "GLFW/glfw3.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "m_math.h"

typedef struct
{
	struct
	{
		GLuint program;
		GLint u_view;
		GLint u_projection;
		GLint u_cubemap;

		GLuint vao, vbo, ibo;
		int indices;

		GLuint texture;
	} sky;


} scene_t;


static GLuint s_loadShader(GLenum type, const char* source)
{
	GLuint shader = glCreateShader(type);
	if (shader == 0)
	{
		fprintf(stderr, "Could not create shader!\n");
		return 0;
	}
	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);
	GLint compiled;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if (!compiled)
	{
		fprintf(stderr, "Could not compile shader!\n");
		GLint infoLen = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
		if (infoLen)
		{
			char* infoLog = (char*)malloc(infoLen);
			glGetShaderInfoLog(shader, infoLen, NULL, infoLog);
			fprintf(stderr, "%s\n", infoLog);
			free(infoLog);
		}
		glDeleteShader(shader);
		return 0;
	}
	return shader;
}

static GLuint s_loadProgram(const char* vp, const char* fp, const char** attributes, int attributeCount)
{
	GLuint vertexShader = s_loadShader(GL_VERTEX_SHADER, vp);
	if (!vertexShader)
		return 0;
	GLuint fragmentShader = s_loadShader(GL_FRAGMENT_SHADER, fp);
	if (!fragmentShader)
	{
		glDeleteShader(vertexShader);
		return 0;
	}

	GLuint program = glCreateProgram();
	if (program == 0)
	{
		fprintf(stderr, "Could not create program!\n");
		return 0;
	}
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);

	for (int i = 0; i < attributeCount; i++)
		glBindAttribLocation(program, i, attributes[i]);

	glLinkProgram(program);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	GLint linked;
	glGetProgramiv(program, GL_LINK_STATUS, &linked);
	if (!linked)
	{
		fprintf(stderr, "Could not link program!\n");
		GLint infoLen = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLen);
		if (infoLen)
		{
			char* infoLog = (char*)malloc(sizeof(char) * infoLen);
			glGetProgramInfoLog(program, infoLen, NULL, infoLog);
			fprintf(stderr, "%s\n", infoLog);
			free(infoLog);
		}
		glDeleteProgram(program);
		return 0;
	}
	return program;
}

static int initScene(scene_t *scene)
{
	int vertexSize = 3 * sizeof(float);
	m_vec3 vertices[] =
	{
		{ 1, -1, 1 },
		{ 1, 1, 1 },
		{ 1, 1, -1 },
		{ -1, 1, -1 },
		{ 1, -1, -1 },
		{ -1, -1, -1 },
		{ -1, -1, 1 },
		{ -1, 1, 1 }
	};

	scene->sky.indices = 14;
	uint16_t indices[] = { 0, 1, 2, 3, 4, 5, 6, 3, 7, 1, 6, 0, 4, 2 };

	glGenVertexArrays(1, &scene->sky.vao);
	glBindVertexArray(scene->sky.vao);

	glGenBuffers(1, &scene->sky.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, scene->sky.vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, vertexSize, (void*)0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &scene->sky.ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, scene->sky.ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * scene->sky.indices, indices, GL_STATIC_DRAW);

	const char* skyAttribs[] =
	{
		"a_position"
	};

	const char* skyVP =
		"#version 150 core\n"
		"in vec3 a_position;\n"
		"uniform mat4 u_view;\n"
		"uniform mat4 u_projection;\n"
		"out vec3 v_direction;\n"

		"void main()\n"
		"{\n"
		"    vec4 position = u_projection * (u_view * vec4(a_position, 0.0));\n"
		"    gl_Position = position.xyww;\n"
		"    v_direction = a_position;\n"
		"}\n";

	const char* skyFP =
		"#version 150 core\n"
		"in vec3 v_direction;\n"
		"uniform samplerCube u_cubemap;\n"
		"out vec4 o_color;\n"

		"void main()\n"
		"{\n"
		"    o_color = vec4(texture(u_cubemap, v_direction).rgb, 1.0);\n"
		"}\n";

	scene->sky.program = s_loadProgram(skyVP, skyFP, skyAttribs, 1);
	if (!scene->sky.program)
	{
		fprintf(stderr, "Error loading mesh shader\n");
		return 0;
	}
	scene->sky.u_view = glGetUniformLocation(scene->sky.program, "u_view");
	scene->sky.u_projection = glGetUniformLocation(scene->sky.program, "u_projection");
	scene->sky.u_cubemap = glGetUniformLocation(scene->sky.program, "u_cubemap");

	#define SKY_DIR "cubemaps/room/"

	const char* skyTextureFiles[] = {
		SKY_DIR "posx.jpg", SKY_DIR "negx.jpg",
		SKY_DIR "posy.jpg", SKY_DIR "negy.jpg",
		SKY_DIR "posz.jpg", SKY_DIR "negz.jpg"
	};
	const m_vec3 skyDir[] = {
		m_v3(1.0f, 0.0f, 0.0f), m_v3(-1.0f, 0.0f, 0.0f),
		m_v3(0.0f, 1.0f, 0.0f), m_v3(0.0f, -1.0f, 0.0f),
		m_v3(0.0f, 0.0f, 1.0f), m_v3(0.0f, 0.0f, -1.0f)
	};
	const m_vec3 skyX[] = {
		m_v3(0.0f, 0.0f, -1.0f), m_v3(0.0f, 0.0f, 1.0f),
		m_v3(-1.0f, 0.0f, 0.0f), m_v3(1.0f, 0.0f, 0.0f),
		m_v3(1.0f, 0.0f, 0.0f), m_v3(-1.0f, 0.0f, 0.0f)
	};
	const m_vec3 skyY[] = {
		m_v3(0.0f, 1.0f, 0.0f), m_v3(0.0f, 1.0f, 0.0f),
		m_v3(0.0f, 0.0f, -1.0f), m_v3(0.0f, 0.0f, 1.0f),
		m_v3(0.0f, 1.0f, 0.0f), m_v3(0.0f, 1.0f, 0.0f)
	};
	glGenTextures(1, &scene->sky.texture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, scene->sky.texture);

	float weightSum = 0.0f;
	for (int i = 0; i < 6; i++)
	{
		int w, h, c;
		unsigned char* stbidata = stbi_load(skyTextureFiles[i], &w, &h, &c, 3);
		if (!stbidata)
		{
			fprintf(stderr, "Error loading sky texture\n");
			return 0;
		}
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, stbidata);

		free(stbidata);
	}
	
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	return 1;
}

static void drawScene(scene_t* scene, float* view, float* projection)
{
	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	// sky
	glDepthMask(GL_FALSE);
	glUseProgram(scene->sky.program);
	glUniformMatrix4fv(scene->sky.u_projection, 1, GL_FALSE, projection);
	glUniformMatrix4fv(scene->sky.u_view, 1, GL_FALSE, view);
	glUniform1i(scene->sky.u_cubemap, 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, scene->sky.texture);
	glBindVertexArray(scene->sky.vao);
	glDrawElements(GL_TRIANGLE_STRIP, scene->sky.indices, GL_UNSIGNED_SHORT, 0);
	glDepthMask(GL_TRUE);
}

static void destroyScene(scene_t *scene)
{
	// sky
	glDeleteProgram(scene->sky.program);
	glDeleteVertexArrays(1, &scene->sky.vao);
	glDeleteBuffers(1, &scene->sky.vbo);
	glDeleteBuffers(1, &scene->sky.ibo);
	glDeleteTextures(1, &scene->sky.texture);
}

static void fpsCameraViewMatrix(GLFWwindow* window, float* view)
{
	// initial camera config
	static float position[] = { 0.0f, 0.0f, 3.0f };
	static float rotation[] = { 0.0f, 0.0f };

	// mouse look
	static double lastMouse[] = { 0.0, 0.0 };
	double mouse[2];
	glfwGetCursorPos(window, &mouse[0], &mouse[1]);
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
	{
		rotation[0] += (float)(mouse[1] - lastMouse[1]) * -0.2f;
		rotation[1] += (float)(mouse[0] - lastMouse[0]) * -0.2f;
	}
	lastMouse[0] = mouse[0];
	lastMouse[1] = mouse[1];

	float rotationY[16], rotationX[16], rotationYX[16];
	m_rotation44(rotationX, rotation[0], 1.0f, 0.0f, 0.0f);
	m_rotation44(rotationY, rotation[1], 0.0f, 1.0f, 0.0f);
	m_mul44(rotationYX, rotationY, rotationX);

	// keyboard movement (WSADEQ)
	float speed = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) ? 0.1f : 0.01f;
	float movement[3] = { 0 };

	{
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) movement[2] -= speed;
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) movement[2] += speed;
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) movement[0] -= speed;
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) movement[0] += speed;
		if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) movement[1] -= speed;
		if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) movement[1] += speed;
	}

	float worldMovement[3];
	m_transform44(worldMovement, rotationYX, movement);
	position[0] += worldMovement[0];
	position[1] += worldMovement[1];
	position[2] += worldMovement[2];

	// construct view matrix
	float inverseRotation[16], inverseTranslation[16];
	m_transpose44(inverseRotation, rotationYX);
	m_translation44(inverseTranslation, -position[0], -position[1], -position[2]);
	m_mul44(view, inverseRotation, inverseTranslation); // = inverse(translation(position) * rotationYX);
}

static void error_callback(int error, const char *description)
{
	fprintf(stderr, "Error: %s\n", description);
}

int main(int argc, char* argv[])
{
	glfwSetErrorCallback(error_callback);
	if (!glfwInit()) return 1;
	glfwWindowHint(GLFW_RED_BITS, 8);
	glfwWindowHint(GLFW_GREEN_BITS, 8);
	glfwWindowHint(GLFW_BLUE_BITS, 8);
	glfwWindowHint(GLFW_ALPHA_BITS, 8);
	glfwWindowHint(GLFW_DEPTH_BITS, 32);
	glfwWindowHint(GLFW_STENCIL_BITS, GLFW_DONT_CARE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
	glfwWindowHint(GLFW_SAMPLES, 4);
	GLFWwindow *window = glfwCreateWindow(1280, 800, "Spherical Harmonics Playground", NULL, NULL);
	if (!window) return 1;
	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	glfwSwapInterval(1);

	scene_t scene = {0};
	if (!initScene(&scene))
	{
		fprintf(stderr, "Could not initialize scene.\n");
		return 1;
	}

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
		{
			destroyScene(&scene);
			if (!initScene(&scene))
			{
				fprintf(stderr, "Could not reinitialize scene.\n");
				break;
			}
		}
		
		static bool show_another_window = true;


		int w, h;
		glfwGetFramebufferSize(window, &w, &h);
		glViewport(0, 0, w, h);
		float view[16], projection[16];
		fpsCameraViewMatrix(window, view);
		m_perspective44(projection, 45.0f, (float)w / (float)h, 0.01f, 100.0f);
		drawScene(&scene, view, projection);

		glfwSwapBuffers(window);
	}

	destroyScene(&scene);
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}