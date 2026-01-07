#include "../include/rigidbody.h"
#include "../include/queue.h"
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

//Crappy augmented flood-fill algorithm implementation taken from here: https://www.geeksforgeeks.org/flood-fill-algorithm/
list *bfs(int index, rigidbody *rb, int* visitedTracker, world_grid *grid) {
    list *indecies = list_alloc(rb->pixel_count, sizeof(int));
    queue *q = queue_alloc(rb->pixel_count, sizeof(int));

    push_value(indecies, int, index);
    enqueue(q, int, index);
    //Need to use the visited tracker otherwise the program doesn't know if we have visited a pixel or not
    //so it keeps looping infinitely
    visitedTracker[index] = 1;

    while (q->size > 0) {
        int currentIndex;
        deqeue(q, int, currentIndex);
        // if (!isAtTopEdge(currentIndex, arrayWidth) && pixels[currentIndex - arrayWidth] != NO_PIXEL_COLOUR && visitedTracker[currentIndex - arrayWidth] == 0) {
        //     indexes.push_back(currentIndex - arrayWidth);
        //     q.push(currentIndex - arrayWidth);
        //     visitedTracker[currentIndex - arrayWidth] = 1;
        // }
        // if (!isAtLeftEdge(currentIndex, arrayWidth) && pixels[currentIndex - 1] != NO_PIXEL_COLOUR && visitedTracker[currentIndex - 1] == 0) {
        //     indexes.push_back(currentIndex - 1);
        //     q.push(currentIndex - 1);
        //     visitedTracker[currentIndex - 1] = 1;
        // }
        // if (!isAtBottomEdge(currentIndex, arrayWidth, arrayLength) && pixels[currentIndex + arrayWidth] != NO_PIXEL_COLOUR && visitedTracker[currentIndex + arrayWidth] == 0) {
        //     indexes.push_back(currentIndex + arrayWidth);
        //     q.push(currentIndex + arrayWidth);
        //     visitedTracker[currentIndex + arrayWidth] = 1;
        // }
        // if (!isAtRightEdge(currentIndex, arrayWidth) && pixels[currentIndex + 1] != NO_PIXEL_COLOUR && visitedTracker[currentIndex + 1] == 0) {
        //     indexes.push_back(currentIndex + 1);
        //     q.push(currentIndex + 1);
        //     visitedTracker[currentIndex + 1] = 1;
        // }
    }

    std::sort(indexes.begin(), indexes.end());

    return indecies;
}

