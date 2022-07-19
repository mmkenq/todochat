
#include "stdint.h"
#include "stddef.h"

#include "graph_2d.h"

/* Fill a given buffer with line.
    TODO: optimize for only using integers
*/
extern void set_line(struct pixel *p_buffer_data_start, struct scs_point point1, struct scs_point point2,
                     uint32_t surface_width, uint32_t surface_height){
    struct line_func_2d func = {
        .dx = point2.scs_x - point1.scs_x,
        .dy = point2.scs_y - point1.scs_y,
        // .x = 0;
        // .y = 0;
        // .b = 2
    };

    // TODO:
    // if(func.x > surface_width || func.y > surface_height) return;

    struct pixel *px = NULL; // no matter
    if(func.dy <= func.dx){
        uint32_t y = point1.scs_y;
        for(uint32_t x = point1.scs_x; x < point2.scs_x; x++){

            if(func.dy * x - func.dx * (y+0.5) >= 0){ //  + func.b ??
                // the ideal line is below
                // so 'y' has advanced
                y++;
            } else{ /* the ideal line is above */}


            px = p_buffer_data_start + y * surface_width + x;
            px->alpha = 255;
            px->red = 200;
            px->green = 60;
            px->blue = 60;
        };
    }
    else {
        uint32_t x = point1.scs_x;
        for(uint32_t y = point1.scs_y; y < point2.scs_y; y++){
            if(func.dy * (x+0.5) - func.dx * y >= 0){}
            else x++;

            px = p_buffer_data_start + y * surface_width + x;
            px->alpha = 255;
            px->red = 200;
            px->green = 60;
            px->blue = 60;
        };
    };
};