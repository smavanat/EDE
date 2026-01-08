#include "../include/rigidbody.h"
#include "../include/queue.h"
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define NO_PIXEL_COLOUR (uint8_t[]){0,0,0,0}

uint32_t rigidbody_to_pixel_pos(uint32_t pos, uint32_t r_width, uint32_t w_width) {
    return ((pos/r_width) * w_width) + (pos % r_width);
}

uint32_t world_to_pixel_pos(ivector2 pos, uint32_t width) {
    return (pos.y * width) + pos.x;
}

ivector2 pixel_to_world_pos(uint32_t pos, uint32_t width) {
    return (ivector2){pos % width, pos / width};
}

//Using Bilinear interpolation to sample the pixel colour in a rotated image
void sample_pixel(float x, float y, pixel **pixel_array, uint32_t width, uint32_t height, uint8_t *ret) {
    //Get four adjacent pixels
    int x0 = floor(x);
    int x1 = ceil(x);
    int y0 = floor(y);
    int y1 = ceil(y);

    //Clamp their positions to be valid indecies
    x0 = clamp(x0, 0, width-1);
    x1 = clamp(x1, 0, width-1);
    y0 = clamp(y0, 0, height-1);
    y1 = clamp(y1, 0, height-1);

    //Get the colours of the pixels, setting them to be blank if the pixel does not exist
    uint8_t *y0x0_colour = (pixel_array[(y0 * width) + x0] == NULL) ? (uint8_t[]){0,0,0,0} : pixel_array[(y0 * width) + x0]->colour;
    uint8_t *y0x1_colour = (pixel_array[(y0 * width) + x1] == NULL) ? (uint8_t[]){0,0,0,0} : pixel_array[(y0 * width) + x1]->colour;
    uint8_t *y1x0_colour = (pixel_array[(y1 * width) + x0] == NULL) ? (uint8_t[]){0,0,0,0} : pixel_array[(y1 * width) + x0]->colour;
    uint8_t *y1x1_colour = (pixel_array[(y1 * width) + x1] == NULL) ? (uint8_t[]){0,0,0,0} : pixel_array[(y1 * width) + x1]->colour;

    float dx = x - x0;
    float dy = y - y0;

    for(int i = 0; i < 4; i++) {
        float val = y0x0_colour[i] * (1-dx) * (1-dy) + y0x1_colour[i] * dx * (1-dy) + y1x0_colour[i] * (1-dx) * dy + y1x1_colour[i] * dx * dy;
        ret[i] = (uint8_t)(val + 0.5f);
    }
}

rigidbody *create_rigidbody(uint32_t id, uint16_t width, uint16_t height, uint8_t colour[4], ivector2 startpos, world_grid *grid) {
    rigidbody *rb = malloc(sizeof(rigidbody));
    rb->height = height;
    rb->width = width;
    rb->pixel_count = width * height;
    memcpy(rb->colour, colour, sizeof(rb->colour));
    rb->pixel_coords = malloc(sizeof(ivector2) * rb->pixel_count);
    for(int y = 0; y < height; y++) {
        for(int x = 0; x < width; x++) {
            memcpy(grid->pixels[((startpos.y + y - (height/2))*grid->width) + startpos.x + x - (width/2)].colour, colour, sizeof(grid->pixels[((startpos.y + y)*grid->width) + startpos.x + x].colour));
            grid->pixels[((startpos.y + y - (height/2))*grid->width) + startpos.x + x - (width/2)].parent_body = id;
            rb->pixel_coords[(y*width) + x ] = (ivector2){x - (width/2), y - (height/2)};
        }
    }

    return rb;
}

rigidbody *create_rigidbody_from_pixels(uint32_t id, uint16_t width, uint16_t height, uint8_t colour[4], ivector2 center, list *pixel_coords, world_grid *grid) {
    rigidbody *rb = malloc(sizeof(rigidbody));
    rb->height = height;
    rb->width = width;
    rb->pixel_count = pixel_coords->size;
    memcpy(rb->colour, colour, sizeof(rb->colour));
    rb->pixel_coords = malloc(sizeof(ivector2) * rb->pixel_count);
    for(int i = 0; i < rb->pixel_count; i++) {
        ivector2 coord = get_value(pixel_coords, ivector2, i);
        rb->pixel_coords[i] = (ivector2){center.x - coord.x, center.y - coord.y};
        grid->pixels[(coord.y * grid->width) + coord.x].parent_body = id;
    }

    return rb;
}

