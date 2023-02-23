#include "Player.h"
#include "src/utils/Math.h"
#include "src/dvar/dvar.h"
#include "src\window\Window.h"

#include <glm/gtx/transform.hpp>

s_playerStruct playerStruct;

void initPlayerPhysics()
{
    playerStruct.phys.position = glm::vec3(0, 5, 0);
    playerStruct.phys.forward = glm::vec3(0, 0, 0);
    playerStruct.phys.up = glm::vec3(0, 1, 0);
    playerStruct.phys.acceleration = glm::vec3(0, 0, 0);
    playerStruct.phys.horizontalAngle = 0;
    playerStruct.phys.verticalAngle = 0;
    playerStruct.phys.isGrounded = false;
                
    playerStruct.phys.mins = glm::vec3(-1, -1, -1);
    playerStruct.phys.maxs = glm::vec3(1, 1, 1);
}

void initPlayerMatrixes()
{
    // Projection matrix : 45&deg; Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
    playerStruct.renderInfo.FOV = 45.0f;
    playerStruct.renderInfo.aspectRatio = 4.0f / 3.0f;
    playerStruct.renderInfo.near = 0.1f;
    playerStruct.renderInfo.far = 1000.0f;

    playerStruct.renderInfo.projectionMatrix = glm::perspective(
        glm::radians(playerStruct.renderInfo.FOV),
        playerStruct.renderInfo.aspectRatio,
        playerStruct.renderInfo.near,
        playerStruct.renderInfo.far);

    playerStruct.renderInfo.viewMatrix = glm::lookAt(
        playerStruct.phys.position,  // Camera is here
        playerStruct.phys.position + playerStruct.phys.forward,     // and looks here : at the same position, plus "direction"
        playerStruct.phys.up      // Head is up (set to 0,-1,0 to look upside-down)
    );
}

void initPlayerStruct()
{
    initPlayerPhysics();
    initPlayerMatrixes();
}

void updatePlayerAngles()
{
    playerStruct.phys.forward = glm::vec3(
        cos(playerStruct.phys.verticalAngle) * sin(playerStruct.phys.horizontalAngle),
        sin(playerStruct.phys.verticalAngle),
        cos(playerStruct.phys.verticalAngle) * cos(playerStruct.phys.horizontalAngle)
    );

    playerStruct.phys.right = glm::vec3(
        sin(playerStruct.phys.horizontalAngle - 3.14f / 2.0f),
        0,
        cos(playerStruct.phys.horizontalAngle - 3.14f / 2.0f)
    );
}

// PI is too precise and breaks camera at the max angle
#define GLM_MAX_VERT_ANGLE 3.1415f / 2.0f

void updateCameraAngles(double deltaTime)
{
    float sens = dvar_getFloat(cl_sensitivity);

    playerStruct.phys.horizontalAngle += sens * deltaTime * float(WINDOW_X / 2 - playerStruct.keyState.mouse_x);
    playerStruct.phys.verticalAngle += sens * deltaTime * float(WINDOW_Y / 2 - playerStruct.keyState.mouse_y);

    if (playerStruct.phys.horizontalAngle > PI)
        playerStruct.phys.horizontalAngle = -PI;
    else if (playerStruct.phys.horizontalAngle < -PI)
        playerStruct.phys.horizontalAngle = PI;
    
    if (playerStruct.phys.verticalAngle > GLM_MAX_VERT_ANGLE)
        playerStruct.phys.verticalAngle = GLM_MAX_VERT_ANGLE;
    else if (playerStruct.phys.verticalAngle < -GLM_MAX_VERT_ANGLE)
        playerStruct.phys.verticalAngle = -GLM_MAX_VERT_ANGLE;

    updatePlayerAngles();
}

void updateViewMatrix()
{
    playerStruct.renderInfo.viewMatrix = glm::lookAt(
        playerStruct.phys.position,           // Camera is here
        playerStruct.phys.position + playerStruct.phys.forward, // and looks here : at the same position, plus "direction"
        playerStruct.phys.up                  // Head is up (set to 0,-1,0 to look upside-down)
    );
}