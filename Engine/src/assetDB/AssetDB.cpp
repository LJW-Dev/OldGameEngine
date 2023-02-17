#include <stdio.h>
#include <fstream>

#define STB_IMAGE_IMPLEMENTATION
#include "src\external\stb_image.h"

#include "src\assetDB\AssetDB.h"
#include "src\OpenGL.h"

XFont s_XFontPool[ASSET_MAX];
XModel s_XModelPool[ASSET_MAX];
XMaterial s_XMaterialPool[ASSET_MAX];
XScript s_XScriptPool[ASSET_MAX];
XworldObject s_WorldObjectPool[1];
XClipMap s_ClipMapPool[1];

const int assetNamesSize = XASSET_MAX + 1;
const char* assetNames[] =
{
	"FONT",
	"MATERIAL",
	"MODEL",
	"CLIPMAP",
	"WORLD",
	"SCRIPT"
};

xAsset s_assetPool[XASSET_MAX + 1];

char* getXAssetName(xAssetType type, char* header)
{
	switch (type)
	{
	case XASSET_FONT:
		return ((XFont*)header)->name;
	case XASSET_MATERIAL:
		return ((XMaterial*)header)->name;
	case XASSET_MODEL:
		return ((XModel*)header)->name;
	case XASSET_WORLD:
		return ((XworldObject*)header)->name;
	case XASSET_CLIPMAP:
		return ((XClipMap*)header)->name;
	case XASSET_SCRIPT:
		return ((XScript*)header)->name;
	default:
		return NULL;
	}
}

XAssetHeader findAsset(xAssetType type, const char* name)
{
	XAssetHeader header;
	header.data = NULL;

	char* asetPool = s_assetPool[type].pool;

	for (int i = 0; i < s_assetPool[type].usedCount; i++)
	{
		if (strcmp(getXAssetName(type, asetPool), name) == 0)
		{
			header.data = asetPool;
			return header;
		}

		asetPool += s_assetPool[type].assetSize;
	}

	printf("couldn't find %s %s\n", assetNames[type], name);

	return header;
}

