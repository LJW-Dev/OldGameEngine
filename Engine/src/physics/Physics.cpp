#include "src\physics\Physics.h"
#include "src\assetDB\AssetDB.h"

#include "src/player/player.h"

clipAsset* worldBounds;

void initWorldPhysics()
{
	worldBounds = findAsset(ASSET_CLIPMAP, "assets\\world\\world.clip", true).clip;
}

enum collisionSide
{
	COL_NONE	= 1 << 0,
	COL_TOP		= 1 << 1,
	COL_BOTTOM	= 1 << 2,
	COL_LEFT	= 1 << 3,
	COL_RIGHT	= 1 << 4,
	COL_FRONT	= 1 << 5,
	COL_BACK	= 1 << 6
};

// returns what side of the cube is being intersected
int isBoundingBoxInCube(clipBound* bounds, clipBound* cubeBounds)
{
	float boundRight = bounds->origin.x + bounds->maxs.x;
	float boundLeft = bounds->origin.x + bounds->mins.x;

	float boundTop = bounds->origin.y + bounds->maxs.y;
	float boundBottom = bounds->origin.y + bounds->mins.y;

	float boundFront = bounds->origin.z + bounds->maxs.z;
	float boundBack = bounds->origin.z + bounds->mins.z;


	float cubeRight = cubeBounds->origin.x + cubeBounds->maxs.x;
	float cubeLeft = cubeBounds->origin.x + cubeBounds->mins.x;

	float cubeTop = cubeBounds->origin.y + cubeBounds->maxs.y;
	float cubeBottom = cubeBounds->origin.y + cubeBounds->mins.y;

	float cubeFront = cubeBounds->origin.z + cubeBounds->maxs.z;
	float cubeBack = cubeBounds->origin.z + cubeBounds->mins.z;

	if (boundLeft > cubeRight && boundRight > cubeRight)
		return COL_NONE;

	if (boundLeft < cubeLeft && boundRight < cubeLeft)
		return COL_NONE;

	if (boundFront > cubeFront && boundBack > cubeFront)
		return COL_NONE;

	if (boundFront < cubeBack && boundBack < cubeBack)
		return COL_NONE;

	if (boundTop > cubeTop && boundBottom > cubeTop)
		return COL_NONE;

	if (boundTop < cubeBottom && boundBottom < cubeBottom)
		return COL_NONE;

	// cubeBounds is in bounds, but where?

	int result = 0;

	if (boundTop > cubeTop && boundBottom <= cubeTop)
		result |= COL_TOP;
	
	if (boundTop >= cubeBottom && boundBottom < cubeBottom)
		result |= COL_BOTTOM;

	if (boundLeft < cubeLeft && boundRight >= cubeLeft)
		result |= COL_LEFT;

	if (boundRight > cubeRight && boundLeft <= cubeRight)
		result |= COL_RIGHT;

	if (boundBack < cubeBack && boundFront >= cubeBack)
		result |= COL_BACK;

	if (boundFront > cubeFront && boundBack <= cubeFront)
		result |= COL_FRONT;


	return result;
}

void updatePhysics()
{
	clipBound nextPosition;

	nextPosition.origin = playerStruct.phys.position + playerStruct.phys.acceleration;
	nextPosition.mins = playerStruct.phys.mins;
	nextPosition.maxs = playerStruct.phys.maxs;

	bool hasBeenOnGround = false;
	
	for (int i = 0; i < worldBounds->numClipBounds; i++)
	{
		int colResult = isBoundingBoxInCube(&nextPosition, &worldBounds->clips[i]);

		if ((colResult & COL_NONE) == COL_NONE)
		{
			if (!hasBeenOnGround)
				playerStruct.phys.isGrounded = false;
		}

		if ((colResult & COL_TOP) == COL_TOP)
		{
			playerStruct.phys.isGrounded = true;
			hasBeenOnGround = true;
			playerStruct.phys.acceleration.y = 0;
		}

		if ((colResult & COL_BOTTOM) == COL_BOTTOM)
		{
			playerStruct.phys.isGrounded = true;
			hasBeenOnGround = true;
			playerStruct.phys.acceleration.y = 0;
		}

		if ((colResult & COL_LEFT) == COL_LEFT)
		{
			playerStruct.phys.acceleration.x = 0;
		}

		if ((colResult & COL_RIGHT) == COL_RIGHT)
		{
			playerStruct.phys.acceleration.x = 0;
		}

		if ((colResult & COL_BACK) == COL_BACK)
		{
			playerStruct.phys.acceleration.z = 0;
		}

		if ((colResult & COL_FRONT) == COL_FRONT)
		{
			playerStruct.phys.acceleration.z = 0;
		}
	}
}

void updateFriction(float deltaTime)
{
	glm::vec3 oposingForce = -playerStruct.phys.acceleration;

	float multiplier = phys_friction * deltaTime;

	if (multiplier > 1) // so high friction values don't make you go the opposite way
		multiplier = 1;

	oposingForce = oposingForce * multiplier;

	playerStruct.phys.acceleration.x += oposingForce.x;
	playerStruct.phys.acceleration.z += oposingForce.z;
}

void updatePlayerPhysics(double deltaTime)
{
	if (!playerStruct.phys.isGrounded)
	{
		// apply gravity
		playerStruct.phys.acceleration.y -= phys_gravity * deltaTime;
	}
	else
	{
		// Move forward
		if (playerStruct.keyState.forwardKey)
		{
			playerStruct.phys.acceleration.x += playerStruct.phys.forward.x * (deltaTime * phys_acceleration);
			playerStruct.phys.acceleration.z += playerStruct.phys.forward.z * (deltaTime * phys_acceleration);
		}
		// Move backward
		if (playerStruct.keyState.backwardsKey)
		{
			playerStruct.phys.acceleration.x  -= playerStruct.phys.forward.x  * (deltaTime * phys_acceleration);
			playerStruct.phys.acceleration.z  -= playerStruct.phys.forward.z  * (deltaTime * phys_acceleration);
		}
		// Strafe right
		if (playerStruct.keyState.rightKey)
		{
			playerStruct.phys.acceleration.x  += playerStruct.phys.right.x  * (deltaTime * phys_acceleration);
			playerStruct.phys.acceleration.z  += playerStruct.phys.right.z  * (deltaTime * phys_acceleration);
		}
		// Strafe left
		if (playerStruct.keyState.leftKey)
		{
			playerStruct.phys.acceleration.x  -= playerStruct.phys.right.x  * (deltaTime * phys_acceleration);
			playerStruct.phys.acceleration.z  -= playerStruct.phys.right.z  * (deltaTime * phys_acceleration);
		}
		// Jump
		if (playerStruct.keyState.jumpKey)
		{
			playerStruct.phys.acceleration.y += phys_jumpForce;
		}

		updateFriction(deltaTime);
	}

	updatePhysics();

	playerStruct.phys.position += playerStruct.phys.acceleration;
}
