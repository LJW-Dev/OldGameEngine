#include "CubeDefaults.h"
#include "StandardOpenGL.h"
#include "ShaderCompile.h"
#include "Window.h"
#include <math.h>
#include <Windows.h>

#include "tinyfiledialogs.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

GLuint shader_ColourID;
GLuint shader_TextureID;

GLuint matrix_ColourID;
GLuint matrix_TextureID;

// This will identify our vertex buffer
GLuint vertexbuffer;
GLuint colorbuffer;
GLuint UVbuffer;

glm::mat4 ProjectionMatrix;
glm::mat4 ViewMatrix;
glm::mat4 ModelMatrix;

#define PI 3.14159265

#define HALF_PI 3.14159265 / 2

#define CUBE_POINTS_COUNT 24 * 3
struct cubePreview
{
	bool isBeingUsed;
	bool sideMoving;
	bool scaling;

	float xPos;
	float yPos;
	float zPos;

	float xScale;
	float yScale;
	float zScale;

	float* points;
	float* colours;
};

struct renderCube
{
	bool isUsed;
	GLuint texture;

	float* vertexes;
	int vertexCount;

	GLfloat* UVs;
	int UVCount;

	glm::vec3 origin;
	glm::vec3 mins;
	glm::vec3 maxs;
};



cubePreview previewCube;

#define RENDER_OBJECTS_MAX 100
int freeRenderObjectIndex = 0;
renderCube renderObjects[RENDER_OBJECTS_MAX];
const char* textureNameArray[RENDER_OBJECTS_MAX];

// position
glm::vec3 position = glm::vec3(0, 0, 5);
// horizontal angle : toward -Z
float horizontalAngle = 3.14f;
// vertical angle : 0, look at the horizon
float verticalAngle = 0.0f;
// Initial Field of View
float initialFoV = 45.0f;

float speed = 10.0f;
float mouseSpeed = 0.5f;

double lastTime = 0;

void computeMatricesFromInputs()
{

	double currentTime = glfwGetTime();
	float deltaTime = float(currentTime - lastTime);
	lastTime = currentTime;

	if (getKeyboardInput(GLFW_KEY_LEFT_CONTROL) != GLFW_PRESS)
		return;


	// Get mouse position
	double xpos, ypos;
	getMousePos(&xpos, &ypos);
	setMousePos(1024 / 2, 768 / 2);

	// Compute new orientation
	horizontalAngle += mouseSpeed * deltaTime * float(1024 / 2 - xpos);
	verticalAngle += mouseSpeed * deltaTime * float(768 / 2 - ypos);

	// Can't spin weirdly and break the camera
	if (horizontalAngle > PI)
		horizontalAngle = -PI;
	else if (horizontalAngle < -PI)
		horizontalAngle = PI;

	if (verticalAngle > HALF_PI)
		verticalAngle = HALF_PI;
	else if (verticalAngle < -HALF_PI)
		verticalAngle = -HALF_PI;

	glm::vec3 direction(
		cos(verticalAngle) * sin(horizontalAngle),
		sin(verticalAngle),
		cos(verticalAngle) * cos(horizontalAngle)
	);

	glm::vec3 right = glm::vec3(
		sin(horizontalAngle - 3.14f / 2.0f),
		0,
		cos(horizontalAngle - 3.14f / 2.0f)
	);

	glm::vec3 up = glm::cross(right, direction);



	// Move forward
	if (getKeyboardInput(GLFW_KEY_W) == GLFW_PRESS) {
		position += direction * deltaTime * speed;
	}
	// Move backward
	if (getKeyboardInput(GLFW_KEY_S) == GLFW_PRESS) {
		position -= direction * deltaTime * speed;
	}
	// Strafe right
	if (getKeyboardInput(GLFW_KEY_D) == GLFW_PRESS) {
		position += right * deltaTime * speed;
	}
	// Strafe left
	if (getKeyboardInput(GLFW_KEY_A) == GLFW_PRESS) {
		position -= right * deltaTime * speed;
	}

	//float FoV = initialFoV - 5 * glfwGetMouseWheel();
	// Camera matrix
	ViewMatrix = glm::lookAt(
		position,           // Camera is here
		position + direction, // and looks here : at the same position, plus "direction"
		up                  // Head is up (set to 0,-1,0 to look upside-down)
	);
}

