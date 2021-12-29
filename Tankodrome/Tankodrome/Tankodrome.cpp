#include <stdlib.h> // necesare pentru citirea shader-elor
#include <stdio.h>
#include <math.h> 
#include "Shader.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tinyObjLoader.h"
#include <GL/glew.h>

#include <GLM.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include <glfw3.h>

#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#pragma comment (lib, "glfw3dll.lib")
#pragma comment (lib, "glew32.lib")
#pragma comment (lib, "OpenGL32.lib")

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
enum ECameraMovementType
{
	UNKNOWN,
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	UP,
	DOWN
};
class Camera
{
private:
	// Default camera values
	const float zNEAR = 0.1f;
	const float zFAR = 500.f;
	const float YAW = -90.0f;
	const float PITCH = 0.0f;
	const float FOV = 45.0f;
	glm::vec3 startPosition;

public:
	Camera(const int width, const int height, const glm::vec3& position)
	{
		startPosition = position;
		Set(width, height, position);
	}

	void Set(const int width, const int height, const glm::vec3& position)
	{
		this->isPerspective = true;
		this->yaw = YAW;
		this->pitch = PITCH;

		this->FoVy = FOV;
		this->width = width;
		this->height = height;
		this->zNear = zNEAR;
		this->zFar = zFAR;

		this->worldUp = glm::vec3(0, 1, 0);
		this->position = position;

		lastX = width / 2.0f;
		lastY = height / 2.0f;
		bFirstMouseMove = true;

		UpdateCameraVectors();
	}

	void Reset(const int width, const int height)
	{
		Set(width, height, startPosition);
	}

	void Reshape(int windowWidth, int windowHeight)
	{
		width = windowWidth;
		height = windowHeight;

		// define the viewport transformation
		glViewport(0, 0, windowWidth, windowHeight);
	}

	const glm::mat4 GetViewMatrix() const
	{
		// Returns the View Matrix
		return glm::lookAt(position, position + forward, up);
	}

	const glm::mat4 GetProjectionMatrix() const
	{
		glm::mat4 Proj = glm::mat4(1);
		if (isPerspective) {
			float aspectRatio = ((float)(width)) / height;
			Proj = glm::perspective(glm::radians(FoVy), aspectRatio, zNear, zFar);
		}
		else {
			float scaleFactor = 2000.f;
			Proj = glm::ortho<float>(
				-width / scaleFactor, width / scaleFactor,
				-height / scaleFactor, height / scaleFactor, -zFar, zFar);
		}
		return Proj;
	}

	void ProcessKeyboard(ECameraMovementType direction, float deltaTime)
	{
		float velocity = (float)(cameraSpeedFactor * deltaTime);
		switch (direction) {
		case ECameraMovementType::FORWARD:
			position += forward * velocity;
			break;
		case ECameraMovementType::BACKWARD:
			position -= forward * velocity;
			break;
		case ECameraMovementType::LEFT:
			position -= right * velocity;
			break;
		case ECameraMovementType::RIGHT:
			position += right * velocity;
			break;
		case ECameraMovementType::UP:
			position += up * velocity;
			break;
		case ECameraMovementType::DOWN:
			position -= up * velocity;
			break;
		}
	}

	void MouseControl(float xPos, float yPos)
	{
		if (bFirstMouseMove) {
			lastX = xPos;
			lastY = yPos;
			bFirstMouseMove = false;
		}

		float xChange = xPos - lastX;
		float yChange = lastY - yPos;
		lastX = xPos;
		lastY = yPos;

		if (fabs(xChange) <= 1e-6 && fabs(yChange) <= 1e-6) {
			return;
		}
		xChange *= mouseSensitivity;
		yChange *= mouseSensitivity;

		ProcessMouseMovement(xChange, yChange);
	}

	void ProcessMouseScroll(float yOffset)
	{
		if (FoVy >= 1.0f && FoVy <= 90.0f) {
			FoVy -= yOffset;
		}
		if (FoVy <= 1.0f)
			FoVy = 1.0f;
		if (FoVy >= 90.0f)
			FoVy = 90.0f;
	}

	const glm::vec3 GetPosition() const
	{
		return position;
	}

private:
	void ProcessMouseMovement(float xOffset, float yOffset, bool constrainPitch = true)
	{
		yaw += xOffset;
		pitch += yOffset;

		//std::cout << "yaw = " << yaw << std::endl;
		//std::cout << "pitch = " << pitch << std::endl;

		// Avem grijã sã nu ne dãm peste cap
		if (constrainPitch) {
			if (pitch > 89.0f)
				pitch = 89.0f;
			if (pitch < -89.0f)
				pitch = -89.0f;
		}

		// Se modificã vectorii camerei pe baza unghiurilor Euler
		UpdateCameraVectors();
	}

