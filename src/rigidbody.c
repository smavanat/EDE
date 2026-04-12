#include "../include/rigidbody.h"
#include "../include/queue.h"
#include <float.h>
#include <limits.h>
#include <math.h>
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

/**
 * Creates a brand new rigidbody component and returns a reference to it
 * @param id the id of its parent entity
 * @param width the width of the rigidbody
 * @param height the height of the rigidbody
 * @param colour the colour of its pixels (currently assuming all pixels in a rigidbody have uniform colour, can change this later)
 * @param centre its centre in worldspace
 * @param grid a pointer to the world grid where this rigidbody is located
 * @return a pointer to the created rigidbody
 */
rigidbody *create_rigidbody(uint32_t id, uint16_t width, uint16_t height, uint8_t colour[4], vector2 centre, world_grid *grid) {
    //Creating the rigidbody and setting some variables
    rigidbody *rb = malloc(sizeof(rigidbody));
    rb->height = height;
    rb->width = width;
    rb->pixel_count = width * height;
    memcpy(rb->colour, colour, sizeof(rb->colour));
    rb->pixel_coords = malloc(sizeof(ivector2) * rb->pixel_count);
    rb->mask = calloc(width * height, sizeof(uint8_t)); //Initialising the pixel mask to be zeroed out

    //Compute half-lengths
    float half_width = (width-1)/2.0f;
    float half_height = (height-1)/2.0f;

    //Compute top-left corner origin
    int top_left_x = (int)floorf(centre.x - half_width);
    int top_left_y = (int)floorf(centre.y - half_height);

    //Iterating over the area of the rigidbody
    for(int y = 0; y < height; y++) {
        for(int x = 0; x < width; x++) {
            memcpy(grid->pixels[((top_left_y + y)*grid->width) + top_left_x + x], colour, sizeof(pixel)); //Setting the grid pixels at these positions to be the colour assigned to the rigidbody
            grid->parents[((top_left_y + y)*grid->width) + top_left_x + x] = id; //Setting the pixel parent id to be this rigidbody's entity parent id
            rb->pixel_coords[(y*width) + x ] = (vector2){x - half_width, y - half_height}; //Setting the relative pixel coords inside the rigidbody
            rb->mask[(y*width) + x] = 1;
        }
    }

    return rb;
}
/**
 * Creates a rigidbody from a set of pixel data rather than just width and height, mostly used for split or non-rectangular rigidbodies
 * @param id the id of its parent entity
 * @param width the width of the rigidbody
 * @param height the height of the rigidbody
 * @param colour the colour of its pixels (currently assuming all pixels in a rigidbody have uniform colour, can change this later)
 * @param centre its centre in worldspace
 * @param pixel_coords a list containing the grid coordinates of the pixels that make up the rigidbody
 * @param grid a pointer to the world grid where this rigidbody is located
 * @return a pointer to the created rigidbody
 */
rigidbody *icreate_rigidbody_from_pixels(uint32_t id, uint16_t width, uint16_t height, uint8_t colour[4], vector2 centre, list *pixel_coords, world_grid *grid) {
    //Creating the rigidbody and setting some variables
    rigidbody *rb = malloc(sizeof(rigidbody));
    rb->height = height;
    rb->width = width;
    rb->pixel_count = pixel_coords->size;
    memcpy(rb->colour, colour, sizeof(rb->colour));
    rb->pixel_coords = malloc(sizeof(vector2) * rb->pixel_count);
    rb->mask = calloc(height * width, sizeof(uint8_t)); //Initialising the pixel mask to be zeroed out

    //Compute the center offset to make sure its in the center of the pixel and not on the side
    float off_x = centre.x - 0.5f;
    float off_y = centre.y - 0.5f;

    //Compute the half-lengths
    float half_width = (width-1)/2.0f;
    float half_height = (height-1)/2.0f;

    //Iterating over the pixels that make up the rigidbody
    for(int i = 0; i < rb->pixel_count; i++) {
        ivector2 coord = get_value(pixel_coords, ivector2, i);
        float local_x = coord.x - off_x;
        float local_y = coord.y - off_y;
        rb->pixel_coords[i] = (vector2){local_x, local_y};

        grid->parents[(coord.y * grid->width) + coord.x] = id; //Setting this pixel to have this rigidbody as its grid parent
        rb->mask[(int)floorf(local_y + half_height + 0.5) * width + (int)floorf(local_x +  half_width + 0.5)] = 1; //Setting the mask to be filled
    }

    return rb;
}

