#include "skybox.h"
#include "player.h"

/*float skyboxVertices[] = {
    // positions          
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};*/

struct Skybox skybox_new() {
	struct Skybox self;

    GL_CHECK(glGenVertexArrays(1, &self.VAO));
    GL_CHECK(glGenBuffers(1, &self.VBO));
    GL_CHECK(glBindVertexArray(self.VAO));

	return self;
}

void skybox_render(struct Skybox *skybox, shader_t shader) {
    glDisable(GL_CULL_FACE);
    glDepthMask(GL_FALSE);
	shader_use(shader);
	shader_setVec3(shader, "sun_position", 0.0, -0.1 + glfwGetTime()*0.01, -1.0);
    shader_setVec2(shader, "viewport_size", window.width, window.height);

    mat4 r, i;
    glm_mat4_mul(camera.projection, camera.view, r);
    glm_mat4_inv(r, i);

    shader_setMat4(shader, "inv_view_projection", i);
    shader_setMat4(shader, "view", camera.view);

	GL_CHECK(glBindVertexArray(skybox->VAO));
	GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, 3));
	GL_CHECK(glBindVertexArray(0));
    glEnable(GL_CULL_FACE);
    glDepthMask(GL_TRUE);
}