	void UpdateCameraVectors()
	{
		// Calculate the new forward vector
		this->forward.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		this->forward.y = sin(glm::radians(pitch));
		this->forward.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		this->forward = glm::normalize(this->forward);
		// Also re-calculate the Right and Up vector
		right = glm::normalize(glm::cross(forward, worldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		up = glm::normalize(glm::cross(right, forward));
	}


protected:
	const float cameraSpeedFactor = 2.5f;
	const float mouseSensitivity = 0.1f;

	// Perspective properties
	float zNear;
	float zFar;
	float FoVy;
	int width;
	int height;
	bool isPerspective;

	glm::vec3 position;
	glm::vec3 forward;
	glm::vec3 right;
	glm::vec3 up;
	glm::vec3 worldUp;

	// Euler Angles
	float yaw;
	float pitch;

	bool bFirstMouseMove = true;
	float lastX = 0.f, lastY = 0.f;
};

GLuint VAO, VBO, EBO;
unsigned int ProgramId;
GLuint ProjMatrixLocation, ViewMatrixLocation, WorldMatrixLocation;
unsigned int texture1Location, texture3Location, texture4Location, texture5Location;

//imports
std::vector<GLuint> Tank1VAO, Tank1VBO, Tank2VAO, Tank2VBO, Tank3VAO, Tank3VBO, PlaneVAO, PlaneVBO;
std::vector<size_t> Tank1VertexCounts, Tank2VertexCounts, Tank3VertexCounts, PlaneVertexCounts;
unsigned int ShapeProgramId;
float planePath = 0, darkeningFactor = 1;
int currentTank = 0;
std::vector<std::tuple<float, float, float>> tankMovement;

//skybox
unsigned int skyboxVAO, skyboxVBO, skyboxEBO;
unsigned int sunVAO, sunVBO;
GLuint cubemapTexture;

Camera* pCamera = nullptr;

void CreateSkyboxVBO(const std::string& strExePath) {

	float skybox[] = {

	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f
	};
	unsigned int skyboxIndices[] =
	{
		1, 2, 6,
		6, 5, 1,
		0, 4, 7,
		7, 3, 0,
		4, 5, 6,
		6, 7, 4,
		0, 3, 2,
		2, 1, 0,
		0, 1, 5,
		5, 4, 0,
		3, 7, 6,
		6, 2, 3
	};
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glGenBuffers(1, &skyboxEBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skybox), &skybox, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyboxEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(skyboxIndices), &skyboxIndices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	//load and create skybox texture
	std::string skyboxPaths[6] = {
		strExePath + "\\skyboxRight.jpg",
		strExePath + "\\skyboxLeft.png",
		strExePath + "\\skyboxTop.png",
		strExePath + "\\skyboxBottom.png",
		strExePath + "\\skyboxFront.jpg",
		strExePath + "\\skyboxBack.png"
	};
	glGenTextures(1, &cubemapTexture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	// These are very important to prevent seams
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	// This might help with seams on some systems
	//glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	// Cycles through all the textures and attaches them to the cubemap object
	for (unsigned int i = 0; i < 6; i++)
	{
		int width, height, nrChannels;
		unsigned char* data = stbi_load(skyboxPaths[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			stbi_set_flip_vertically_on_load(false);
			glTexImage2D
			(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0,
				GL_RGB,
				width,
				height,
				0,
				GL_RGB,
				GL_UNSIGNED_BYTE,
				data
			);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Failed to load texture: " << skyboxPaths[i] << std::endl;
			stbi_image_free(data);
		}
	}
}
void CreateSunVBO()
{
	float vertices[] = {
	   -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	   0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	   0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	   0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	   -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
	   -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

	   -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
	   0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
	   0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
	   0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
	   -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
	   -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

	   -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
	   -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
	   -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
	   -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
	   -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
	   -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

	   0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
	   0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
	   0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
	   0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
	   0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
	   0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

	   -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
	   0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
	   0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
	   0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
	   -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
	   -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

	   -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
	   0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
	   0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
	   0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
	   -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
	   -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
	};

	glGenVertexArrays(1, &sunVBO);
	glGenBuffers(1, &sunVBO);

	glBindBuffer(GL_ARRAY_BUFFER, sunVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindVertexArray(sunVAO);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glGenVertexArrays(1, &sunVAO);
	glBindVertexArray(sunVAO);

	glBindBuffer(GL_ARRAY_BUFFER, sunVBO);
	// note that we update the lamp's position attribute's stride to reflect the updated buffer data
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

}
void CreateVBO()
{
	float square[] = {
				 0.0f, 0.0f, 1.0f,   1.0f, 1.0f, 1.0f,   0.0f, 0.0f,   1.0f, 0.0f, 1.0f, //jos stanga
				 1.0f, 0.0f, 1.0f,   1.0f, 1.0f, 1.0f,   1.0f, 0.0f,   1.0f, 0.0f, 1.0f,// jos dreapta
				 0.0f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f,   0.0f, 1.0f,   1.0f, 0.0f, 1.0f,//sus stanga
				 1.0f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f,   1.0f, 1.0f,   1.0f, 0.0f, 1.0f,//sus dreapta
	};
	unsigned int squareIndices[] = {
	 0,1,2,
	 2,1,3
	};
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(square), square, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(squareIndices), squareIndices, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// color attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	// texture coord attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
	//normal attribute
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(8 * sizeof(float)));
	glEnableVertexAttribArray(3);

}
void DestroyVBO()
{
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
	glDeleteVertexArrays(1, &skyboxVAO);
	glDeleteBuffers(1, &skyboxVBO);
	glDeleteBuffers(1, &skyboxEBO);
}
void CreateShaders()
{
	/*VertexShaderId = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(VertexShaderId, 1, &VertexShader, NULL);
	glCompileShader(VertexShaderId);

	FragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(FragmentShaderId, 1, &FragmentShader, NULL);
	glCompileShader(FragmentShaderId);

	ProgramId = glCreateProgram();
	glAttachShader(ProgramId, VertexShaderId);
	glAttachShader(ProgramId, FragmentShaderId);
	glLinkProgram(ProgramId);

	GLint Success = 0;
	GLchar ErrorLog[1024] = { 0 };

	glGetProgramiv(ProgramId, GL_LINK_STATUS, &Success);
	if (Success == 0) {
		glGetProgramInfoLog(ProgramId, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
		exit(1);
	}

	glValidateProgram(ProgramId);
	glGetProgramiv(ProgramId, GL_VALIDATE_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(ProgramId, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
		exit(1);
	}
	*/
	glUseProgram(ProgramId);

	ProjMatrixLocation = glGetUniformLocation(ProgramId, "ProjMatrix");
	ViewMatrixLocation = glGetUniformLocation(ProgramId, "ViewMatrix");
	WorldMatrixLocation = glGetUniformLocation(ProgramId, "WorldMatrix");

	glUniform1i(glGetUniformLocation(ProgramId, "texture1"), 0);

}
void DestroyShaders()
{
	glUseProgram(0);

	//glDetachShader(ProgramId, VertexShaderId);
	//glDetachShader(ProgramId, FragmentShaderId);

	//glDeleteShader(FragmentShaderId);
	//glDeleteShader(VertexShaderId);

	//glDeleteProgram(ProgramId);
}

void CreateTextures(const std::string& strExePath)
{
	// load and create a texture 
	// -------------------------
	// texture 1
	// ---------
	glGenTextures(1, &texture1Location);
	glBindTexture(GL_TEXTURE_2D, texture1Location);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load image, create texture and generate mipmaps
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
	unsigned char* data = stbi_load((strExePath + "\\Stones.jpg").c_str(), &width, &height, &nrChannels, 0);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);

	// texture 3
	// ---------
	glGenTextures(1, &texture3Location);
	glBindTexture(GL_TEXTURE_2D, texture3Location);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load image, create texture and generate mipmaps
	data = stbi_load((strExePath + "\\TexturaTank.png").c_str(), &width, &height, &nrChannels, 0);
	if (data) {
		// note that the awesomeface.png has transparency and thus an alpha channel, so make sure to tell OpenGL the data type is of GL_RGBA
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);
	// texture 4
	// ---------
	glGenTextures(1, &texture4Location);
	glBindTexture(GL_TEXTURE_2D, texture4Location);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load image, create texture and generate mipmaps
	data = stbi_load((strExePath + "\\TexturaTank2.jpg").c_str(), &width, &height, &nrChannels, 0);
	if (data) {
		// note that the awesomeface.png has transparency and thus an alpha channel, so make sure to tell OpenGL the data type is of GL_RGBA
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);

	// texture 5
	// ---------
	glGenTextures(1, &texture5Location);
	glBindTexture(GL_TEXTURE_2D, texture5Location);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load image, create texture and generate mipmaps
	data = stbi_load((strExePath + "\\TexturaTank3.jpg").c_str(), &width, &height, &nrChannels, 0);
	if (data) {
		// note that the awesomeface.png has transparency and thus an alpha channel, so make sure to tell OpenGL the data type is of GL_RGBA
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);
}
void importTank1(const std::string& strExePath) {
	std::string inputfile = strExePath + "\\Tank1.obj";
	tinyobj::ObjReaderConfig reader_config;
	reader_config.mtl_search_path = strExePath + "\\"; // Path to material files
	reader_config.triangulate = true;
	reader_config.vertex_color = true;
	tinyobj::ObjReader reader;

	if (!reader.ParseFromFile(inputfile, reader_config)) {
		if (!reader.Error().empty()) {
			std::cerr << "TinyObjReader: " << reader.Error();
		}
		exit(1);
	}

	if (!reader.Warning().empty()) {
		std::cout << "TinyObjReader: " << reader.Warning();
	}
	auto& attrib = reader.GetAttrib();
	auto& shapes = reader.GetShapes();
	auto& materials = reader.GetMaterials();

	std::vector <std::vector<float>> shapeVertices{};
	std::cout << inputfile << std::endl;
	printf("# of vertices  = %d\n", (int)(attrib.vertices.size()) / 3);
	printf("# of normals   = %d\n", (int)(attrib.normals.size()) / 3);
	printf("# of texcoords = %d\n", (int)(attrib.texcoords.size()) / 2);
	printf("# of vertex colors = %d\n", (int)(attrib.colors.size()) / 3);
	printf("# of materials = %d\n", (int)materials.size());
	printf("# of shapes    = %d\n", (int)shapes.size());

	// Loop over shapes
	for (size_t s = 0; s < shapes.size(); s++) {
		// Loop over faces(polygon)
		size_t index_offset = 0;

		shapeVertices.emplace_back();

		for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
			size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);
			std::vector<float>& currentShapeBuffer = shapeVertices.back();

			// per-face material
			auto& material = materials[shapes[s].mesh.material_ids[f]];
			tinyobj::real_t red = material.diffuse[0];
			tinyobj::real_t green = material.diffuse[1];
			tinyobj::real_t blue = material.diffuse[2];

			// Loop over vertices in the face.
			for (size_t v = 0; v < fv; v++) {
				// access to vertex
				tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
				tinyobj::real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
				tinyobj::real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
				tinyobj::real_t vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];

				// Check if `normal_index` is zero or positive. negative = no normal data
				tinyobj::real_t nx = 0;
				tinyobj::real_t ny = 0;
				tinyobj::real_t nz = 0;

				if (idx.normal_index >= 0) {
					nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
					ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
					nz = attrib.normals[3 * size_t(idx.normal_index) + 2];
				}

				// Check if `texcoord_index` is zero or positive. negative = no texcoord data
				tinyobj::real_t tx = 0;
				tinyobj::real_t ty = 0;

				if (idx.texcoord_index >= 0) {
					tx = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
					ty = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
				}

				// Optional: vertex colors
				//tinyobj::real_t red = attrib.colors[3 * size_t(idx.vertex_index) + 0];
				//tinyobj::real_t green = attrib.colors[3 * size_t(idx.vertex_index) + 1];
				//tinyobj::real_t blue = attrib.colors[3 * size_t(idx.vertex_index) + 2];

				currentShapeBuffer.insert(currentShapeBuffer.end(), {
					vx, vy, vz, nx, ny, nz, tx, ty, red, green, blue });
			}
			index_offset += fv;


		}
	}

