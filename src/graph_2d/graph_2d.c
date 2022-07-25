
#include "stdint.h"
#include "stddef.h"

#include "graph_2d.h"

/* Fill a given buffer with line.
    TODO: optimize for only using integers
*/
extern void set_line(struct pixel *p_buffer_data_start, struct scs_point point1, struct scs_point point2,
                     uint32_t surface_width, uint32_t surface_height
){
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


/*struct pixel {
    // little-endian ARGB
    uint8_t blue;
    uint8_t green;
    uint8_t red;
    uint8_t alpha;
};
*/



/* This is where our code gets dependent on little-endianness.
    WL_SHM_FORMAT_ARGB8888. 32-bit ARGB format, [31:0] A:R:G:B 8:8:8:8 little endian.
    In little-endian systems the compiler will generate the
    code in little endian. That is why we should not invert the colors
    and should write them 'as they are', in the format: 0xAARRGGBB
    (i.e hex format with alpha channel at first byte)
    Meaning, that 0xffff0000 is full red
    0xff00ff00 is full green
    0xff0000ff is full blue
    0xffdeed55 is yellowish
*/
extern void set_rect(struct pixel * const p_buffer_data_start, struct scs_point p,
                     uint32_t width, uint32_t height
){

    const uint32_t user_color = 0xff3d8069; // greenish
    uint64_t color = (uint64_t)user_color << 32 | user_color;


    // uint32_t is more than enough for 1920x1200 offset (i.e = 2304000px < UINT32_MAX)
    // so we can store it
    uint32_t offset = 1280 * p.scs_y;
    uint64_t *px = NULL;

    // NOTE: We fill up 2px at a time.
    // That is why we go from 0 to width/2 at the second 'for' loop.
    // This is faster but less accurate in the case when we are given odd width (x % 2 != 0).
    // We have 2 ways to implement the width loop (the second one):
    //  - Show 1 less pixel, i.e width/2
    //  - Or show 1 more pixel, i.e (width+1)/2
    // In our case, we show 1 less pixel.
    for(uint32_t y = 0; y < height; y++){
        px = (uint64_t*)(p_buffer_data_start + offset + p.scs_x);
        for(uint32_t x = 0; x < width/2; x++){ 
            *px = color;   
            px++;
        }
        offset += 1280;
    };

};



/* TODO: alternative function
    Short explanation: 
        - more precise
        - endian - independent
*/
/*extern void set_rect(struct pixel * const p_buffer_data_start, struct scs_point p,
                     uint32_t width, uint32_t height
){
    struct pixel *px = p_buffer_data_start + 1280 * p.scs_y +  p.scs_x;

    for(uint16_t y = p.scs_y; y < (p.scs_y + height); y++){
        for(uint16_t x = p.scs_x; x < p.scs_x + width; x++){
            px->alpha = 255;
            px->red = 0;
            px->green = 0;
            px->blue = 0;
            px++;
        };
        px = p_buffer_data_start + 1280 * y + p.scs_x;
    };

};
*/