std::pair<Sprite, Transform> constructNewPixelBuffer(std::vector<int> indexes, Uint32* pixels, int arrayWidth, Sprite& s, Transform& t, SDL_Renderer* gRenderer) {
    Uint32* newPixelBuffer;
    Sprite newSprite;
    int width = 0;
    int height = (int)(indexes.back() / arrayWidth) - (int)(indexes.front() / arrayWidth) + 1; //Why is the height including the pixel buffer??? Surely that shouldn't come up

    int startLinePos = indexes[0] % arrayWidth;
    int endLinePos = indexes[0] % arrayWidth;

    for (int i = 1; i < indexes.size() - 1; i++) {
        //THE SMALLEST startLinePos AND BIGGEST endLinePos DO NOT HAVE TO BE ON THE SAME ROW
        //if the pixel ahead of the current one is on the same row but the one behind is on a different row 
        //we have a startrow. But we only want to update the value if it's % is smaller than the current one
        //as this indicates it is further to the left.
        if (startLinePos > indexes[i] % arrayWidth) {
            startLinePos = indexes[i] % arrayWidth;
        }
        //If the pixel behind the current one is on the same row but the one ahead is on a new row, 
        //we have an endrow. But we only want to update the value if its % is bigger than the current one 
        //as this indicates it is further to the right.
        if (endLinePos < indexes[i] % arrayWidth) {
            endLinePos = indexes[i] % arrayWidth;
        }
    }
    width = endLinePos - startLinePos;
    width = width + 1;

    //Essentially, in order for marching squares to work, there has to be a one-pixel wide colourless perimeter around
    //the texture. This is so that there is an actual "border" for marching squares to trace around, and I think this is 
    //the least code and computationally expenseive method of implementing this, as the other way would be to have
    //marching squares "imagine" such a border, which would require a lot of checking of assumptions and pixel positions
    //ie, more code and work.
    //To implement this, we need to increase height and width by 2 (1 pixel for each side). We can increase height 
    //temporarily when we call it, but we need to increase width permanently, as otherwise it gets messed up in 
    //multiplication calls.
    width += 2;
    height += 2;

    //Creating the pixel buffer for the new texture
    newPixelBuffer = new Uint32[(width) * (height)];
    //The memset here is actually making all the pixels have an alpha of 255 for some reason, even though noPixelColour has an alpha of 0.
    //memset(newPixelBuffer, noPixelColour, width * height * sizeof(Uint32));//Filling it with transparent pixels
    //Using a for loop instead of memset fixes the alpha problem here.
    for (int i = 0; i < ((width) * (height)); i++) {
        newPixelBuffer[i] = NO_PIXEL_COLOUR;
    }

    //Populating the new pixel buffer with data
    //startLinePos acts as an offset to figure out how far left the texture has moved. 
    //Current heigh measures the current row of the new pixel buffer we are on
    int currentHeight = 1; //Since the first row will be the blank pixel perimeter.
    for (int i = 0; i < indexes.size(); i++) {
        //If the pixels are on different rows, increment the current height by their difference.
        if (i != 0 && (floor(indexes[i] / arrayWidth) > floor(indexes[i - 1] / arrayWidth))) {
            currentHeight += floor(indexes[i] / arrayWidth) - floor(indexes[i - 1] / arrayWidth);
        }
        //Add 1 here as an offset to the LHS perimeter. The RHS and BHS perimeters will be automatically accounted for
        //as the code will never reach them, so no need to worry about that.
        newPixelBuffer[(currentHeight * (width)) + ((indexes[i] % arrayWidth) - startLinePos) + 1] = pixels[indexes[i]];
    }

    //Edge case of 1x1 textures. Need to do newPixelBuffer[5], since the fifth pixel would be the middle one in a 3x3 grid.
    if (indexes.size() == 1) {
        newPixelBuffer[5] = pixels[indexes[0]];
    }

    //This works now. Cieling it gives the correct value. However I now get a memory error somewhere
    float originX = ceilf(getOrigin(s, t).x + (startLinePos)) - 1.0f; //Assume original textures also have transparent border
    float originY = ceilf(getOrigin(s, t).y + ((int)floor(indexes[0] / arrayWidth))) - 1.0f;

    //Have to manually calculate the center from the origin here.
    float centreX = originX + floorf((width) / 2.0f); //floor instead of ceil because 0-indexed
    float centreY = originY + floorf((height) / 2.0f);

    //Set this as a pointer as otherwise this variable will be destroyed once this method finishes.
    newSprite = createSprite(width, height, newPixelBuffer, gRenderer);
    Transform newTransform = Transform((Vector2){centreX, centreY}, t.rotation);

    cleanup(pixels, indexes);
    return std::make_pair(newSprite, newTransform);

    //delete[] newPixelBuffer; //This needs to be commented out since we are actively using newPixelBuffer to create our texture.
}

std::vector<std::pair<Sprite, Transform>> splitTextureAtEdge(Sprite& s, Transform& t, SDL_Renderer* gRenderer) {
    if (!s.needsSplitting) return {};
    int width = s.surfacePixels->w;
    int height = s.surfacePixels->h;

    //Get the texture pixels
    Uint32* pixels = getPixels32(s); //This has the correct alpha values for the pixels (checked)
    //A placement int that gets the length of the pixel 1D array
    int arrayLength = width * height;
    //A bitmap that remembers if we visited a pixel before or not.
    int* visitedTracker = new int[arrayLength];
    //Initialising visitedTracker to all 0.
    memset(visitedTracker, 0, arrayLength * sizeof(int));
    //Pixel buffer vector
    std::vector<int> possibleStarts;
    //Vector for all the new textures that are being formed. This method will return them
    std::vector<std::pair<Sprite, Transform>> retArr;

    //For loop to get all the split texture parts.
    for (int i = 0; i < arrayLength; i++) {
        if (pixels[i] != NO_PIXEL_COLOUR) {
            possibleStarts = bfs(i, width, arrayLength, pixels, visitedTracker);
            if (!possibleStarts.empty()) {
                retArr.push_back(constructNewPixelBuffer(possibleStarts, pixels, width, s, t, gRenderer));
            }
        }
    }

    delete[] visitedTracker;
    return retArr;
}
