#include "OpenGL_Import.h"

#include "src\external\stb_image.h"
#include "src\assetDB\AssetDB.h"

GLuint imoprtTextureIntoGL(int width, int height, unsigned char* data)
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

GLuint loadMaterialAssetIntoGL(const char* materialName)
{
    XMaterial* material = findAsset(XASSET_MATERIAL, materialName).XMaterial;
    if (material == NULL)
    {
        printf("Material not found!\n");
        return -1;
    }

    return imoprtTextureIntoGL(material->width, material->height, material->imageData);
}

GLuint loadDiskImageIntoGL(const char* imageName)
{
    int width;
    int height;
    int channels;

    unsigned char* imageData = stbi_load(imageName, &width, &height, &channels, 0);

    return imoprtTextureIntoGL(width, height, imageData);
}