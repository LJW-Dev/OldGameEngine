#define STB_IMAGE_IMPLEMENTATION
#include "src\external\stb_image.h"
#include "src\assetDB\AssetDB.h"
#include "src\openGL\OpenGL_Import.h"
#include "src/error/error.h"

fontAsset fontPool[ASSET_MAX];
modelAsset modelPool[ASSET_MAX];
materialAsset materialPool[ASSET_MAX];
scriptAsset scriptPool[ASSET_MAX];
worldAsset worldPool[1];
clipAsset clipPool[1];

s_assetPool assetPool[ASSET_MAX + 1];

const int assetNamesSize = ASSET_MAX + 1;
const char* assetNames[] =
{
	"FONT",
	"MATERIAL",
	"MODEL",
	"CLIPMAP",
	"WORLD",
	"SCRIPT"
};

const char* getAssetTypeName(e_assetType type)
{
	return assetNames[type];
}

char* getAssetName(e_assetType type, assetHeader header)
{
	switch (type)
	{
	case ASSET_FONT:
		return header.font->name;
	case ASSET_MATERIAL:
		return header.material->name;
	case ASSET_MODEL:
		return header.model->name;
	case ASSET_WORLD:
		return header.world->name;
	case ASSET_CLIPMAP:
		return header.clip->name;
	case ASSET_SCRIPT:
		return header.script->name;
	default:
		return NULL;
	}
}

assetHeader findAsset(e_assetType type, const char* name, bool errorifMissing)
{
	assetHeader header;
	char* asetPool = assetPool[type].pool;

	for (int i = 0; i < assetPool[type].usedCount; i++)
	{
		header.data = asetPool;
		if (strcmp(getAssetName(type, header), name) == 0)
		{
			return header;
		}

		asetPool += assetPool[type].assetSize;
	}

	if(errorifMissing)
		error_exit("couldn't find %s %s asset\n", getAssetTypeName(type), name);

	header.data = NULL;
	return header;
}

assetHeader getNextInPool(e_assetType type)
{
	assetHeader header;
	header.data = NULL;

	if (assetPool[type].usedCount == assetPool[type].maxPoolSize)
	{
		error_exit("Tried to load more than %i %s assets.\n", assetPool[type].maxPoolSize, getAssetTypeName(type));
	}
	
	header.data = assetPool[type].freeHead;

	assetPool[type].usedCount++;
	assetPool[type].freeHead += assetPool[type].assetSize;

	return header;
}

void readNullTermString(FILE* file, char* buffer)
{
	int bufPos = 0;
	while (fread_s(&buffer[bufPos], sizeof(char), sizeof(char), 1, file) != 0)
	{
		if (buffer[bufPos] == '\0')
			break;

		bufPos++;
	}
}

void load_modelAsset(FILE* file, char* name)
{
	modelAsset* header = getNextInPool(ASSET_MODEL).model;

	header->name = name;

	char matName[MAX_ASSET_NAME_LEN];
	readNullTermString(file, matName);
	header->texture = findAsset(ASSET_MATERIAL, matName, true).material;

	fread_s(&header->vertexCount, sizeof(int), sizeof(int), 1, file);
	fread_s(&header->UVCount, sizeof(int), sizeof(int), 1, file);

	header->vertexes = new float[header->vertexCount];
	header->UVs = new float[header->UVCount];

	fread_s(header->vertexes, header->vertexCount * sizeof(float), sizeof(float), header->vertexCount, file);
	fread_s(header->UVs, header->UVCount * sizeof(float), sizeof(float), header->UVCount, file);
}