GLuint loadMaterialIntoGL(const char* materialName)
{
	int width;
	int height;
	int channels;

	unsigned char* imageInfo = stbi_load(materialName, &width, &height, &channels, 0);

	// Create one OpenGL texture
	GLuint textureID;
	glGenTextures(1, &textureID);

	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, imageInfo);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);

	if (textureID > RENDER_OBJECTS_MAX - 1)
	{
		printf("too many textures for texture name buffer!");
	}
	else
	{
		textureNameArray[textureID] = materialName;
	}

	// Return the ID of the texture we just created
	return textureID;
}

GLfloat g_uv_buffer_data[] =
{
		1.0f, 0.0f,
		0.0f, 0.0f,
		1.0f, 1.0f,

		0.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 1.0f,


		1.0f, 0.0f,
		0.0f, 0.0f,
		1.0f, 1.0f,

		0.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 1.0f,


		1.0f, 0.0f,
		0.0f, 0.0f,
		1.0f, 1.0f,

		0.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 1.0f,



		1.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 1.0f,

		1.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 1.0f,


		1.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 1.0f,

		0.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 0.0f,


		1.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 1.0f,

		1.0f, 0.0f,
		0.0f, 0.0f,
		0.0f, 1.0f
};

void convertPreviewToObject()
{
	previewCube.isBeingUsed = false;

	if (freeRenderObjectIndex >= RENDER_OBJECTS_MAX)
	{
		printf("Too many objects!");
		return;
	}

	glm::vec3 top_right = glm::vec3(1, 1, 1);
	glm::vec3 bottom_left = glm::vec3(-1, -1, -1);
	
	glm::vec3 scaling = glm::vec3(previewCube.xScale, previewCube.yScale, previewCube.zScale);
	glm::vec3 movement = glm::vec3(previewCube.xPos, previewCube.yPos, previewCube.zPos);
	
	glm::vec3 out_right = top_right * scaling;
	glm::vec3 out_left = bottom_left * scaling;

	glm::vec3 top_1 = out_right + movement;
	glm::vec3 top_2 = glm::vec3(out_right.x, out_right.y, -out_right.z) + movement;
	glm::vec3 top_3 = glm::vec3(-out_right.x, out_right.y, -out_right.z) + movement;
	glm::vec3 top_4 = glm::vec3(-out_right.x, out_right.y, out_right.z) + movement;

	glm::vec3 bottom_1 = out_left + movement;
	glm::vec3 bottom_2 = glm::vec3(out_left.x, out_left.y, -out_left.z) + movement;
	glm::vec3 bottom_3 = glm::vec3(-out_left.x, out_left.y, -out_left.z) + movement;
	glm::vec3 bottom_4 = glm::vec3(-out_left.x, out_left.y, out_left.z) + movement;

	float cubeVerticies[] = 
	{
		top_1.x, top_1.y, top_1.z,
		top_2.x, top_2.y, top_2.z,
		top_4.x, top_4.y, top_4.z,

		top_2.x, top_2.y, top_2.z,
		top_3.x, top_3.y, top_3.z,
		top_4.x, top_4.y, top_4.z,


		top_1.x, top_1.y, top_1.z,
		top_4.x, top_4.y, top_4.z,
		bottom_3.x, bottom_3.y, bottom_3.z,
		
		top_4.x, top_4.y, top_4.z,
		bottom_2.x, bottom_2.y, bottom_2.z,
		bottom_3.x, bottom_3.y, bottom_3.z,
		
		
		top_4.x, top_4.y, top_4.z,
		top_3.x, top_3.y, top_3.z,
		bottom_2.x, bottom_2.y, bottom_2.z,
		
		top_3.x, top_3.y, top_3.z,
		bottom_1.x, bottom_1.y, bottom_1.z,
		bottom_2.x, bottom_2.y, bottom_2.z,

		bottom_4.x, bottom_4.y, bottom_4.z,
		bottom_2.x, bottom_2.y, bottom_2.z,
		bottom_1.x, bottom_1.y, bottom_1.z,
		
		bottom_4.x, bottom_4.y, bottom_4.z,
		bottom_3.x, bottom_3.y, bottom_3.z,
		bottom_2.x, bottom_2.y, bottom_2.z,
		


		top_2.x, top_2.y, top_2.z,
		bottom_3.x, bottom_3.y, bottom_3.z,
		bottom_4.x, bottom_4.y, bottom_4.z,

		top_1.x, top_1.y, top_1.z,
		bottom_3.x, bottom_3.y, bottom_3.z,
		top_2.x, top_2.y, top_2.z,
		
		
		top_3.x, top_3.y, top_3.z,
		bottom_4.x, bottom_4.y, bottom_4.z,
		bottom_1.x, bottom_1.y, bottom_1.z,
		
		top_3.x, top_3.y, top_3.z,
		top_2.x, top_2.y, top_2.z,
		bottom_4.x, bottom_4.y, bottom_4.z,
		
		
	};

	int fcount = sizeof(cubeVerticies) / sizeof(float);

	float* obj_verts = new float[fcount];
	memcpy(obj_verts, cubeVerticies, fcount * sizeof(float));

	renderCube* object = &renderObjects[freeRenderObjectIndex];
	freeRenderObjectIndex++;
	object->isUsed = true;

	
	object->vertexes = obj_verts;
	object->UVs = g_uv_buffer_data;

	object->vertexCount = fcount;
	object->UVCount = sizeof(g_uv_buffer_data) / sizeof(float);

	char* fileName = tinyfd_openFileDialog(NULL, NULL, 0, NULL, NULL, 0);
	int bufLen = strlen(fileName) + 1;
	char* newBuffer = new char[bufLen];
	strcpy_s(newBuffer, bufLen, fileName);

	object->texture = loadMaterialIntoGL(newBuffer);

	object->origin = movement;
	object->mins = out_left;
	object->maxs = out_right;

	//printf("%f\n", object->origin.x);
	//printf("%f\n", object->origin.y);
	//printf("%f\n", object->origin.z);
	//
	//printf("%f\n", object->mins.x);
	//printf("%f\n", object->mins.y);
	//printf("%f\n", object->mins.z);
	//
	//printf("%f\n", object->maxs.x);
	//printf("%f\n", object->maxs.y);
	//printf("%f\n", object->maxs.z);
}