void erasePixels(int radius, int x, int y, world_grid *grid, list *rbs) {
    if (radius > 0) {
        for (int w = 0; w < radius * 2; w++)
        {
            for (int h = 0; h < radius * 2; h++)
            {
                int dx = radius - w; // horizontal offset
                int dy = radius - h; // vertical offset
                if ((dx * dx + dy * dy) < (radius * radius) && (x + dx < grid->width) && (x + dx > -1) && (y + dy < grid->height) && (y + dy > -1))
                {
                    memcpy(grid->pixels[(y + dy) * grid->width + (x + dx)].colour, NO_PIXEL_COLOUR, sizeof(grid->pixels[(y + dy) * grid->width + (x + dx)].colour));
                    // if(grid->pixels[(y + dy) * grid->width + (x + dx)].parent_body != -1) push_value(rbs, int8_t, grid->pixels[(y + dy) * grid->width + (x + dx)].parent_body);
                    ivector2 new_pos = (ivector2){x+dx, y+dy};
                    if(grid->pixels[(y + dy) * grid->width + (x + dx)].parent_body != -1) push_value(rbs, ivector2, new_pos);
                }
            }
        }
    }
    else {
        memcpy(grid->pixels[(y * grid->width) + x].colour, NO_PIXEL_COLOUR, sizeof(grid->pixels[(y * grid->width) + x].colour));
        ivector2 new_pos = (ivector2){x, y};
        if(grid->pixels[(y * grid->width) + x].parent_body != -1) push_value(rbs, ivector2, new_pos);
        // if(grid->pixels[(y * grid->width) + x].parent_body != -1) push_value(rbs, int8_t, grid->pixels[(y * grid->width) + x].parent_body);
    }
}

/**
 * MARCHING SQUARES IMPLEMENTATION
 */

//Marching squares: First we need to get the starting pixel. This is just done by iterating over the array until 
//                  we find a non-transparent pixel
//                  Then we need to find the square value of it and the four pixels surrounding it
//                  Then based on that square value (and in the special saddle cases also on the previous square value)
//                  we choose a new direction to move the "analysis" and add the currently analysed pixel to a vector;
//                  Good source code and ideas from here: https://emanueleferonato.com/2013/03/01/using-marching-squares-algorithm-to-trace-the-contour-of-an-image/
//                  And here: https://barradeau.com/blog/?p=391

//Determines what marching squares 'square' we are currently in
int get_current_square(ivector2 start_coord, world_grid *grid, int32_t id) {
    int result = 0;
    int index = (start_coord.y * grid->width) + start_coord.x;

    // Top-left pixel
    if (grid->pixels[index].parent_body == id) result += 1;

    // Top-right pixel
    if (start_coord.x + 1 < grid->width && grid->pixels[index + 1].parent_body == id) result += 2;

    // Bottom-left pixel
    if (start_coord.y + 1 < grid->height && grid->pixels[index + grid->width].parent_body == id) result += 4;

    // Bottom-right pixel
    if (start_coord.x + 1 < grid->width && start_coord.y + 1 < grid->height && grid->pixels[index + grid->width + 1].parent_body == id) result += 8;

    return result;
}

//Actual marching squares method.
list *marching_squares(list *pixel_coords, world_grid *grid, int32_t id) {
    ivector2 start_point = get_value(pixel_coords, ivector2, 0);
    list *contour_points = list_alloc(pixel_coords->size, sizeof(ivector2));
    //If the texture is filled on the LHS, we will end up with 15 as our first currentSquare. 
    //To avoid this, we simply offset startPoint one to the left, to get 12 as our currentSquare, 
    //and then marching squares handles the rest.
    if (get_current_square(start_point, grid, id) == 15) {
        if(start_point.x > 0) start_point.x -= 1;
        else if( start_point.y > 0) start_point.y -= 1;
        else fprintf(stderr, "Congratulations, you've somehow managed to make a rigidbody in the top-left corner of the world grid. I don't know how to deal with this, so it's broken for now\n");
    }

    int stepX = 0, stepY = 0;
    int prevX = 0, prevY = 0;
    ivector2 current_point = start_point;
    bool closed_loop = false;

    while (!closed_loop) {
        int current_square = get_current_square(current_point, grid, id);

        // Movement lookup based on currentSquare value
        switch (current_square) {
        case 1: case 13:
            stepX = 0; stepY = -1;
            break;
        case 8: case 10: case 11:
            stepX = 0; stepY = 1;
            break;
        case 4: case 12: case 14:
            stepX = -1; stepY = 0;
            break;
        case 2: case 3: case 7:
            stepX = 1; stepY = 0;
            break;
        case 5:
            stepX = 0; stepY = -1;
            break;
        case 6:
            stepX = (prevY == -1) ? -1 : 1;
            stepY = 0;
            break;
        case 9:
            stepX = 0;
            stepY = (prevX == 1) ? -1 : 1;
            break;
        default:
            printf("Unhandled or empty square encountered at coordinate: (%i, %i)\n", current_point.x, current_point.y);
            return contour_points;
        }

        current_point.x += stepX;
        current_point.y += stepY;

        // Boundary checks. Should not happen but here just in case.
        if(current_point.x < 0 || current_point.x >= grid->width || current_point.y < 0 || current_point.y >= grid->height) {

            printf("Out-of-bounds detected at index: (%i, %i)\n", current_point.x, current_point.y);
            return contour_points;
        }

        push_value(contour_points, ivector2, current_point);
        prevX = stepX;
        prevY = stepY;

        if (current_point.x == start_point.x && current_point.y == start_point.y) closed_loop = true;
    }

    return contour_points;
}

