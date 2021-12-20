#include <iostream>         // cout, cerr
#include <cstdlib>          // EXIT_FAILURE
#include <GL/glew.h>        // GLEW library
#include <GLFW/glfw3.h>     // GLFW library
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>      // Image loading Utility functions

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <camera.h> // Camera class

using namespace std; // Standard namespace

/*Shader program Macro*/
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

int ortho = 0;//  variable for ortho switch

// Unnamed namespace
namespace
{
	const char* const WINDOW_TITLE = "Cigar"; // Macro for window title

	// Variables for window width and height
	const int WINDOW_WIDTH = 800;
	const int WINDOW_HEIGHT = 600;

	// Stores the GL data relative to a given mesh
	struct GLMesh
	{
		GLuint vao[1];         // Handle for the vertex array object
		GLuint vbo[1];         // Handle for the vertex buffer object
		GLuint nVertices[1];    // Number of indices of the mesh

		GLuint Lvao;         // Handle for the vertex array object
		GLuint Lvbo;         // Handle for the vertex buffer object
	};

	// Main GLFW window
	GLFWwindow* gWindow = nullptr;
	// Triangle mesh data
	GLMesh gMesh;
	// Texture
	GLuint gTextureId;
	glm::vec2 gUVScale(1.0f, 1.0f);
	GLint gTexWrapMode = GL_REPEAT;

	// Shader programs
	GLuint gCubeProgramId;
	GLuint gLampProgramId;
	// GLuint gLampProgramId2;

	 // camera
	Camera gCamera(glm::vec3(0.0f, 3.0f, 20.0f));
	float gLastX = WINDOW_WIDTH / 2.0f;
	float gLastY = WINDOW_HEIGHT / 2.0f;
	bool gFirstMouse = true;

	// timing
	float gDeltaTime = 0.0f; // time between current frame and last frame
	float gLastFrame = 0.0f;

	// Subject position and scale
	glm::vec3 gCubePosition(0.0f, 0.0f, 0.0f);
	glm::vec3 gCubeScale(2.0f);

	// Cube and light color
	//m::vec3 gObjectColor(0.6f, 0.5f, 0.75f);
	glm::vec3 gObjectColor(1.0f, 1.0f, 0.0f);
	glm::vec3 gObjectColor2(1.0f, 0.0f, 0.0f);

	glm::vec3 gLightColor(1.0f, 1.0f, 1.0f);
	glm::vec3 gLightColor2(1.0f, 0.0f, 0.0f);

	// Light position and scale
	glm::vec3 gLightPosition(5.0f, 5.0f, 10.0f);
	glm::vec3 gLightScale(2.0f);

	glm::vec3 gLightPosition2(0.6f, -0.8f, 1.3f);
	glm::vec3 gLightScale2(0.5f);

	// Lamp animation
	bool gIsLampOrbiting = false;
}

/* User-defined Function prototypes to:
 * initialize the program, set the window size,
 * redraw graphics on the window when resized,
 * and render graphics on the screen
 */
bool UInitialize(int, char* [], GLFWwindow** window);
void UResizeWindow(GLFWwindow* window, int width, int height);
void UProcessInput(GLFWwindow* window);
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos);
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void UCreateMesh(GLMesh& mesh);
void UDestroyMesh(GLMesh& mesh);
bool UCreateTexture(const char* filename, GLuint& textureId);
void UDestroyTexture(GLuint textureId);
void URender();
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
void UDestroyShaderProgram(GLuint programId);


/* Cube Vertex Shader Source Code*/
const GLchar* cubeVertexShaderSource = GLSL(440,

	layout(location = 0) in vec3 position; // VAP position 0 for vertex position data
layout(location = 1) in vec3 normal; // VAP position 1 for normals
layout(location = 2) in vec2 textureCoordinate;

out vec3 vertexNormal; // For outgoing normals to fragment shader
out vec3 vertexFragmentPos; // For outgoing color / pixels to fragment shader
out vec2 vertexTextureCoordinate;

//Uniform / Global variables for the  transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;


void main()
{
	gl_Position = projection * view * model * vec4(position, 1.0f); // Transforms vertices into clip coordinates

	vertexFragmentPos = vec3(model * vec4(position, 1.0f)); // Gets fragment / pixel position in world space only (exclude view and projection)

	vertexNormal = mat3(transpose(inverse(model))) * normal; // get normal vectors in world space only and exclude normal translation properties
	vertexTextureCoordinate = textureCoordinate;
}
);


/* Cube Fragment Shader Source Code*/
const GLchar* cubeFragmentShaderSource = GLSL(440,

	in vec3 vertexNormal; // For incoming normals
in vec3 vertexFragmentPos; // For incoming fragment position
in vec2 vertexTextureCoordinate;

out vec4 fragmentColor; // For outgoing cube color to the GPU

// Uniform / Global variables for object color, light color, light position, and camera/view position
uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;

uniform vec3 objectColor2;
uniform vec3 lightColor2;
uniform vec3 lightPos2;


uniform vec3 viewPosition;
uniform sampler2D uTexture; // Useful when working with multiple textures
uniform vec2 uvScale;

void main()
{
	/*Phong lighting model calculations to generate ambient, diffuse, and specular components*/

	//Calculate Ambient lighting*/
	float ambientStrength = 1.0f; // Set ambient or global lighting strength
	vec3 ambient = ambientStrength * lightColor; // Generate ambient light color

	float ambientStrength2 = 0.004f; // Set ambient or global lighting strength for 2nd
	vec3 ambient2 = ambientStrength2 * lightColor2; // Generate ambient light color for 2nd


	//Calculate Diffuse lighting*/
	vec3 norm = normalize(vertexNormal); // Normalize vectors to 1 unit
	vec3 lightDirection = normalize(lightPos - vertexFragmentPos); // Calculate distance (light direction) between light source and fragments/pixels on cube
	float impact = max(dot(norm, lightDirection), 0.0);// Calculate diffuse impact by generating dot product of normal and light
	vec3 diffuse = impact * lightColor; // Generate diffuse light color

	vec3 lightDirection2 = normalize(lightPos2 - vertexFragmentPos); // Calculate distance (light direction) between light source and fragments/pixels on cube
	float impact2 = max(dot(norm, lightDirection2), 0.0);// Calculate diffuse impact by generating dot product of normal and light
	vec3 diffuse2 = impact2 * lightColor2; // Generate diffuse light color

	//Calculate Specular lighting*/
	float specularIntensity = 0.3f; // Set specular light strength
	float specularIntensity2 = 0.5f; // Set specular light strength
	float highlightSize = 16.0f; // Set specular highlight size
	float highlightSize2 = 0.3f; // Set specular highlight size
	vec3 viewDir = normalize(viewPosition - vertexFragmentPos); // Calculate view direction
	vec3 reflectDir = reflect(-lightDirection, norm);// Calculate reflection vector
	vec3 reflectDir2 = reflect(-lightDirection2, norm);// Calculate reflection vector
	//Calculate specular component
	float specularComponent = pow(max(dot(viewDir, reflectDir), 0.0), highlightSize);
	float specularComponent2 = pow(max(dot(viewDir, reflectDir2), 0.0), highlightSize2);
	vec3 specular = specularIntensity * specularComponent * lightColor;
	vec3 specular2 = specularIntensity2 * specularComponent2 * lightColor2;

	// Texture holds the color to be used for all three components
	vec4 textureColor = texture(uTexture, vertexTextureCoordinate * uvScale);

	// Calculate phong result
	vec3 phong = (ambient + diffuse + specular) * textureColor.xyz;
	phong += (ambient2 + diffuse2 + specular2) * textureColor.xyz;

	fragmentColor = vec4(phong, 1.0); // Send lighting results to GPU
}
);


/* Lamp Shader Source Code*/
const GLchar* lampVertexShaderSource = GLSL(440,

	layout(location = 0) in vec3 position; // VAP position 0 for vertex position data

		//Uniform / Global variables for the  transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * view * model * vec4(position, 1.0f); // Transforms vertices into clip coordinates
}
);


/* Fragment Shader Source Code*/
const GLchar* lampFragmentShaderSource = GLSL(440,

	out vec4 fragmentColor; // For outgoing lamp color (smaller cube) to the GPU

void main()
{
	fragmentColor = vec4(1.0f); // Set color to white (1.0f,1.0f,1.0f) with alpha 1.0
}
);


// Images are loaded with Y axis going down, but OpenGL's Y axis goes up, so let's flip it
void flipImageVertically(unsigned char* image, int width, int height, int channels)
{
	for (int j = 0; j < height / 2; ++j)
	{
		int index1 = j * width * channels;
		int index2 = (height - 1 - j) * width * channels;

		for (int i = width * channels; i > 0; --i)
		{
			unsigned char tmp = image[index1];
			image[index1] = image[index2];
			image[index2] = tmp;
			++index1;
			++index2;
		}
	}
}


int main(int argc, char* argv[])
{
	if (!UInitialize(argc, argv, &gWindow))
		return EXIT_FAILURE;

	// Create the mesh
	UCreateMesh(gMesh); // Calls the function to create the Vertex Buffer Object

	// Create the shader programs
	if (!UCreateShaderProgram(cubeVertexShaderSource, cubeFragmentShaderSource, gCubeProgramId))
		return EXIT_FAILURE;

	if (!UCreateShaderProgram(lampVertexShaderSource, lampFragmentShaderSource, gLampProgramId))
		return EXIT_FAILURE;

	// Load texture
	const char* texFilename = "cigarlowpoly.png";
	if (!UCreateTexture(texFilename, gTextureId))
	{
		cout << "Failed to load texture " << texFilename << endl;
		return EXIT_FAILURE;
	}
	// tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
	glUseProgram(gCubeProgramId);
	// We set the texture as texture unit 0
	glUniform1i(glGetUniformLocation(gCubeProgramId, "uTexture"), 0);

	// Sets the background color of the window to black (it will be implicitely used by glClear)
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	// render loop
	// -----------
	while (!glfwWindowShouldClose(gWindow))
	{
		// per-frame timing
		// --------------------
		float currentFrame = glfwGetTime();
		gDeltaTime = currentFrame - gLastFrame;
		gLastFrame = currentFrame;

		// input
		// -----
		UProcessInput(gWindow);

		// Render this frame
		URender();

		glfwPollEvents();
	}

	// Release mesh data
	UDestroyMesh(gMesh);

	// Release texture
	UDestroyTexture(gTextureId);

	// Release shader programs
	UDestroyShaderProgram(gCubeProgramId);
	UDestroyShaderProgram(gLampProgramId);

	exit(EXIT_SUCCESS); // Terminates the program successfully
}


