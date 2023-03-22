#include "OpenGL_Draw.h"
#include "src/openGL/Compile/ShaderCompile.h"
#include "src/player/Player.h"

#include <vector>

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

void printText2D(XFont* font, const char* text, int x, int y, int size)
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

    draw2D(font->texture->openGLTexture, &vertices[0], vertices.size() * sizeof(glm::vec2), vertices.size(), &UVs[0], UVs.size() * sizeof(glm::vec2));
}

void drawWorld(XworldObject* worldObj)
{
    glUseProgram(shaderTextureInfo.shaderID);

    glUniformMatrix4fv(shaderTextureInfo.uniform_MVP, 1, GL_FALSE, &playerStruct.renderInfo.ViewProjectionMatrix[0][0]);

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

void drawObject(float x, float y, float z, float rotX, float rotY, float rotZ, XModel* model, XMaterial* material)
{
    glUseProgram(shaderTextureInfo.shaderID);
    
    glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, z));
    glm::mat4 MVP = playerStruct.renderInfo.ViewProjectionMatrix * translationMatrix;

    // Send our transformation to the currently bound shader, in the "MVP" uniform
    // This is done in the main loop since each model will have a different MVP matrix (At least for the M part)
    glUniformMatrix4fv(shaderTextureInfo.uniform_MVP, 1, GL_FALSE, &MVP[0][0]);

    glBindTexture(GL_TEXTURE_2D, material->openGLTexture);

    glBindBuffer(GL_ARRAY_BUFFER, shaderTextureInfo.vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, model->vertexCount * sizeof(float), model->vertexes, GL_STATIC_DRAW); //size is num of bytes

    glBindBuffer(GL_ARRAY_BUFFER, shaderTextureInfo.UVBuffer);
    glBufferData(GL_ARRAY_BUFFER, model->UVCount * sizeof(float), model->UVs, GL_STATIC_DRAW); //size is num of bytes

    glDrawArrays(GL_TRIANGLES, 0, model->vertexCount / 3);  //indecies are an xyz point in space, so div by 3 as vertexCount is a count of floats in the array
}

void drawEntities(entityInfo* entArray)
{
    glUseProgram(shaderTextureInfo.shaderID);

    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        entityInfo ent = entArray[i];

        if (!ent.isUsed)
            continue;

        glm::vec3 entPos = glm::vec3(ent.xPos, ent.yPos, ent.zPos);

        glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), entPos);
        glm::mat4 rotationnMatrix = glm::rotate(glm::mat4(1.0f), 3.0f, entPos);
        
        glm::mat4 Model = translationMatrix * rotationnMatrix;
        glm::mat4 MVP = playerStruct.renderInfo.ViewProjectionMatrix * Model;

        // Send our transformation to the currently bound shader, in the "MVP" uniform
        // This is done in the main loop since each model will have a different MVP matrix (At least for the M part)
        glUniformMatrix4fv(shaderTextureInfo.uniform_MVP, 1, GL_FALSE, &MVP[0][0]);

        glBindTexture(GL_TEXTURE_2D, ent.texture);

        glBindBuffer(GL_ARRAY_BUFFER, shaderTextureInfo.vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, ent.model->vertexCount * sizeof(float), ent.model->vertexes, GL_STATIC_DRAW); //size is num of bytes

        glBindBuffer(GL_ARRAY_BUFFER, shaderTextureInfo.UVBuffer);
        glBufferData(GL_ARRAY_BUFFER, ent.model->UVCount * sizeof(float), ent.model->UVs, GL_STATIC_DRAW); //size is num of bytes

        glDrawArrays(GL_TRIANGLES, 0, ent.model->vertexCount / 3);  //indecies are an xyz point in space, so div by 3 as vertexCount is a count of floats in the array
    }
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