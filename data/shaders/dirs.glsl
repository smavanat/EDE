#define DOWN_LEFT  0
#define DOWN       1
#define DOWN_RIGHT 2
#define RIGHT      3
#define UP_RIGHT   4
#define UP         5
#define UP_LEFT    6
#define LEFT       7

const ivec2 OFFSETS[8] = ivec2[8] (
    ivec2(-1,1),ivec2(0,1),ivec2(1,1),ivec2(1,0),
    ivec2(1,-1),ivec2(0,-1),ivec2(-1,-1),ivec2(-1,0)
);