// Initialize GLFW, GLEW, and create a window
bool UInitialize(int argc, char* argv[], GLFWwindow** window)
{
	// GLFW: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// GLFW: window creation
	// ---------------------
	* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
	if (*window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(*window);
	glfwSetFramebufferSizeCallback(*window, UResizeWindow);
	glfwSetCursorPosCallback(*window, UMousePositionCallback);
	glfwSetScrollCallback(*window, UMouseScrollCallback);
	glfwSetMouseButtonCallback(*window, UMouseButtonCallback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// GLEW: initialize
	// ----------------
	// Note: if using GLEW version 1.13 or earlier
	glewExperimental = GL_TRUE;
	GLenum GlewInitResult = glewInit();

	if (GLEW_OK != GlewInitResult)
	{
		std::cerr << glewGetErrorString(GlewInitResult) << std::endl;
		return false;
	}

	// Displays GPU OpenGL version
	cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << endl;

	return true;
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void UProcessInput(GLFWwindow* window)
{
	static const float cameraSpeed = 2.5f;

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		gCamera.ProcessKeyboard(FORWARD, gDeltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		gCamera.ProcessKeyboard(BACKWARD, gDeltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		gCamera.ProcessKeyboard(LEFT, gDeltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		gCamera.ProcessKeyboard(RIGHT, gDeltaTime);
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		gCamera.ProcessKeyboard(UP, gDeltaTime);
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		gCamera.ProcessKeyboard(DOWN, gDeltaTime);
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
		ortho = 0;
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		ortho = 1;



}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void UResizeWindow(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos)
{
	if (gFirstMouse)
	{
		gLastX = xpos;
		gLastY = ypos;
		gFirstMouse = false;
	}

	float xoffset = xpos - gLastX;
	float yoffset = gLastY - ypos; // reversed since y-coordinates go from bottom to top

	gLastX = xpos;
	gLastY = ypos;

	gCamera.ProcessMouseMovement(xoffset, yoffset);
}


// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	gCamera.ProcessMouseScroll(yoffset);
}

// glfw: handle mouse button events
// --------------------------------
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	switch (button)
	{
	case GLFW_MOUSE_BUTTON_LEFT:
	{
		if (action == GLFW_PRESS)
			cout << "Left mouse button pressed" << endl;
		else
			cout << "Left mouse button released" << endl;
	}
	break;

	case GLFW_MOUSE_BUTTON_MIDDLE:
	{
		if (action == GLFW_PRESS)
			cout << "Middle mouse button pressed" << endl;
		else
			cout << "Middle mouse button released" << endl;
	}
	break;

	case GLFW_MOUSE_BUTTON_RIGHT:
	{
		if (action == GLFW_PRESS)
			cout << "Right mouse button pressed" << endl;
		else
			cout << "Right mouse button released" << endl;
	}
	break;

	default:
		cout << "Unhandled mouse button event" << endl;
		break;
	}
}


// Functioned called to render a frame
// Functioned called to render a frame
void URender()
{
	// Lamp orbits around the origin
	const float angularVelocity = glm::radians(45.0f);
	if (gIsLampOrbiting)
	{
		glm::vec4 newPosition = glm::rotate(angularVelocity * gDeltaTime, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(gLightPosition, 1.0f);
		gLightPosition.x = newPosition.x;
		gLightPosition.y = newPosition.y;
		gLightPosition.z = newPosition.z;
	}

	// Enable z-depth
	glEnable(GL_DEPTH_TEST);

	// Clear the frame and z buffers
	glClearColor(0.086f, 0.024f, 0.004f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Activate the cube VAO (used by cube and lamp)
	glBindVertexArray(gMesh.vao[0]);
	glBindVertexArray(gMesh.Lvao);
	// CUBE: draw cube
	//----------------
	// Set the shader to be used
	glUseProgram(gCubeProgramId);

	// Model matrix: transformations are applied right-to-left order
	glm::mat4 model = glm::translate(gCubePosition) * glm::scale(gCubeScale);

	// camera/view transformation
	glm::mat4 view = gCamera.GetViewMatrix();
	glm::mat4 rotation = glm::rotate(90.0f, glm::vec3(1.0, 1.0f, 0.0f));

	// Creates a perspective projection
	glm::mat4 viewport;
	if (ortho == 0)
		viewport = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);
	else
		viewport = glm::ortho((GLfloat)-8.0f, (GLfloat)+8.0f, (GLfloat)-6.0f, (GLfloat)+6.0f, 0.1f, 100.f);
	glm::mat4 projection = viewport;


	// Retrieves and passes transform matrices to the Shader program
	GLint modelLoc = glGetUniformLocation(gCubeProgramId, "model");
	GLint viewLoc = glGetUniformLocation(gCubeProgramId, "view");
	GLint projLoc = glGetUniformLocation(gCubeProgramId, "projection");

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	// Reference matrix uniforms from the Cube Shader program for the cub color, light color, light position, and camera position
	GLint objectColorLoc = glGetUniformLocation(gCubeProgramId, "objectColor");
	GLint lightColorLoc = glGetUniformLocation(gCubeProgramId, "lightColor");
	GLint lightPositionLoc = glGetUniformLocation(gCubeProgramId, "lightPos");
	GLint viewPositionLoc = glGetUniformLocation(gCubeProgramId, "viewPosition");

	GLint objectColorLoc2 = glGetUniformLocation(gCubeProgramId, "objectColor2");
	GLint lightColorLoc2 = glGetUniformLocation(gCubeProgramId, "lightColor2");
	GLint lightPositionLoc2 = glGetUniformLocation(gCubeProgramId, "lightPos2");
	GLint viewPositionLoc2 = glGetUniformLocation(gCubeProgramId, "viewPosition2");

	// Pass color, light, and camera data to the Cube Shader program's corresponding uniforms
	glUniform3f(objectColorLoc, gObjectColor.r, gObjectColor.g, gObjectColor.b);
	glUniform3f(lightColorLoc, gLightColor.r, gLightColor.g, gLightColor.b);
	glUniform3f(lightPositionLoc, gLightPosition.x, gLightPosition.y, gLightPosition.z);

	glUniform3f(objectColorLoc2, gObjectColor2.r, gObjectColor2.g, gObjectColor2.b);
	glUniform3f(lightColorLoc2, gLightColor2.r, gLightColor2.g, gLightColor2.b);
	glUniform3f(lightPositionLoc2, gLightPosition2.x, gLightPosition2.y, gLightPosition2.z);

	const glm::vec3 cameraPosition = gCamera.Position;
	glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);

	GLint UVScaleLoc = glGetUniformLocation(gCubeProgramId, "uvScale");
	glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));

	// bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gTextureId);

	// Draws the triangles
	glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices[0]);


	// LAMP: draw lamp
	//----------------
	glUseProgram(gLampProgramId);

	//Transform the smaller cube used as a visual que for the light source
   // model = glm::translate(gLightPosition) * glm::scale(gLightScale);

	// Reference matrix uniforms from the Lamp Shader program
	modelLoc = glGetUniformLocation(gLampProgramId, "model");
	viewLoc = glGetUniformLocation(gLampProgramId, "view");
	projLoc = glGetUniformLocation(gLampProgramId, "projection");

	// Pass matrix data to the Lamp Shader program's matrix uniforms
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	// LAMP: draw lamp
	//----------------


	// Pass matrix data to the Lamp Shader program's matrix uniforms
	glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices[0]);

	// Deactivate the Vertex Array Object and shader program
	glBindVertexArray(0);
	glUseProgram(0);

	// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
	glfwSwapBuffers(gWindow);    // Flips the the back buffer with the front buffer every frame.
}