rigidbody *fcreate_rigidbody_from_pixels(uint32_t id, uint16_t width, uint16_t height, uint8_t colour[4], vector2 centre, list *pixel_coords, world_grid *grid) {
    //Creating the rigidbody and setting some variables
    rigidbody *rb = malloc(sizeof(rigidbody));
    rb->height = height;
    rb->width = width;
    rb->pixel_count = pixel_coords->size;
    memcpy(rb->colour, colour, sizeof(rb->colour));
    rb->pixel_coords = malloc(sizeof(vector2) * rb->pixel_count);
    rb->mask = calloc(height * width, sizeof(uint8_t)); //Initialising the pixel mask to be zeroed out

    // //Compute the center offset to make sure its in the center of the pixel and not on the side
    // float off_x = centre.x;
    // float off_y = centre.y;

    //Compute the half-lengths
    float half_width = (width-1)/2.0f;
    float half_height = (height-1)/2.0f;

    // printf("Rigidbody Pixel Coords:\n");
    //Iterating over the pixels that make up the rigidbody
    for(int i = 0; i < rb->pixel_count; i++) {
        vector2 coord = get_value(pixel_coords, vector2, i);
        // printf("World Coord: (%f, %f) ", coord.x, coord.y);
        float local_x = coord.x - centre.x;
        float local_y = coord.y - centre.y;
        // printf("Local Coord: (%f, %f) ", local_x, local_y);
        rb->pixel_coords[i] = (vector2){local_x, local_y};

        // printf("Mask Coord: (%i, %i)\n", (int)floorf(local_x +  half_width + 0.5f), (int)floorf(local_y + half_height + 0.5f));
        rb->mask[(int)floorf(local_y + half_height + 0.5) * width + (int)floorf(local_x +  half_width + 0.5)] = 1; //Setting the mask to be filled
    }

    return rb;
}

/**
 * MARCHING SQUARES IMPLEMENTATION
 *
 *Marching squares: First we need to get the starting pixel. This is just done by iterating over the array until
 *                  we find a non-transparent pixel
 *                  Then we need to find the square value of it and the four pixels surrounding it
 *                  Then based on that square value (and in the special saddle cases also on the previous square value)
 *                  we choose a new direction to move the "analysis" and add the currently analysed pixel to a vector;
 *                  Good source code and ideas from here: https://emanueleferonato.com/2013/03/01/using-marching-squares-algorithm-to-trace-the-contour-of-an-image/
 *                  And here: https://barradeau.com/blog/?p=391
 */

/**
 * Determines what marching squares state we are currently in based on a 2x2 pixel grid
 * Only performs marching squares on pixels that belong to the same rigidbody
 * @param start_coord the grid coordinate of the top left pixel in the pixel grid
 * @param grid the world_grid we are working on currently
 * @param id the id of the entity the rigidbody belongs to. Used to determine if pixels belong to the same rigidbody
 * @return the value associated with the current marching squares state
 */
int get_current_square_old(ivector2 start_coord, world_grid *grid, int32_t id) {
    int result = 0; //Accumulator of the current marching square state. Each pixel in the grid corresponds to a power of 2
    int x = start_coord.x;
    int y = start_coord.y;

    #define PARENT(x, y) grid->parents[(y)*grid->width + (x)]

    // Top-left pixel
    if (x >= 0 && x < grid->width && y >= 0 && y < grid->height && PARENT(x, y)== id) result += 1;

    // Top-right pixel
    if (x + 1 >= 0 && x + 1 < grid->width && y >= 0 && y < grid->height && PARENT(x+1, y)== id) result += 2;

    // Bottom-left pixel
    if (x >= 0 && x < grid->width && y + 1 >= 0 && y + 1 < grid->height && PARENT(x, y+1) == id) result += 4;

    // Bottom-right pixel
    if (x + 1 >= 0 && x + 1 < grid->width && y + 1 >= 0 && y + 1 < grid->height && PARENT(x+1, y+1) == id) result += 8;

    return result;
}
int sample_mask(rigidbody *rb, int x, int y) {
    if (x < 0 || x >= rb->width || y < 0 || y >= rb->height)
        return 0;  // Outside grid counts as empty
    return rb->mask[y * rb->width + x] ? 1 : 0;
}

