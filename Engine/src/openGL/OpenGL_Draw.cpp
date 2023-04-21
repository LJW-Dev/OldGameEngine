#include "OpenGL_Draw.h"
#include "src/openGL/Compile/ShaderCompile.h"
#include "src/player/Player.h"

#include <vector>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>


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
shader_textInfo shader2DTextureInfo;
shader_colourInfo shaderColourInfo;

void resetFrame()
{
    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void draw2DTextured(GLuint texture, void* verts, int vertexCount, void* UVs, int UVCount)
{
    // Bind shader
    glUseProgram(shader2DTextureInfo.shaderID);

    glBindTexture(GL_TEXTURE_2D, texture);

    glBindBuffer(GL_ARRAY_BUFFER, shader2DTextureInfo.vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(glm::vec2), verts, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, shader2DTextureInfo.UVBuffer);
    glBufferData(GL_ARRAY_BUFFER, UVCount * sizeof(glm::vec2), UVs, GL_STATIC_DRAW);
    
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);
}

void draw3DTextured(GLuint texture, void* verts, int vertexCount, void* UVs, int UVCount)
{
    glUseProgram(shaderTextureInfo.shaderID);

    glBindTexture(GL_TEXTURE_2D, texture);

    glBindBuffer(GL_ARRAY_BUFFER, shaderTextureInfo.vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(float), verts, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, shaderTextureInfo.UVBuffer);
    glBufferData(GL_ARRAY_BUFFER, UVCount * sizeof(float), UVs, GL_STATIC_DRAW);

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

    draw2DTextured(texture, &vertices[0], vertices.size(), &UVs[0], UVs.size());
}

void drawText2D(fontAsset* font, const char* text, int x, int y, int size)
{
    if (text[0] == '\x0')
        return;

    std::vector<glm::vec2> vertices;
    std::vector<glm::vec2> UVs;

    for (int i = 0; text[i] != '\x00'; i++)
    {
        char glyphWidth = font->glyphWidthArray[text[i]];
        float charWidth = float(glyphWidth) / (font->glyphWidth * font->lettersPerLine);
        float charHeight = 1.0f / font->lettersPerLine;

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

        int line_x = text[i] % font->lettersPerLine;
        int line_y = text[i] / font->lettersPerLine;

        float uv_x = float(line_x) / font->lettersPerLine;
        float uv_y = float(line_y) / font->lettersPerLine;

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
    }

    draw2DTextured(font->texture->openGLTexture, &vertices[0], vertices.size(),&UVs[0], UVs.size());
}

void drawWorld(worldAsset* worldObj)
{
    glUseProgram(shaderTextureInfo.shaderID);

    glUniformMatrix4fv(shaderTextureInfo.uniform_MVP, 1, GL_FALSE, &playerStruct.renderInfo.ViewProjectionMatrix[0][0]);

    for (int i = 0; i < worldObj->numObjs; i++) 
    {
       draw3DTextured(worldObj->objTextureArray[i], worldObj->vertexDataArray[i].data, worldObj->vertexDataArray[i].count, worldObj->UVDataArray[i].data, worldObj->UVDataArray[i].count);
    }
}

void drawObject(float x, float y, float z, float rotX, float rotY, float rotZ, modelAsset* model)
{
    glUseProgram(shaderTextureInfo.shaderID);
    
    glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, z));
    glm::mat4 MVP = playerStruct.renderInfo.ViewProjectionMatrix * translationMatrix;

    // Send our transformation to the currently bound shader, in the "MVP" uniform
    // This is done in the main loop since each model will have a different MVP matrix (At least for the M part)
    glUniformMatrix4fv(shaderTextureInfo.uniform_MVP, 1, GL_FALSE, &MVP[0][0]);

    draw3DTextured(model->texture->openGLTexture, model->vertexes, model->vertexCount, model->UVs, model->UVCount);
}

void drawEntity(entityInfo* ent)
{
    glUseProgram(shaderTextureInfo.shaderID);

    glm::vec3 entPos = glm::vec3(ent->xPos, ent->yPos, ent->zPos);

    glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), entPos);
    glm::mat4 rotationnMatrix = glm::rotate(glm::mat4(1.0f), 3.0f, entPos);
    
    glm::mat4 MVP = playerStruct.renderInfo.ViewProjectionMatrix * translationMatrix;

    // Send our transformation to the currently bound shader, in the "MVP" uniform
    // This is done in the main loop since each model will have a different MVP matrix (At least for the M part)
    glUniformMatrix4fv(shaderTextureInfo.uniform_MVP, 1, GL_FALSE, &MVP[0][0]);

    draw3DTextured(ent->model->texture->openGLTexture, ent->model->vertexes, ent->model->vertexCount, ent->model->UVs, ent->model->UVCount);
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
    shader2DTextureInfo.vertexBufferIndex = 2;
    shader2DTextureInfo.UVBufferIndex = 3;

    // Initialize VBO
    glGenBuffers(1, &shader2DTextureInfo.vertexBuffer);
    glGenBuffers(1, &shader2DTextureInfo.UVBuffer);

    // Initialize Shader
    shader2DTextureInfo.shaderID = LoadShaders("shaders\\Text_VertexShader.vertexshader", "shaders\\Text_FragmentShader.fragmentshader");

    // Initialize uniforms' IDs
    shader2DTextureInfo.uniform_textureSampler = glGetUniformLocation(shader2DTextureInfo.shaderID, "textureSampler");

    // 1rst attribute buffer : vertices
    glEnableVertexAttribArray(shader2DTextureInfo.vertexBufferIndex);
    glBindBuffer(GL_ARRAY_BUFFER, shader2DTextureInfo.vertexBuffer);
    glVertexAttribPointer(
        shader2DTextureInfo.vertexBufferIndex,
        2, // 2 as only x,y
        GL_FLOAT,
        GL_FALSE,
        0,
        NULL);

    // 2nd attribute buffer : UVs
    glEnableVertexAttribArray(shader2DTextureInfo.UVBufferIndex);
    glBindBuffer(GL_ARRAY_BUFFER, shader2DTextureInfo.UVBuffer);
    glVertexAttribPointer(
        shader2DTextureInfo.UVBufferIndex,
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