// Implements the UCreateMesh function
void UCreateMesh(GLMesh& mesh)
{
	// Position and Color data
	GLfloat verts[] = {
		//Positions          //Normals
		// ------------------------------------------------------
		//Back Face          //Negative Z Normal  Texture Coords.
		2.848235,   -0.750000,   2.772352,   0.0000,   -0.1007,   0.9949,   0.890659,   0.467699,
		3.000000,   1.500000,   3.000000,   0.0000,   -0.1007,   0.9949,   0.888340,   0.693074,
		2.000000,   1.500000,   3.000000,   0.0000,   -0.1007,   0.9949,   0.866652,   0.681762,
		4.000000,   -0.750000,   2.899560,   0.0000,   1.0000,   0.0000,   0.241206,   0.697623,
		2.500000,   -0.750000,   3.649560,   0.0000,   1.0000,   0.0000,   0.365543,   0.869300,
		4.000000,   -0.750000,   1.399560,   0.0000,   1.0000,   0.0000,   0.239364,   0.343698,
		4.000000,   -1.250000,   2.899560,   0.4472,   0.0000,   0.8944,   0.215926,   0.623001,
		2.500000,   -0.750000,   3.649560,   0.4472,   0.0000,   0.8944,   0.124493,   0.701175,
		4.000000,   -0.750000,   2.899560,   0.4472,   0.0000,   0.8944,   0.215926,   0.701175,
		-1.000000,   -0.750000,   3.149560,   0.0000,   1.0000,   0.0000,   0.659496,   0.737572,
		-0.600000,   -0.750000,   3.149560,   0.0000,   1.0000,   0.0000,   0.626614,   0.739217,
		-0.600000,   -0.750000,   3.649560,   0.0000,   1.0000,   0.0000,   0.627322,   0.857197,
		-1.500000,   -0.750000,   3.649560,   0.0000,   1.0000,   0.0000,   0.701363,   0.853374,
		-1.000000,   -0.750000,   3.149560,   0.0000,   1.0000,   0.0000,   0.659496,   0.737572,
		-0.600000,   -0.750000,   3.649560,   0.0000,   1.0000,   0.0000,   0.627322,   0.857197,
		-0.500000,   -0.850000,   3.149560,   0.0000,   0.0000,   -1.0000,   0.145078,   0.403405,
		0.000000,   -1.050000,   3.149560,   0.0000,   0.0000,   -1.0000,   0.172338,   0.434675,
		-1.000000,   -1.050000,   3.149560,   0.0000,   0.0000,   -1.0000,   0.117818,   0.434675,
		-0.500000,   -0.841882,   0.649560,   -0.6765,   0.7363,   0.0120,   0.611379,   0.151864,
		-0.400000,   -0.750000,   0.649560,   -0.6765,   0.7363,   0.0120,   0.600176,   0.152108,
		-0.500000,   -0.850000,   1.149560,   -0.6765,   0.7363,   0.0120,   0.612105,   0.269456,
		2.500000,   -0.750000,   3.649560,   0.4472,   0.0000,   0.8944,   0.231946,   0.551159,
		4.000000,   -0.750000,   2.899560,   0.4472,   0.0000,   0.8944,   0.140513,   0.551159,
		2.500000,   -0.500000,   3.649560,   0.4472,   0.0000,   0.8944,   0.231946,   0.512071,
		1.000000,   -0.750000,   2.926301,   -1.0000,   0.0000,   0.0000,   0.232868,   0.568034,
		1.000000,   -0.500000,   2.926301,   -1.0000,   0.0000,   0.0000,   0.232868,   0.607122,
		1.000000,   -0.500000,   1.399560,   -1.0000,   0.0000,   0.0000,   0.149630,   0.607122,
		2.500000,   -0.500000,   0.899560,   -0.4472,   0.0000,   0.8944,   0.086928,   0.588294,
		3.750000,   -0.750000,   1.524560,   -0.4472,   0.0000,   0.8944,   0.163122,   0.567870,
		3.750000,   -0.500000,   1.524560,   -0.4472,   0.0000,   0.8944,   0.163122,   0.588294,
		4.000000,   -0.500000,   1.399560,   0.0000,   1.0000,   0.0000,   0.260363,   0.569683,
		3.750000,   -0.500000,   1.524560,   0.0000,   1.0000,   0.0000,   0.251220,   0.551415,
		2.500000,   -0.500000,   0.649560,   0.0000,   1.0000,   0.0000,   0.168930,   0.569683,
		4.000000,   -0.500000,   2.899560,   0.0000,   1.0000,   0.0000,   0.047167,   0.615872,
		2.500000,   -0.500000,   3.649560,   0.0000,   1.0000,   0.0000,   0.138600,   0.615873,
		2.500000,   -0.500000,   3.399560,   0.0000,   1.0000,   0.0000,   0.132504,   0.580912,
		1.250000,   -0.500000,   2.774560,   1.0000,   0.0000,   0.0000,   0.092272,   0.814717,
		1.250000,   -0.750000,   2.774560,   1.0000,   0.0000,   0.0000,   0.092272,   0.853804,
		1.250000,   -0.750000,   1.524560,   1.0000,   0.0000,   0.0000,   0.024122,   0.853804,
		1.000000,   -0.500000,   2.926301,   0.0000,   1.0000,   0.0000,   0.252168,   0.588951,
		1.250000,   -0.500000,   2.774560,   0.0000,   1.0000,   0.0000,   0.243896,   0.609375,
		1.250000,   -0.500000,   1.524560,   0.0000,   1.0000,   0.0000,   0.175745,   0.609375,
		2.500000,   -0.750000,   0.899560,   -0.4472,   0.0000,   0.8944,   0.086928,   0.567870,
		2.500000,   -0.500000,   0.899560,   -0.4472,   0.0000,   0.8944,   0.086928,   0.588294,
		3.750000,   -0.750000,   1.524560,   -0.4472,   0.0000,   0.8944,   0.163122,   0.567870,
		-0.500000,   -0.850000,   3.649560,   0.0000,   0.0000,   1.0000,   0.218982,   0.890536,
		-0.600000,   -0.750000,   3.649560,   0.0000,   0.0000,   1.0000,   0.224434,   0.874901,
		-1.500000,   -1.250000,   3.649560,   0.0000,   0.0000,   1.0000,   0.273502,   0.953075,
		-0.600000,   -0.750000,   1.149560,   0.0000,   0.0000,   1.0000,   0.146825,   0.718450,
		-1.000000,   -0.750000,   1.149560,   0.0000,   0.0000,   1.0000,   0.168633,   0.718450,
		-1.000000,   -1.050000,   1.149560,   0.0000,   0.0000,   1.0000,   0.168633,   0.765355,
		-2.500000,   -0.750000,   3.149560,   0.0000,   1.0000,   0.0000,   0.782860,   0.731029,
		-1.500000,   -0.750000,   2.149560,   0.0000,   1.0000,   0.0000,   0.699110,   0.499688,
		-1.500000,   -0.750000,   3.649560,   0.0000,   1.0000,   0.0000,   0.701363,   0.853374,
		2.500000,   -0.500000,   3.399560,   0.4472,   0.0000,   -0.8944,   0.155726,   0.492470,
		1.250000,   -0.500000,   2.774560,   0.4472,   0.0000,   -0.8944,   0.231920,   0.492470,
		1.250000,   -0.750000,   2.774560,   0.4472,   0.0000,   -0.8944,   0.231920,   0.453383,
		2.500000,   -1.250000,   0.649560,   0.4472,   0.0000,   -0.8944,   0.118449,   0.629551,
		2.500000,   -0.750000,   0.649560,   0.4472,   0.0000,   -0.8944,   0.118449,   0.707638,
		4.000000,   -0.750000,   1.399560,   0.4472,   0.0000,   -0.8944,   0.027119,   0.707638,
		1.250000,   -0.750000,   1.524560,   1.0000,   0.0000,   0.0000,   0.024122,   0.853804,
		1.250000,   -0.500000,   2.774560,   1.0000,   0.0000,   0.0000,   0.092272,   0.814717,
		1.250000,   -0.500000,   1.524560,   1.0000,   0.0000,   0.0000,   0.024122,   0.814717,
		2.500000,   -0.750000,   3.399560,   -0.4472,   0.0000,   -0.8944,   0.283099,   0.792221,
		2.500000,   -0.500000,   3.399560,   -0.4472,   0.0000,   -0.8944,   0.283099,   0.831309,
		3.750000,   -0.500000,   2.774560,   -0.4472,   0.0000,   -0.8944,   0.206905,   0.831309,
		-1.500000,   -1.250000,   3.649560,   -0.4472,   0.0000,   0.8944,   0.273502,   0.953075,
		-1.500000,   -0.750000,   3.649560,   -0.4472,   0.0000,   0.8944,   0.273502,   0.874901,
		-2.500000,   -0.750000,   3.149560,   -0.4472,   0.0000,   0.8944,   0.334457,   0.874901,
		-2.500000,   -1.250000,   1.149560,   -0.4472,   0.0000,   -0.8944,   0.443497,   0.953075,
		-1.500000,   -0.750000,   0.649560,   -0.4472,   0.0000,   -0.8944,   0.504452,   0.874901,
		-2.500000,   -0.750000,   1.149560,   -0.4472,   0.0000,   -0.8944,   0.443497,   0.874901,
		0.000000,   -0.750000,   3.149560,   -0.8944,   0.0000,   -0.4472,   0.172338,   0.387770,
		0.000000,   -1.050000,   3.149560,   -0.8944,   0.0000,   -0.4472,   0.172338,   0.434675,
		0.500000,   -1.050000,   2.149560,   -0.8944,   0.0000,   -0.4472,   0.233293,   0.434675,
		-0.400000,   -0.750000,   3.149560,   0.0000,   1.0000,   0.0000,   0.603334,   0.740374,
		-0.400000,   -0.750000,   3.649560,   0.0000,   1.0000,   0.0000,   0.604044,   0.858350,
		0.000000,   -0.750000,   3.149560,   0.0000,   1.0000,   0.0000,   0.570460,   0.741981,
		-0.400000,   -0.750000,   3.649560,   0.0000,   0.0000,   1.0000,   0.213530,   0.874902,
		-0.500000,   -0.850000,   3.649560,   0.0000,   0.0000,   1.0000,   0.218982,   0.890536,
		2.500000,   -1.250000,   3.649560,   0.0000,   0.0000,   1.0000,   0.055421,   0.953075,
		3.750000,   -0.500000,   2.774560,   -0.4472,   0.0000,   -0.8944,   0.206905,   0.831309,
		3.750000,   -0.750000,   2.774560,   -0.4472,   0.0000,   -0.8944,   0.206905,   0.792221,
		2.500000,   -0.750000,   3.399560,   -0.4472,   0.0000,   -0.8944,   0.283099,   0.792221,
		-0.400000,   -0.750000,   1.149560,   -0.0000,   0.0000,   1.0000,   0.135921,   0.718450,
		-0.500000,   -0.850000,   1.149560,   -0.0000,   0.0000,   1.0000,   0.141373,   0.734085,
		0.000000,   -1.050000,   1.149560,   -0.0000,   0.0000,   1.0000,   0.114113,   0.765355,
		0.500000,   -0.750000,   2.149560,   0.0000,   1.0000,   0.0000,   0.527996,   0.508251,
		2.500000,   -0.750000,   0.649560,   0.0000,   1.0000,   0.0000,   0.361858,   0.161451,
		2.500000,   -0.750000,   3.649560,   0.0000,   1.0000,   0.0000,   0.365543,   0.869300,
		-0.600000,   -0.750000,   3.649560,   0.7071,   0.7071,   0.0000,   0.627322,   0.857197,
		-0.500000,   -0.850000,   3.149560,   0.7071,   0.7071,   0.0000,   0.614974,   0.739795,
		-0.600000,   -0.750000,   3.149560,   0.7071,   0.7071,   0.0000,   0.626614,   0.739217,
		-1.000000,   -1.050000,   3.149560,   0.0000,   0.0000,   -1.0000,   0.117818,   0.434675,
		-1.000000,   -0.750000,   3.149560,   0.0000,   0.0000,   -1.0000,   0.117818,   0.387770,
		-0.600000,   -0.750000,   3.149560,   0.0000,   0.0000,   -1.0000,   0.139626,   0.387770,
		-1.500000,   -1.050000,   2.149560,   0.0000,   1.0000,   0.0000,   0.550332,   0.499939,
		0.000000,   -1.050000,   1.149560,   0.0000,   1.0000,   0.0000,   0.619330,   0.621843,
		-1.000000,   -1.050000,   1.149560,   0.0000,   1.0000,   0.0000,   0.573331,   0.621843,
		-2.500000,   -1.250000,   3.149560,   -1.0000,   0.0000,   0.0000,   0.334457,   0.953075,
		-2.500000,   -0.750000,   3.149560,   -1.0000,   0.0000,   0.0000,   0.334457,   0.874901,
		-2.500000,   -0.750000,   1.149560,   -1.0000,   0.0000,   0.0000,   0.443497,   0.874901,
		-2.500000,   -0.750000,   1.149560,   0.0000,   1.0000,   0.0000,   0.779693,   0.259416,
		-2.500000,   -0.750000,   3.149560,   0.0000,   1.0000,   0.0000,   0.782860,   0.731029,
		-1.500000,   -0.750000,   2.149560,   0.0000,   1.0000,   0.0000,   0.699110,   0.499688,
		-0.600000,   -0.750000,   0.649560,   0.6765,   0.7363,   0.0120,   0.622579,   0.150975,
		-0.500000,   -0.841882,   0.649560,   0.6765,   0.7363,   0.0120,   0.611379,   0.151864,
		-0.500000,   -0.850000,   1.149560,   0.6765,   0.7363,   0.0120,   0.612105,   0.269456,
		-0.500000,   -0.850000,   1.149560,   0.0000,   0.0000,   1.0000,   0.141373,   0.734085,
		-0.600000,   -0.750000,   1.149560,   0.0000,   0.0000,   1.0000,   0.146825,   0.718450,
		-1.000000,   -1.050000,   1.149560,   0.0000,   0.0000,   1.0000,   0.168633,   0.765355,
		4.000000,   -1.250000,   1.399560,   1.0000,   0.0000,   0.0000,   0.126619,   0.290150,
		4.000000,   -0.746703,   1.399560,   1.0000,   0.0000,   0.0000,   0.126619,   0.368840,
		4.000000,   -0.750000,   2.899560,   1.0000,   0.0000,   0.0000,   0.044838,   0.368324,
		-1.500000,   -0.750000,   0.649560,   0.0000,   1.0000,   0.0000,   0.696653,   0.146207,
		-1.000000,   -0.750000,   1.149560,   0.0000,   1.0000,   0.0000,   0.656443,   0.266595,
		-1.500000,   -0.750000,   2.149560,   0.0000,   1.0000,   0.0000,   0.699110,   0.499688,
		-1.500000,   -1.250000,   0.649560,   0.0000,   0.0000,   -1.0000,   0.504452,   0.953075,
		-0.600000,   -0.750000,   0.649560,   0.0000,   0.0000,   -1.0000,   0.553521,   0.874901,
		-0.500000,   -0.841882,   0.649560,   0.0000,   0.0000,   -1.0000,   0.558973,   0.889266,
		4.000000,   -0.746703,   1.399560,   1.0000,   0.0000,   0.0000,   0.124548,   0.525166,
		4.000000,   -0.500000,   1.399560,   1.0000,   0.0000,   0.0000,   0.124548,   0.563738,
		4.000000,   -0.500000,   2.899560,   1.0000,   0.0000,   0.0000,   0.042768,   0.563738,
		1.000000,   -0.500000,   1.399560,   0.0000,   1.0000,   0.0000,   0.260363,   0.570183,
		1.250000,   -0.500000,   1.524560,   0.0000,   1.0000,   0.0000,   0.251220,   0.588451,
		2.500000,   -0.500000,   0.649560,   0.0000,   1.0000,   0.0000,   0.168930,   0.570183,
		2.500000,   -0.750000,   3.649560,   0.0000,   0.0000,   1.0000,   0.055421,   0.874900,
		-0.400000,   -0.750000,   3.649560,   0.0000,   0.0000,   1.0000,   0.213530,   0.874902,
		2.500000,   -1.250000,   3.649560,   0.0000,   0.0000,   1.0000,   0.055421,   0.953075,
		0.000000,   -0.750000,   1.149560,   0.0000,   0.0000,   1.0000,   0.114113,   0.718450,
		-0.400000,   -0.750000,   1.149560,   0.0000,   0.0000,   1.0000,   0.135921,   0.718450,
		0.000000,   -1.050000,   1.149560,   0.0000,   0.0000,   1.0000,   0.114113,   0.765355,
		0.000000,   -1.050000,   3.149560,   0.0000,   1.0000,   0.0000,   0.619330,   0.378036,
		0.000000,   -1.050000,   1.149560,   0.0000,   1.0000,   0.0000,   0.619330,   0.621843,
		-1.500000,   -1.050000,   2.149560,   0.0000,   1.0000,   0.0000,   0.550332,   0.499939,
		-1.000000,   -0.750000,   1.149560,   0.8944,   0.0000,   0.4472,   0.168633,   0.718450,
		-1.500000,   -0.750000,   2.149560,   0.8944,   0.0000,   0.4472,   0.229588,   0.718450,
		-1.500000,   -1.050000,   2.149560,   0.8944,   0.0000,   0.4472,   0.229588,   0.765355,
		2.500000,   -0.750000,   0.649560,   -0.4472,   0.0000,   -0.8944,   0.148073,   0.494563,
		1.000000,   -0.750000,   1.399560,   -0.4472,   0.0000,   -0.8944,   0.056640,   0.494563,
		2.500000,   -0.500000,   0.649560,   -0.4472,   0.0000,   -0.8944,   0.148073,   0.455476,
		1.000000,   -0.750000,   1.399560,   -0.4472,   0.0000,   -0.8944,   0.056640,   0.494563,
		1.000000,   -0.500000,   1.399560,   -0.4472,   0.0000,   -0.8944,   0.056640,   0.455476,
		2.500000,   -0.500000,   0.649560,   -0.4472,   0.0000,   -0.8944,   0.148073,   0.455476,
		-0.400000,   -0.750000,   1.149560,   -0.7071,   0.7071,   0.0000,   0.600524,   0.269868,
		-0.500000,   -0.850000,   1.149560,   -0.7071,   0.7071,   0.0000,   0.612105,   0.269456,
		-0.400000,   -0.750000,   0.649560,   -0.7071,   0.7071,   0.0000,   0.600176,   0.152108,
		-0.400000,   -0.750000,   1.149560,   0.0000,   1.0000,   0.0000,   0.600524,   0.269868,
		-0.400000,   -0.750000,   0.649560,   0.0000,   1.0000,   0.0000,   0.600176,   0.152108,
		0.000000,   -0.750000,   1.149560,   0.0000,   1.0000,   0.0000,   0.567761,   0.271095,
		-0.400000,   -0.750000,   0.649560,   0.0000,   0.0000,   -1.0000,   0.564425,   0.874901,
		2.500000,   -1.250000,   0.649560,   0.0000,   0.0000,   -1.0000,   0.722533,   0.953075,
		2.500000,   -0.750000,   0.649560,   0.0000,   0.0000,   -1.0000,   0.722533,   0.874900,
		-0.400000,   -0.750000,   0.649560,   0.0000,   0.0000,   -1.0000,   0.564425,   0.874901,
		2.500000,   -1.250000,   0.649560,   0.0000,   0.0000,   -1.0000,   0.722533,   0.953075,
		-0.500000,   -0.841882,   0.649560,   0.0000,   0.0000,   -1.0000,   0.558973,   0.889266,
		-0.400000,   -0.750000,   0.649560,   0.0000,   1.0000,   -0.0000,   0.600176,   0.152108,
		2.500000,   -0.750000,   0.649560,   0.0000,   1.0000,   -0.0000,   0.361858,   0.161451,
		0.000000,   -0.750000,   1.149560,   0.0000,   1.0000,   -0.0000,   0.567761,   0.271095,
		2.500000,   -1.250000,   0.649560,   0.4472,   0.0000,   -0.8944,   0.118449,   0.629551,
		4.000000,   -0.750000,   1.399560,   0.4472,   0.0000,   -0.8944,   0.027119,   0.707638,
		4.000000,   -1.250000,   1.399560,   0.4472,   0.0000,   -0.8944,   0.027119,   0.629551,
		2.500000,   -1.250000,   0.649560,   0.0000,   0.0000,   -1.0000,   0.722533,   0.953075,
		-0.500000,   -0.841882,   0.649560,   0.0000,   0.0000,   -1.0000,   0.558973,   0.889266,
		-1.500000,   -1.250000,   0.649560,   0.0000,   0.0000,   -1.0000,   0.504452,   0.953075,
		4.000000,   -1.250000,   1.399560,   -1.0000,   0.0000,   0.0000,   0.027119,   0.629551,
		4.000000,   -0.750000,   1.399560,   -1.0000,   0.0000,   0.0000,   0.027119,   0.707638,
		4.000000,   -0.746703,   1.399560,   -1.0000,   0.0000,   0.0000,   0.026644,   0.707625,
		4.000000,   -1.250000,   1.399560,   1.0000,   0.0000,   0.0000,   0.126619,   0.290150,
		4.000000,   -0.750000,   2.899560,   1.0000,   0.0000,   0.0000,   0.044838,   0.368324,
		4.000000,   -1.250000,   2.899560,   1.0000,   0.0000,   0.0000,   0.044838,   0.290150,
		-0.600000,   -0.750000,   1.149560,   0.7071,   0.7071,   0.0000,   0.623684,   0.268696,
		-0.500000,   -0.850000,   1.149560,   0.7071,   0.7071,   0.0000,   0.612105,   0.269456,
		-0.600000,   -0.750000,   0.649560,   0.7071,   0.7071,   0.0000,   0.622579,   0.150975,
		-0.600000,   -0.750000,   1.149560,   0.0000,   1.0000,   0.0000,   0.623684,   0.268696,
		-0.600000,   -0.750000,   0.649560,   0.0000,   1.0000,   0.0000,   0.622579,   0.150975,
		-1.000000,   -0.750000,   1.149560,   0.0000,   1.0000,   0.0000,   0.656443,   0.266595,
		-0.600000,   -0.750000,   0.649560,   0.0000,   0.0000,   -1.0000,   0.553521,   0.874901,
		-1.500000,   -1.250000,   0.649560,   0.0000,   0.0000,   -1.0000,   0.504452,   0.953075,
		-1.500000,   -0.750000,   0.649560,   0.0000,   0.0000,   -1.0000,   0.504452,   0.874901,
		-0.600000,   -0.750000,   0.649560,   0.0000,   1.0000,   0.0000,   0.622579,   0.150975,
		-1.500000,   -0.750000,   0.649560,   0.0000,   1.0000,   0.0000,   0.696653,   0.146207,
		-1.000000,   -0.750000,   1.149560,   0.0000,   1.0000,   0.0000,   0.656443,   0.266595,
		-0.500000,   -0.850000,   1.149560,   0.0000,   0.0000,   1.0000,   0.141373,   0.734085,
		-1.000000,   -1.050000,   1.149560,   0.0000,   0.0000,   1.0000,   0.168633,   0.765355,
		0.000000,   -1.050000,   1.149560,   0.0000,   0.0000,   1.0000,   0.114113,   0.765355,
		2.500000,   -0.750000,   0.649560,   0.4472,   0.0000,   -0.8944,   0.101792,   0.850071,
		2.500000,   -0.500000,   0.649560,   0.4472,   0.0000,   -0.8944,   0.101792,   0.810984,
		4.000000,   -0.746703,   1.399560,   0.4472,   0.0000,   -0.8944,   0.193225,   0.849556,
		4.000000,   -0.746703,   1.399560,   0.4472,   0.0000,   -0.8944,   0.193225,   0.849556,
		2.500000,   -0.500000,   0.649560,   0.4472,   0.0000,   -0.8944,   0.101792,   0.810984,
		4.000000,   -0.500000,   1.399560,   0.4472,   0.0000,   -0.8944,   0.193225,   0.810984,
		4.000000,   -0.746703,   1.399560,   1.0000,   0.0000,   0.0000,   0.124548,   0.525166,
		4.000000,   -0.750000,   2.899560,   1.0000,   0.0000,   0.0000,   0.042768,   0.524651,
		4.000000,   -0.500000,   2.899560,   1.0000,   0.0000,   0.0000,   0.042768,   0.563738,
		4.000000,   -0.746703,   1.399560,   -1.0000,   0.0000,   0.0000,   0.026644,   0.707625,
		4.000000,   -0.750000,   1.399560,   -1.0000,   0.0000,   0.0000,   0.027119,   0.707638,
		4.000000,   -0.750000,   2.899560,   -1.0000,   0.0000,   0.0000,   0.026631,   0.785712,
		1.000000,   -0.750000,   2.926301,   -1.0000,   0.0000,   0.0000,   0.232868,   0.568034,
		1.000000,   -0.500000,   1.399560,   -1.0000,   0.0000,   0.0000,   0.149630,   0.607122,
		1.000000,   -0.750000,   1.399560,   -1.0000,   0.0000,   0.0000,   0.149630,   0.568034,
		1.000000,   -0.750000,   2.926301,   -0.4343,   0.0000,   0.9008,   0.226038,   0.354170,
		2.500000,   -0.500000,   3.649560,   -0.4343,   0.0000,   0.9008,   0.135248,   0.315083,
		2.500000,   -0.750000,   3.649560,   -0.4343,   0.0000,   0.9008,   0.135248,   0.354170,
		1.000000,   -0.750000,   2.926301,   -0.4343,   0.0000,   0.9008,   0.226038,   0.354170,
		2.500000,   -0.500000,   3.649560,   -0.4343,   0.0000,   0.9008,   0.135248,   0.315083,
		1.000000,   -0.500000,   2.926301,   -0.4343,   0.0000,   0.9008,   0.226038,   0.315083,
		4.000000,   -0.750000,   2.899560,   0.4472,   0.0000,   0.8944,   0.140513,   0.551159,
		2.500000,   -0.500000,   3.649560,   0.4472,   0.0000,   0.8944,   0.231946,   0.512071,
		4.000000,   -0.500000,   2.899560,   0.4472,   0.0000,   0.8944,   0.140513,   0.512071,
		-1.500000,   -0.750000,   0.649560,   0.0000,   1.0000,   0.0000,   0.696653,   0.146207,
		-1.500000,   -0.750000,   2.149560,   0.0000,   1.0000,   0.0000,   0.699110,   0.499688,
		-2.500000,   -0.750000,   1.149560,   0.0000,   1.0000,   0.0000,   0.779693,   0.259416,
		-1.500000,   -0.750000,   0.649560,   -0.4472,   0.0000,   -0.8944,   0.504452,   0.874901,
		-2.500000,   -1.250000,   1.149560,   -0.4472,   0.0000,   -0.8944,   0.443497,   0.953075,
		-1.500000,   -1.250000,   0.649560,   -0.4472,   0.0000,   -0.8944,   0.504452,   0.953075,
		2.500000,   -0.500000,   3.649560,   0.0000,   1.0000,   0.0000,   0.259721,   0.609875,
		1.250000,   -0.500000,   2.774560,   0.0000,   1.0000,   0.0000,   0.177615,   0.629912,
		2.500000,   -0.500000,   3.399560,   0.0000,   1.0000,   0.0000,   0.253801,   0.628272,
		2.500000,   -0.500000,   3.649560,   0.0000,   1.0000,   0.0000,   0.259721,   0.609875,
		1.000000,   -0.500000,   2.926301,   0.0000,   1.0000,   0.0000,   0.168930,   0.609875,
		1.250000,   -0.500000,   2.774560,   0.0000,   1.0000,   0.0000,   0.177615,   0.629912,
		2.500000,   -0.500000,   3.399560,   0.4472,   0.0000,   -0.8944,   0.155726,   0.492470,
		2.500000,   -0.750000,   3.399560,   0.4472,   0.0000,   -0.8944,   0.155726,   0.453383,
		1.250000,   -0.750000,   2.774560,   0.4472,   0.0000,   -0.8944,   0.231920,   0.453383,
		2.500000,   -0.500000,   3.399560,   0.0000,   1.0000,   0.0000,   0.132504,   0.580912,
		3.750000,   -0.500000,   2.774560,   0.0000,   1.0000,   0.0000,   0.056310,   0.580912,
		4.000000,   -0.500000,   2.899560,   0.0000,   1.0000,   0.0000,   0.047167,   0.615872,
		3.750000,   -0.500000,   2.774560,   0.0000,   1.0000,   0.0000,   0.093743,   0.630143,
		4.000000,   -0.500000,   1.399560,   0.0000,   1.0000,   0.0000,   0.168708,   0.609718,
		3.750000,   -0.500000,   1.524560,   0.0000,   1.0000,   0.0000,   0.161893,   0.630143,
		3.750000,   -0.500000,   2.774560,   -1.0000,   0.0000,   0.0000,   0.094972,   0.546945,
		3.750000,   -0.750000,   1.524560,   -1.0000,   0.0000,   0.0000,   0.163122,   0.567370,
		3.750000,   -0.750000,   2.774560,   -1.0000,   0.0000,   0.0000,   0.094972,   0.567370,
		3.750000,   -0.500000,   2.774560,   -1.0000,   0.0000,   0.0000,   0.094972,   0.546945,
		3.750000,   -0.500000,   1.524560,   -1.0000,   0.0000,   0.0000,   0.163122,   0.546945,
		3.750000,   -0.750000,   1.524560,   -1.0000,   0.0000,   0.0000,   0.163122,   0.567370,
		3.750000,   -0.500000,   2.774560,   0.0000,   1.0000,   0.0000,   0.093743,   0.630143,
		4.000000,   -0.500000,   1.399560,   0.0000,   1.0000,   0.0000,   0.168708,   0.609718,
		4.000000,   -0.500000,   2.899560,   0.0000,   1.0000,   0.0000,   0.086928,   0.609718,
		1.000000,   -0.500000,   1.399560,   0.0000,   1.0000,   0.0000,   0.168930,   0.588951,
		1.250000,   -0.500000,   1.524560,   0.0000,   1.0000,   0.0000,   0.175745,   0.609375,
		1.000000,   -0.500000,   2.926301,   0.0000,   1.0000,   0.0000,   0.252168,   0.588951,
		0.500000,   -0.750000,   2.149560,   0.0000,   1.0000,   0.0000,   0.527996,   0.508251,
		2.500000,   -0.750000,   0.649560,   0.0000,   1.0000,   0.0000,   0.361858,   0.161451,
		0.000000,   -0.750000,   1.149560,   0.0000,   1.0000,   0.0000,   0.567761,   0.271095,
		0.500000,   -0.750000,   2.149560,   0.0000,   1.0000,   0.0000,   0.527996,   0.508251,
		2.500000,   -0.750000,   3.649560,   0.0000,   1.0000,   0.0000,   0.365543,   0.869300,
		0.000000,   -0.750000,   3.149560,   0.0000,   1.0000,   0.0000,   0.570460,   0.741981,
		0.500000,   -0.750000,   2.149560,   -0.8944,   0.0000,   0.4472,   0.053157,   0.718450,
		0.000000,   -1.050000,   1.149560,   -0.8944,   0.0000,   0.4472,   0.114113,   0.765355,
		0.500000,   -1.050000,   2.149560,   -0.8944,   0.0000,   0.4472,   0.053157,   0.765355,
		0.500000,   -0.750000,   2.149560,   -0.8944,   0.0000,   0.4472,   0.053157,   0.718450,
		0.000000,   -1.050000,   1.149560,   -0.8944,   0.0000,   0.4472,   0.114113,   0.765355,
		0.000000,   -0.750000,   1.149560,   -0.8944,   0.0000,   0.4472,   0.114113,   0.718450,
		0.500000,   -0.750000,   2.149560,   -0.8944,   0.0000,   -0.4472,   0.233293,   0.387770,
		0.500000,   -1.050000,   2.149560,   -0.8944,   0.0000,   -0.4472,   0.233293,   0.434675,
		0.000000,   -0.750000,   3.149560,   -0.8944,   0.0000,   -0.4472,   0.172338,   0.387770,
		-1.000000,   -0.750000,   1.149560,   0.8944,   0.0000,   0.4472,   0.168633,   0.718450,
		-1.500000,   -1.050000,   2.149560,   0.8944,   0.0000,   0.4472,   0.229588,   0.765355,
		-1.000000,   -1.050000,   1.149560,   0.8944,   0.0000,   0.4472,   0.168633,   0.765355,
		2.500000,   -0.750000,   3.649560,   0.0000,   1.0000,   0.0000,   0.365543,   0.869300,
		0.000000,   -0.750000,   3.149560,   0.0000,   1.0000,   0.0000,   0.570460,   0.741981,
		-0.400000,   -0.750000,   3.649560,   0.0000,   1.0000,   0.0000,   0.604044,   0.858350,
		2.500000,   -0.750000,   3.649560,   0.4472,   0.0000,   0.8944,   0.124493,   0.701175,
		4.000000,   -1.250000,   2.899560,   0.4472,   0.0000,   0.8944,   0.215926,   0.623001,
		2.500000,   -1.250000,   3.649560,   0.4472,   0.0000,   0.8944,   0.124493,   0.623001,
		2.500000,   -0.750000,   3.649560,   0.0000,   1.0000,   0.0000,   0.365543,   0.869300,
		2.500000,   -0.750000,   0.649560,   0.0000,   1.0000,   0.0000,   0.361858,   0.161451,
		4.000000,   -0.750000,   1.399560,   0.0000,   1.0000,   0.0000,   0.239364,   0.343698,
		-0.500000,   -0.850000,   3.649560,   -0.7071,   0.7071,   0.0000,   0.615683,   0.857774,
		-0.500000,   -0.850000,   3.149560,   -0.7071,   0.7071,   0.0000,   0.614974,   0.739795,
		-0.400000,   -0.750000,   3.649560,   -0.7071,   0.7071,   0.0000,   0.604044,   0.858350,
		-0.500000,   -0.850000,   3.649560,   0.7071,   0.7071,   0.0000,   0.615683,   0.857774,
		-0.500000,   -0.850000,   3.149560,   0.7071,   0.7071,   0.0000,   0.614974,   0.739795,
		-0.600000,   -0.750000,   3.649560,   0.7071,   0.7071,   0.0000,   0.627322,   0.857197,
		-0.500000,   -0.850000,   3.649560,   0.0000,   0.0000,   1.0000,   0.218982,   0.890536,
		2.500000,   -1.250000,   3.649560,   0.0000,   0.0000,   1.0000,   0.055421,   0.953075,
		-1.500000,   -1.250000,   3.649560,   0.0000,   0.0000,   1.0000,   0.273502,   0.953075,
		-0.400000,   -0.750000,   3.649560,   -0.7071,   0.7071,   0.0000,   0.604044,   0.858350,
		-0.500000,   -0.850000,   3.149560,   -0.7071,   0.7071,   0.0000,   0.614974,   0.739795,
		-0.400000,   -0.750000,   3.149560,   -0.7071,   0.7071,   0.0000,   0.603334,   0.740374,
		-0.400000,   -0.750000,   3.149560,   0.0000,   0.0000,   -1.0000,   0.150530,   0.387770,
		0.000000,   -1.050000,   3.149560,   0.0000,   0.0000,   -1.0000,   0.172338,   0.434675,
		0.000000,   -0.750000,   3.149560,   0.0000,   0.0000,   -1.0000,   0.172338,   0.387770,
		-0.400000,   -0.750000,   3.149560,   0.0000,   0.0000,   -1.0000,   0.150530,   0.387770,
		0.000000,   -1.050000,   3.149560,   0.0000,   0.0000,   -1.0000,   0.172338,   0.434675,
		-0.500000,   -0.850000,   3.149560,   0.0000,   0.0000,   -1.0000,   0.145078,   0.403405,
		1.250000,   -0.750000,   1.524560,   0.4472,   0.0000,   0.8944,   0.163122,   0.609218,
		2.500000,   -0.500000,   0.899560,   0.4472,   0.0000,   0.8944,   0.086928,   0.588794,
		2.500000,   -0.750000,   0.899560,   0.4472,   0.0000,   0.8944,   0.086928,   0.609218,
		1.250000,   -0.750000,   1.524560,   0.4472,   0.0000,   0.8944,   0.163122,   0.609218,
		2.500000,   -0.500000,   0.899560,   0.4472,   0.0000,   0.8944,   0.086928,   0.588794,
		1.250000,   -0.500000,   1.524560,   0.4472,   0.0000,   0.8944,   0.163122,   0.588794,
		1.250000,   -0.500000,   1.524560,   0.0000,   1.0000,   0.0000,   0.251220,   0.588451,
		2.500000,   -0.500000,   0.899560,   0.0000,   1.0000,   0.0000,   0.175026,   0.588451,
		2.500000,   -0.500000,   0.649560,   0.0000,   1.0000,   0.0000,   0.168930,   0.570183,
		-2.500000,   -1.250000,   3.149560,   -1.0000,   0.0000,   0.0000,   0.334457,   0.953075,
		-2.500000,   -0.750000,   1.149560,   -1.0000,   0.0000,   0.0000,   0.443497,   0.874901,
		-2.500000,   -1.250000,   1.149560,   -1.0000,   0.0000,   0.0000,   0.443497,   0.953075,
		-2.500000,   -1.250000,   3.149560,   -0.4472,   0.0000,   0.8944,   0.334457,   0.953075,
		-2.500000,   -0.750000,   3.149560,   -0.4472,   0.0000,   0.8944,   0.334457,   0.874901,
		-1.500000,   -1.250000,   3.649560,   -0.4472,   0.0000,   0.8944,   0.273502,   0.953075,
		-0.500000,   -0.850000,   3.149560,   0.0000,   0.0000,   -1.0000,   0.145078,   0.403405,
		-1.000000,   -1.050000,   3.149560,   0.0000,   0.0000,   -1.0000,   0.117818,   0.434675,
		-0.600000,   -0.750000,   3.149560,   0.0000,   0.0000,   -1.0000,   0.139626,   0.387770,
		2.500000,   -0.500000,   0.899560,   0.0000,   1.0000,   0.0000,   0.175026,   0.551415,
		2.500000,   -0.500000,   0.649560,   0.0000,   1.0000,   0.0000,   0.168930,   0.569683,
		3.750000,   -0.500000,   1.524560,   0.0000,   1.0000,   0.0000,   0.251220,   0.551415,
		-1.500000,   -0.750000,   2.149560,   0.8944,   0.0000,   -0.4472,   0.056862,   0.387770,
		-1.000000,   -1.050000,   3.149560,   0.8944,   0.0000,   -0.4472,   0.117818,   0.434675,
		-1.000000,   -0.750000,   3.149560,   0.8944,   0.0000,   -0.4472,   0.117818,   0.387770,
		-1.500000,   -0.750000,   2.149560,   0.8944,   0.0000,   -0.4472,   0.056862,   0.387770,
		-1.000000,   -1.050000,   3.149560,   0.8944,   0.0000,   -0.4472,   0.117818,   0.434675,
		-1.500000,   -1.050000,   2.149560,   0.8944,   0.0000,   -0.4472,   0.056862,   0.434675,
		-1.500000,   -0.750000,   2.149560,   0.0000,   1.0000,   0.0000,   0.699110,   0.499688,
		-1.000000,   -0.750000,   3.149560,   0.0000,   1.0000,   0.0000,   0.659496,   0.737572,
		-1.500000,   -0.750000,   3.649560,   0.0000,   1.0000,   0.0000,   0.701363,   0.853374,
		-1.500000,   -1.250000,   3.649560,   0.0000,   0.0000,   1.0000,   0.273502,   0.953075,
		-0.600000,   -0.750000,   3.649560,   0.0000,   0.0000,   1.0000,   0.224434,   0.874901,
		-1.500000,   -0.750000,   3.649560,   0.0000,   0.0000,   1.0000,   0.273502,   0.874901,
		0.000000,   -1.050000,   1.149560,   0.0000,   1.0000,   0.0000,   0.619330,   0.621843,
		0.500000,   -1.050000,   2.149560,   0.0000,   1.0000,   0.0000,   0.642329,   0.499939,
		0.000000,   -1.050000,   3.149560,   0.0000,   1.0000,   0.0000,   0.619330,   0.378036,
		-1.000000,   -1.050000,   3.149560,   0.0000,   1.0000,   0.0000,   0.573331,   0.378036,
		0.000000,   -1.050000,   3.149560,   0.0000,   1.0000,   0.0000,   0.619330,   0.378036,
		-1.500000,   -1.050000,   2.149560,   0.0000,   1.0000,   0.0000,   0.550332,   0.499939,
		3.400000,   1.500000,   2.250000,   0.8773,   -0.1065,   -0.4679,   0.906895,   0.692206,
		3.000000,   1.500000,   1.500000,   0.8773,   -0.1065,   -0.4679,   0.925309,   0.681762,
		2.848235,   -0.750000,   1.727648,   0.8773,   -0.1065,   -0.4679,   0.916406,   0.459820,
		2.000000,   1.500000,   3.000000,   0.0000,   -0.1007,   0.9949,   0.866652,   0.681762,
		2.848235,   -0.750000,   2.772352,   0.0000,   -0.1007,   0.9949,   0.890659,   0.467699,
		2.151765,   -0.750000,   2.772352,   0.0000,   -0.1007,   0.9949,   0.875554,   0.459820,
		2.151765,   -0.750000,   1.727648,   -0.8773,   -0.1065,   -0.4679,   0.831384,   0.473549,
		1.873177,   -0.750000,   2.250000,   -0.8773,   -0.1065,   -0.4679,   0.844307,   0.472956,
		2.000000,   1.500000,   1.500000,   -0.8773,   -0.1065,   -0.4679,   0.829056,   0.698923,
		2.848235,   -0.750000,   1.727648,   0.0000,   -0.1007,   -0.9949,   0.816279,   0.465658,
		3.000000,   1.500000,   1.500000,   0.0000,   -0.1007,   -0.9949,   0.807368,   0.687592,
		2.000000,   1.500000,   1.500000,   0.0000,   -0.1007,   -0.9949,   0.829056,   0.698923,
		3.126823,   -0.750000,   2.250000,   0.8773,   -0.1065,   0.4679,   0.903582,   0.467094,
		2.848235,   -0.750000,   2.772352,   0.8773,   -0.1065,   0.4679,   0.890659,   0.467699,
		3.000000,   1.500000,   3.000000,   0.8773,   -0.1065,   0.4679,   0.888340,   0.693074,
		1.873177,   -0.750000,   2.250000,   -0.8766,   -0.1070,   -0.4692,   0.844307,   0.472956,
		2.000000,   1.500000,   1.500000,   -0.8766,   -0.1070,   -0.4692,   0.829056,   0.698923,
		1.600000,   1.488308,   2.250000,   -0.8766,   -0.1070,   -0.4692,   0.847608,   0.696913,
		2.000000,   1.500000,   1.500000,   0.0000,   -0.1007,   -0.9949,   0.829056,   0.698923,
		2.848235,   -0.750000,   1.727648,   0.0000,   -0.1007,   -0.9949,   0.816279,   0.465658,
		2.151765,   -0.750000,   1.727648,   0.0000,   -0.1007,   -0.9949,   0.831384,   0.473549,
		1.600000,   1.488308,   2.250000,   -0.8773,   -0.1071,   0.4679,   0.847608,   0.696913,
		1.873177,   -0.750000,   2.250000,   -0.8773,   -0.1071,   0.4679,   0.844307,   0.472956,
		2.151765,   -0.750000,   2.772352,   -0.8773,   -0.1071,   0.4679,   0.857130,   0.465658,
		2.151765,   -0.750000,   2.772352,   -0.8766,   -0.1066,   0.4692,   0.857130,   0.465658,
		1.600000,   1.488308,   2.250000,   -0.8766,   -0.1066,   0.4692,   0.847608,   0.696913,
		2.000000,   1.500000,   3.000000,   -0.8766,   -0.1066,   0.4692,   0.866052,   0.687583,
		3.000000,   1.500000,   3.000000,   0.8773,   -0.1065,   0.4679,   0.888340,   0.693074,
		3.400000,   1.500000,   2.250000,   0.8773,   -0.1065,   0.4679,   0.906895,   0.692206,
		3.126823,   -0.750000,   2.250000,   0.8773,   -0.1065,   0.4679,   0.903582,   0.467094,
		3.126823,   -0.750000,   2.250000,   0.8773,   -0.1065,   -0.4679,   0.903582,   0.467094,
		2.848235,   -0.750000,   1.727648,   0.8773,   -0.1065,   -0.4679,   0.916406,   0.459820,
		3.400000,   1.500000,   2.250000,   0.8773,   -0.1065,   -0.4679,   0.906895,   0.692206,
		10.590752,   -1.258675,   -6.067754,   0.0000,   1.0000,   0.0000,   0.972537,   0.394588,
		-9.570175,   -1.258675,   -6.067754,   0.0000,   1.0000,   0.0000,   0.814577,   0.394588,
		10.590752,   -1.258675,   10.534896,   0.0000,   1.0000,   0.0000,   0.972537,   0.004603,
		-9.570175,   -1.258675,   -6.067754,   0.0000,   1.0000,   -0.0000,   0.814577,   0.394588,
		-9.570175,   -1.258675,   10.534896,   0.0000,   1.0000,   -0.0000,   0.814577,   0.004603,
		10.590752,   -1.258675,   10.534896,   0.0000,   1.0000,   -0.0000,   0.972537,   0.004603,
		0.203589,   -0.799335,   1.536257,   0.7353,   0.2300,   0.6375,   0.838632,   0.891097,
		-1.476801,   -0.466772,   3.354340,   0.7353,   0.2300,   0.6375,   0.963802,   0.891057,
		-1.491681,   -0.293016,   3.308803,   0.7353,   0.2300,   0.6375,   0.963667,   0.861939,
		-1.476801,   -0.466772,   3.354340,   0.5783,   -0.5190,   0.6294,   0.958355,   0.655744,
		-1.559990,   -0.618956,   3.305288,   0.5783,   -0.5190,   0.6294,   0.950168,   0.655744,
		-1.720241,   -0.418594,   3.617728,   0.5783,   -0.5190,   0.6294,   0.958355,   0.714582,
		0.120400,   -0.951519,   1.487205,   0.6727,   -0.1331,   -0.7278, 0.977062, 0.524627,
		0.188709,   -0.625579,   1.490720,   0.6727,   -0.1331,   -0.7278, 0.972221, 0.490646,
		-0.116361,   -0.899437,   1.258848,   0.6727,   -0.1331,   -0.7278, 0.965375, 0.538702,
		-1.559990,   -0.618956,   3.305288,   0.0826,   -0.9640,   0.2526,   0.950168,   0.655744,
		-1.803430,   -0.570777,   3.568676,   0.0826,   -0.9640,   0.2526,   0.950168,   0.714582,
		-1.935958,   -0.612240,   3.453770,   0.0826,   -0.9640,   0.2526,   0.941981,   0.714582,
		-0.012128,   -0.992982,   1.372299,   0.0826,   -0.9640,   0.2526,   0.838614,   0.953967,
		-1.559990,   -0.618956,   3.305288,   0.0826,   -0.9640,   0.2526,   0.964372,   0.920979,
		-1.692518,   -0.660419,   3.190382,   0.0826,   -0.9640,   0.2526,   0.965360,   0.951659,
		-1.692518,   -0.660419,   3.190382,   -0.4615,   -0.8443,   -0.2721,   0.941981,   0.655744,
		-1.796751,   -0.566874,   3.076931,   -0.4615,   -0.8443,   -0.2721,   0.933794,   0.655744,
		-1.935958,   -0.612240,   3.453770,   -0.4615,   -0.8443,   -0.2721,   0.941981,   0.714582,
		-0.116361,   -0.899437,   1.258848,   0.6727,   -0.1331,   -0.7278, 0.965375, 0.538702,
		-0.131241,   -0.725680,   1.213311,   0.6727,   -0.1331,   -0.7278, 0.960534, 0.524627,
		-0.048052,   -0.573497,   1.262363,   0.6727,   -0.1331,   -0.7278, 0.960534, 0.504721,
		-1.796751,   -0.566874,   3.076931,   -0.7353,   -0.2300,   -0.6375,   0.999290,   0.655744,
		-2.040192,   -0.518695,   3.340319,   -0.7353,   -0.2300,   -0.6375,   0.999290,   0.714582,
		-2.055072,   -0.344939,   3.294783,   -0.7353,   -0.2300,   -0.6375,   0.991104,   0.714582,
		-0.131241,   -0.725680,   1.213311,   -0.5783,   0.5190,   -0.6294,   0.838890,   0.765457,
		-1.811632,   -0.393117,   3.031395,   -0.5783,   0.5190,   -0.6294,   0.965583,   0.771918,
		-1.728442,   -0.240934,   3.080446,   -0.5783,   0.5190,   -0.6294,   0.964507,   0.802956,
		-1.811632,   -0.393117,   3.031395,   -0.5783,   0.5190,   -0.6294,   0.991104,   0.655744,
		-2.055072,   -0.344939,   3.294783,   -0.5783,   0.5190,   -0.6294,   0.991104,   0.714582,
		-1.971882,   -0.192755,   3.343834,   -0.5783,   0.5190,   -0.6294,   0.982916,   0.714582,
		-0.048052,   -0.573497,   1.262363,   0.6727,   -0.1331,   -0.7278, 0.960534, 0.504721,
		0.084476,   -0.532034,   1.377269,   0.6727,   -0.1331,   -0.7278, 0.965375, 0.490646,
		0.188709,   -0.625579,   1.490720,   0.6727,   -0.1331,   -0.7278, 0.972221, 0.490646,
		-1.728442,   -0.240934,   3.080446,   -0.0826,   0.9640,   -0.2526,   0.982916,   0.655744,
		-1.971882,   -0.192755,   3.343834,   -0.0826,   0.9640,   -0.2526,   0.982916,   0.714582,
		-1.839355,   -0.151292,   3.458741,   -0.0826,   0.9640,   -0.2526,   0.974729,   0.714582,
		0.084476,   -0.532034,   1.377269,   -0.0826,   0.9640,   -0.2526,   0.838728,   0.828317,
		-1.728442,   -0.240934,   3.080446,   -0.0826,   0.9640,   -0.2526,   0.964507,   0.802956,
		-1.595914,   -0.199471,   3.195353,   -0.0826,   0.9640,   -0.2526,   0.963866,   0.832835,
		-1.595914,   -0.199471,   3.195353,   0.4615,   0.8443,   0.2721,   0.974729,   0.655744,
		-1.839355,   -0.151292,   3.458741,   0.4615,   0.8443,   0.2721,   0.974729,   0.714582,
		-1.735121,   -0.244837,   3.572191,   0.4615,   0.8443,   0.2721,   0.966542,   0.714582,
		0.188709,   -0.625579,   1.490720,   0.4615,   0.8443,   0.2721,   0.838670,   0.859704,
		-1.595914,   -0.199471,   3.195353,   0.4615,   0.8443,   0.2721,   0.963866,   0.832835,
		-1.491681,   -0.293016,   3.308803,   0.4615,   0.8443,   0.2721,   0.963667,   0.861939,
		-1.803430,   -0.570777,   3.568676,   -0.1710,   -0.8472,   0.5029,   0.980724,   0.912637,
		-1.995804,   -0.459620,   3.690495,   -0.1710,   -0.8472,   0.5029,   0.993249,   0.901487,
		-2.072132,   -0.483500,   3.624317,   -0.1710,   -0.8472,   0.5029,   0.995347,   0.920783,
		-1.720241,   -0.418594,   3.617728,   0.2898,   -0.4335,   0.8533,   0.979530,   0.887166,
		-1.947893,   -0.371973,   3.718746,   0.2898,   -0.4335,   0.8533,   0.991922,   0.881864,
		-1.995804,   -0.459620,   3.690495,   0.2898,   -0.4335,   0.8533,   0.993249,   0.901487,
		-1.935958,   -0.612240,   3.453770,   -0.6769,   -0.7359,   0.0151,   0.982714,   0.940000,
		-2.072132,   -0.483500,   3.624317,   -0.6769,   -0.7359,   0.0151,   0.995347,   0.920783,
		-2.132163,   -0.429624,   3.558977,   -0.6769,   -0.7359,   0.0151,   0.998470,   0.939022,
		-2.040192,   -0.518695,   3.340319,   -0.9314,   -0.1648,   -0.3246,   0.986791,   0.754995,
		-2.132163,   -0.429624,   3.558977,   -0.9314,   -0.1648,   -0.3246,   0.998470,   0.785985,
		-2.140733,   -0.329552,   3.532751,   -0.9314,   -0.1648,   -0.3246,   0.995521,   0.804070,
		-2.055072,   -0.344939,   3.294783,   -0.7854,   0.5315,   -0.3171,   0.983054,   0.784599,
		-1.971882,   -0.192755,   3.343834,   -0.7854,   0.5315,   -0.3171,   0.980840,   0.811386,
		-2.140733,   -0.329552,   3.532751,   -0.7854,   0.5315,   -0.3171,   0.995521,   0.804070,
		-1.971882,   -0.192755,   3.343834,   -0.3245,   0.9453,   0.0332,   0.980840,   0.811386,
		-1.839355,   -0.151292,   3.458741,   -0.3245,   0.9453,   0.0332,   0.979586,   0.836977,
		-2.092822,   -0.241905,   3.561002,   -0.3245,   0.9453,   0.0332,   0.993342,   0.823197,
		-1.735121,   -0.244837,   3.572191,   0.4358,   0.2629,   0.8608,   0.979141,   0.862083,
		-1.956463,   -0.271900,   3.692520,   0.4358,   0.2629,   0.8608,   0.991307,   0.862274,
		-1.947893,   -0.371973,   3.718746,   0.4358,   0.2629,   0.8608,   0.991922,   0.881864,
		-2.140733,   -0.329552,   3.532751,   -0.6727,   0.1331,   0.7278,   0.995790,   0.741529,
		-2.092822,   -0.241905,   3.561002,   -0.6727,   0.1331,   0.7278,   0.993426,   0.735653,
		-2.016494,   -0.218025,   3.627180,   -0.6727,   0.1331,   0.7278,   0.990082,   0.735653,
		0.203589,   -0.799335,   1.536257,   0.5783,   -0.5190,   0.6294,   0.838632,   0.891097,
		-1.476801,   -0.466772,   3.354340,   0.5783,   -0.5190,   0.6294,   0.963802,   0.891057,
		0.120400,   -0.951519,   1.487205,   0.5783,   -0.5190,   0.6294,   0.838614,   0.922512,
		0.203589,   -0.799335,   1.536257,   0.7353,   0.2300,   0.6375,   0.838632,   0.891097,
		-1.491681,   -0.293016,   3.308803,   0.7353,   0.2300,   0.6375,   0.963667,   0.861939,
		0.188709,   -0.625579,   1.490720,   0.7353,   0.2300,   0.6375,   0.838670,   0.859704,
		0.203589,   -0.799335,   1.536257,   0.6727,   -0.1331,   -0.7278, 0.977062, 0.504721,
		0.188709,   -0.625579,   1.490720,   0.6727,   -0.1331,   -0.7278, 0.972221, 0.490646,
		0.120400,   -0.951519,   1.487205,   0.6727,   -0.1331,   -0.7278, 0.977062, 0.524627,
		0.120400,   -0.951519,   1.487205,   0.0826,   -0.9640,   0.2526,   0.838614,   0.922512,
		-0.012128,   -0.992982,   1.372299,   0.0826,   -0.9640,   0.2526,   0.838614,   0.953967,
		-1.559990,   -0.618956,   3.305288,   0.0826,   -0.9640,   0.2526,   0.964372,   0.920979,
		0.120400,   -0.951519,   1.487205,   0.6727,   -0.1331,   -0.7278, 0.977062, 0.524627,
		-0.012128,   -0.992982,   1.372299,   0.6727,   -0.1331,   -0.7278, 0.972221, 0.538702,
		-0.116361,   -0.899437,   1.258848,   0.6727,   -0.1331,   -0.7278, 0.965375, 0.538702,
		0.120400,   -0.951519,   1.487205,   0.5783,   -0.5190,   0.6294,   0.838614,   0.922512,
		-1.476801,   -0.466772,   3.354340,   0.5783,   -0.5190,   0.6294,   0.963802,   0.891057,
		-1.559990,   -0.618956,   3.305288,   0.5783,   -0.5190,   0.6294,   0.964372,   0.920979,
		-1.559990,   -0.618956,   3.305288,   0.0826,   -0.9640,   0.2526,   0.950168,   0.655744,
		-1.935958,   -0.612240,   3.453770,   0.0826,   -0.9640,   0.2526,   0.941981,   0.714582,
		-1.692518,   -0.660419,   3.190382,   0.0826,   -0.9640,   0.2526,   0.941981,   0.655744,
		-1.559990,   -0.618956,   3.305288,   0.5783,   -0.5190,   0.6294,   0.950168,   0.655744,
		-1.720241,   -0.418594,   3.617728,   0.5783,   -0.5190,   0.6294,   0.958355,   0.714582,
		-1.803430,   -0.570777,   3.568676,   0.5783,   -0.5190,   0.6294,   0.950168,   0.714582,
		-1.476801,   -0.466772,   3.354340,   0.7353,   0.2300,   0.6375,   0.958355,   0.655744,
		-1.735121,   -0.244837,   3.572191,   0.7353,   0.2300,   0.6375,   0.966542,   0.714582,
		-1.491681,   -0.293016,   3.308803,   0.7353,   0.2300,   0.6375,   0.966542,   0.655744,
		-1.476801,   -0.466772,   3.354340,   0.7353,   0.2300,   0.6375,   0.958355,   0.655744,
		-1.735121,   -0.244837,   3.572191,   0.7353,   0.2300,   0.6375,   0.966542,   0.714582,
		-1.720241,   -0.418594,   3.617728,   0.7353,   0.2300,   0.6375,   0.958355,   0.714582,
		-0.012128,   -0.992982,   1.372299,   -0.4615,   -0.8443,   -0.2721,   0.838614,   0.953967,
		-0.116361,   -0.899437,   1.258848,   -0.4615,   -0.8443,   -0.2721,   0.838629,   0.985475,
		-1.692518,   -0.660419,   3.190382,   -0.4615,   -0.8443,   -0.2721,   0.965360,   0.951659,
		-1.692518,   -0.660419,   3.190382,   -0.4615,   -0.8443,   -0.2721,   0.965360,   0.951659,
		-0.116361,   -0.899437,   1.258848,   -0.4615,   -0.8443,   -0.2721,   0.838629,   0.985475,
		-1.796751,   -0.566874,   3.076931,   -0.4615,   -0.8443,   -0.2721,   0.966614,   0.984231,
		-0.116361,   -0.899437,   1.258848,   -0.7353,   -0.2300,   -0.6375,   0.839000,   0.733877,
		-0.131241,   -0.725680,   1.213311,   -0.7353,   -0.2300,   -0.6375,   0.838890,   0.765457,
		-1.796751,   -0.566874,   3.076931,   -0.7353,   -0.2300,   -0.6375,   0.967018,   0.739059,
		-0.116361,   -0.899437,   1.258848,   0.6727,   -0.1331,   -0.7278, 0.965375, 0.538702,
		0.188709,   -0.625579,   1.490720,   0.6727,   -0.1331,   -0.7278, 0.972221, 0.490646,
		-0.048052,   -0.573497,   1.262363,   0.6727,   -0.1331,   -0.7278, 0.960534, 0.504721,
		-1.796751,   -0.566874,   3.076931,   -0.7353,   -0.2300,   -0.6375,   0.999290,   0.655744,
		-2.055072,   -0.344939,   3.294783,   -0.7353,   -0.2300,   -0.6375,   0.991104,   0.714582,
		-1.811632,   -0.393117,   3.031395,   -0.7353,   -0.2300,   -0.6375,   0.991104,   0.655744,
		-1.796751,   -0.566874,   3.076931,   -0.4615,   -0.8443,   -0.2721,   0.933794,   0.655744,
		-1.935958,   -0.612240,   3.453770,   -0.4615,   -0.8443,   -0.2721,   0.941981,   0.714582,
		-2.040192,   -0.518695,   3.340319,   -0.4615,   -0.8443,   -0.2721,   0.933794,   0.714582,
		-1.796751,   -0.566874,   3.076931,   -0.7353,   -0.2300,   -0.6375,   0.967018,   0.739059,
		-0.131241,   -0.725680,   1.213311,   -0.7353,   -0.2300,   -0.6375,   0.838890,   0.765457,
		-1.811632,   -0.393117,   3.031395,   -0.7353,   -0.2300,   -0.6375,   0.965583,   0.771918,
		-0.131241,   -0.725680,   1.213311,   -0.5783,   0.5190,   -0.6294,   0.838890,   0.765457,
		-1.728442,   -0.240934,   3.080446,   -0.5783,   0.5190,   -0.6294,   0.964507,   0.802956,
		-0.048052,   -0.573497,   1.262363,   -0.5783,   0.5190,   -0.6294,   0.838805,   0.796903,
		-1.811632,   -0.393117,   3.031395,   -0.5783,   0.5190,   -0.6294,   0.991104,   0.655744,
		-1.971882,   -0.192755,   3.343834,   -0.5783,   0.5190,   -0.6294,   0.982916,   0.714582,
		-1.728442,   -0.240934,   3.080446,   -0.5783,   0.5190,   -0.6294,   0.982916,   0.655744,
		-0.048052,   -0.573497,   1.262363,   -0.0826,   0.9640,   -0.2526,   0.838805,   0.796903,
		-1.728442,   -0.240934,   3.080446,   -0.0826,   0.9640,   -0.2526,   0.964507,   0.802956,
		0.084476,   -0.532034,   1.377269,   -0.0826,   0.9640,   -0.2526,   0.838728,   0.828317,
		-1.728442,   -0.240934,   3.080446,   -0.0826,   0.9640,   -0.2526,   0.982916,   0.655744,
		-1.839355,   -0.151292,   3.458741,   -0.0826,   0.9640,   -0.2526,   0.974729,   0.714582,
		-1.595914,   -0.199471,   3.195353,   -0.0826,   0.9640,   -0.2526,   0.974729,   0.655744,
		0.084476,   -0.532034,   1.377269,   0.4615,   0.8443,   0.2721,   0.838728,   0.828317,
		0.188709,   -0.625579,   1.490720,   0.4615,   0.8443,   0.2721,   0.838670,   0.859704,
		-1.595914,   -0.199471,   3.195353,   0.4615,   0.8443,   0.2721,   0.963866,   0.832835,
		-1.595914,   -0.199471,   3.195353,   0.4615,   0.8443,   0.2721,   0.974729,   0.655744,
		-1.735121,   -0.244837,   3.572191,   0.4615,   0.8443,   0.2721,   0.966542,   0.714582,
		-1.491681,   -0.293016,   3.308803,   0.4615,   0.8443,   0.2721,   0.966542,   0.655744,
		-1.803430,   -0.570777,   3.568676,   -0.1710,   -0.8472,   0.5029,   0.980724,   0.912637,
		-2.072132,   -0.483500,   3.624317,   -0.1710,   -0.8472,   0.5029,   0.995347,   0.920783,
		-1.935958,   -0.612240,   3.453770,   -0.1710,   -0.8472,   0.5029,   0.982714,   0.940000,
		-1.803430,   -0.570777,   3.568676,   0.2898,   -0.4335,   0.8533,   0.980724,   0.912637,
		-1.720241,   -0.418594,   3.617728,   0.2898,   -0.4335,   0.8533,   0.979530,   0.887166,
		-1.995804,   -0.459620,   3.690495,   0.2898,   -0.4335,   0.8533,   0.993249,   0.901487,
		-1.720241,   -0.418594,   3.617728,   0.4358,   0.2629,   0.8608,   0.979530,   0.887166,
		-1.735121,   -0.244837,   3.572191,   0.4358,   0.2629,   0.8608,   0.979141,   0.862083,
		-1.947893,   -0.371973,   3.718746,   0.4358,   0.2629,   0.8608,   0.991922,   0.881864,
		-1.935958,   -0.612240,   3.453770,   -0.6769,   -0.7359,   0.0150,   0.982714,   0.940000,
		-2.040192,   -0.518695,   3.340319,   -0.6769,   -0.7359,   0.0150,   0.986205,   0.969848,
		-2.132163,   -0.429624,   3.558977,   -0.6769,   -0.7359,   0.0150,   0.998470,   0.939022,
		-2.040192,   -0.518695,   3.340319,   -0.9314,   -0.1648,   -0.3246,   0.986791,   0.754995,
		-2.140733,   -0.329552,   3.532751,   -0.9314,   -0.1648,   -0.3246,   0.995521,   0.804070,
		-2.055072,   -0.344939,   3.294783,   -0.9314,   -0.1648,   -0.3246,   0.983054,   0.784599,
		-1.971882,   -0.192755,   3.343834,   -0.7854,   0.5315,   -0.3171,   0.980840,   0.811386,
		-2.140733,   -0.329552,   3.532751,   -0.7854,   0.5315,   -0.3171,   0.995521,   0.804070,
		-2.092822,   -0.241905,   3.561002,   -0.7854,   0.5315,   -0.3171,   0.993342,   0.823197,
		-1.839355,   -0.151292,   3.458741,   0.1813,   0.8340,   0.5211,   0.979586,   0.836977,
		-2.016494,   -0.218025,   3.627180,   0.1813,   0.8340,   0.5211,   0.991964,   0.842726,
		-1.735121,   -0.244837,   3.572191,   0.1813,   0.8340,   0.5211,   0.979141,   0.862083,
		-1.839355,   -0.151292,   3.458741,   -0.3245,   0.9453,   0.0332,   0.979586,   0.836977,
		-2.092822,   -0.241905,   3.561002,   -0.3245,   0.9453,   0.0332,   0.993342,   0.823197,
		-2.016494,   -0.218025,   3.627180,   -0.3245,   0.9453,   0.0332,   0.991964,   0.842726,
		-1.735121,   -0.244837,   3.572191,   0.1813,   0.8340,   0.5211,   0.979141,   0.862083,
		-2.016494,   -0.218025,   3.627180,   0.1813,   0.8340,   0.5211,   0.991964,   0.842726,
		-1.956463,   -0.271900,   3.692520,   0.1813,   0.8340,   0.5211,   0.991307,   0.862274,
		-1.995804,   -0.459620,   3.690495,   -0.6727,   0.1331,   0.7278,   0.990082,   0.755715,
		-2.016494,   -0.218025,   3.627180,   -0.6727,   0.1331,   0.7278,   0.990082,   0.735653,
		-1.947893,   -0.371973,   3.718746,   -0.6727,   0.1331,   0.7278,   0.987718,   0.749839,
		-1.995804,   -0.459620,   3.690495,   -0.6727,   0.1331,   0.7278,   0.990082,   0.755715,
		-2.016494,   -0.218025,   3.627180,   -0.6727,   0.1331,   0.7278,   0.990082,   0.735653,
		-2.072132,   -0.483500,   3.624317,   -0.6727,   0.1331,   0.7278,   0.993426,   0.755715,
		-1.947893,   -0.371973,   3.718746,   -0.6727,   0.1331,   0.7278,   0.987718,   0.749839,
		-2.016494,   -0.218025,   3.627180,   -0.6727,   0.1331,   0.7278,   0.990082,   0.735653,
		-1.956463,   -0.271900,   3.692520,   -0.6727,   0.1331,   0.7278,   0.987718,   0.741529,
		-2.072132,   -0.483500,   3.624317,   -0.6727,   0.1331,   0.7278,   0.993426,   0.755715,
		-2.140733,   -0.329552,   3.532751,   -0.6727,   0.1331,   0.7278,   0.995790,   0.741529,
		-2.132163,   -0.429624,   3.558977,   -0.6727,   0.1331,   0.7278,   0.995790,   0.749839,
		-2.072132,   -0.483500,   3.624317,   -0.6727,   0.1331,   0.72781,   0.993426,   0.755715,
		-2.140733,   -0.329552,   3.532751,   -0.6727,   0.1331,   0.72781,   0.995790,   0.741529,
		-2.016494,   -0.218025,   3.627180,   -0.6727,   0.1331,   0.72781,   0.990082,   0.735653,
		//plane extension
			-9.570176, -1.258675, 10.534896, 0.0000, 1.0000, 0.0000, 0.374822, 0.000527,
			-23.520500, -1.258675, 25.202240, 0.0000, 1.0000, 0.0000, 0.373354, 0.090548,
			24.222895, -1.2586756, 25.202240, 0.0000, 1.0000, 0.0000, 0.266165, 0.090548,
			10.590751, -1.258675, -6.067754, 0.0000, 1.0000, 0.0000, 0.725650, 0.000940,
			24.222895, -1.258675, -22.541180, 0.0000, 1.0000, 0.0000, 0.736238, 0.091170,
			24.222895, -1.258675, 25.202240, 0.0000, 1.0000, 0.0000, 0.622560, 0.091170,
			-9.570176, -1.258675, -6.067754, 0.0000, 1.0000, 0.0000, 0.606039, 0.000859,
			-23.520500, -1.258675, -22.541180, 0.0000, 1.0000, 0.0000, 0.616623, 0.091139,
			-23.520500, -1.258675, 25.202240, 0.0000, 1.0000, 0.0000, 0.502984, 0.091139,
			-23.520500, -1.258675, -22.541180, 0.0000, 1.0000, 0.0000, 0.386533, 0.090787,
			24.222895, -1.258675, -22.541180, 0.0000, 1.0000, 0.0000, 0.493722, 0.090787,
			-9.570176, -1.258675, -6.067754, 0.0000, 1.0000, 0.0000, 0.385065, 0.000656,
			24.222895, -1.258675, -22.541180, 0.0000, 1.0000, 0.0000, 0.493722, 0.090787,
			10.590751, -1.258675, -6.067754, 0.0000, 1.0000, 0.0000, 0.495904, 0.000656,
			-9.570176, -1.258675, -6.067754, 0.0000, 1.0000, 0.0000, 0.385065, 0.000656,
			-9.570176, -1.258675, 10.534896, 0.0000, 1.0000, 0.0000, 0.509269, 0.000859,
			-9.570176, -1.258675, -6.067754, 0.0000, 1.0000, 0.0000, 0.606039, 0.000859,
			-23.520500, -1.258675, 25.202240, 0.0000, 1.0000, 0.0000, 0.502984, 0.091139,
			10.590751, -1.258675, 10.534896, 0.0000, 1.0000, 0.0000, 0.263983, 0.000527,
			-9.570176, -1.258675, 10.534896, 0.0000, 1.0000, 0.0000, 0.374822, 0.000527,
			24.222895, -1.258675, 25.202240, 0.0000, 1.0000, 0.0000, 0.266165, 0.090548,
			10.590751, -1.258675, 10.534896, 0.0000, 1.0000, 0.0000, 0.628848, 0.000940,
			10.590751, -1.258675, -6.067754, 0.0000, 1.0000, 0.0000, 0.725650, 0.000940,
			24.222895, -1.258675, 25.202240, 0.0000, 1.0000, 0.0000, 0.622560, 0.091170
	};

	const GLuint floatsPerVertex = 3;
	const GLuint floatsPerNormal = 3;
	const GLuint floatsPerUV = 2;

	mesh.nVertices[0] = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerNormal + floatsPerUV));

	glGenVertexArrays(1, &mesh.vao[0]); // we can also generate multiple VAOs or buffers at the same time
	glBindVertexArray(mesh.vao[0]);

	glGenVertexArrays(1, &mesh.Lvao); // we can also generate multiple VAOs or buffers at the same time
	glBindVertexArray(mesh.Lvao);

	// Create 2 buffers: first one for the vertex data; second one for the indices
	glGenBuffers(1, &mesh.vbo[0]);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo[0]); // Activates the buffer

	glGenBuffers(1, &mesh.Lvbo);
	glBindBuffer(GL_ARRAY_BUFFER, mesh.Lvbo); // Activates the buffer

	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

	// Strides between vertex coordinates is 6 (x, y, z, r, g, b, a). A tightly packed stride is 0.
	GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV);// The number of floats before each

	// Create Vertex Attribute Pointers
	glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
	glEnableVertexAttribArray(2);
}