XAssetHeader getNextInPool(xAssetType type)
{
	XAssetHeader header;
	header.data = NULL;

	if (s_assetPool[type].usedCount >= s_assetPool[type].maxPoolSize)
	{
		return header;
	}
	
	header.data = s_assetPool[type].freeHead;

	s_assetPool[type].usedCount++;
	s_assetPool[type].freeHead += s_assetPool[type].assetSize;

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

void load_XModel(FILE* file, char* name)
{
	XModel* header = getNextInPool(XASSET_MODEL).XModel;

	if (header == NULL)
	{
		printf("Too many Xmodels!");
		return;
	}

	header->name = name;

	fread_s(&header->vertexCount, sizeof(int), sizeof(int), 1, file);
	fread_s(&header->UVCount, sizeof(int), sizeof(int), 1, file);

	header->vertexes = new float[header->vertexCount];
	header->UVs = new float[header->UVCount];

	fread_s(header->vertexes, header->vertexCount * sizeof(float), sizeof(float), header->vertexCount, file);
	fread_s(header->UVs, header->UVCount * sizeof(float), sizeof(float), header->UVCount, file);
}

void load_XFont(FILE* file, char* name)
{
	XFont* header = getNextInPool(XASSET_FONT).XFont;

	if (header == NULL)
	{
		printf("Too many XFonts!");
		return;
	}

	header->name = name;

	char matName[MAX_ASSET_NAME_LEN];
	readNullTermString(file, matName);
	header->texture = loadImageIntoGL(matName);

	fread_s(&header->lettersPerLine, sizeof(int), sizeof(int), 1, file);
	fread_s(&header->glyphHeight, sizeof(int), sizeof(int), 1, file);
	fread_s(&header->glyphWidth, sizeof(int), sizeof(int), 1, file);

	fread_s(header->glyphWidthArray, sizeof(header->glyphWidthArray), sizeof(char), sizeof(header->glyphWidthArray), file);
}

void load_XMaterial(FILE* file, char* name)
{
	XMaterial* matHeader = getNextInPool(XASSET_MATERIAL).XMaterial;
	if (matHeader == NULL)
	{
		printf("Too many materials!");
		return;
	}
	matHeader->name = name;

	int channels;
	matHeader->imageData = stbi_load_from_file(file, &matHeader->width, &matHeader->height, &channels, 0);
}

void load_WorldObject(FILE* file, char* name)
{
	XworldObject* header = getNextInPool(XASSET_WORLD).World;

	if (header == NULL)
	{
		printf("Too many Xmodels!");
		return;
	}

	header->name = name;

	fread_s(&header->numObjs, sizeof(header->numObjs), sizeof(header->numObjs), 1, file);

	header->objTextureArray = new unsigned int[header->numObjs];
	header->vertexDataArray = new worldData[header->numObjs];
	header->UVDataArray = new worldData[header->numObjs];

	char nameBuffer[1000];
	for (int i = 0; i < header->numObjs; i++)
	{
		readNullTermString(file, nameBuffer);
		header->objTextureArray[i] = loadImageIntoGL(nameBuffer);

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

void load_clipWorld(FILE* file, char* name)
{
	XClipMap* header = getNextInPool(XASSET_CLIPMAP).Clip;

	header->name = name;

	fread_s(&header->numClipBounds, sizeof(int), sizeof(int), 1, file);
	header->clips = new ClipBound[header->numClipBounds];

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

void load_XScript(FILE* file, char* name)
{
	XScript* header = getNextInPool(XASSET_CLIPMAP).Script;

	header->name = name;

	fseek(file, 0, SEEK_END);
	long len = ftell(file);
	fseek(file, 0, SEEK_SET);

	header->script = new char[len];
	fread_s(header->script, len, sizeof(char), len, file);
}

void loadAsset(xAssetType type, char* assetName)
{
	FILE* file;
	if (fopen_s(&file, assetName, "rb") != 0)
	{
		printf("Asset %s not found.\n", assetName);
		return;
	}

	switch (type)
	{
	case XASSET_FONT:
		load_XFont(file, assetName);
		break;

	case XASSET_MATERIAL:
		load_XMaterial(file, assetName);
		break;

	case XASSET_MODEL:
		load_XModel(file, assetName);
		break;

	case XASSET_WORLD:
		load_WorldObject(file, assetName);
		break;

	case XASSET_CLIPMAP:
		load_clipWorld(file, assetName);
		break;

	case XASSET_SCRIPT:
		load_XScript(file, assetName);
		break;
	}

	fclose(file);
}

xAssetType convertStrToType(char* assetType)
{
	for (int i = 0; i < assetNamesSize; i++)
	{
		if (!strcmp(assetNames[i], assetType))
			return static_cast<xAssetType>(i);
	}

	return XASSET_NULL;
}

void loadAssets()
{
	FILE* assetFile;

	if (fopen_s(&assetFile, "assets\\assets.txt", "r") != 0)
	{
		printf("Unable to open assetFile.\n");
		return;
	}

	char assetTypeStr[MAX_ASSET_NAME_LEN];
	char* assetName;
	while (fscanf_s(assetFile, 
		"%s %s", 
		assetTypeStr, MAX_ASSET_NAME_LEN, 
		assetName = new char[MAX_ASSET_NAME_LEN], MAX_ASSET_NAME_LEN
	) != -1)
	{
		xAssetType assetType = convertStrToType(assetTypeStr);

		if(assetType != XASSET_NULL)
			loadAsset(assetType, assetName);
	}
}

void initXAssetPool()
{
	s_assetPool[XASSET_FONT].maxPoolSize = ASSET_MAX;
	s_assetPool[XASSET_FONT].usedCount = 0;
	s_assetPool[XASSET_FONT].assetSize = sizeof(XFont);
	s_assetPool[XASSET_FONT].freeHead = (char*)s_XFontPool;
	s_assetPool[XASSET_FONT].pool = (char*)s_XFontPool;

	s_assetPool[XASSET_MATERIAL].maxPoolSize = ASSET_MAX;
	s_assetPool[XASSET_MATERIAL].usedCount = 0;
	s_assetPool[XASSET_MATERIAL].assetSize = sizeof(XMaterial);
	s_assetPool[XASSET_MATERIAL].freeHead = (char*)s_XMaterialPool;
	s_assetPool[XASSET_MATERIAL].pool = (char*)s_XMaterialPool;

	s_assetPool[XASSET_MODEL].maxPoolSize = ASSET_MAX;
	s_assetPool[XASSET_MODEL].usedCount = 0;
	s_assetPool[XASSET_MODEL].assetSize = sizeof(XModel);
	s_assetPool[XASSET_MODEL].freeHead = (char*)s_XModelPool;
	s_assetPool[XASSET_MODEL].pool = (char*)s_XModelPool;

	s_assetPool[XASSET_CLIPMAP].maxPoolSize = 1;
	s_assetPool[XASSET_CLIPMAP].usedCount = 0;
	s_assetPool[XASSET_CLIPMAP].assetSize = sizeof(XClipMap);
	s_assetPool[XASSET_CLIPMAP].freeHead = (char*)s_ClipMapPool;
	s_assetPool[XASSET_CLIPMAP].pool = (char*)s_ClipMapPool;

	s_assetPool[XASSET_WORLD].maxPoolSize = 1;
	s_assetPool[XASSET_WORLD].usedCount = 0;
	s_assetPool[XASSET_WORLD].assetSize = sizeof(XworldObject);
	s_assetPool[XASSET_WORLD].freeHead = (char*)s_WorldObjectPool;
	s_assetPool[XASSET_WORLD].pool = (char*)s_WorldObjectPool;

	s_assetPool[XASSET_SCRIPT].maxPoolSize = ASSET_MAX;
	s_assetPool[XASSET_SCRIPT].usedCount = 0;
	s_assetPool[XASSET_SCRIPT].assetSize = sizeof(XScript);
	s_assetPool[XASSET_SCRIPT].freeHead = (char*)s_XScriptPool;
	s_assetPool[XASSET_SCRIPT].pool = (char*)s_XScriptPool;

	loadAssets();
}