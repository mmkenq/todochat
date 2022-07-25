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


/* Rectangle Coordinate System point
    TODO: explanation
*/
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



#endif