	Tank1VAO.resize(shapes.size());
	Tank1VBO.resize(shapes.size());
	Tank1VertexCounts.resize(shapes.size());

	for (size_t s = 0; s < shapes.size(); s++) {
		glGenVertexArrays(1, &Tank1VAO[s]);
		glGenBuffers(1, &Tank1VBO[s]);
		glBindVertexArray(Tank1VAO[s]);

		glBindBuffer(GL_ARRAY_BUFFER, Tank1VBO[s]);
		glBufferData(GL_ARRAY_BUFFER, shapeVertices[s].size() * sizeof(float), shapeVertices[s].data(), GL_STATIC_DRAW);

		float attribSize = 11 * sizeof(float);

		Tank1VertexCounts[s] = shapeVertices[s].size() / 11;

		// position attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, attribSize, (void*)0);
		glEnableVertexAttribArray(0);

		// color attribute
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, attribSize, (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);

		// texture coord attribute
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, attribSize, (void*)(6 * sizeof(float)));
		glEnableVertexAttribArray(2);

		// colors attribute
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, attribSize, (void*)(8 * sizeof(float)));
		glEnableVertexAttribArray(3);

	}
}
void importTank2(const std::string& strExePath) {
	std::string inputfile = strExePath + "\\Tank2.obj";
	tinyobj::ObjReaderConfig reader_config;
	reader_config.mtl_search_path = strExePath + "\\"; // Path to material files
	reader_config.triangulate = true;
	reader_config.vertex_color = true;
	tinyobj::ObjReader reader;

	if (!reader.ParseFromFile(inputfile, reader_config)) {
		if (!reader.Error().empty()) {
			std::cerr << "TinyObjReader: " << reader.Error();
		}
		exit(1);
	}

	if (!reader.Warning().empty()) {
		std::cout << "TinyObjReader: " << reader.Warning();
	}
	auto& attrib = reader.GetAttrib();
	auto& shapes = reader.GetShapes();
	auto& materials = reader.GetMaterials();

	std::vector <std::vector<float>> shapeVertices{};
	std::cout << inputfile << std::endl;
	printf("# of vertices  = %d\n", (int)(attrib.vertices.size()) / 3);
	printf("# of normals   = %d\n", (int)(attrib.normals.size()) / 3);
	printf("# of texcoords = %d\n", (int)(attrib.texcoords.size()) / 2);
	printf("# of vertex colors = %d\n", (int)(attrib.colors.size()) / 3);
	printf("# of materials = %d\n", (int)materials.size());
	printf("# of shapes    = %d\n\n", (int)shapes.size());

	// Loop over shapes
	for (size_t s = 0; s < shapes.size(); s++) {
		// Loop over faces(polygon)
		size_t index_offset = 0;

		shapeVertices.emplace_back();

		for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
			size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);
			std::vector<float>& currentShapeBuffer = shapeVertices.back();

			// per-face material
			auto& material = materials[shapes[s].mesh.material_ids[f]];
			tinyobj::real_t red = material.diffuse[0];
			tinyobj::real_t green = material.diffuse[1];
			tinyobj::real_t blue = material.diffuse[2];

			// Loop over vertices in the face.
			for (size_t v = 0; v < fv; v++) {
				// access to vertex
				tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
				tinyobj::real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
				tinyobj::real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
				tinyobj::real_t vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];

				// Check if `normal_index` is zero or positive. negative = no normal data
				tinyobj::real_t nx = 0;
				tinyobj::real_t ny = 0;
				tinyobj::real_t nz = 0;

				if (idx.normal_index >= 0) {
					nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
					ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
					nz = attrib.normals[3 * size_t(idx.normal_index) + 2];
				}

				// Check if `texcoord_index` is zero or positive. negative = no texcoord data
				tinyobj::real_t tx = 0;
				tinyobj::real_t ty = 0;

				if (idx.texcoord_index >= 0) {
					tx = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
					ty = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
				}

				// Optional: vertex colors
				//tinyobj::real_t red = attrib.colors[3 * size_t(idx.vertex_index) + 0];
				//tinyobj::real_t green = attrib.colors[3 * size_t(idx.vertex_index) + 1];
				//tinyobj::real_t blue = attrib.colors[3 * size_t(idx.vertex_index) + 2];

				currentShapeBuffer.insert(currentShapeBuffer.end(), {
					vx, vy, vz, nx, ny, nz, tx, ty, red, green, blue });
			}
			index_offset += fv;


		}
	}

	Tank2VAO.resize(shapes.size());
	Tank2VBO.resize(shapes.size());
	Tank2VertexCounts.resize(shapes.size());

	for (size_t s = 0; s < shapes.size(); s++) {
		glGenVertexArrays(1, &Tank2VAO[s]);
		glGenBuffers(1, &Tank2VBO[s]);
		glBindVertexArray(Tank2VAO[s]);

		glBindBuffer(GL_ARRAY_BUFFER, Tank2VBO[s]);
		glBufferData(GL_ARRAY_BUFFER, shapeVertices[s].size() * sizeof(float), shapeVertices[s].data(), GL_STATIC_DRAW);

		float attribSize = 11 * sizeof(float);

		Tank2VertexCounts[s] = shapeVertices[s].size() / 11;

		// position attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, attribSize, (void*)0);
		glEnableVertexAttribArray(0);

		// color attribute
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, attribSize, (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);

		// texture coord attribute
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, attribSize, (void*)(6 * sizeof(float)));
		glEnableVertexAttribArray(2);

		// colors attribute
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, attribSize, (void*)(8 * sizeof(float)));
		glEnableVertexAttribArray(3);

	}
}
void importTank3(const std::string& strExePath) {
	std::string inputfile = strExePath + "\\Tank3.obj";
	tinyobj::ObjReaderConfig reader_config;
	reader_config.mtl_search_path = strExePath + "\\"; // Path to material files
	reader_config.triangulate = true;
	reader_config.vertex_color = true;
	tinyobj::ObjReader reader;

	if (!reader.ParseFromFile(inputfile, reader_config)) {
		if (!reader.Error().empty()) {
			std::cerr << "TinyObjReader: " << reader.Error();
		}
		exit(1);
	}

	if (!reader.Warning().empty()) {
		std::cout << "TinyObjReader: " << reader.Warning();
	}
	auto& attrib = reader.GetAttrib();
	auto& shapes = reader.GetShapes();
	auto& materials = reader.GetMaterials();

	std::vector <std::vector<float>> shapeVertices{};
	std::cout << inputfile << std::endl;
	printf("# of vertices  = %d\n", (int)(attrib.vertices.size()) / 3);
	printf("# of normals   = %d\n", (int)(attrib.normals.size()) / 3);
	printf("# of texcoords = %d\n", (int)(attrib.texcoords.size()) / 2);
	printf("# of vertex colors = %d\n", (int)(attrib.colors.size()) / 3);
	printf("# of materials = %d\n", (int)materials.size());
	printf("# of shapes    = %d\n\n", (int)shapes.size());

	// Loop over shapes
	for (size_t s = 0; s < shapes.size(); s++) {
		// Loop over faces(polygon)
		size_t index_offset = 0;

		shapeVertices.emplace_back();

		for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
			size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);
			std::vector<float>& currentShapeBuffer = shapeVertices.back();

			// per-face material
			auto& material = materials[shapes[s].mesh.material_ids[f]];
			tinyobj::real_t red = material.diffuse[0];
			tinyobj::real_t green = material.diffuse[1];
			tinyobj::real_t blue = material.diffuse[2];

			// Loop over vertices in the face.
			for (size_t v = 0; v < fv; v++) {
				// access to vertex
				tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
				tinyobj::real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
				tinyobj::real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
				tinyobj::real_t vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];

				// Check if `normal_index` is zero or positive. negative = no normal data
				tinyobj::real_t nx = 0;
				tinyobj::real_t ny = 0;
				tinyobj::real_t nz = 0;

				if (idx.normal_index >= 0) {
					nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
					ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
					nz = attrib.normals[3 * size_t(idx.normal_index) + 2];
				}

				// Check if `texcoord_index` is zero or positive. negative = no texcoord data
				tinyobj::real_t tx = 0;
				tinyobj::real_t ty = 0;

				if (idx.texcoord_index >= 0) {
					tx = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
					ty = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
				}

				// Optional: vertex colors
				//tinyobj::real_t red = attrib.colors[3 * size_t(idx.vertex_index) + 0];
				//tinyobj::real_t green = attrib.colors[3 * size_t(idx.vertex_index) + 1];
				//tinyobj::real_t blue = attrib.colors[3 * size_t(idx.vertex_index) + 2];

				currentShapeBuffer.insert(currentShapeBuffer.end(), {
					vx, vy, vz, nx, ny, nz, tx, ty, red, green, blue });
			}
			index_offset += fv;


		}
	}

	Tank3VAO.resize(shapes.size());
	Tank3VBO.resize(shapes.size());
	Tank3VertexCounts.resize(shapes.size());

	for (size_t s = 0; s < shapes.size(); s++) {
		glGenVertexArrays(1, &Tank3VAO[s]);
		glGenBuffers(1, &Tank3VBO[s]);
		glBindVertexArray(Tank3VAO[s]);

		glBindBuffer(GL_ARRAY_BUFFER, Tank3VBO[s]);
		glBufferData(GL_ARRAY_BUFFER, shapeVertices[s].size() * sizeof(float), shapeVertices[s].data(), GL_STATIC_DRAW);

		float attribSize = 11 * sizeof(float);

		Tank3VertexCounts[s] = shapeVertices[s].size() / 11;

		// position attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, attribSize, (void*)0);
		glEnableVertexAttribArray(0);

		// color attribute
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, attribSize, (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);

		// texture coord attribute
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, attribSize, (void*)(6 * sizeof(float)));
		glEnableVertexAttribArray(2);

		// colors attribute
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, attribSize, (void*)(8 * sizeof(float)));
		glEnableVertexAttribArray(3);

	}
}
void importPlane(const std::string& strExePath) {
	std::string inputfile = strExePath + "\\Plane.obj";
	tinyobj::ObjReaderConfig reader_config;
	reader_config.mtl_search_path = strExePath + "\\"; // Path to material files
	reader_config.triangulate = true;
	reader_config.vertex_color = true;
	tinyobj::ObjReader reader;

	if (!reader.ParseFromFile(inputfile, reader_config)) {
		if (!reader.Error().empty()) {
			std::cerr << "TinyObjReader: " << reader.Error();
		}
		exit(1);
	}

	if (!reader.Warning().empty()) {
		std::cout << "TinyObjReader: " << reader.Warning();
	}
	auto& attrib = reader.GetAttrib();
	auto& shapes = reader.GetShapes();
	auto& materials = reader.GetMaterials();

	std::vector <std::vector<float>> shapeVertices{};
	std::cout << inputfile << std::endl;
	printf("# of vertices  = %d\n", (int)(attrib.vertices.size()) / 3);
	printf("# of normals   = %d\n", (int)(attrib.normals.size()) / 3);
	printf("# of texcoords = %d\n", (int)(attrib.texcoords.size()) / 2);
	printf("# of vertex colors = %d\n", (int)(attrib.colors.size()) / 3);
	printf("# of materials = %d\n", (int)materials.size());
	printf("# of shapes    = %d\n\n", (int)shapes.size());

	// Loop over shapes
	for (size_t s = 0; s < shapes.size(); s++) {
		// Loop over faces(polygon)
		size_t index_offset = 0;

		shapeVertices.emplace_back();

		for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
			size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);
			std::vector<float>& currentShapeBuffer = shapeVertices.back();

			// per-face material
			auto& material = materials[shapes[s].mesh.material_ids[f]];
			tinyobj::real_t red = material.diffuse[0];
			tinyobj::real_t green = material.diffuse[1];
			tinyobj::real_t blue = material.diffuse[2];

			// Loop over vertices in the face.
			for (size_t v = 0; v < fv; v++) {
				// access to vertex
				tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
				tinyobj::real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
				tinyobj::real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
				tinyobj::real_t vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];

				// Check if `normal_index` is zero or positive. negative = no normal data
				tinyobj::real_t nx = 0;
				tinyobj::real_t ny = 0;
				tinyobj::real_t nz = 0;

				if (idx.normal_index >= 0) {
					nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
					ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
					nz = attrib.normals[3 * size_t(idx.normal_index) + 2];
				}

				// Check if `texcoord_index` is zero or positive. negative = no texcoord data
				tinyobj::real_t tx = 0;
				tinyobj::real_t ty = 0;

				if (idx.texcoord_index >= 0) {
					tx = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
					ty = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
				}

				// Optional: vertex colors
				//tinyobj::real_t red = attrib.colors[3 * size_t(idx.vertex_index) + 0];
				//tinyobj::real_t green = attrib.colors[3 * size_t(idx.vertex_index) + 1];
				//tinyobj::real_t blue = attrib.colors[3 * size_t(idx.vertex_index) + 2];

				currentShapeBuffer.insert(currentShapeBuffer.end(), {
					vx, vy, vz, nx, ny, nz, tx, ty, red, green, blue });
			}
			index_offset += fv;


		}
	}

	PlaneVAO.resize(shapes.size());
	PlaneVBO.resize(shapes.size());
	PlaneVertexCounts.resize(shapes.size());

	for (size_t s = 0; s < shapes.size(); s++) {
		glGenVertexArrays(1, &PlaneVAO[s]);
		glGenBuffers(1, &PlaneVBO[s]);
		glBindVertexArray(PlaneVAO[s]);

		glBindBuffer(GL_ARRAY_BUFFER, PlaneVBO[s]);
		glBufferData(GL_ARRAY_BUFFER, shapeVertices[s].size() * sizeof(float), shapeVertices[s].data(), GL_STATIC_DRAW);

		float attribSize = 11 * sizeof(float);

		PlaneVertexCounts[s] = shapeVertices[s].size() / 11;

		// position attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, attribSize, (void*)0);
		glEnableVertexAttribArray(0);

		// color attribute
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, attribSize, (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);

		// texture coord attribute
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, attribSize, (void*)(6 * sizeof(float)));
		glEnableVertexAttribArray(2);

		// colors attribute
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, attribSize, (void*)(8 * sizeof(float)));
		glEnableVertexAttribArray(3);

	}
}
void Initialize(const std::string& strExePath)
{
	glClearColor(0.5f, 0.8f, 0.9f, 1.0f); // culoarea de fond a ecranului

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_COLOR_MATERIAL);
	glDisable(GL_LIGHTING);
	tankMovement.push_back(std::make_tuple(0.0f, 0.0f, 0.0f));
	tankMovement.push_back(std::make_tuple(0.0f, 2.0f, 0.0f));
	tankMovement.push_back(std::make_tuple(0.0f, 4.0f, 0.0f));
	tankMovement.push_back(std::make_tuple(-3.0f, -2.0f, 0.0f));
	tankMovement.push_back(std::make_tuple(-4.5f, -2.0f, 0.0f));
	tankMovement.push_back(std::make_tuple(-6.0f, -2.0f, 0.0f));
	tankMovement.push_back(std::make_tuple(-9.0f, 0.0f, glm::pi<float>() / 2));
	tankMovement.push_back(std::make_tuple(-9.0f, 2.0f, glm::pi<float>() / 2));
	tankMovement.push_back(std::make_tuple(-9.0f, 4.0f, glm::pi<float>() / 2));
	//glFrontFace(GL_CCW);
	//glCullFace(GL_BACK);
	CreateSkyboxVBO(strExePath);
	CreateSunVBO();
	CreateVBO();
	CreateShaders();
	CreateTextures(strExePath);
	importTank1(strExePath);
	importTank2(strExePath);
	importTank3(strExePath);
	importPlane(strExePath);
	// Create camera
	pCamera = new Camera(SCR_WIDTH, SCR_HEIGHT, glm::vec3(0.5, 4.0, 10));
}

