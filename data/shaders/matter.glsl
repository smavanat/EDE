#version 430

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