void exportGraphicsToFile()
{
	FILE* file;
	if (fopen_s(&file, "out\\world.model", "wb") != 0)
	{
		printf("world file not able to be created.\n");
		return;
	}

	int objCount = 0;
	for (const renderCube& object : renderObjects)
	{
		if (object.isUsed == false)
			continue;

		objCount++;
	}

	fwrite(&objCount, sizeof(int), 1, file);

	for (const renderCube& object : renderObjects)
	{
		if (object.isUsed == false)
			continue;

		const char* texture = textureNameArray[object.texture];
		fwrite(texture, sizeof(char), strlen(texture) + 1, file);

		fwrite(&object.vertexCount, sizeof(int), 1, file);
		fwrite(&object.UVCount, sizeof(int), 1, file);

		fwrite(object.vertexes, sizeof(float), object.vertexCount, file);
		fwrite(object.UVs, sizeof(float), object.UVCount, file);

		for (int i = 0; i < object.vertexCount; i++)
		{
			printf("%f\n", object.vertexes[i]);
		}
		printf("\n");
	}

	

	fclose(file);
}

void exportClipsToFile()
{
	FILE* file;
	if (fopen_s(&file, "out\\world.clip", "wb") != 0)
	{
		printf("world clip file not able to be created.\n");
		return;
	}

	int objCount = 0;
	for (const renderCube& object : renderObjects)
	{
		if (object.isUsed == false)
			continue;

		objCount++;
	}

	fwrite(&objCount, sizeof(int), 1, file);

	for (const renderCube& object : renderObjects)
	{
		if (object.isUsed == false)
			continue;

		fwrite(&object.origin.x, sizeof(float), 1, file);
		fwrite(&object.origin.y, sizeof(float), 1, file);
		fwrite(&object.origin.z, sizeof(float), 1, file);

		fwrite(&object.mins.x, sizeof(float), 1, file);
		fwrite(&object.mins.y, sizeof(float), 1, file);
		fwrite(&object.mins.z, sizeof(float), 1, file);

		fwrite(&object.maxs.x, sizeof(float), 1, file);
		fwrite(&object.maxs.y, sizeof(float), 1, file);
		fwrite(&object.maxs.z, sizeof(float), 1, file);

		printf("%f\n", object.origin.x);
		printf("%f\n", object.origin.y);
		printf("%f\n", object.origin.z);
		printf("%f\n", object.mins.x);
		printf("%f\n", object.mins.y);
		printf("%f\n", object.mins.z);
		printf("%f\n", object.maxs.x);
		printf("%f\n", object.maxs.y);
		printf("%f\n", object.maxs.z);

		printf("\n");
		printf("\n");
	}

	fclose(file);
}

void exportWorldToFile()
{
	CreateDirectoryA("out", NULL);

	exportGraphicsToFile();
	exportClipsToFile();
}