/**
 * RAMER-DOUGLAS-PEUCKER IMPLEMENTATION
 */
//Code source: https://editor.p5js.org/codingtrain/sketches/SQjSugKn6
float lineDist(ivector2 point, ivector2 start_point, ivector2 end_point) {
    //The source for this very cursed single line of code can be found here : https://en.wikipedia.org/wiki/Distance_from_a_point_to_a_line#Line_defined_by_two_points
    return abs(((end_point.y - start_point.y) * point.x) -
        ((end_point.x - start_point.x) * point.y) +
        (end_point.x * start_point.y) - (end_point.y * start_point.x)) /
        sqrt(pow((end_point.y - start_point.y), 2) + pow((end_point.x - start_point.x), 2));
}

int findFurthest(list* all_points, int a, int b, int epsilon) {
    float recordDistance = -1;
    int furthestIndex = -1;
    ivector2 start = get_value(all_points, ivector2, a);
    ivector2 end = get_value(all_points, ivector2, b);
    for (int i = a + 1; i < b; i++) {
        float d = lineDist(get_value(all_points, ivector2, i), start, end);
        if (d > recordDistance) {
            recordDistance = d;
            furthestIndex = i;
        }
    }
    if (recordDistance > epsilon) return furthestIndex;
    else return -1;
}

//This method would be used for lines that do not join up
void rdp(int startIndex, int endIndex, int epsilon, list *all_points, list* rdp_points) {
    int next_index = findFurthest(all_points, startIndex, endIndex, epsilon);
    if (next_index > 0) {
        if (startIndex != next_index) {
            rdp(startIndex, next_index, epsilon, all_points, rdp_points);
        }
        push_value(rdp_points, ivector2, get_value(all_points, ivector2, next_index));
        if (endIndex != next_index) {
            rdp(next_index, endIndex, epsilon, all_points, rdp_points);
        }
    }
}

/**
 * CODE FOR MAKING NEW RIGIDBODIES AFTER ERASURE
 */
//Crappy augmented flood-fill algorithm implementation taken from here: https://www.geeksforgeeks.org/flood-fill-algorithm/
list *bfs(ivector2 start, pixel *grid_coords, int size, int width, int height, int id) {
    list *indecies = list_alloc(size, sizeof(ivector2));
    queue *q = queue_alloc(size, sizeof(ivector2));

    push_value(indecies, ivector2, start);
    enqueue(q, ivector2, start);

    while (q->size > 0) {
        ivector2 current_pixel;
        deqeue(q, ivector2, current_pixel);
        if(current_pixel.y > 0 && grid_coords[((current_pixel.y - 1) * width) + current_pixel.x].parent_body == id) {
            ivector2 new_pos = (ivector2){current_pixel.x, current_pixel.y-1};
            push_value(indecies, ivector2, new_pos);
            enqueue(q, ivector2, new_pos);
            grid_coords[((current_pixel.y - 1) * width) + current_pixel.x].parent_body = -1;
        }
        if(current_pixel.x > 0 && grid_coords[((current_pixel.y) * width) + current_pixel.x - 1].parent_body == id) {
            ivector2 new_pos = (ivector2){current_pixel.x-1, current_pixel.y};
            push_value(indecies, ivector2, new_pos);
            enqueue(q, ivector2, new_pos);
            grid_coords[((current_pixel.y) * width) + current_pixel.x-1].parent_body = -1;
        }
        if(current_pixel.y < height - 1 && grid_coords[((current_pixel.y + 1) * width) + current_pixel.x].parent_body == id) {
            ivector2 new_pos = (ivector2){current_pixel.x, current_pixel.y+1};
            push_value(indecies, ivector2, new_pos);
            enqueue(q, ivector2, new_pos);
            grid_coords[((current_pixel.y + 1) * width) + current_pixel.x].parent_body = -1;
        }
        if(current_pixel.x < width - 1 && grid_coords[((current_pixel.y) * width) + current_pixel.x+1].parent_body == id) {
            ivector2 new_pos = (ivector2){current_pixel.x+1, current_pixel.y};
            push_value(indecies, ivector2, new_pos);
            enqueue(q, ivector2, new_pos);
            grid_coords[((current_pixel.y) * width) + current_pixel.x+1].parent_body = -1;
        }
    }

    //NEED TO SORT THE INDECIES HERE
    free_queue(q);
    return indecies;
}

