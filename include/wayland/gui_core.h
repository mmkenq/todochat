#ifndef GUI_CORE_H
#define GUI_CORE_H

#include "stdint.h"

#include "graph_2d.h"
// #include "wayland-client.h" // we only use pointers to types like 'wl_buffer', 'wl_surface', etc.

#define WL_SURFACE_ROOT_MAX_WIDTH 1280 // px
#define WL_SURFACE_ROOT_MAX_HEIGHT 720 // px
#define WL_SURFACE_ROOT_MAX_STRIDE WL_SURFACE_ROOT_MAX_WIDTH * 4 // px
#define WL_SURFACE_ROOT_BUFFER_SIZE WL_SURFACE_ROOT_MAX_WIDTH * WL_SURFACE_ROOT_MAX_HEIGHT * 4 // bytes

#define WL_SURFACE_ROOT_MIN_WIDTH 500  // px
#define WL_SURFACE_ROOT_MIN_HEIGHT 500 // px

#define CURSOR_SURFACE_WIDTH 16   // px
#define CURSOR_SURFACE_HEIGHT 16 // px
#define CURSOR_SURFACE_STRIDE CURSOR_SURFACE_WIDTH * 4 // px
#define CURSOR_SURFACE_BUFFER_SIZE CURSOR_SURFACE_WIDTH * CURSOR_SURFACE_HEIGHT * 4 // bytes

// TODO: Find out why "+ 4" bytes at the end?? Why do we use 4 more bytes than we need?
#define WL_SHARED_MEMORY_POOL_SIZE (WL_SURFACE_ROOT_BUFFER_SIZE + CURSOR_SURFACE_BUFFER_SIZE) + 4 // bytes 


struct wl_buffer_state_s {
    struct wl_buffer *p_wl_buffer;
    struct pixel *p_buffer_data_start;

    // This value is used as a semaphore, and changes between 0 and 1.
    // In other words, it's a bool variable.
    // We could use C bitfields like so: char allowed_to_write: 1;
    // But we need speed, and managing C bitfields "under the hood" (or under the compiler)
    // is slower than just managing a struct member.
    char allowed_to_write;


    // TODO: think of a use.
    char padding[7];
};


struct wl_surface_state_s {
    struct wl_surface *p_wl_surface;
    struct wl_buffer_state_s current_buffer_state;

    uint32_t width;
    uint32_t height;
    // other_buffer_state, shared pool?
};



/* Fills a given SURFACE with cursor.
	TODO: rename to 'set_cursor()'
*/
extern void set_cursor(struct wl_surface_state_s * const p_cursor_surface);


/* Fills a given SURFACE with line.
	TODO: short explanation
*/
extern void set_line(struct wl_surface_state_s * const p_wl_surface_state, struct scs_point p1, struct scs_point p2);


/* Fills a given SURFACE with rectangle.
	NOTE: fills up 2px at a time.
    Faster.
    Less accurate with odd width.
    Depends on little-endian.
*/
extern void set_rect(struct wl_surface_state_s * const p_wl_surface_state,
						struct scs_point p,
						const uint32_t width,
						const uint32_t height,
						const uint32_t color);


/* Fills a given SURFACE with rectangle.
    Slower.
    Accurate with odd width.
    Endian - independent.
*/
extern void set_rect_a(struct wl_surface_state_s * const p_wl_surface_state,
                        struct scs_point p,
                        const uint32_t width,
                        const uint32_t height,
                        const uint32_t color);


#endif