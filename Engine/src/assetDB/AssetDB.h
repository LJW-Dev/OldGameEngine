#pragma once

#include <glm/glm.hpp>
using namespace glm;

enum e_assetType
{
	ASSET_NULL = -1,

	ASSET_FONT = 0,
	ASSET_MATERIAL,
	ASSET_MODEL,
	ASSET_CLIPMAP,
	ASSET_WORLD,
	ASSET_SCRIPT,

	ASSET_MAX = ASSET_SCRIPT // Change when adding new assets
};

struct s_assetPool
{
	int maxPoolSize;
	int usedCount;
	int assetSize;
	char* freeHead;
	char* pool;
};

struct scriptAsset
{
	char* name;

	int length;
	char* script;
};

struct materialAsset
{
	char* name;

	int width;
	int height;
	int channels;

	int openGLTexture;
};

struct fontAsset
{
	char* name;
	materialAsset* texture;
	
	int lettersPerLine;
	int glyphHeight;
	int glyphWidth;

	char glyphWidthArray[256];
};

struct modelAsset
{
	char* name;

	float* vertexes;
	int vertexCount;

	float* UVs;
	int UVCount;

	materialAsset* texture;
};

struct worldData
{
	int count;
	float* data;
};

struct worldAsset
{
	char* name;

	int numObjs;
	unsigned int* objTextureArray;

	worldData* vertexDataArray;
	worldData* UVDataArray;
};

struct clipBound
{
	glm::vec3 origin;
	glm::vec3 mins;
	glm::vec3 maxs;
};

struct clipAsset
{
	char* name;

	int numClipBounds;
	clipBound* clips;
};

union assetHeader
{
	modelAsset* model;
	fontAsset* font;
	materialAsset* material;
	worldAsset* world;
	clipAsset* clip;
	scriptAsset* script;
	void* data;
};

#define ASSET_MAX 5

#define MAX_ASSET_NAME_LEN 0x100

void initAssetPool();
assetHeader findAsset(e_assetType type, const char* name, bool errorifMissing);
void loadAssets();