int get_current_square(ivector2 start_coord, rigidbody *rb) {
    int result = 0; //Accumulator of the current marching square state. Each pixel in the grid corresponds to a power of 2
    int x = start_coord.x;
    int y = start_coord.y;

    // result |= sample_mask(rb, x,     y)     << 0; // top-left
    // result |= sample_mask(rb, x + 1, y)     << 1; // top-right
    // result |= sample_mask(rb, x,     y + 1) << 2; // bottom-left
    // result |= sample_mask(rb, x + 1, y + 1) << 3; // bottom-right
    if(x >= 0 && x < rb->width && y >= 0 && y < rb->height && rb->mask[y * rb->width + x] != 0) result += 1;
    if(x+1 >= 0 && x+1 < rb->width && y >= 0 && y < rb->height && rb->mask[y * rb->width + x+1] != 0) result += 2;
    if(x >= 0 && x < rb->width && y+1 >= 0 && y+1 < rb->height && rb->mask[(y+1) * rb->width + x] != 0) result += 4;
    if(x+1 >= 0 && x+1 < rb->width && y+1 >= 0 && y+1 < rb->height && rb->mask[(y+1) * rb->width + x+1] != 0) result += 8;

    return result;
}

/**
 * Finds the top left corner of the shape we are performing marching squares on to act as the start position.
 * @param pixel_coords the coordinates of the pixels that make up the shape we are trying to find the outline of
 * @return the top-left coordinate in pixel_coords
 */
ivector2 get_start_point_old(list* pixel_coords) {
    ivector2 start = get_value(pixel_coords, ivector2, 0);

    for(int i = 1; i < pixel_coords->size; i++) {
        ivector2 coord = get_value(pixel_coords, ivector2, i);
        if (coord.y < start.y || (coord.y == start.y && coord.x < start.x)) {
            start = coord;
        }
    }
    return start;
}

ivector2 get_start_point(list* pixel_coords, rigidbody *rb) {
    ivector2 best = {0, 0};
    bool found = false;

    for (int i = 0; i < pixel_coords->size; i++) {
        ivector2 p = get_value(pixel_coords, ivector2, i);

        // Skip non-boundary pixels
        if (!sample_mask(rb, p.x, p.y)) continue;

        bool is_boundary =
            !sample_mask(rb, p.x - 1, p.y) ||
            !sample_mask(rb, p.x + 1, p.y) ||
            !sample_mask(rb, p.x, p.y - 1) ||
            !sample_mask(rb, p.x, p.y + 1);

        if (!is_boundary) continue;

        if (!found ||
            p.y < best.y ||
            (p.y == best.y && p.x < best.x)) {
            best = p;
            found = true;
        }
    }

    return best;
}

/**
 * Implementation of marching squares. Performs marching squares in a conter-clockwise fashion to find the outline of a given rigidbody
 * @param pixel_coords a pointer to a list of coordinates representing the rigidbody shape we are performing marching squares on
 * @param grid a pointer to the world_grid the rigidbody is contained in
 * @param id the id of the entity the rigidbody is a part of
 * @return a pointer to a list of ivector2 coordinates which are the outline of the rigidbody
 */