void load_fontAsset(FILE* file, char* name)
{
	fontAsset* header = getNextInPool(ASSET_FONT).font;

	header->name = name;

	char matName[MAX_ASSET_NAME_LEN];
	readNullTermString(file, matName);
	header->texture = findAsset(ASSET_MATERIAL, matName, true).material;

	fread_s(&header->lettersPerLine, sizeof(int), sizeof(int), 1, file);
	fread_s(&header->glyphHeight, sizeof(int), sizeof(int), 1, file);
	fread_s(&header->glyphWidth, sizeof(int), sizeof(int), 1, file);

	fread_s(header->glyphWidthArray, sizeof(header->glyphWidthArray), sizeof(char), sizeof(header->glyphWidthArray), file);
}

void load_materialAsset(FILE* file, char* name)
{
	materialAsset* matHeader = getNextInPool(ASSET_MATERIAL).material;

	matHeader->name = name;
	unsigned char* imageData = stbi_load_from_file(file, &matHeader->width, &matHeader->height, &matHeader->channels, 0);
	matHeader->openGLTexture = imoprtTextureIntoGL(matHeader->width, matHeader->height, imageData);
	free(imageData);
}

void load_worldAsset(FILE* file, char* name)
{
	worldAsset* header = getNextInPool(ASSET_WORLD).world;

	header->name = name;

	fread_s(&header->numObjs, sizeof(header->numObjs), sizeof(header->numObjs), 1, file);

	header->objTextureArray = new unsigned int[header->numObjs];
	header->vertexDataArray = new worldData[header->numObjs];
	header->UVDataArray = new worldData[header->numObjs];

	char nameBuffer[1000];
	for (int i = 0; i < header->numObjs; i++)
	{
		readNullTermString(file, nameBuffer);
		header->objTextureArray[i] = loadDiskImageIntoGL(nameBuffer);

		int vertexCount;
		int UVCount;
		fread_s(&vertexCount, sizeof(vertexCount), sizeof(vertexCount), 1, file);
		fread_s(&UVCount, sizeof(UVCount), sizeof(UVCount), 1, file);

		header->vertexDataArray[i].count = vertexCount;
		header->UVDataArray[i].count = UVCount;

		header->vertexDataArray[i].data = new float[vertexCount];
		header->UVDataArray[i].data = new float[UVCount];

		fread_s(header->vertexDataArray[i].data, vertexCount * sizeof(float), sizeof(float), vertexCount, file);
		fread_s(header->UVDataArray[i].data, UVCount * sizeof(float), sizeof(float), UVCount, file);
	}
}

void load_clipAsset(FILE* file, char* name)
{
	clipAsset* header = getNextInPool(ASSET_CLIPMAP).clip;

	header->name = name;

	fread_s(&header->numClipBounds, sizeof(int), sizeof(int), 1, file);
	header->clips = new clipBound[header->numClipBounds];

	float vec3Buffer[3 * 3];
	for (int i = 0; i < header->numClipBounds; i++)
	{
		fread_s(&vec3Buffer, sizeof(vec3Buffer), sizeof(float), 3 * 3, file);
		header->clips[i].origin.x = vec3Buffer[0];
		header->clips[i].origin.y = vec3Buffer[1];
		header->clips[i].origin.z = vec3Buffer[2];

		header->clips[i].mins.x = vec3Buffer[3];
		header->clips[i].mins.y = vec3Buffer[4];
		header->clips[i].mins.z = vec3Buffer[5];

		header->clips[i].maxs.x = vec3Buffer[6];
		header->clips[i].maxs.y = vec3Buffer[7];
		header->clips[i].maxs.z = vec3Buffer[8];
	}
}

void load_scriptAsset(FILE* file, char* name)
{
	scriptAsset* header = getNextInPool(ASSET_SCRIPT).script;

	header->name = name;

	fseek(file, 0, SEEK_END);
	header->length = ftell(file);
	fseek(file, 0, SEEK_SET);

	header->script = new char[header->length];
	fread_s(header->script, header->length, sizeof(char), header->length, file);
}