void construct_new_rigidbody(list *pixel_coords, world_grid *grid, uint8_t colour[4], float zIndex, float angle, plaza *p, b2WorldId world_id) {
    int minX = INT_MAX, maxX = INT_MIN, minY = INT_MIN, maxY = INT_MIN;

    for(int i = 0; i < pixel_coords->size; i++) {
        ivector2 coords = get_value(pixel_coords, ivector2, i);
        if(coords.x < minX) minX = coords.x;
        if(coords.x > maxX) maxX = coords.x;
        if(coords.y < minY) minY = coords.y;
        if(coords.y > maxY) maxY = coords.y;
    }

    int width = maxX - minX;
    int height = maxY - minY;

    entity e = create_entity(p);
    transform *t = create_transform((vector2){minX + (width/2), minY + (height/2)}, zIndex, angle);
    add_component_to_entity(p, e, TRANSFORM, t);

    rigidbody *rb = create_rigidbody_from_pixels(e, width, height, colour, (ivector2){minX + (width/2), minY + (height/2)}, pixel_coords, grid);
    add_component_to_entity(p, e, RIGIDBODY, rb);

    list *ms_points = marching_squares(pixel_coords, grid, e);
    list *rdp_points = list_alloc(ms_points->size, sizeof(ivector2));
    vector2 *points = malloc(sizeof(vector2) * rdp_points->size);
    for(int i = 0; i < rdp_points->size; i++) {
        points[i] = (vector2){get_value(rdp_points, ivector2, i).x, get_value(rdp_points, ivector2, i).y};
    }
    rdp(0, ms_points->size, 3, ms_points, rdp_points);
    collider *c = malloc(sizeof(collider));
    c->type = POLYGON;
    c->collider_id = create_polygon_collider(points, rdp_points->size, (vector2){minX + (width/2), minY + (height/2)}, t->angle, world_id, b2_dynamicBody);

    free(ms_points);
    free(rdp_points);
    free(points);
}

void split_rigidbody(entity id, plaza *p, world_grid *grid, b2WorldId world_id) {
    transform *t = get_component_from_entity(p, id, TRANSFORM);
    rigidbody *rb = get_component_from_entity(p, id, RIGIDBODY);
    list *new_rigidbody_pixels = list_alloc(5, sizeof(list *));

    ivector2 *grid_coords = malloc(sizeof(ivector2) * rb->pixel_count);
    pixel *grid_pixels = malloc(sizeof(pixel) * grid->width * grid->height);
    memcpy(grid_pixels, grid->pixels, sizeof(pixel) * grid->width * grid->height);

    for(int i = 0; i < rb->pixel_count; i++) {
        grid_coords[i] = (ivector2){(int)t->position.x + rb->pixel_coords[i].x, (int)t->position.y + rb->pixel_coords[i].y};
    }

    for(int i = 0; i < rb->pixel_count; i++) {
        if(grid->pixels[(grid_coords[i].y * grid->width) + grid_coords[i].x].parent_body == id) {
            push_value(new_rigidbody_pixels, list *, bfs(grid_coords[i], grid_pixels, rb->pixel_count, grid->width, rb->height, id));
        }
    }

    if(new_rigidbody_pixels->size > 1) {
        //Make new rigidbodies here
        for(int i = 0; i < new_rigidbody_pixels->size; i++) {
            construct_new_rigidbody(get_value(new_rigidbody_pixels, list *, i), grid, rb->colour, t->zIndex, t->angle, p, world_id);
        }
    }
    if(new_rigidbody_pixels->size != 1) {
        destroy_entity(p, id);
    }
    free(grid_coords);
    free(grid_pixels);
    for(int i = 0; i < new_rigidbody_pixels->size; i++) {
        free_list(get_value(new_rigidbody_pixels, list *, i));
    }
    free_list(new_rigidbody_pixels);
}
