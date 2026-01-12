#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// --- NUEVO: Variables para animación ---
uniform float uTime;  // Tiempo recibido desde App.cpp
uniform bool isWater; // ¿Es el objeto agua?

void main()
{
    vec3 finalPos = aPos;

    // APLICAR OLAS SOLO SI ES AGUA
    if(isWater) {
        // Fórmula de onda: Seno y Coseno combinados para un movimiento orgánico
        // 0.5 * x + uTime: Velocidad y frecuencia en X
        // 0.3 * z + uTime: Velocidad y frecuencia en Z
        // * 0.5: Amplitud (altura) de la ola
        float wave = sin(aPos.x * 0.5 + uTime) * 0.25 + cos(aPos.z * 0.3 + uTime) * 0.25;
        finalPos.y += wave;
    }

    FragPos = vec3(model * vec4(finalPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal; 
    TexCoord = aTexCoord;
    
    gl_Position = projection * view * vec4(FragPos, 1.0);
}