#ifndef GRAPH_2D_H
#define GRAPH_2D_H

#include "stdint.h"

struct pixel {
    // little-endian ARGB
    uint8_t blue;
    uint8_t green;
    uint8_t red;
    uint8_t alpha;
};

/* Rectangle Coordinate System point */
struct rcs_point {
    // TODO: int16_t or int32_t ?
};

/* Screen Coordinate System point
    TODO: we can make them uint16_t
    and add some more members
*/
struct scs_point {
    uint32_t scs_x;
    uint32_t scs_y;
};


/* The slope-intercept form of a line is written as
	y = kx + b
	Where m is the "slope" or "rise over run" or "dy/dx"
*/
struct line_func_2d {
	uint32_t dx; // 
	uint32_t dy; // the "slope" is (dy/dx) = k
	// uint32_t x;
	// uint32_t y;
	// uint32_t b;
};

/* Fill a given BUFFER with line. */
extern void set_line(struct pixel *p_buffer_data_start, struct scs_point point1, struct scs_point point2,
                     uint32_t surface_width, uint32_t surface_height);

/* Think of how to make a function set_line, that "fills a given SURFACE with line."
    without including wl_surface_state_s struct to graph2d.h
*/



/* Fill a given buffer with rectanglt
    Cheatsheet: 
    int       *      mutable_pointer_to_mutable_int;
    int const *      mutable_pointer_to_constant_int;
    int       *const constant_pointer_to_mutable_int;
    int const *const constant_pointer_to_constant_int;
*/
extern void set_rect(struct pixel * const p_buffer_data_start, struct scs_point p,
                     uint32_t width, uint32_t height);


#endif