void RenderCube()
{
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

	int indexArraySize;
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &indexArraySize);
	glDrawElements(GL_TRIANGLES, indexArraySize / sizeof(unsigned int), GL_UNSIGNED_INT, 0);
}

void RenderTank1() {
	glUseProgram(ShapeProgramId);

	unsigned int projMatrixLocation = glGetUniformLocation(ShapeProgramId, "ProjMatrix");
	unsigned int viewMatrixLocation = glGetUniformLocation(ShapeProgramId, "ViewMatrix");
	unsigned int worldMatrixLocation = glGetUniformLocation(ShapeProgramId, "WorldMatrix");

	glm::mat4 projection = pCamera->GetProjectionMatrix();
	glUniformMatrix4fv(projMatrixLocation, 1, GL_FALSE, glm::value_ptr(projection));

	glm::mat4 view = pCamera->GetViewMatrix();
	glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, glm::value_ptr(view));

	//outdated st
	//glm::mat4 worldTransf = glm::scale(glm::translate(glm::mat4(1.0), glm::vec3(0, 1, 0)), glm::vec3(0.01, 0.01, 0.01));
	//glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, glm::value_ptr(worldTransf));

	//drawing and scaling the object in the meant place
	glUniform1i(glGetUniformLocation(ShapeProgramId, "texture1"), 1);
	glUniform1f(glGetUniformLocation(ShapeProgramId, "selected"), currentTank == 0 ? 1.5 : 1.0);
	glm::mat4 position = glm::translate(glm::mat4(1.0), glm::vec3(std::get<0>(tankMovement[0]), 0.08, std::get<1>(tankMovement[0])));
	position = glm::scale(position, glm::vec3(0.01f, 0.01f, 0.01f));
	position = glm::rotate(position, std::get<2>(tankMovement[0]), glm::vec3(0, 1, 0));
	glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &position[0][0]);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texture3Location);

	for (size_t s = 0; s < Tank1VAO.size(); s++) {
		GLuint& vao = Tank1VAO[s];
		GLuint& vbo = Tank1VBO[s];

		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);

		glDrawArrays(GL_TRIANGLES, 0, Tank1VertexCounts[s]);
	}

	//drawing the same obj again
	glUniform1i(glGetUniformLocation(ShapeProgramId, "texture1"), 2);
	glUniform1f(glGetUniformLocation(ShapeProgramId, "selected"), currentTank == 1 ? 1.5 : 1.0);
	position = glm::scale(glm::translate(glm::mat4(1.0), glm::vec3(std::get<0>(tankMovement[1]), 0.08f, std::get<1>(tankMovement[1]))), glm::vec3(0.01f, 0.01f, 0.01f));
	position = glm::rotate(position, std::get<2>(tankMovement[1]), glm::vec3(0, 1, 0));
	glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &position[0][0]);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, texture4Location);

	for (size_t s = 0; s < Tank1VAO.size(); s++) {
		GLuint& vao = Tank1VAO[s];
		GLuint& vbo = Tank1VBO[s];

		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);

		glDrawArrays(GL_TRIANGLES, 0, Tank1VertexCounts[s]);
	}

	//and again
	glUniform1i(glGetUniformLocation(ShapeProgramId, "texture1"), 3);
	glUniform1f(glGetUniformLocation(ShapeProgramId, "selected"), currentTank == 2 ? 1.5 : 1.0);
	position = glm::scale(glm::translate(glm::mat4(1.0), glm::vec3(std::get<0>(tankMovement[2]), 0.08f, std::get<1>(tankMovement[2]))), glm::vec3(0.01f, 0.01f, 0.01f));
	position = glm::rotate(position, std::get<2>(tankMovement[2]), glm::vec3(0, 1, 0));
	glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &position[0][0]);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, texture5Location);

	for (size_t s = 0; s < Tank1VAO.size(); s++) {
		GLuint& vao = Tank1VAO[s];
		GLuint& vbo = Tank1VBO[s];

		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);

		glDrawArrays(GL_TRIANGLES, 0, Tank1VertexCounts[s]);
	}
}
void RenderTank2() {
	//glUseProgram(ShapeProgramId);

	//unsigned int projMatrixLocation = glGetUniformLocation(ShapeProgramId, "ProjMatrix");
	//unsigned int viewMatrixLocation = glGetUniformLocation(ShapeProgramId, "ViewMatrix");
	unsigned int worldMatrixLocation = glGetUniformLocation(ShapeProgramId, "WorldMatrix");
	/*
	glm::mat4 projection = pCamera->GetProjectionMatrix();
	glUniformMatrix4fv(projMatrixLocation, 1, GL_FALSE, glm::value_ptr(projection));

	glm::mat4 view = pCamera->GetViewMatrix();
	glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, glm::value_ptr(view));*/
	//glm::vec3(-3.0f+inainte * glm::sin(rotatie), 0.4f, -2.0f+inainte * glm::cos(rotatie))
	//drawing and scaling the object in the meant place
	//glm::vec3(-3.0f + tankX, 0.4f, -2.0f + inainte)
	glUniform1i(glGetUniformLocation(ShapeProgramId, "texture1"), 3);
	glUniform1f(glGetUniformLocation(ShapeProgramId, "selected"), currentTank == 3 ? 1.5 : 1.0);
	glm::mat4 position = glm::translate(glm::mat4(1.0), glm::vec3(std::get<0>(tankMovement[3]), 0.4f, std::get<1>(tankMovement[3])));
	position = glm::scale(position, glm::vec3(0.035f, 0.035f, 0.035f));
	position = glm::rotate(position, std::get<2>(tankMovement[3]), glm::vec3(0, 1, 0));
	glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &position[0][0]);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, texture5Location);

	for (size_t s = 0; s < Tank2VAO.size(); s++) {
		GLuint& vao = Tank2VAO[s];
		GLuint& vbo = Tank2VBO[s];

		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);

		glDrawArrays(GL_TRIANGLES, 0, Tank2VertexCounts[s]);
	}

	//drawing and scaling the object in the meant place
	//glm::vec3(-3.0f+inainte * glm::sin(rotatie), 0.4f, -2.0f+inainte * glm::cos(rotatie))
	glUniform1i(glGetUniformLocation(ShapeProgramId, "texture1"), 1);
	glUniform1f(glGetUniformLocation(ShapeProgramId, "selected"), currentTank == 4 ? 1.5 : 1.0);
	position = glm::scale(glm::translate(glm::mat4(1.0), glm::vec3(std::get<0>(tankMovement[4]), 0.4f, std::get<1>(tankMovement[4]))), glm::vec3(0.035f, 0.035f, 0.035f));
	position = glm::rotate(position, std::get<2>(tankMovement[4]), glm::vec3(0, 1, 0));
	glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &position[0][0]);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texture3Location);

	for (size_t s = 0; s < Tank2VAO.size(); s++) {
		GLuint& vao = Tank2VAO[s];
		GLuint& vbo = Tank2VBO[s];

		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);

		glDrawArrays(GL_TRIANGLES, 0, Tank2VertexCounts[s]);
	}

	//drawing and scaling the object in the meant place
	glUniform1i(glGetUniformLocation(ShapeProgramId, "texture1"), 2);
	glUniform1f(glGetUniformLocation(ShapeProgramId, "selected"), currentTank == 5 ? 1.5 : 1.0);
	position = glm::scale(glm::translate(glm::mat4(1.0), glm::vec3(std::get<0>(tankMovement[5]), 0.4f, std::get<1>(tankMovement[5]))), glm::vec3(0.035f, 0.035f, 0.035f));
	position = glm::rotate(position, std::get<2>(tankMovement[5]), glm::vec3(0, 1, 0));
	glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &position[0][0]);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, texture4Location);

	for (size_t s = 0; s < Tank2VAO.size(); s++) {
		GLuint& vao = Tank2VAO[s];
		GLuint& vbo = Tank2VBO[s];

		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);

		glDrawArrays(GL_TRIANGLES, 0, Tank2VertexCounts[s]);
	}
}
void RenderTank3() {
	//glUseProgram(ShapeProgramId);

	//unsigned int projMatrixLocation = glGetUniformLocation(ShapeProgramId, "ProjMatrix");
	unsigned int viewMatrixLocation = glGetUniformLocation(ShapeProgramId, "ViewMatrix");
	unsigned int worldMatrixLocation = glGetUniformLocation(ShapeProgramId, "WorldMatrix");
	/*
	glm::mat4 projection = pCamera->GetProjectionMatrix();
	glUniformMatrix4fv(projMatrixLocation, 1, GL_FALSE, glm::value_ptr(projection));

	glm::mat4 view = pCamera->GetViewMatrix();
	glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, glm::value_ptr(view));*/

	//drawing and scaling the object in the meant place
	glUniform1i(glGetUniformLocation(ShapeProgramId, "texture1"), 3);
	glUniform1f(glGetUniformLocation(ShapeProgramId, "selected"), currentTank == 6 ? 1.5 : 1.0);
	glm::mat4 position = glm::scale(glm::translate(glm::mat4(1.0), glm::vec3(std::get<0>(tankMovement[6]), 0.025f, std::get<1>(tankMovement[6]))), glm::vec3(0.5f, 0.5f, 0.5f));
	position = glm::rotate(position, std::get<2>(tankMovement[6]), glm::vec3(0, 1, 0));
	glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &position[0][0]);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, texture5Location);

	for (size_t s = 0; s < Tank3VAO.size(); s++) {
		GLuint& vao = Tank3VAO[s];
		GLuint& vbo = Tank3VBO[s];

		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);

		glDrawArrays(GL_TRIANGLES, 0, Tank3VertexCounts[s]);
	}

	//drawing and scaling the object in the meant place
	glUniform1i(glGetUniformLocation(ShapeProgramId, "texture1"), 1);
	glUniform1f(glGetUniformLocation(ShapeProgramId, "selected"), currentTank == 7 ? 1.5 : 1.0);
	position = glm::scale(glm::translate(glm::mat4(1.0), glm::vec3(std::get<0>(tankMovement[7]), 0.025f, std::get<1>(tankMovement[7]))), glm::vec3(0.5f, 0.5f, 0.5f));
	position = glm::rotate(position, std::get<2>(tankMovement[7]), glm::vec3(0, 1, 0));
	glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &position[0][0]);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texture3Location);

	for (size_t s = 0; s < Tank3VAO.size(); s++) {
		GLuint& vao = Tank3VAO[s];
		GLuint& vbo = Tank3VBO[s];

		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);

		glDrawArrays(GL_TRIANGLES, 0, Tank3VertexCounts[s]);
	}

	//drawing and scaling the object in the meant place
	glUniform1i(glGetUniformLocation(ShapeProgramId, "texture1"), 2);
	glUniform1f(glGetUniformLocation(ShapeProgramId, "selected"), currentTank == 8 ? 1.5 : 1.0);
	position = glm::scale(glm::translate(glm::mat4(1.0), glm::vec3(std::get<0>(tankMovement[8]), 0.025f, std::get<1>(tankMovement[8]))), glm::vec3(0.5f, 0.5f, 0.5f));
	position = glm::rotate(position, std::get<2>(tankMovement[8]), glm::vec3(0, 1, 0));
	glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &position[0][0]);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, texture4Location);

	for (size_t s = 0; s < Tank3VAO.size(); s++) {
		GLuint& vao = Tank3VAO[s];
		GLuint& vbo = Tank3VBO[s];

		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);

		glDrawArrays(GL_TRIANGLES, 0, Tank3VertexCounts[s]);
	}
}
void RenderPlane() {
	//glUseProgram(ShapeProgramId);

	//unsigned int projMatrixLocation = glGetUniformLocation(ShapeProgramId, "ProjMatrix");
	unsigned int viewMatrixLocation = glGetUniformLocation(ShapeProgramId, "ViewMatrix");
	unsigned int worldMatrixLocation = glGetUniformLocation(ShapeProgramId, "WorldMatrix");
	/*
	glm::mat4 projection = pCamera->GetProjectionMatrix();
	glUniformMatrix4fv(projMatrixLocation, 1, GL_FALSE, glm::value_ptr(projection));

	glm::mat4 view = pCamera->GetViewMatrix();
	glUniformMatrix4fv(viewMatrixLocation, 1, GL_FALSE, glm::value_ptr(view));*/

	//drawing and scaling the object in the meant place
	glUniform1i(glGetUniformLocation(ShapeProgramId, "texture1"), 3);
	glm::mat4 position = glm::scale(glm::translate(glm::mat4(1.0), glm::vec3(10 * glm::sin(planePath), 5.00f, 10 * glm::cos(planePath))), glm::vec3(0.25f, 0.25f, 0.25f));
	position = glm::rotate(position, planePath + glm::pi<float>() / 2, glm::vec3(0, 1, 0));
	glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &position[0][0]);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, texture5Location);

	for (size_t s = 0; s < PlaneVAO.size(); s++) {
		GLuint& vao = PlaneVAO[s];
		GLuint& vbo = PlaneVBO[s];

		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);

		glDrawArrays(GL_TRIANGLES, 0, PlaneVertexCounts[s]);
	}
}
void RenderFunction()
{
	//glm::vec3 cubePositions[] = {
	//   glm::vec3(0.0f,  0.0f,   0.0f),//centru
	//   glm::vec3(1.0f,  0.0f,   0.0f),//dreapta
	//   glm::vec3(-1.0f,  0.0f,   0.0f),//stanga
	//   glm::vec3(0.0f,  0.0f,   -1.0f),//sus
	//   glm::vec3(0.0f,  0.0f,   1.0f),//jos
	//   glm::vec3(-1.0f,  0.0f,   -1.0f),//sus stanga
	//   glm::vec3(1.0f,  0.0f,   -1.0f),//sus dreapta
	//   glm::vec3(1.0f,  0.0f,   1.0f),//jos dreapta
	//   glm::vec3(-1.0f,  0.0f,   1.0f),//jos stanga
	//};

	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//glUseProgram(ProgramId);

	// bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture1Location);
	/*glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texture2Location);*/

	//glm::mat4 projection = pCamera->GetProjectionMatrix();
	//glUniformMatrix4fv(ProjMatrixLocation, 1, GL_FALSE, glm::value_ptr(projection));

	////uncomment this to move camera with mouse
	//glm::mat4 view = pCamera->GetViewMatrix();
	//glUniformMatrix4fv(ViewMatrixLocation, 1, GL_FALSE, glm::value_ptr(view));

	//comment this to move camera with mouse
	/*glm::mat4 view;
	float radius = 10.0f;
	float camX = sin(glfwGetTime()) * radius;
	float camZ = cos(glfwGetTime()) * radius;
	view = glm::lookAt(glm::vec3(camX, 0.0f, camZ), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(ViewMatrixLocation, 1, GL_FALSE, glm::value_ptr(view));*/
	//pana aici

	glBindVertexArray(VAO);

	/*for (unsigned int i = 0; i < sizeof(cubePositions) / sizeof(cubePositions[0]); i++) {
		//calculate the model matrix for each object and pass it to shader before drawing
		glm::mat4 worldTransf = glm::translate(glm::mat4(1.0), cubePositions[i]);
		glUniformMatrix4fv(WorldMatrixLocation, 1, GL_FALSE, glm::value_ptr(worldTransf));

		RenderCube();
	}*/

	//platform size
	int size = 20;
	for (int x = -size; x <= size; x++) {
		for (int z = -size; z <= size; z++) {
			glm::mat4 worldTransf = glm::translate(glm::mat4(1.0), glm::vec3(x, 0.0f, z));
			glUniformMatrix4fv(WorldMatrixLocation, 1, GL_FALSE, glm::value_ptr(worldTransf));
			RenderCube();
		}
	}
	RenderTank1();
	RenderTank2();
	RenderTank3();
	RenderPlane();
}

