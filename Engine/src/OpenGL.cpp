#include "src\openGL\compile\StandardOpenGL.h"
#include "src\openGL\compile\ShaderCompile.h"
#include "src\window\Window.h"
#include "src\assetDB\AssetDB.h"
#include "src\OpenGL.h"
#include "src\physics\Physics.h"
#include "src/console/Console.h"

#include "src/player/Player.h"

#include <math.h>

#include <vector>

//#define STB_IMAGE_IMPLEMENTATION
#include "src\external\stb_image.h"

struct shader_textureInfo
{
    GLuint shaderID;

    GLuint vertexBuffer;
    GLuint UVBuffer;

    GLuint vertexBufferIndex;
    GLuint UVBufferIndex;

    GLuint uniform_MVP;
    GLuint uniform_textureSampler;
};

struct shader_textInfo
{
    GLuint shaderID;

    GLuint vertexBuffer;
    GLuint UVBuffer;

    GLuint vertexBufferIndex;
    GLuint UVBufferIndex;

    GLuint uniform_textureSampler;
};

struct shader_colourInfo
{
    GLuint shaderID;

    GLuint vertexBuffer;
    GLuint colourBuffer;

    GLuint vertexBufferIndex;
    GLuint colourBufferIndex;

    GLuint uniform_MVP;
};

shader_textureInfo shaderTextureInfo;
shader_textInfo shaderTextInfo;
shader_colourInfo shaderColourInfo;

struct renderObject
{
    bool isUsed;

    XModel* model;
    GLuint texture;
    float xPos;
    float yPos;
    float zPos;

    float xRot;
    float yRot;
    float zRot;

    float xScale;
    float yScale;
    float zScale;
};

XFont* renderFont;
renderObject renderObjects[4];

XworldObject* worldObj;


void draw2D(GLuint texture, void* verts, int vertSize, int vertexCount, void* UVs, int UVSize)
{
    // Bind shader
    glUseProgram(shaderTextInfo.shaderID);

    glBindTexture(GL_TEXTURE_2D, texture);

    glBindBuffer(GL_ARRAY_BUFFER, shaderTextInfo.vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, vertSize, verts, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, shaderTextInfo.UVBuffer);
    glBufferData(GL_ARRAY_BUFFER, UVSize, UVs, GL_STATIC_DRAW);

    // Draw call
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
}

void drawImage2D(GLuint texture, int x, int y, int size)
{
    std::vector<glm::vec2> vertices;
    std::vector<glm::vec2> UVs;

    glm::vec2 vertex_up_left = glm::vec2(x + size, y + size);
    glm::vec2 vertex_up_right = glm::vec2(x + size + size, y + size);
    glm::vec2 vertex_down_right = glm::vec2(x + size + size, y);
    glm::vec2 vertex_down_left = glm::vec2(x + size, y);

    vertices.push_back(vertex_up_left);
    vertices.push_back(vertex_down_left);
    vertices.push_back(vertex_up_right);

    vertices.push_back(vertex_down_right);
    vertices.push_back(vertex_up_right);
    vertices.push_back(vertex_down_left);

    glm::vec2 uv_up_left = glm::vec2(0, 0);
    glm::vec2 uv_up_right = glm::vec2(1, 0);
    glm::vec2 uv_down_left = glm::vec2(0, 1);
    glm::vec2 uv_down_right = glm::vec2(1, 1);

    UVs.push_back(uv_up_left);
    UVs.push_back(uv_down_left);
    UVs.push_back(uv_up_right);

    UVs.push_back(uv_down_right);
    UVs.push_back(uv_up_right);
    UVs.push_back(uv_down_left);

    draw2D(texture, &vertices[0], vertices.size() * sizeof(glm::vec2), vertices.size(), &UVs[0], UVs.size() * sizeof(glm::vec2));
}

