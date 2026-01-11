#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

// Definimos las direcciones posibles de movimiento
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

// Valores por defecto
const float YAW = -90.0f; // Mirando hacia -Z
const float PITCH = 0.0f;
const float SPEED = 20.5f; // Tu velocidad actual
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;

class Camera
{
public:
    // Atributos de la cámara
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

    // Ángulos de Euler
    float Yaw;
    float Pitch;

    // Opciones de la cámara
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;
    float AspectRatio;

    // Constructor con vectores
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH)
        : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
    {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    // Devuelve la matriz de vista (LookAt)
    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(Position, Position + Front, Up);
    }

    // Devuelve la matriz de proyección
    glm::mat4 GetProjectionMatrix() const {
        return glm::perspective(glm::radians(Zoom), AspectRatio, 0.1f, 1000.0f);
    }

    // Procesa el teclado
    void ProcessKeyboard(Camera_Movement direction, float deltaTime)
    {
        float velocity = MovementSpeed * deltaTime;
        if (direction == FORWARD)
            Position += Front * velocity;
        if (direction == BACKWARD)
            Position -= Front * velocity;
        if (direction == LEFT)
            Position -= Right * velocity;
        if (direction == RIGHT)
            Position += Right * velocity;

        // Nota: Si quieres un estilo FPS "real" donde no vuelas arriba/abajo,
        // podrías hacer: Position.y = 0.0f; (o la altura fija) después de moverte.
    }

    // Procesa el movimiento del ratón
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
    {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw += xoffset;
        Pitch += yoffset;

        // Evitar que la pantalla se de vuelta (gimbal lock)
        if (constrainPitch)
        {
            if (Pitch > 89.0f)
                Pitch = 89.0f;
            if (Pitch < -89.0f)
                Pitch = -89.0f;
        }

        // Actualizar vectores con los nuevos ángulos
        updateCameraVectors();
    }

    void SetAspectRatio(float aspect) {
        AspectRatio = aspect;
    }

private:
    // Calcula el vector frontal basándose en los ángulos de Euler
    void updateCameraVectors()
    {
        // Calcular el nuevo vector Frontal
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);

        // Recalcular el vector Right (derecha) y Up (arriba)
        Right = glm::normalize(glm::cross(Front, WorldUp));
        Up = glm::normalize(glm::cross(Right, Front));
    }
};