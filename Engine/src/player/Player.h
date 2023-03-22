#pragma once

#include <glm/glm.hpp>
using namespace glm;

struct s_keyState
{
    double mouse_x;
    double mouse_y;

    bool jumpKey;
    bool forwardKey;
    bool backwardsKey;
    bool leftKey;
    bool rightKey;
};

struct s_playerRenderInfo
{
    glm::mat4 projectionMatrix;
    glm::mat4 viewMatrix;

    glm::mat4 ViewProjectionMatrix;

    float FOV;
    float aspectRatio;
    float near;
    float far;
};

struct playerPhys
{
    glm::vec3 position;
    glm::vec3 acceleration;

    float horizontalAngle;
    float verticalAngle;

    glm::vec3 forward;
    glm::vec3 right;
    glm::vec3 up;

    glm::vec3 mins;
    glm::vec3 maxs;

    bool isGrounded;
};

struct s_playerStruct
{
    s_playerRenderInfo renderInfo;
    playerPhys phys;
    
    s_keyState keyState;
};

extern s_playerStruct playerStruct;

void initPlayerStruct();
void updateCameraAngles(double deltaTime);
void updateViewMatrix();