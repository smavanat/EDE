//Specialisation constants
layout (constant_id = 0) const int canvas_size_x = 1;
layout (constant_id = 1) const int canvas_size_y = 1;
layout (constant_id = 2) const int empty_matter = 1;
layout (local_size_x_id = 3, local_size_y_id = 4, local_size_z = 1);

//Buffers
layout (set = 0, binding = 0) restrict buffer MatterInBuffer {uint matter_in[]};
layout (set = 0, binding = 1) restrict writeonly buffer MatterOutBuffer {uint matter_out[]};
layout (set = 0, binding = 2, rgba8) restrict uinform writeonly image2D canvas_img;

uniform uint u_sim_step;
uniform uint u_move_step;

struct Matter {
    uint matter;
    uint colour;
};

Matter new_matter(uint matter) {
    Matter m;
    m.matter = (matter & uint(255));
    m.colour = matter >> uint(8);
    return m;
}

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

ivec2 cur_pos() {
    return ivec2(gl_GlobalInvocationID.xy);
}
int idx(ivec2 p) {
    return p.y * canvas_size_x + p.x;
}
bool inside(ivec2 p) {
    return p.x >= 0 && p.x < canvas_size_x && p.y >= 0 && p.y < canvas_size_y;
}
Matter read_matter(ivec2 p) {
    return new_matter(matter_in[idx(p)]);
}
void write_matter(ivec2 p, Matter m) {
    matter_out[idx(p)] = matter_to_uint(m);
}
Matter get_neighbor(ivec2 p, int d) {
    ivec2 np = p + OFFSETS[d];
    return inside(np) ? read_matter(np) : empty_matter();
}
bool is_empty(Matter m) {
    return m.matter == 0u;
}
bool is_gravity(Matter m) {
    return m.matter == 1u;
}
bool at_top(ivec2 p) {
    return p.y == 0;
}
bool at_bottom(ivec2 p) {
    return p.y == canvas_size_y-1;
}
bool at_left(ivec2 p) {
    return p.x == 0;
}
bool at_right(ivec2 p) {
    return p.x == canvas_size_x-1;
}
bool falls_on_empty(Matter f, Matter t) {
    return is_gravity(f) && is_empty(t);
}
bool slides_on_empty(Matter fd, Matter td, Matter fdn) {
  return is_gravity(fd) && !is_empty(fdn) && is_empty(td);
}
