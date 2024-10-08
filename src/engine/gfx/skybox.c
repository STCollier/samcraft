#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "../core/shader.h"
#include "../core/camera.h"
#include "../core/window.h"
#include "../func/player.h"

#include "skybox.h"

struct Skybox skybox_new() {
	struct Skybox self;

    glGenVertexArrays(1, &self.VAO);
    glGenBuffers(1, &self.VBO);
    glBindVertexArray(self.VAO);

	return self;
}

void skybox_render(struct Skybox skybox, shader_t shader) {
    glDisable(GL_CULL_FACE);
    glDepthMask(GL_FALSE);
	shader_use(shader);
    shader_setVec2(shader, "viewport_size", window.width, window.height);

    mat4 r, i;
    glm_mat4_mul(camera.projection, camera.view, r);
    glm_mat4_inv(r, i);

    shader_setMat4(shader, "inv_view_projection", i);
    shader_setMat4(shader, "view", camera.view);
    shader_setFloat(shader, "iTime", 1.0);

	glBindVertexArray(skybox.VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glBindVertexArray(0);
    glEnable(GL_CULL_FACE);
    glDepthMask(GL_TRUE);
}