list *marching_squares_old(list *pixel_coords, world_grid *grid, int32_t id) {
    ivector2 start_point = get_start_point_old(pixel_coords); //Getting the start position of marching squares
    // printf("Marching Sqaures start point: (%i, %i)\n", start_point.x, start_point.y);
    list *contour_points = list_alloc(pixel_coords->size, sizeof(ivector2)); //Return value
    //If the texture is filled on the LHS, we will end up with 15 as our first currentSquare.
    //To avoid this, we simply offset startPoint one to the left, to get 12 as our currentSquare,
    //and then marching squares handles the rest.
    if (get_current_square_old(start_point, grid, id) == 15) {
        if(start_point.x > 0) start_point.x -= 1;
        else if( start_point.y > 0) start_point.y -= 1;
        else fprintf(stderr, "Congratulations, you've somehow managed to make a rigidbody in the top-left corner of the world grid. I don't know how to deal with this, so it's broken for now\n");
    }
    // printf("Start after modification (%i, %i)\n", start_point.x, start_point.y);

    //Which direction we are moving in the current and previous iterations of marching squares
    int stepX = 0, stepY = 0;
    int prevX = 0, prevY = 0;
    ivector2 current_point = start_point;
    bool closed_loop = false; //Bool to see if we have reached the start again

    while (!closed_loop) {
        int current_square = get_current_square_old(current_point, grid, id);

        // Movement lookup based on currentSquare value
        switch (current_square) {
        case 1: case 5: case 13:
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

        //Moving the current point
        current_point.x += stepX;
        current_point.y += stepY;

        // Boundary checks. Should not happen but here just in case.
        if(current_point.x < -1 || current_point.x > grid->width || current_point.y < -1 || current_point.y > grid->height) {

            printf("Out-of-bounds detected at index: (%i, %i)\n", current_point.x, current_point.y);
            return contour_points;
        }

        //Adding the current outline point to the returned list and updating the previous values for the next iteration
        push_value(contour_points, ivector2, current_point);
        prevX = stepX;
        prevY = stepY;

        if (current_point.x == start_point.x && current_point.y == start_point.y) closed_loop = true; //Exit the loop once we have looped around the shape
    }

    return contour_points;
}

list *marching_squares(list *pixel_coords, rigidbody *rb) {
    ivector2 start_point = get_start_point_old(pixel_coords); //Getting the start position of marching squares
    //Adjust the starting point so it moves along the edge of the image
    start_point.x -= 1;
    start_point.y -= 1;
    printf("MS start point: (%i, %i)\n", start_point.x, start_point.y);
    list *contour_points = list_alloc(pixel_coords->size, sizeof(ivector2)); //Return value

    //Which direction we are moving in the current and previous iterations of marching squares
    int stepX = 0, stepY = 0;
    int prevX = 0, prevY = 0;
    ivector2 current_point = start_point;
    bool closed_loop = false; //Bool to see if we have reached the start again

    while (!closed_loop) {
        int current_square = get_current_square(current_point, rb);

        // Movement lookup based on currentSquare value
        switch (current_square) {
        case 1: case 5: case 13:
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

        //Moving the current point
        current_point.x += stepX;
        current_point.y += stepY;

        // Boundary checks. Should not happen but here just in case.
        if(current_point.x < -1 || current_point.x > rb->width || current_point.y < -1 || current_point.y > rb->height) {

            printf("Out-of-bounds detected at index: (%i, %i)\n", current_point.x, current_point.y);
            return contour_points;
        }

        //Adding the current outline point to the returned list and updating the previous values for the next iteration
        push_value(contour_points, ivector2, current_point);
        prevX = stepX;
        prevY = stepY;

        if (current_point.x == start_point.x && current_point.y == start_point.y) closed_loop = true; //Exit the loop once we have looped around the shape
    }

    return contour_points;
}

/**
 * RAMER-DOUGLAS-PEUCKER IMPLEMENTATION
 * Adapted from: https://editor.p5js.org/codingtrain/sketches/SQjSugKn6
 * RDP simplifies a line by drawing by recursively splitting the line into halves, drawing a line between the start and the end, and culling points that are greater than some distance
 * epsilon from this line
 */

/**
 * Determines the distance from a point to a line. The code was adapted from this formula: https://en.wikipedia.org/wiki/Distance_from_a_point_to_a_line#Line_defined_by_two_points
 * @param point the point whose distance we want to determine
 * @param start_point the start point of the line
 * @param end_point the end point of the line
 * @return the distance between the point point and the line represented by start_point and end_point as a float
 */
float lineDist(vector2 point, vector2 start_point, vector2 end_point) {
    return fabs(((end_point.y - start_point.y) * point.x) -
        ((end_point.x - start_point.x) * point.y) +
        (end_point.x * start_point.y) - (end_point.y * start_point.x)) /
        sqrt(pow((end_point.y - start_point.y), 2) + pow((end_point.x - start_point.x), 2));
}

/**
 * Returns the furthest point between indices a and b in all_points whose distance from the line between the points at indecies a and b is greater than epsilon
 * Version of the function for floating point vector2
 * @param all_points the list of points that we are performing RDP on
 * @param a the start index of the range that we are working over. Also represents one end of the line we are trying to find the furthest point from
 * @param b the end index of the range we are working over. Also represents one end of the line we are trying to find the furthest point from
 * @param epsilon the culling value. Only consider points whose distance from the line a -> b is greater than epsilon
 * @return the index of the furthest point whose distance from a -> b is greater than epsilon if one exists, or -1 otherwise
 */
int ffindFurthest(list* all_points, int a, int b, float epsilon) {
    float recordDistance = -1; //Holds the longest distance a point has acheived from a -> b
    int furthestIndex = -1; //Holds the index of the furthest point from a -> b

    //Get start and end points of line
    vector2 start = get_value(all_points, vector2, a);
    vector2 end = get_value(all_points, vector2, b);

    //Iterate over all points in range (a, b) in all_points
    for (int i = a + 1; i < b; i++) {
        float d = lineDist(get_value(all_points, vector2, i), start, end); //Find the distance of the point from a -> b
        if (d > recordDistance + 1e-6f) { //If the current point is further away than the current furtherest, set it to be the furthest
            recordDistance = d;
            furthestIndex = i;
        }
    }
    if (recordDistance > epsilon + 1e-6f) return furthestIndex; //Only return the index if its greater than epsilon
    else return -1;
}

/**
 * Recursive implementation of the Ramer-Douglas-Peucker algorithm. Version of the algorithm for floating point vector2 point inputs
 * @param start_index the index of the start of the portion of the line this iteration of RDP will be working on
 * @param end_index the index of the end of the portion of the line this iteration of RDP will be working on
 * @param epsilon the culling value. Only consider points whose distance from the line portion is greater than epsilon
 * @param all_points a pointer to a list containing the points that make up the entire line we are working over
 * @param rdp_points a pointer to a list to which all points that are kept post-rdp are added
 */
void frdp(int start_index, int end_index, float epsilon, list *all_points, list* rdp_points) {
    int next_index = ffindFurthest(all_points, start_index, end_index, epsilon);
    if (next_index > 0) {
        if (start_index != next_index) {
            frdp(start_index, next_index, epsilon, all_points, rdp_points);
        }
        push_value(rdp_points, vector2, get_value(all_points, vector2, next_index));
        if (end_index != next_index) {
            frdp(next_index, end_index, epsilon, all_points, rdp_points);
        }
    }
}

/**
 * Determines the cross product of the vectors (b-a) and (c-a)
 * @param a the start point of the cross product area
 * @param b an end point of the cross product area
 * @param c an end point of the cross product area
 * @return (b-a) x (c-a)
 */
long long cross(ivector2 a, ivector2 b, ivector2 c) {
    long long abx = (long long)b.x - a.x;
    long long aby = (long long)b.y - a.y;
    long long acx = (long long)c.x - a.x;
    long long acy = (long long)c.y - a.y;

    return abx * acy - aby * acx;
}

/**
 * Returns the furthest point between indices a and b in all_points whose distance from the line between the points at indecies a and b is greater than epsilon
 * Version of the function for integer ivector2
 * @param all_points the list of points that we are performing RDP on
 * @param a the start index of the range that we are working over. Also represents one end of the line we are trying to find the furthest point from
 * @param b the end index of the range we are working over. Also represents one end of the line we are trying to find the furthest point from
 * @param epsilon the culling value. Only consider points whose distance from the line a -> b is greater than epsilon
 * @return the index of the furthest point whose distance from a -> b is greater than epsilon if one exists, or -1 otherwise
 */
int ifind_furthest(list *all_points, int a, int b, float epsilon) {
    ivector2 start = get_value(all_points, ivector2, a); //Getting vector at index a
    ivector2 end = get_value(all_points, ivector2, b); //Getting the vector at index b

    //Computing the distance squared between the start and end
    long long dx = (long long)end.x - start.x;
    long long dy = (long long)end.y - start.y;
    long long len_sq = dx*dx + dy*dy;

    //Compute square of threshold. Use squares since this removes the floating point weirdness in an integer context that would come from using sqrt
    long long threshold = (long long)(epsilon * epsilon * (float)len_sq);
    // long long threshold = (long long)(epsilon * epsilon * len_sq);
    // long long threshold = (long long)(epsilon * epsilon * len_sq * len_sq);

    long long max_dist_sq = 0; //Square of the distance of the furthest point we have currently found
    int furthest = -1; //Index of the furthest point we have currently found
    for(int i = a + 1; i < b; i++) {
        ivector2 p = get_value(all_points, ivector2, i);
        long long area = cross(start, end, p);
        long long dist_sq = area * area; //Getting the distance
        // long long dist_sq = (area * area) / dist_sq;

        if(dist_sq > max_dist_sq) { //Finding the new maximum
            max_dist_sq = dist_sq;
            furthest = i;
        }
    }

    if(max_dist_sq > threshold) return furthest; //Returning if we are above the threshold
    return -1; //Otherwise return sentinel value
}

/**
 * Recursive implementation of the Ramer-Douglas-Peucker algorithm. Version of the algorithm for integer ivector2 point inputs
 * @param start_index the index of the start of the portion of the line this iteration of RDP will be working on
 * @param end_index the index of the end of the portion of the line this iteration of RDP will be working on
 * @param epsilon the culling value. Only consider points whose distance from the line portion is greater than epsilon
 * @param all_points a pointer to a list containing the points that make up the entire line we are working over
 * @param rdp_points a pointer to a list to which all points that are kept post-rdp are added
 */
void irdp(int start_index, int end_index, float epsilon, list *all_points, list *rdp_points) {
    int next_index = ifind_furthest(all_points, start_index, end_index, epsilon);
    if(next_index == -1) { //Base case
        push_value(rdp_points, ivector2, get_value(all_points, ivector2, start_index));
        return;
    }
    irdp(start_index, next_index, epsilon, all_points, rdp_points);
    irdp(next_index, end_index, epsilon, all_points, rdp_points);
}

/**
 * CODE FOR MAKING NEW RIGIDBODIES AFTER ERASURE
 * This section is the functions that deal with 'dirty' rigidbodies (those that have had at least one pixel erased)
 * This is done by running flood-fill to determine the regions of pixels in the remaining rigidbody structure
 * For each region found, we run marching squares and rdp to determine and simplify the outline of the region (implemented above)
 * Then we delete the old entity and its components, and make a new entity, transform, rigidbody, and collider for each region
 */

/**
 * Augmented flood-fill algorithm implementation taken from here: https://www.geeksforgeeks.org/flood-fill-algorithm/
 * Flood-fill is essentially bfs run over pixels, but it also modifies the pixels it runs over, in this case setting the parent rigidbody id to -1 (no parent)
 * @param start the start coordinate for flood-fill
 * @param grid_coords a pointer to an array of pixels representing the grid flood-fill is operating over
 * @param size the max size that the returned region could be
 * @param width the width of the pixel grid we are working with
 * @param height the height of the pixel grid we are working with
 * @param id the id of the parent entity of the rigidbody
 * @return a list of ivector2 representing the coordinates of a region of pixels whose parent_body matches id
 */
list *flood_fill(ivector2 start, uint8_t *rb_layout, int width, int height) {
    //Initialising variables
    list *region_coords = list_alloc(width * height, sizeof(ivector2)); //Returning coordinates of filled points in a region of the rigidbody pixel mask
    queue *q = queue_alloc(width * height, sizeof(ivector2)); //Queue of values to be visited in bfs
    bool ok;
 
    //Pushing the start position to the returning list and the bfs queue
    push_value(region_coords, ivector2, start);
    enqueue(q, ivector2, start);
    rb_layout[(start.y * width) + start.x] = 0; //Setting the parent_body to be -1 to ensure this coordinate is not visited again

    //BFS implementation
    while (q->size > 0) {
        //Get the next element to visit
        ivector2 current_pixel;
        dequeue(q, ivector2, current_pixel, ok);
        if(!ok) break;
        // printf("Current pixel coords: (%i, %i) ", current_pixel.x, current_pixel.y);
        // printf("Index: %i ", ((current_pixel.y) * width) + current_pixel.x);
        // printf("Parent Body: %i\n", grid_coords[((current_pixel.y - 1) * width) + current_pixel.x].parent_body);

        //Visit the top, down, left and right neighbours
        if(current_pixel.y > 0 && rb_layout[((current_pixel.y - 1) * width) + current_pixel.x]) { //Check that the top neighbour is still in the grid and has the correct parent_body
            //Add the neighbour to the return list and the bfs queue
            ivector2 new_pos = (ivector2){current_pixel.x, current_pixel.y-1};
            push_value(region_coords, ivector2, new_pos);
            enqueue(q, ivector2, new_pos);
            rb_layout[((current_pixel.y - 1) * width) + current_pixel.x] = 0; //Set its parent_body to -1 so never visited again
        }
        if(current_pixel.x > 0 && rb_layout[((current_pixel.y) * width) + current_pixel.x - 1]) { //Check that the right neighbour is still in the grid and has the correct parent_body
            //Add the neighbour to the return list and the bfs queue
            ivector2 new_pos = (ivector2){current_pixel.x-1, current_pixel.y};
            push_value(region_coords, ivector2, new_pos);
            enqueue(q, ivector2, new_pos);
            rb_layout[((current_pixel.y) * width) + current_pixel.x-1] = 0; //Set its parent_body to -1 so never visited again
        }
        if(current_pixel.y < height - 1 && rb_layout[((current_pixel.y + 1) * width) + current_pixel.x]) { //Check that the bottom neighbour is still in the grid and has the correct parent_body
            //Add the neighbour to the return list and the bfs queue
            ivector2 new_pos = (ivector2){current_pixel.x, current_pixel.y+1};
            push_value(region_coords, ivector2, new_pos);
            enqueue(q, ivector2, new_pos);
            rb_layout[((current_pixel.y + 1) * width) + current_pixel.x] = 0; //Set its parent_body to -1 so never visited again
        }
        if(current_pixel.x < width - 1 && rb_layout[((current_pixel.y) * width) + current_pixel.x+1]) { //Check that the left neighbour is still in the grid and has the correct parent_body
            //Add the neighbour to the return list and the bfs queue
            ivector2 new_pos = (ivector2){current_pixel.x+1, current_pixel.y};
            push_value(region_coords, ivector2, new_pos);
            enqueue(q, ivector2, new_pos);
            rb_layout[((current_pixel.y) * width) + current_pixel.x+1] = 0; //Set its parent_body to -1 so never visited again
        }
    }

    free_queue(q); //Cleanup
    return region_coords;
}

/**
 * Creates a new rigidbody and its associated transform and collider, assigning them all to a new entity, given a region of pixels
 * @param pixel_coords a pointer to a list of ivector2 containing the coordinates of the pixels making up the new region
 * @param grid a pointer to the world_grid we are working in
 * @param colour an array representing the colour of the rigidbody
 * @param zIndex the zIndex of the new transform generated
 * @param angle the angle of the new transform generated
 * @param p a pointer to the plaza managing entity creation
 * @param world_id the box2d world we are working with
 * @param type the b2BodyType of the collider we should be generating
 */
void construct_new_rigidbody(list *pixel_coords, world_grid *grid, rigidbody *old_rb, transform *old_t, plaza *p, b2WorldId world_id, b2BodyType type) {
    float minX = FLT_MAX, maxX = -FLT_MAX, minY = FLT_MAX, maxY = -FLT_MAX;
    float old_top_left_x = old_t->position.x - ((old_rb->width - 1) * 0.5);
    float old_top_left_y = old_t->position.y - ((old_rb->height - 1) * 0.5);
    list *world_coords = list_alloc(pixel_coords->size, sizeof(vector2));

    printf("Pixel Coords: \n");
    for(int i = 0; i < pixel_coords->size; i++) {
        ivector2 coord = get_value(pixel_coords, ivector2, i);
        vector2 world_coord = (vector2){old_top_left_x + coord.x, old_top_left_y + coord.y};
        push_value(world_coords, vector2, world_coord);

        if(world_coord.x < minX) minX = world_coord.x;
        if(world_coord.x > maxX) maxX = world_coord.x;
        if(world_coord.y < minY) minY = world_coord.y;
        if(world_coord.y > maxY) maxY = world_coord.y;
        printf("(%i, %i) ", coord.x, coord.y);
    }
    printf("\n");

    // printf("Old rigidbody mask:\n");
    // for(int i = 0; i < old_rb->height; i++) {
    //     for(int j = 0; j < old_rb->width; j++) {
    //         printf("%i ", old_rb->mask[i*old_rb->width + j]);
    //     }
    //     printf("\n");
    // }
    //
    // printf("MinX: %f, MaxX: %f, MinY: %f, MaxY: %f\n", minX, maxX, minY, maxY);
    //Calculating dimensions and centre
    int width = (int)roundf(maxX - minX + 1);
    int height = (int)roundf(maxY - minY + 1);
    float centreX = minX + ((width-1) * 0.5f);
    float centreY = minY + ((height-1) * 0.5f);
    // printf("Width: %i, Height: %i\n", width, height);
    // printf("Center: (%f, %f)\n", centreX, centreY);

    entity e = create_entity(p); //Creating the new entity

    //Creating the new transform -> position is the new centre, but keeps old zIndex and angle
    transform *t = create_transform((vector2){centreX, centreY}, old_t->zIndex, old_t->rotation);
    add_component_to_entity(p, e, TRANSFORM, t);

    //Creating the new rigidbody -> Need to copy over the pixel coords and calculate their relative position to the new centre
    rigidbody *rb = fcreate_rigidbody_from_pixels(e, width, height, old_rb->colour, t->position, world_coords, grid);
    add_component_to_entity(p, e, RIGIDBODY, rb);

    //Marching squares on the outline of the pixels
    list *ms_points = marching_squares(pixel_coords, old_rb);
    // printf("Marching Squares size: %lu\n", ms_points->size);
    printf("Marching Squares Points:\n");
    for(int i = 0; i < ms_points->size; i++) {
        printf("(%i, %i), ", get_value(ms_points, ivector2, i).x, get_value(ms_points, ivector2, i).y);
    }
    printf("\n");
    //Ramer-Douglas-Peucker to simplify the ms
    list *rdp_points = list_alloc(ms_points->size, sizeof(ivector2));
    irdp(0, ms_points->size-1, 0.6f, ms_points, rdp_points);

    //Sometimes it doesn't add the last point?
    ivector2 last_rdp = get_value(rdp_points, ivector2, rdp_points->size-1);
    ivector2 last_ms = get_value(ms_points, ivector2, ms_points->size-1);
    if(last_rdp.x != last_ms.x || last_rdp.y != last_ms.y) {
        push_value(rdp_points, ivector2, last_ms);
    }
    // push_value(rdp_points, ivector2, get_value(rdp_points, ivector2, 0)); //Need to add to get full circle of points //NOTE: Think don't need?
    printf("RDP Points:\n");
    for(int i = 0; i < rdp_points->size; i++) {
        printf("(%i, %i), ", get_value(rdp_points, ivector2, i).x, get_value(rdp_points, ivector2, i).y);
    }
    printf("\n");

    //Convert the points into vector2 from ivector2
    vector2 *points = malloc(sizeof(vector2) * rdp_points->size);
    for(int i = 0; i < rdp_points->size; i++) {
        points[i] = (vector2){get_value(rdp_points, ivector2, i).x, get_value(rdp_points, ivector2, i).y};
    }

    //Create the new polygon collider
    collider *c = malloc(sizeof(collider));
    c->type = POLYGON;
    c->collider_id = create_polygon_collider(points, rdp_points->size, t->position, t->rotation, world_id, type);
    add_component_to_entity(p, e, COLLIDER, c);
    printf("New entity's signature: %i\n", p->entitySignatures[e]);

    for(int i = 0; i < pixel_coords->size; i++) {
        ivector2 coord = get_value(pixel_coords, ivector2, i);
        old_rb->mask[(coord.y*old_rb->width) + (coord.x)] = 0;
    }
    free_list(world_coords);
    free_list(ms_points);
    free_list(rdp_points);
    free(points);
}

/**
 * Splits a 'dirty' (has erased pixels) rigidbody into new rigidbodies if necessary, or adjusts its collider if not
 * @param id the entity to which the rigidbody belongs
 * @param p a pointer to the plaza managing entities
 * @param grid a pointer to the world_grid where the rigidbody is located
 * @param world_id the id of the box2d world where the collider of the rigidbody is located
 */
void split_rigidbody(entity id, plaza *p, world_grid *grid, b2WorldId world_id) {
    //Getting relevant components from the entity
    transform *t = get_component_from_entity(p, id, TRANSFORM);
    printf("Old Center: (%f, %f)\n", t->position.x, t->position.y);
    rigidbody *rb = get_component_from_entity(p, id, RIGIDBODY);

    //List to hold all of the new regions that may have been formed when the rigidbody had some of its pixels erased
    list *new_rigidbody_regions= list_alloc(5, sizeof(list *));

    //Creating a temporary duplicate of the pixel data in rigidbody mask to use for flood-fill (so we don't overwrite existing data)
    uint8_t *rb_layout = malloc(rb->height * rb->width * sizeof(uint8_t));
    memcpy(rb_layout, rb->mask, rb->height * rb->width * sizeof(uint8_t));

    //Finding all of the seperate regions that are now contained in the old rigidbody after erasure
    for(int i = 0; i < rb->height * rb->width; i++) {
        if(rb_layout[i]) {
            ivector2 pos = (ivector2){i % rb->width, i / rb->width};
            list *region = flood_fill(pos, rb_layout, rb->width, rb->height);
            printf("Rigidbody region size: %lu\n", region->size);
            if(region->size > 0){ //Adding non-empty regions to the list to be processed
                push_value(new_rigidbody_regions, list *, region);
            }
        }
    }

    //printf("Num regions: %lu\n", new_rigidbody_regions->size);
    if(new_rigidbody_regions->size > 0) {
        collider *c = get_component_from_entity(p, id, COLLIDER);//Get the collider so we can get some of its data
        b2BodyType type = b2Body_GetType(c->collider_id);
        //Make new rigidbodies here
        for(int i = 0; i < new_rigidbody_regions->size; i++) {
            construct_new_rigidbody(get_value(new_rigidbody_regions, list *, i), grid, rb, t, p, world_id, type); //Make a new rigidbody with the given data
        }
    }

    destroy_entity(p, id); //Destroy the original entity and its child components
    printf("Number of entities left: %i\n", p->entities->count);

    //Cleanup
    free(rb_layout);
    for(int i = 0; i < new_rigidbody_regions->size; i++) {
        free_list(get_value(new_rigidbody_regions, list *, i));
    }
    free_list(new_rigidbody_regions);
}