float scaleAmount = 0.05;

void updateCubes()
{
	if (previewCube.isBeingUsed)
	{
		if (getKeyboardInput(GLFW_KEY_ENTER) == GLFW_PRESS)
		{
			convertPreviewToObject();
			return;
		}

		if (getKeyboardInput(GLFW_KEY_Z) == GLFW_PRESS)
			previewCube.sideMoving = true;
		else
			previewCube.sideMoving = false;

		if (getKeyboardInput(GLFW_KEY_X) == GLFW_PRESS)
			previewCube.scaling = true;
		else
			previewCube.scaling = false;

		if (getKeyboardInput(GLFW_KEY_UP) == GLFW_PRESS)
		{
			if (previewCube.scaling)
				previewCube.yScale += scaleAmount;
			else
				previewCube.yPos += scaleAmount;
		}
		else if (getKeyboardInput(GLFW_KEY_DOWN) == GLFW_PRESS)
		{
			if (previewCube.scaling)
				previewCube.yScale -= scaleAmount;
			else
				previewCube.yPos -= scaleAmount;
		}
		else if (getKeyboardInput(GLFW_KEY_LEFT) == GLFW_PRESS)
		{
			if (previewCube.scaling)
			{
				if (previewCube.sideMoving)
					previewCube.zScale += scaleAmount;
				else
					previewCube.xScale += scaleAmount;
			}
			else
			{
				if (previewCube.sideMoving)
					previewCube.zPos += scaleAmount;
				else
					previewCube.xPos += scaleAmount;
			}
		}
		else if (getKeyboardInput(GLFW_KEY_RIGHT) == GLFW_PRESS)
		{
			if (previewCube.scaling)
			{
				if (previewCube.sideMoving)
					previewCube.zScale -= scaleAmount;
				else
					previewCube.xScale -= scaleAmount;
			}
			else
			{
				if (previewCube.sideMoving)
					previewCube.zPos -= scaleAmount;
				else
					previewCube.xPos -= scaleAmount;
			}
		}

		if (previewCube.xScale < 0)
			previewCube.xScale = 0;
		if (previewCube.yScale < 0)
			previewCube.yScale = 0;
		if (previewCube.zScale < 0)
			previewCube.zScale = 0;
	}
	else
	{
		if (getKeyboardInput(GLFW_KEY_LEFT_ALT) == GLFW_PRESS)
		{
			previewCube.isBeingUsed = true;
			previewCube.xPos = 0;
			previewCube.yPos = 0;
			previewCube.zPos = 0;

			previewCube.xScale = 1;
			previewCube.yScale = 1;
			previewCube.zScale = 1;

			previewCube.points = preview_cube_points;
			previewCube.colours = preview_cube_colours;
		}
		else if (getKeyboardInput(GLFW_KEY_P) == GLFW_PRESS)
		{
			exportWorldToFile();
		}
	}
}

void drawScene()
{
	// Clear the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	computeMatricesFromInputs();
	updateCubes();
	
	glUseProgram(shader_TextureID);
	for (const renderCube& object : renderObjects)
	{
		if (object.isUsed == false)
			continue;

		glm::mat4 MVP = ProjectionMatrix * ViewMatrix;

		// Send our transformation to the currently bound shader, in the "MVP" uniform
		// This is done in the main loop since each model will have a different MVP matrix (At least for the M part)
		glUniformMatrix4fv(matrix_TextureID, 1, GL_FALSE, &MVP[0][0]);

		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, object.texture);

		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glBufferData(GL_ARRAY_BUFFER, object.vertexCount * sizeof(float), object.vertexes, GL_STATIC_DRAW); //size is num of bytes

		glBindBuffer(GL_ARRAY_BUFFER, UVbuffer);
		glBufferData(GL_ARRAY_BUFFER, object.UVCount * sizeof(float), object.UVs, GL_STATIC_DRAW); //size is num of bytes

		// Draw the triangle !
		glDrawArrays(GL_TRIANGLES, 0, object.vertexCount / 3); //indecies are an xyz point in space, so div by 3 as vertexCount is a count of floats in the array
	}

	glUseProgram(shader_ColourID);
	if (previewCube.isBeingUsed)
	{
		glm::mat4 myScalingMatrix = glm::scale(glm::vec3(previewCube.xScale, previewCube.yScale, previewCube.zScale));
		glm::mat4 myTranslationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(previewCube.xPos, previewCube.yPos, previewCube.zPos));
		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * (myTranslationMatrix * myScalingMatrix);
	
		glUniformMatrix4fv(matrix_ColourID, 1, GL_FALSE, &MVP[0][0]);
	
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glBufferData(GL_ARRAY_BUFFER, CUBE_POINTS_COUNT * sizeof(float), previewCube.points, GL_STATIC_DRAW);
	
		glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
		glBufferData(GL_ARRAY_BUFFER, CUBE_POINTS_COUNT * sizeof(float), previewCube.colours, GL_STATIC_DRAW);
	
		// Draw the triangle !
		glDrawArrays(GL_LINES, 0, CUBE_POINTS_COUNT / 2); //indecies are an xyz point in space, so div by 3 as vertexCount is a count of floats in the array
	}
}