void loadAsset(e_assetType type, char* assetName)
{
	FILE* file;
	if (fopen_s(&file, assetName, "rb") != 0)
	{
		error_noexit("Asset %s not found.\n", assetName);
		return;
	}

	switch (type)
	{
	case ASSET_FONT:
		load_fontAsset(file, assetName);
		break;

	case ASSET_MATERIAL:
		load_materialAsset(file, assetName);
		break;

	case ASSET_MODEL:
		load_modelAsset(file, assetName);
		break;

	case ASSET_WORLD:
		load_worldAsset(file, assetName);
		break;

	case ASSET_CLIPMAP:
		load_clipAsset(file, assetName);
		break;

	case ASSET_SCRIPT:
		load_scriptAsset(file, assetName);
		break;
	}

	fclose(file);
}

e_assetType convertStrToType(char* assetType)
{
	for (int i = 0; i < assetNamesSize; i++)
	{
		if (!strcmp(assetNames[i], assetType))
			return static_cast<e_assetType>(i);
	}

	return ASSET_NULL;
}

void loadAssets()
{
	FILE* assetFile;

	if (fopen_s(&assetFile, "assets\\assets.txt", "r") != 0)
	{
		error_exit("Unable to open assetFile.\n");
	}

	char assetTypeStr[MAX_ASSET_NAME_LEN];
	char* assetName;
	while (fscanf_s(assetFile, 
		"%s %s", 
		assetTypeStr, MAX_ASSET_NAME_LEN, 
		assetName = new char[MAX_ASSET_NAME_LEN], MAX_ASSET_NAME_LEN
	) != -1)
	{
		e_assetType assetType = convertStrToType(assetTypeStr);

		if(assetType != ASSET_NULL)
			loadAsset(assetType, assetName);
	}
}

void initAssetPool()
{
	assetPool[ASSET_FONT].maxPoolSize = ASSET_MAX;
	assetPool[ASSET_FONT].usedCount = 0;
	assetPool[ASSET_FONT].assetSize = sizeof(fontAsset);
	assetPool[ASSET_FONT].freeHead = (char*)fontPool;
	assetPool[ASSET_FONT].pool = (char*)fontPool;

	assetPool[ASSET_MATERIAL].maxPoolSize = ASSET_MAX;
	assetPool[ASSET_MATERIAL].usedCount = 0;
	assetPool[ASSET_MATERIAL].assetSize = sizeof(materialAsset);
	assetPool[ASSET_MATERIAL].freeHead = (char*)materialPool;
	assetPool[ASSET_MATERIAL].pool = (char*)materialPool;

	assetPool[ASSET_MODEL].maxPoolSize = ASSET_MAX;
	assetPool[ASSET_MODEL].usedCount = 0;
	assetPool[ASSET_MODEL].assetSize = sizeof(modelAsset);
	assetPool[ASSET_MODEL].freeHead = (char*)modelPool;
	assetPool[ASSET_MODEL].pool = (char*)modelPool;

	assetPool[ASSET_CLIPMAP].maxPoolSize = 1;
	assetPool[ASSET_CLIPMAP].usedCount = 0;
	assetPool[ASSET_CLIPMAP].assetSize = sizeof(clipAsset);
	assetPool[ASSET_CLIPMAP].freeHead = (char*)clipPool;
	assetPool[ASSET_CLIPMAP].pool = (char*)clipPool;

	assetPool[ASSET_WORLD].maxPoolSize = 1;
	assetPool[ASSET_WORLD].usedCount = 0;
	assetPool[ASSET_WORLD].assetSize = sizeof(worldAsset);
	assetPool[ASSET_WORLD].freeHead = (char*)worldPool;
	assetPool[ASSET_WORLD].pool = (char*)worldPool;

	assetPool[ASSET_SCRIPT].maxPoolSize = ASSET_MAX;
	assetPool[ASSET_SCRIPT].usedCount = 0;
	assetPool[ASSET_SCRIPT].assetSize = sizeof(scriptAsset);
	assetPool[ASSET_SCRIPT].freeHead = (char*)scriptPool;
	assetPool[ASSET_SCRIPT].pool = (char*)scriptPool;

	loadAssets();
}