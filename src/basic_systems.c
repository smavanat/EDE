#include "../include/basic_systems.h"

void render_system_init(plaza *p, ecs_system *s) {
    s->signature = 0 | (1 << SPRITE); //Add the sprite bit to the signature
}
void render_system_update(plaza *p, ecs_system *s, float dt) {
    for(int i = 0; i < s->entities->size; i++) {
        for(int j = 0; j < ((archetype **)s->entities->data)[i]->size; j++) {
            sprite *sprite = get_component_from_entity(p, ((archetype **)s->entities->data)[i]->entities[j], SPRITE);

            // glBindTexture(GL_TEXTURE_2D, sprite->texture);
            // mat3 *transform;
            // vec2 tv = (vec2){0.5f, -0.5f};
            // glm_translate2d(transform, &tv);
            // glm_rotate2d(transform, )

            // use(sprite->shader);
            // glBindVertexArray(sprite->VAO);
            // glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            render_push_quad(gRenderer, sprite->coords, sprite->colours, sprite->uv, sprite->texture);
        }
    }
}