void printText2D(const char* text, int x, int y, int size)
{
    if (text[0] == '\x0')
        return;

    
    std::vector<glm::vec2> vertices;
    std::vector<glm::vec2> UVs;

    for (int i = 0; text[i] != '\x00'; i++)
    {
        char glyphWidth = renderFont->glyphWidthArray[text[i]];
        float charWidth = float(glyphWidth) / (renderFont->glyphWidth * renderFont->lettersPerLine);
        float charHeight = 1.0f / renderFont->lettersPerLine;

        int xsize = glyphWidth;

        glm::vec2 vertex_up_left = glm::vec2(x + i * xsize, y + size);
        glm::vec2 vertex_up_right = glm::vec2(x + i * xsize + xsize, y + size);
        glm::vec2 vertex_down_right = glm::vec2(x + i * xsize + xsize, y);
        glm::vec2 vertex_down_left = glm::vec2(x + i * xsize, y);

        vertices.push_back(vertex_up_left);
        vertices.push_back(vertex_down_left);
        vertices.push_back(vertex_up_right);

        vertices.push_back(vertex_down_right);
        vertices.push_back(vertex_up_right);
        vertices.push_back(vertex_down_left);

        int line_x = text[i] % renderFont->lettersPerLine;
        int line_y = text[i] / renderFont->lettersPerLine;

        float uv_x = float(line_x) / renderFont->lettersPerLine;
        float uv_y = float(line_y) / renderFont->lettersPerLine;

        

        glm::vec2 uv_up_left = glm::vec2(uv_x, uv_y);
        glm::vec2 uv_up_right = glm::vec2(uv_x + charWidth, uv_y);
        glm::vec2 uv_down_left = glm::vec2(uv_x, uv_y + charHeight);
        glm::vec2 uv_down_right = glm::vec2(uv_x + charWidth, uv_y + charHeight);

        UVs.push_back(uv_up_left);
        UVs.push_back(uv_down_left);
        UVs.push_back(uv_up_right);

        UVs.push_back(uv_down_right);
        UVs.push_back(uv_up_right);
        UVs.push_back(uv_down_left);


        //float totalLines = static_cast<float>(renderFont->letterCount / renderFont->lettersPerLine);
        //float totalLettersPerLine = static_cast<float>(renderFont->lettersPerLine);
        //
        //float line = static_cast<float>(j / renderFont->lettersPerLine);
        //float letterLineOffset = static_cast<float>(j % renderFont->lettersPerLine);
        //
        //
        //float uv_x = letterLineOffset / totalLettersPerLine;
        //float uv_y = line / totalLines;
        //
        //float charWidth = 1 / totalLettersPerLine;

        //glm::vec2 uv_up_left = glm::vec2(uv_x, uv_y);
        //glm::vec2 uv_up_right = glm::vec2(uv_x + charWidth, uv_y);
        //glm::vec2 uv_down_left = glm::vec2(uv_x, uv_y + charWidth);
        //glm::vec2 uv_down_right = glm::vec2(uv_x + charWidth, uv_y + charWidth);
        //
        //UVs.push_back(uv_up_left);
        //UVs.push_back(uv_down_left);
        //UVs.push_back(uv_up_right);
        //
        //UVs.push_back(uv_down_right);
        //UVs.push_back(uv_up_right);
        //UVs.push_back(uv_down_left);
        //
        //
        //for (int j = 0; j < renderFont->letterCount; j++)
        //{
        //    if (renderFont->letterArray[j] == text[i])
        //    {
        //        
        //
        //        break;
        //    }
        //}
    }

    draw2D(renderFont->texture, &vertices[0], vertices.size() * sizeof(glm::vec2), vertices.size(), &UVs[0], UVs.size() * sizeof(glm::vec2));
}

void drawWorld()
{
    glUseProgram(shaderTextureInfo.shaderID);

    glm::mat4 MVP = playerStruct.renderInfo.projectionMatrix * playerStruct.renderInfo.viewMatrix;

    glUniformMatrix4fv(shaderTextureInfo.uniform_MVP, 1, GL_FALSE, &MVP[0][0]);

    for (int i = 0; i < worldObj->numObjs; i++)
    {
        glBindTexture(GL_TEXTURE_2D, worldObj->objTextureArray[i]);

        glBindBuffer(GL_ARRAY_BUFFER, shaderTextureInfo.vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, worldObj->vertexDataArray[i].count * sizeof(float), worldObj->vertexDataArray[i].data, GL_STATIC_DRAW); //size is num of bytes

        glBindBuffer(GL_ARRAY_BUFFER, shaderTextureInfo.UVBuffer);
        glBufferData(GL_ARRAY_BUFFER, worldObj->UVDataArray[i].count * sizeof(float), worldObj->UVDataArray[i].data, GL_STATIC_DRAW); //size is num of bytes

        glDrawArrays(GL_TRIANGLES, 0, worldObj->vertexDataArray[i].count / 3);
    }
}