void UDestroyMesh(GLMesh& mesh)
{
	glDeleteVertexArrays(1, &mesh.vao[0]);
	glDeleteBuffers(1, &mesh.vao[0]);
	glDeleteVertexArrays(1, &mesh.Lvao);
	glDeleteBuffers(1, &mesh.Lvbo);
}


/*Generate and load the texture*/
bool UCreateTexture(const char* filename, GLuint& textureId)
{
	int width, height, channels;
	unsigned char* image = stbi_load(filename, &width, &height, &channels, 0);
	if (image)
	{
		flipImageVertically(image, width, height, channels);

		glGenTextures(1, &textureId);
		glBindTexture(GL_TEXTURE_2D, textureId);

		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		if (channels == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		else if (channels == 4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		else
		{
			cout << "Not implemented to handle image with " << channels << " channels" << endl;
			return false;
		}

		glGenerateMipmap(GL_TEXTURE_2D);

		stbi_image_free(image);
		glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

		return true;
	}

	// Error loading the image
	return false;
}


void UDestroyTexture(GLuint textureId)
{
	glGenTextures(1, &textureId);
}


// Implements the UCreateShaders function
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId)
{
	// Compilation and linkage error reporting
	int success = 0;
	char infoLog[512];

	// Create a Shader program object.
	programId = glCreateProgram();

	// Create the vertex and fragment shader objects
	GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

	// Retrive the shader source
	glShaderSource(vertexShaderId, 1, &vtxShaderSource, NULL);
	glShaderSource(fragmentShaderId, 1, &fragShaderSource, NULL);

	// Compile the vertex shader, and print compilation errors (if any)
	glCompileShader(vertexShaderId); // compile the vertex shader
	// check for shader compile errors
	glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShaderId, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;

		return false;
	}

	glCompileShader(fragmentShaderId); // compile the fragment shader
	// check for shader compile errors
	glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShaderId, sizeof(infoLog), NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;

		return false;
	}

	// Attached compiled shaders to the shader program
	glAttachShader(programId, vertexShaderId);
	glAttachShader(programId, fragmentShaderId);

	glLinkProgram(programId);   // links the shader program
	// check for linking errors
	glGetProgramiv(programId, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(programId, sizeof(infoLog), NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;

		return false;
	}

	glUseProgram(programId);    // Uses the shader program

	return true;
}


void UDestroyShaderProgram(GLuint programId)
{
	glDeleteProgram(programId);
}
