
#include "stdint.h"
#include "stddef.h"

#include "graph_2d.h"
#include "gui_core.h"




/* Fills a given surface with cursor. (cursorv renderer)
*/
extern void set_cursor(struct wl_surface_state_s * const p_cursor_surface
){
    // TODO
    // struct wl_cursor_v cursor = get_cursor_by_name(cursor_name);
    
    for (int y = 0; y < p_cursor_surface->height; y++){
        for (int x = 0; x < y + 2 ; x++){
            struct pixel *px = p_cursor_surface->current_buffer_state.p_buffer_data_start
                                + y * p_cursor_surface->width
                                + x;
            // ARGB little endian
            // green
            px->blue = 130;
            px->green = 150;
            px->red = 100;
            px->alpha = 255;
        };
    };
};





/* Fill a given surface with line.
    TODO: optimize for only using integers
*/
extern void set_line(struct wl_surface_state_s * const p_wl_surface_state, struct scs_point p1, struct scs_point p2                     
){
    struct line_func_2d func = {
        .dx = p2.scs_x - p1.scs_x,
        .dy = p2.scs_y - p1.scs_y,
        // .x = 0;
        // .y = 0;
        // .b = 2
    };

    // TODO:
    // if(func.x > surface_width || func.y > surface_height) return;

    struct pixel *px = NULL; // no matter
    if(func.dy <= func.dx){
        uint32_t y = p1.scs_y;
        for(uint32_t x = p1.scs_x; x < p2.scs_x; x++){

            if(func.dy * x - func.dx * (y+0.5) >= 0){ //  + func.b ??
                // the ideal line is below
                // so 'y' has advanced
                y++;
            } else{ /* the ideal line is above */}


            px = p_wl_surface_state->current_buffer_state.p_buffer_data_start
                    + y * p_wl_surface_state->width
                    + x;
            px->alpha = 255;
            px->red = 200;
            px->green = 60;
            px->blue = 60;
        };
    }
    else {
        uint32_t x = p1.scs_x;
        for(uint32_t y = p1.scs_y; y < p2.scs_y; y++){
            if(func.dy * (x+0.5) - func.dx * y >= 0){}
            else x++;

            px = p_wl_surface_state->current_buffer_state.p_buffer_data_start
                    + y * p_wl_surface_state->width
                    + x;
            px->alpha = 255;
            px->red = 200;
            px->green = 60;
            px->blue = 60;
        };
    };
};





/* Fills a given surface with rectangle.
    This is where our code gets dependent on little-endianness.
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
extern void set_rect(struct wl_surface_state_s * const p_wl_surface_state,
                        struct scs_point p,
                        const uint32_t width,
                        const uint32_t height,
                        const uint32_t color
){
    // doubled - color
    uint64_t dcolor = (uint64_t)color << 32 | color;


    // uint32_t is more than enough for 1920x1200 offset (i.e = 2304000px < UINT32_MAX)
    // so we can store it
    uint32_t offset = p_wl_surface_state->width * p.scs_y;
    uint64_t *px = NULL;

    // NOTE: We fill up 2px at a time.
    // That is why we go from 0 to width/2 at the second 'for' loop.
    // This is faster but less accurate in the case when we are given odd width (x % 2 != 0).
    // We have 2 ways to implement the width loop (the second one):
    //  - Show 1 less pixel, i.e width/2
    //  - Or show 1 more pixel, i.e (width+1)/2
    // In our case, we show 1 less pixel.
    for(uint32_t y = 0; y < height; y++){
        px = (uint64_t*)(p_wl_surface_state->current_buffer_state.p_buffer_data_start + offset + p.scs_x);
        for(uint32_t x = 0; x < width/2; x++){  // TODO: get rid of division in cycle
            *px = dcolor;   
            px++;
        }
        offset += p_wl_surface_state->width;
    };

};


/* Alternative function.
    Slower.
    Accurate with odd width.
    Endian - independent?
*/
extern void set_rect_a(struct wl_surface_state_s * const p_wl_surface_state,
                        struct scs_point p,
                        const uint32_t width,
                        const uint32_t height,
                        const uint32_t color
){
    struct pixel *px = p_wl_surface_state->current_buffer_state.p_buffer_data_start
                        + p_wl_surface_state->width * p.scs_y
                        + p.scs_x;

    uint8_t a = (color & 0xff000000) >> 24;
    uint8_t r = (color & 0x00ff0000) >> 16;
    uint8_t g = (color & 0x0000ff00) >> 8;
    uint8_t b = (color & 0x000000ff);

    for(uint16_t y = p.scs_y; y < (p.scs_y + height); y++){
        for(uint16_t x = p.scs_x; x < p.scs_x + width; x++){
            px->alpha = a;
            px->red = r;
            px->green = g;
            px->blue = b;
            px++;
        };
        px = p_wl_surface_state->current_buffer_state.p_buffer_data_start
                + p_wl_surface_state->width * y
                + p.scs_x;
    };

};
