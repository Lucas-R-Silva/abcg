#version 330 core

in vec2 fragTexCoord;

out vec4 outColor;

uniform sampler2D birdTexture;  // Textura do pássaro

void main() {
    outColor = texture(birdTexture, fragTexCoord);
}