void drawRenderObjects()
{
    glUseProgram(shaderTextureInfo.shaderID);

    for (const renderObject& object : renderObjects)
    {
        if (!object.isUsed)
            continue;

        glm::mat4 myScalingMatrix = glm::scale(glm::vec3(object.xScale, object.yScale, object.zScale));

        glm::vec3 myRotationAxis(0.0f, 1.0f, 0.0f);

        glm::mat4 myTranslationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(object.xPos, object.yPos, object.zPos));
        //glm::mat4 Model = myTranslationMatrix * myRotationMatrix * myScalingMatrix;
        glm::mat4 Model = myTranslationMatrix * myScalingMatrix;
        glm::mat4 MVP = playerStruct.renderInfo.projectionMatrix * playerStruct.renderInfo.viewMatrix * Model;

        // Send our transformation to the currently bound shader, in the "MVP" uniform
        // This is done in the main loop since each model will have a different MVP matrix (At least for the M part)
        glUniformMatrix4fv(shaderTextureInfo.uniform_MVP, 1, GL_FALSE, &MVP[0][0]);

        glBindTexture(GL_TEXTURE_2D, object.texture);

        glBindBuffer(GL_ARRAY_BUFFER, shaderTextureInfo.vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, object.model->vertexCount * sizeof(float), object.model->vertexes, GL_STATIC_DRAW); //size is num of bytes

        glBindBuffer(GL_ARRAY_BUFFER, shaderTextureInfo.UVBuffer);
        glBufferData(GL_ARRAY_BUFFER, object.model->UVCount * sizeof(float), object.model->UVs, GL_STATIC_DRAW); //size is num of bytes

        glDrawArrays(GL_TRIANGLES, 0, object.model->vertexCount / 3);  //indecies are an xyz point in space, so div by 3 as vertexCount is a count of floats in the array
    }
}

int eeeee = 0;
void drawText()
{
    char buffer[100];
    sprintf_s(buffer, "COUNT %i", eeeee);
    //printText2D(buffer, 0, 0, 60);


    //drawImage2D(renderObjects[0].texture, 200, 200, 200);
    //drawImage2D(renderObjects[0].texture, 200, 400, 200);
    //drawImage2D(renderObjects[0].texture, 200, 600, 200);
    eeeee++;
}

void drawBoundingBoxes()
{
    /*glUseProgram(shaderColourInfo.shaderID);

    glm::mat4 MVP = player.renderInfo.projectionMatrix * player.renderInfo.viewMatrix;
    glUniformMatrix4fv(shaderColourInfo.uniform_MVP, 1, GL_FALSE, &MVP[0][0]);

    XClipMap* worldBounds = findAsset(XASSET_CLIPMAP, "assets\\world\\world.clip").Clip;

    for (int i = 0; i < worldBounds->numClipBounds; i++)
    {
        ClipBound* clip = &worldBounds->clips[i];
        glm::vec3 min = clip->origin + clip->mins;
        glm::vec3 max = clip->origin + clip->maxs;

        float vertexbuffer[] = {
                min.x, min.y, min.z,
                max.x, max.y, max.z
        };

        glBindBuffer(GL_ARRAY_BUFFER, shaderColourInfo.vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(float), vertexbuffer, GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, shaderColourInfo.colourBuffer);
        glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(float), preview_cube_colours, GL_STATIC_DRAW);

        glDrawArrays(GL_LINES, 0, 2);
    
        
    }*/
}

//double lastTime = 0;
void drawScene(float deltaTime)
{
    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    char buffer[100];
    sprintf_s(buffer, "DELTATIME %f", deltaTime);
    printText2D(buffer, 0, 50, 40);

    drawWorld();

    drawRenderObjects();

    drawText();

    if(con_isOpen())
        drawConsole();
}

GLuint imoprtMaterialIntoGL(int width, int height, unsigned char* data)
{
    GLuint textureID;
    glGenTextures(1, &textureID);

    // "Bind" the newly created texture : all future texture functions will modify this texture
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    //glGenerateMipmap(GL_TEXTURE_2D);

    // Return the ID of the texture we just created
    return textureID;
}

GLuint loadMaterialIntoGL(const char* materialName) 
{
    XMaterial* material = findAsset(XASSET_MATERIAL, materialName).XMaterial;
    if (material == NULL)
    {
        printf("Material not found!\n");
        return -1;
    }

    return imoprtMaterialIntoGL(material->width, material->height, material->imageData);
}

GLuint loadImageIntoGL(const char* imageName)
{
    int width;
    int height;
    int channels;

    unsigned char* imageData = stbi_load(imageName, &width, &height, &channels, 0);

    return imoprtMaterialIntoGL(width, height, imageData);
}

void setupRenderObjects()
{
    
}

void setupWorld()
{
    renderFont = findAsset(XASSET_FONT, "assets\\fonts\\font.bin").XFont;
    worldObj = findAsset(XASSET_WORLD, "assets\\world\\world.model").World;
}

