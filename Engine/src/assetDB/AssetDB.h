#pragma once

#include <glm/glm.hpp>
using namespace glm;

enum xAssetType
{
	XASSET_NULL = -1,

	XASSET_FONT = 0,
	XASSET_MATERIAL,
	XASSET_MODEL,
	XASSET_CLIPMAP,
	XASSET_WORLD,
	XASSET_SCRIPT,

	XASSET_MAX = XASSET_SCRIPT // Change when adding new assets
};

struct xAsset
{
	int maxPoolSize;
	int usedCount;
	int assetSize;
	char* freeHead;
	char* pool;
};

struct XScript
{
	char* name;

	int scrLen;
	int mainFuncPtr;
	char* script;
};

struct XMaterial
{
	char* name;

	int width;
	int height;
	unsigned char* imageData;

	int openGLTexture;
};

struct XFont
{
	char* name;
	XMaterial* texture;
	
	int lettersPerLine;
	int glyphHeight;
	int glyphWidth;

	char glyphWidthArray[256];
};

struct XModel
{
	char* name;

	float* vertexes;
	int vertexCount;

	float* UVs;
	int UVCount;
};

struct worldData
{
	int count;
	float* data;
};

struct XworldObject
{
	char* name;

	int numObjs;
	unsigned int* objTextureArray;

	worldData* vertexDataArray;
	worldData* UVDataArray;
};

struct ClipBound
{
	glm::vec3 origin;
	glm::vec3 mins;
	glm::vec3 maxs;
};

struct XClipMap
{
	char* name;

	int numClipBounds;
	ClipBound* clips;
};

union XAssetHeader
{
	XModel* XModel;
	XFont* XFont;
	XMaterial* XMaterial;
	XworldObject* World;
	XClipMap* Clip;
	XScript* Script;
	void* data;
};

#define ASSET_MAX 5

#define MAX_ASSET_NAME_LEN 0x100

void initXAssetPool();
XAssetHeader findAsset(xAssetType type, const char* name);
void loadAssets();