#version 330 core

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inTexCoord;

out vec2 fragTexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * model * vec4(inPosition, 0.0, 1.0);
    fragTexCoord = inTexCoord;
}