void initViewProjection()
{
	// Projection matrix : 45&deg; Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	ProjectionMatrix = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);

	ViewMatrix = glm::lookAt(
		position,           // Camera is here
		glm::vec3(0, 0, 0), // and looks here : at the same position, plus "direction"
		glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
	);
}

void initOpenGL()
{
	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Generate 3 buffers
	glGenBuffers(1, &vertexbuffer);
	glGenBuffers(1, &colorbuffer);
	glGenBuffers(1, &UVbuffer);

	// Create and compile our GLSL program from the shaders
	shader_ColourID = LoadShaders("shaders\\ColourVertexShader.vertexshader", "shaders\\ColourFragmentShader.fragmentshader");
	shader_TextureID = LoadShaders("shaders\\TextureVertexShader.vertexshader", "shaders\\TextureFragmentShader.fragmentshader");

	// Get a handle for our "MVP" uniform
	// Only during the initialisation
	matrix_ColourID = glGetUniformLocation(shader_ColourID, "MVP");
	matrix_TextureID = glGetUniformLocation(shader_TextureID, "MVP");

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);
	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);

	// 1st attribute buffer : vertices
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		nullptr             // array buffer offset
	);

	// 2nd attribute buffer : colors
	glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(
		1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
		3,                                // size
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		nullptr                           // array buffer offset
	);

	glBindBuffer(GL_ARRAY_BUFFER, UVbuffer);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(
		2,                                // attribute. No particular reason for 1, but must match the layout in the shader.
		2,                                // size : U+V => 2
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		nullptr                           // array buffer offset
	);


	initViewProjection();

	//Turn on wireframe
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	//turn off wireframe
	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

//glm::mat4 myMatrix = glm::translate(glm::mat4(), glm::vec3(10.0f, 0.0f, 0.0f));
//glm::vec4 myVector(10.0f, 10.0f, 10.0f, 0.0f);
//glm::vec4 transformedVector = myMatrix * myVector; // guess the result
//
//glm::vec3 myRotationAxis(1.0f, 1.0f, 1.0f);
//glm::rotate(90.0f, myRotationAxis);
//
//glm::mat4 myScalingMatrix = glm::scale(glm::vec3(1.0f, 1.0f, 1.0f)); //will scale relative to origin so always scale first

//TransformedVector = TranslationMatrix * RotationMatrix * ScaleMatrix * OriginalVector; //This lines actually performs the scaling FIRST, and THEN the rotation, and THEN the translation.

//glm::mat4 myModelMatrix = myTranslationMatrix * myRotationMatrix * myScaleMatrix;
//glm::vec4 myTransformedVector = myModelMatrix * myOriginalVector;

//glm::mat4 ViewMatrix = glm::translate(glm::mat4(), glm::vec3(-3.0f, 0.0f, 0.0f));

//glm::mat4 CameraMatrix = glm::lookAt(
//    cameraPosition, // the position of your camera, in world space
//    cameraTarget,   // where you want to look at, in world space
//    upVector        // probably glm::vec3(0,1,0), but (0,-1,0) would make you looking upside-down, which can be great too
//);

// Generates a really hard-to-read matrix, but a normal, standard 4x4 matrix nonetheless
//glm::mat4 projectionMatrix = glm::perspective(
//    glm::radians(FoV), // The vertical Field of View, in radians: the amount of "zoom". Think "camera lens". Usually between 90° (extra wide) and 30° (quite zoomed in)
//    4.0f / 3.0f,       // Aspect Ratio. Depends on the size of your window. Notice that 4/3 == 800/600 == 1280/960, sounds familiar ?
//    0.1f,              // Near clipping plane. Keep as big as possible, or you'll get precision issues.
//    100.0f             // Far clipping plane. Keep as little as possible.
//);



// One color for each vertex. They were generated randomly.
