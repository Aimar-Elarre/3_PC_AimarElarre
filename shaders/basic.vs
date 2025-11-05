#version 330 core
layout (location = 0) in vec3 aPos;
void main() {
    vec3 position = aPos;
    position.x += 0.5;
    gl_Position = vec4(position, 1);
}