void Cleanup()
{
	DestroyShaders();
	DestroyVBO();


	delete pCamera;
}
void drawSkybox(Shader skyboxShader) {
	// Since the cubemap will always have a depth of 1.0, we need that equal sign so it doesn't get discarded
	glDepthFunc(GL_LEQUAL);

	skyboxShader.Activate();
	//lapse between day and night
	glUniform1f(glGetUniformLocation(skyboxShader.ID, "darkeningFactor"), darkeningFactor);
	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 projection = glm::mat4(1.0f);
	// We make the mat4 into a mat3 and then a mat4 again in order to get rid of the last row and column
	// The last row and column affect the translation of the skybox (which we don't want to affect)
	view = glm::mat4(glm::mat3(pCamera->GetViewMatrix()));
	projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader.ID, "view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	// Draws the cubemap as the last object so we can save a bit of performance by discarding all fragments
	// where an object is present (a depth of 1.0f will always fail against any object's depth value)
	glBindVertexArray(skyboxVAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	// Switch back to the normal depth function
	glDepthFunc(GL_LESS);
}
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

// timing
double deltaTime = 0.0f;    // time between current frame and last frame
double lastFrame = 0.0f;

int main(int argc, char** argv)
{

	glm::vec3 sunPos(0.0f, 0.0f, 2.0f); // sun pos
	std::string strFullExeFileName = argv[0];
	std::string strExePath;
	const size_t last_slash_idx = strFullExeFileName.rfind('\\');
	if (std::string::npos != last_slash_idx) {
		strExePath = strFullExeFileName.substr(0, last_slash_idx);
	}

	// glfw: initialize and configure
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


	// glfw window creation
	//GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Tankodrome", NULL, NULL);
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Tankodrome", glfwGetPrimaryMonitor(), nullptr);//for fullscreen
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetKeyCallback(window, key_callback);


	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glewInit();

	Shader skyboxShader("Skybox.vs", "Skybox.fs");
	skyboxShader.Activate();
	glUniform1i(glGetUniformLocation(skyboxShader.ID, "skybox"), 0);

	Shader platformShader("Platform.vs", "Platform.fs");
	platformShader.Activate();
	ProgramId = platformShader.ID;

	Shader shapeShader("Model.vs", "Model.fs");
	shapeShader.Activate();
	ShapeProgramId = shapeShader.ID;
	glUniform1i(glGetUniformLocation(ShapeProgramId, "texture1"), 2);

	Shader lampShader("Lamp.vs", "Lamp.fs");

	Initialize(strExePath);
	glm::vec3 lightPos(0.0f, 0.0f, 0.0f);
	glm::vec3 correction(0.0f, 0.0f, 20.0f);

	float darkness = 0.001;

	// render loop
	while (!glfwWindowShouldClose(window)) {
		planePath += 0.01; //movement
		// per-frame time logic
		double currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		//light related stuff
		lightPos.x = 20 * glm::sin(0.1 * currentFrame);
		lightPos.y = 20 * glm::cos(0.1 * currentFrame);

		if (glm::sin(0.1*currentFrame) > 0.98 && darkness > 0) {

			darkness = -0.001;
			darkeningFactor -= 0.01;
		}
		if (glm::sin(0.1 * currentFrame) < -0.98 && darkness < 0) {

			darkness = 0.001;
			darkeningFactor += 0.01;

		}
		if (darkeningFactor > 0.2 && darkeningFactor < 1.0) {
			darkeningFactor += darkness;
		}


		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		platformShader.Activate();
		platformShader.SetVec3("objectColor", 1.0f, 1.0f, 1.0f);
		platformShader.SetVec3("lightColor", 1.0f, 1.0f, 1.0f);
		platformShader.SetVec3("lightPos", lightPos + correction);
		platformShader.SetFloat("Ka", 0.25);
		platformShader.SetFloat("Kd", 0.1);
		platformShader.SetFloat("Ks", 0.1);
		platformShader.SetFloat("Kspec", 1);
		platformShader.SetVec3("viewPos", pCamera->GetPosition());
		platformShader.SetMat4("ProjMatrix", pCamera->GetProjectionMatrix());
		platformShader.SetMat4("ViewMatrix", pCamera->GetViewMatrix());

		//glm::mat4 model = glm::scale(glm::mat4(1.0), glm::vec3(3.0f));
		//lightingShader.SetMat4("model", model);

		// render
		RenderFunction();

		lampShader.Activate();
		lampShader.SetMat4("projection", pCamera->GetProjectionMatrix());
		lampShader.SetMat4("view", pCamera->GetViewMatrix());
		glm::mat4 model = glm::translate(glm::mat4(1.0), lightPos);
		model = glm::translate(model, glm::vec3(1.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.0f));
		lampShader.SetMat4("model", model);

		glBindVertexArray(sunVAO);
		glBindBuffer(GL_ARRAY_BUFFER, sunVBO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		drawSkybox(skyboxShader);

		// input
		processInput(window);


		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	skyboxShader.Delete();
	platformShader.Delete();
	shapeShader.Delete();
	lampShader.Delete();
	Cleanup();

	// glfw: terminate, clearing all previously allocated GLFW resources
	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)//W for front
		pCamera->ProcessKeyboard(FORWARD, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)//S for back
		pCamera->ProcessKeyboard(BACKWARD, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)//A for left
		pCamera->ProcessKeyboard(LEFT, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)//D for right
		pCamera->ProcessKeyboard(RIGHT, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)//Q for up
		pCamera->ProcessKeyboard(UP, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)//E for down
		pCamera->ProcessKeyboard(DOWN, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {//reset la pozitia initiala
		int width, height;
		glfwGetWindowSize(window, &width, &height);
		pCamera->Reset(width, height);

	}

	float inainte = 0;

	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
		inainte += 0.01;
	}

	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		inainte -= 0.01;
	}

	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
		std::get<2>(tankMovement[currentTank]) += 0.005;
	}

	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
		std::get<2>(tankMovement[currentTank]) -= 0.005;
	}

	if (currentTank < 3) {
		std::get<0>(tankMovement[currentTank]) += inainte * glm::sin(std::get<2>(tankMovement[currentTank]) - glm::pi<float>() / 2);
		std::get<1>(tankMovement[currentTank]) += inainte * glm::cos(std::get<2>(tankMovement[currentTank]) - glm::pi<float>() / 2);
	}
	else {
		std::get<0>(tankMovement[currentTank]) += inainte * glm::sin(std::get<2>(tankMovement[currentTank]));
		std::get<1>(tankMovement[currentTank]) += inainte * glm::cos(std::get<2>(tankMovement[currentTank]));
	}
	//tankX += inainte * glm::sin(rotatie);
	//tankZ += inainte * glm::cos(rotatie);

}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	pCamera->Reshape(width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	pCamera->MouseControl((float)xpos, (float)ypos);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yOffset)
{
	pCamera->ProcessMouseScroll((float)yOffset);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	/*static float s_fMixValue = 0.5;
	if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) {
		if (s_fMixValue < 1.0) {
			s_fMixValue += 0.1;
			glUniform1f(glGetUniformLocation(ProgramId, "mixValue"), s_fMixValue);
		}
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		if (s_fMixValue > 0.0) {
			s_fMixValue -= 0.1;
			glUniform1f(glGetUniformLocation(ProgramId, "mixValue"), s_fMixValue);
		}
	}*/

	if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS) {
		currentTank++;
		if (currentTank >= tankMovement.size()) {
			currentTank %= tankMovement.size();
		}
	}

}