void setupOpenGLTextureShaders()
{
    shaderTextureInfo.vertexBufferIndex = 0;
    shaderTextureInfo.UVBufferIndex = 1;

    // Generate 1 buffer, put the resulting identifier in vertexbuffer
    glGenBuffers(1, &shaderTextureInfo.vertexBuffer);
    glGenBuffers(1, &shaderTextureInfo.UVBuffer);
    // The following commands will talk about our 'vertexbuffer' buffer


    // Create and compile our GLSL program from the shaders
    shaderTextureInfo.shaderID = LoadShaders("shaders\\Texture_VertexShader.vertexshader", "shaders\\Texture_FragmentShader.fragmentshader");

    // Get a handle for our "MVP" uniform
    // Only during the initialisation
    shaderTextureInfo.uniform_MVP = glGetUniformLocation(shaderTextureInfo.shaderID, "MVP");
    shaderTextureInfo.uniform_textureSampler = glGetUniformLocation(shaderTextureInfo.shaderID, "textureSampler");

    // 1st attribute buffer : vertices
    glEnableVertexAttribArray(shaderTextureInfo.vertexBufferIndex);
    glBindBuffer(GL_ARRAY_BUFFER, shaderTextureInfo.vertexBuffer);
    glVertexAttribPointer(
        shaderTextureInfo.vertexBufferIndex,
        3,                  // size
        GL_FLOAT,           // type
        GL_FALSE,           // normalized?
        0,                  // stride
        NULL                // array buffer offset
    );

    // 2nd attribute buffer : UV
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, shaderTextureInfo.UVBuffer);
    glVertexAttribPointer(
        shaderTextureInfo.UVBufferIndex,
        2,                                // size : U+V => 2
        GL_FLOAT,                         // type
        GL_FALSE,                         // normalized?
        0,                                // stride
        NULL                            // array buffer offset
    );
}

void setupOpenGLTextShaders()
{
    shaderTextInfo.vertexBufferIndex = 2;
    shaderTextInfo.UVBufferIndex = 3;

    // Initialize VBO
    glGenBuffers(1, &shaderTextInfo.vertexBuffer);
    glGenBuffers(1, &shaderTextInfo.UVBuffer);

    // Initialize Shader
    shaderTextInfo.shaderID = LoadShaders("shaders\\Text_VertexShader.vertexshader", "shaders\\Text_FragmentShader.fragmentshader");

    // Initialize uniforms' IDs
    shaderTextInfo.uniform_textureSampler = glGetUniformLocation(shaderTextInfo.shaderID, "textureSampler");

    // 1rst attribute buffer : vertices
    glEnableVertexAttribArray(shaderTextInfo.vertexBufferIndex);
    glBindBuffer(GL_ARRAY_BUFFER, shaderTextInfo.vertexBuffer);
    glVertexAttribPointer(
        shaderTextInfo.vertexBufferIndex, 
        2, // 2 as only x,y
        GL_FLOAT, 
        GL_FALSE, 
        0, 
        NULL);

    // 2nd attribute buffer : UVs
    glEnableVertexAttribArray(shaderTextInfo.UVBufferIndex);
    glBindBuffer(GL_ARRAY_BUFFER, shaderTextInfo.UVBuffer);
    glVertexAttribPointer(
        shaderTextInfo.UVBufferIndex,
        2,
        GL_FLOAT,
        GL_FALSE,
        0, 
        NULL);
}

void setupOpenGLColourShaders()
{
    shaderColourInfo.vertexBufferIndex = 4;
    shaderColourInfo.colourBufferIndex = 5;
    
    glGenBuffers(1, &shaderColourInfo.vertexBuffer);
    glGenBuffers(1, &shaderColourInfo.colourBuffer);

    shaderColourInfo.shaderID = LoadShaders("shaders\\Colour_VertexShader.vertexshader", "shaders\\Colour_FragmentShader.fragmentshader");

    shaderColourInfo.uniform_MVP = glGetUniformLocation(shaderColourInfo.shaderID, "MVP");

    glEnableVertexAttribArray(shaderColourInfo.vertexBufferIndex);
    glBindBuffer(GL_ARRAY_BUFFER, shaderColourInfo.vertexBuffer);
    glVertexAttribPointer(
        shaderColourInfo.vertexBufferIndex,
        3,
        GL_FLOAT,
        GL_FALSE,
        0,
        NULL);

    glEnableVertexAttribArray(shaderColourInfo.colourBufferIndex);
    glBindBuffer(GL_ARRAY_BUFFER, shaderColourInfo.colourBuffer);
    glVertexAttribPointer(
        shaderColourInfo.colourBufferIndex,
        3,                                // size
        GL_FLOAT,                         // type
        GL_FALSE,                         // normalized?
        0,                                // stride
        NULL                           // array buffer offset
    );
}

void setupShaders()
{
    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    setupOpenGLTextureShaders();

    setupOpenGLColourShaders();

    setupOpenGLTextShaders();
}

void setupOpenGLSettings()
{
    glEnable(GL_DEPTH_TEST); // Enable depth test
    glDepthFunc(GL_LESS); // Accept fragment if it closer to the camera than the former one
    glEnable(GL_CULL_FACE); // Cull triangles which normal is not towards the camera

    //Turn on wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    //turn off wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}



void initOpenGL()
{
    setupShaders();

    setupOpenGLSettings();

    setupWorld();

    setupRenderObjects();
}