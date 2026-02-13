#ifndef __WORLD_H__
#define __WORLD_H__
#include "plaza.h"
#include "system.h"

//Centerpoint of the ECS manages entities through the plaza and systems by itself
typedef struct {
    plaza *p; //The plaza to manage entities
    list *systems; //NEED TO MAKE THIS A PRIORITY QUEUE SO WE CAN ORDER THE WAY IN WHICH SYSTEMS RUN
    b2WorldId world_id; //The id of the box2d world the ECS uses
} world;

/*
 * Creating the world
 * @param world_id the id of the box2d world that we are using
 * @return a pointer to the created world struct on the heap
 */
world *world_alloc(b2WorldId world_id);
/*
 * Initialises all the systems stored in the world
 * @param w the world whose systems need to be initialised
 */
void world_init(world *w);
/*
 * Updating all the systems stored in the world
 * @param w the world whose systems need to be updated
 * @param dt the timestep between the current and previous update
 */
void world_update(world *w, float dt);
/*
 * Updates all of the systems to have the entities that match their signature
 * @param w the world whose systems need to be updated
 */
void sys_query(world *w);
/*
 * Adding a new system to the world 
 * @param w the world the system will be added to
 * @param init func the initialisation function for the system
 * @param update_func the update function for the system
 */
void add_system(world *w, void (*init_func)(plaza *, ecs_system *), void (*update_func)(plaza *, ecs_system *, float));

#endif
