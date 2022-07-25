#include "stdio.h"
#include "fcntl.h" // shm_open(), O_* constants
// #include "sys/stat.h" // shm_open(), mode constants
#include "unistd.h" 
#include "string.h"
#include "sys/mman.h" // mmap()

#include "wayland-client.h"
#include "xdg_shell.h"

#include "graph_2d.h"

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

// TODO: Find out why "+ 4" bytes at the end??
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


struct wl_pointer_state_s {
    struct wl_surface_state_s wl_surface_pointer_state;

    // Used for drawing on pointer events
    struct wl_surface_state_s *p_wl_surface_entered_state;

    // TODO: ?
    // int32_t cursor_offset_from_pointer_left_x;
    // int32_t cursor_offset_from_pointer_up_y;
    // const char *cursor_name;

    // TODO: ?
    // struct wl_cursor_v { // _v for "vector"
    // };

    // struct wl_cursor_i { // _i for "image"
    // };
}; 


struct wl_globals_state_s {
    struct wl_shm *p_wl_shm;
    struct wl_seat *p_wl_seat;
    struct wl_compositor *p_wl_compositor;
    struct xdg_wm_base *p_xdg_wm_base;
};

// TODO:
/*struct wl_shm_pools_state_s {
    struct wl_shm_pool *p_current_pool;
};*/


struct wl_state_s {
    struct wl_display *p_wl_display;    // Singleton
    struct wl_registry *p_wl_registry;  // The main oject through which we get current_globals_state

    struct wl_shm_pool *p_current_pool; // The only used pool yet
    struct wl_globals_state_s current_globals_state;
    struct wl_pointer_state_s current_pointer_state;

    struct wl_surface_state_s current_root_toplevel_state;
    // TODO: decide, if we need to create subsurface for debug info
};


/* cursorv RENDERER */
static void cursor_render(struct wl_pointer_state_s* p_pointer_state)
{
    // TODO
    // struct wl_cursor_v cursor = get_cursor_by_name(cursor_name);
    
    for (int y = 0; y < p_pointer_state->wl_surface_pointer_state.height; y++){
        for (int x = 0; x < y + 2 ; x++){
            struct pixel *px = p_pointer_state->wl_surface_pointer_state.current_buffer_state.p_buffer_data_start
                                + y * p_pointer_state->wl_surface_pointer_state.width
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


/* POINTER LISTENER */
static void pointer_surface_enter_handler(
    void *p_pointer_data, // app data recieved
    struct wl_pointer *p_pointer,
    uint32_t serial,
    struct wl_surface *p_surface_entered_by_the_pointer,
    wl_fixed_t surface_x,
    wl_fixed_t surface_y
){
    // printf("pointer enter, X: %d Y: %d (px)\n", wl_fixed_to_int(surface_x), wl_fixed_to_int(surface_y));

    struct wl_pointer_state_s* p_pointer_state = (struct wl_pointer_state_s*)p_pointer_data;
    // TODO: change the surface entered of the pointer state
    // (*(p_pointer_state->p_wl_surface_entered_state)).p_wl_surface = p_surface_entered_by_the_pointer;

    // Set the CURSOR ROLE to the surface
    // Render the CURSOR
    // Show it
    wl_pointer_set_cursor(p_pointer, serial, p_pointer_state->wl_surface_pointer_state.p_wl_surface,
                          0, 0); // TODO: cursor_offset members in wl_pointer_state_s
    cursor_render(p_pointer_state);
    fprintf(stderr, "Pointer enter. Cursor rendered.\n");
    wl_surface_commit(p_pointer_state->wl_surface_pointer_state.p_wl_surface);

};

static void pointer_surface_leave_handler(
    void *p_pointer_data,
    struct wl_pointer *p_pointer,
    uint32_t serial,
    struct wl_surface *p_surface_left_by_the_pointer
){

};

static void pointer_surface_motion_handler(
    void *p_pointer_data,
    struct wl_pointer *p_pointer,
    uint32_t time,
    wl_fixed_t surface_x,
    wl_fixed_t surface_y
){
    // printf("X: %d\n", wl_fixed_to_int(surface_x));
    // printf("Y: %d\n", wl_fixed_to_int(surface_y));

    struct wl_pointer_state_s* p_pointer_state = (struct wl_pointer_state_s*)p_pointer_data;

    // Drawing a line
    // struct scs_point p1 = { .scs_x = 2, .scs_y = 6 };
    // struct scs_point p2 = { .scs_x = 60, .scs_y = 20 };
    static uint32_t race_error = 0;
    int x = wl_fixed_to_int(surface_x);
    int y = wl_fixed_to_int(surface_y);
    if(x > 0 && y > 0 && x < WL_SURFACE_ROOT_MAX_WIDTH && y < WL_SURFACE_ROOT_MAX_HEIGHT){
        struct scs_point p1 = { .scs_x = 0, .scs_y = 0 };
        struct scs_point p2 = { .scs_x = x, .scs_y = y };
        struct wl_surface_state_s *p_surface_entered_state = p_pointer_state->p_wl_surface_entered_state;

        if(p_surface_entered_state->current_buffer_state.allowed_to_write){
            p_surface_entered_state->current_buffer_state.allowed_to_write = 0;
            set_line(p_surface_entered_state->current_buffer_state.p_buffer_data_start, p1, p2, p_surface_entered_state->width, p_surface_entered_state->height);
            // TODO: damage less rectangle
            wl_surface_damage_buffer(p_surface_entered_state->p_wl_surface, 0, 0, x, y);
            // wl_surface_attach(p_surface_entered_state->p_wl_surface, p_surface_entered_state->current_buffer_state.p_wl_buffer, 0, 0);
            // wl_surface_commit(p_surface_entered_state->p_wl_surface);
        }
        else{
            fprintf(stderr, "Surface buffer RACE ERROR (on pointer motion): %d\n", ++race_error);
            // TODO: create new buffer which IS allowed to write..
        };
    };

};

static void pointer_surface_button_handler(
    void *p_pointer_data,
    struct wl_pointer *p_pointer,
    uint32_t serial,
    uint32_t time,
    uint32_t button,
    uint32_t state
){

};

static void pointer_surface_axis_handler(
    void *p_pointer_data,
    struct wl_pointer *p_pointer,
    uint32_t time,
    uint32_t axis,
    wl_fixed_t value
){

};

static void pointer_surface_frame_handler(
    void *p_pointer_data,
    struct wl_pointer *p_pointer
){
    struct wl_pointer_state_s* p_pointer_state = (struct wl_pointer_state_s*)p_pointer_data;

    wl_surface_attach(p_pointer_state->p_wl_surface_entered_state->p_wl_surface,
                      p_pointer_state->p_wl_surface_entered_state->current_buffer_state.p_wl_buffer, 0, 0);
    wl_surface_commit(p_pointer_state->p_wl_surface_entered_state->p_wl_surface);
};

static void pointer_surface_source_handler(

){

};

static void pointer_surface_stop_handler(

){

};

static void pointer_surface_discrete_handler(

){

};

static void pointer_surface_value120_handler(

){

};

struct wl_pointer_listener pointer_listener = {
    .enter = pointer_surface_enter_handler,
    .leave = pointer_surface_leave_handler,
    .motion = pointer_surface_motion_handler,
    .button = pointer_surface_button_handler,
    .axis = pointer_surface_axis_handler,
    .frame = pointer_surface_frame_handler,
    .axis_source = pointer_surface_source_handler,
    .axis_stop = pointer_surface_stop_handler,
    .axis_discrete = pointer_surface_discrete_handler,
    .axis_value120 = pointer_surface_value120_handler
};


/* RENDER SURFACE
    TODO: rewrite with wl_surface_state_s
*/
static void surface_render(struct pixel* p_buffer_data_start, uint32_t width, uint32_t height,
                            uint32_t r,
                            uint32_t g,
                            uint32_t b,
                            uint32_t a){
    // y = mx + b <=> y = (dy/dx)x + b <=> 0 = xdy - ydx + b
    
    struct pixel *px = NULL;
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {

            px = p_buffer_data_start + y * width + x;

            // purple
            px->alpha = a;
            px->red = r;
            px->green = g;
            px->blue = b;
        };
    };

    // struct scs_point p1 = { .scs_x = 2, .scs_y = 6 };
    // struct scs_point p2 = { .scs_x = 60, .scs_y = 20 };

    // set_line(p_buffer_data_start, p1, p2);
};



static void buffer_release_handler(
    void *p_data, // app data recieved
    struct wl_buffer *p_wl_buffer
){
    struct wl_buffer_state_s *p_wl_buffer_state = (struct wl_buffer_state_s*)p_data;
    p_wl_buffer_state->allowed_to_write = 1;
    // printf("The server has read the %p buffer data. This buffer can be reused.\n", p_wl_buffer_state->p_wl_buffer);        
};

struct wl_buffer_listener buffer_listener = {
    .release = buffer_release_handler
};


/* REGISTY LISTENER */
static void registry_global_handler(
    void *p_data, // app data recieved
    struct wl_registry *p_registry,
    uint32_t name,
    const char *interface,
    uint32_t version
){
    // printf("interface: '%s', version: %u, name: %u\n", interface, version, name);
    struct wl_globals_state_s *p_current_globals_state = (struct wl_globals_state_s*)p_data;

    // TODO? : move to switch(some_hash_function(interface)){case HASH1: ...}
    if (strcmp(interface, "wl_compositor") == 0){
        p_current_globals_state->p_wl_compositor = wl_registry_bind(p_registry, name, &wl_compositor_interface, version);
    }
    else if(strcmp(interface, "wl_shm") == 0){
        p_current_globals_state->p_wl_shm = wl_registry_bind(p_registry, name, &wl_shm_interface, version);
    }
    else if(strcmp(interface, "wl_seat") == 0){
        p_current_globals_state->p_wl_seat = wl_registry_bind(p_registry, name, &wl_seat_interface, version);
    }
    else if(strcmp(interface, "xdg_wm_base") == 0){
        p_current_globals_state->p_xdg_wm_base = wl_registry_bind(p_registry, name, &xdg_wm_base_interface, version);
    };
};

static void registry_global_remove_handler(
    void *p_data,
    struct wl_registry *p_registry,
    uint32_t name
){
    // struct wl_state_s *p_wl_state = (struct wl_state_s*)p_data;
    fprintf(stderr, "global removed: %u\n", name);
};

struct wl_registry_listener registry_listener = {
    .global = registry_global_handler,
    .global_remove = registry_global_remove_handler
};



/* XDG_SURFACE_LISTENER */
static void xdg_surface_configure_handler(
    void *p_data,
    struct xdg_surface *p_xdg_surface,
    uint32_t serial
){
    xdg_surface_ack_configure(p_xdg_surface, serial);
};

struct xdg_surface_listener xdg_surface_listener = {
    .configure = xdg_surface_configure_handler
};

static void wl_callback_done_handler(
    void *p_data,
    struct wl_callback *p_wl_callback,
    uint32_t callback_data
){
    struct wl_state_s *p_wl_state = (struct wl_state_s*)p_data;
    wl_callback_destroy(p_wl_callback);

    // We can't reuse the wl_buffer because it should be resized.
    // So we create a new one.
    wl_buffer_destroy(p_wl_state->current_root_toplevel_state.current_buffer_state.p_wl_buffer);
    struct wl_buffer *p_wl_buffer_new = wl_shm_pool_create_buffer(p_wl_state->p_current_pool, 0,
                                            p_wl_state->current_root_toplevel_state.width,
                                            p_wl_state->current_root_toplevel_state.height,
                                            p_wl_state->current_root_toplevel_state.width * 4,
                                            WL_SHM_FORMAT_ARGB8888);
    p_wl_state->current_root_toplevel_state.current_buffer_state.p_wl_buffer = p_wl_buffer_new;
    p_wl_state->current_root_toplevel_state.current_buffer_state.allowed_to_write = 0;
    // p_wl_state->current_root_toplevel_state.current_buffer_state.p_buffer_data_start; // NO CHANGE, because it's the same pool 
    wl_buffer_add_listener(p_wl_buffer_new, &buffer_listener, &p_wl_state->current_root_toplevel_state.current_buffer_state);


    surface_render(p_wl_state->current_root_toplevel_state.current_buffer_state.p_buffer_data_start,
                    p_wl_state->current_root_toplevel_state.width,
                    p_wl_state->current_root_toplevel_state.height,
                    60, 100, 130, 255);
    wl_surface_damage_buffer(p_wl_state->current_root_toplevel_state.p_wl_surface, 0, 0, WL_SURFACE_ROOT_MAX_WIDTH, WL_SURFACE_ROOT_MAX_HEIGHT);
    wl_surface_attach(p_wl_state->current_root_toplevel_state.p_wl_surface, p_wl_state->current_root_toplevel_state.current_buffer_state.p_wl_buffer, 0, 0);
    wl_surface_commit(p_wl_state->current_root_toplevel_state.p_wl_surface);
};

struct wl_callback_listener wl_callback_listener = {
    .done = wl_callback_done_handler
};


/* XDG_TOPLEVEL_LESTENER */
static void xdg_toplevel_configure_handler(
    void *p_data,
    struct xdg_toplevel *p_xdg_toplevel,
    int32_t width,
    int32_t height,
    struct wl_array *p_states
){
    // fprintf(stderr, "toplevel configure %d w %d h\n", width, height);
    struct wl_state_s *p_wl_state = (struct wl_state_s*)p_data;

    // After creating xdg_surface we are setting it up:
    // E.g for setting up toplevel surface, we:
    //  - assign toplevel role with xdg_surface_get_toplevel(),
    //  - set up a title for the surface with xdg_toplevel_set_title(),
    //  - set up max/min size,
    //  - etc.
    // ...After all that configuring, we as a "client" must perform an initial commit
    // without any buffer attached. We must do so, to let the server (i.e compositor) save those changes.
    // And after this, the server will reply with xdg_toplevel.configure and xdg_surface.configure events.
    // In the former we may find that width and height are zero.
    // TODO: EXPLAIN WHY ZERO?
    if(width == 0 && height == 0){
        // TODO: make "load" window
        fprintf(stderr, "The compositor is configuring the surface..\n");
        return;
    }
    else if(width + height > WL_SURFACE_ROOT_MAX_WIDTH + WL_SURFACE_ROOT_MAX_HEIGHT) return;
    p_wl_state->current_root_toplevel_state.width = width;
    p_wl_state->current_root_toplevel_state.height = height;

    struct wl_callback *p_new_frame = wl_surface_frame(p_wl_state->current_root_toplevel_state.p_wl_surface);
    wl_callback_add_listener(p_new_frame, &wl_callback_listener, p_wl_state);
    wl_surface_commit(p_wl_state->current_root_toplevel_state.p_wl_surface);

};

static void xdg_toplevel_close_handler(
    void *p_data,
    struct xdg_toplevel *p_xdg_toplevel
){
    fprintf(stderr, "toplevel close\n");
};

static void xdg_toplevel_configure_bounds_handler(
    void *p_data,
    struct xdg_toplevel *p_xdg_toplevel,
    int32_t width,
    int32_t height
){

};

struct xdg_toplevel_listener xdg_toplevel_listener = {
    .configure = xdg_toplevel_configure_handler,
    .close = xdg_toplevel_close_handler,
    .configure_bounds = xdg_toplevel_configure_bounds_handler
};


/*
    Memory managment for wayland.
    This function creates memory pools and buffers inside these pools.
    The rules of creating as follows:
    TODO: spicify the rules

    TODO: decide whether make the function independent and move to another file..
*/
void create_wl_memory(){
    // TODO
};


extern void init_wayland(){
    struct wl_state_s wl_state;

    fprintf(stderr, "Sizeof struct wl_state: %ld\n", sizeof(struct wl_state_s));
    fprintf(stderr, "Sizeof struct wl_buffer_state_s: %ld\n", sizeof(struct wl_buffer_state_s));


    /* Connect to wl_display */
    wl_state.p_wl_display = wl_display_connect(NULL);
    if(wl_state.p_wl_display){
        // TODO: return SUCCES connect info
    }
    else {
        // TODO: return ERROR info
        return;
    };

    /* Get all required registries */
    wl_state.p_wl_registry = wl_display_get_registry(wl_state.p_wl_display);
    wl_registry_add_listener(wl_state.p_wl_registry, &registry_listener, &wl_state.current_globals_state);


    // TODO: replace with wl_display.sync ???
    wl_display_roundtrip(wl_state.p_wl_display);
    if( wl_state.current_globals_state.p_wl_seat
        && wl_state.current_globals_state.p_wl_shm
        && wl_state.current_globals_state.p_wl_compositor
        && wl_state.current_globals_state.p_xdg_wm_base
    ){ 
        // TODO: return SUCCES info
        // printf("Got all required globals!\n");
    } else {
        printf("Some required globals unavailable\n");
        return;
    };

    /* Create SHARED MEMORY POOL
        Within memory pool we can create multiple buffers.
        And with multiple buffers we can attach them to multiple wl_surface'es
        TODO: move to create_wl_memory()
    */
    uint32_t shm_fd = shm_open("/todochat_wl_shm_pool", O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
    ftruncate(shm_fd, WL_SHARED_MEMORY_POOL_SIZE);
    uint8_t *p_pool_data_start = mmap(NULL, WL_SHARED_MEMORY_POOL_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    struct wl_shm_pool* p_pool = wl_shm_create_pool(wl_state.current_globals_state.p_wl_shm, shm_fd, WL_SHARED_MEMORY_POOL_SIZE);
    // wl_shm_pool_set_user_data(struct wl_shm_pool *p_pool, void *p_user_data);
    wl_state.p_current_pool = p_pool;

    // Comment it for seeing pool data at /dev/shn.
    // Commenting the line will prevent deleting the shared memory pool from physical disk.
    // This may be used to see pool data in a text editor
    // and debugging (adjusting) buffers in the pool
    shm_unlink("/todochat_wl_shm_pool");

    // "shm_unlink() removes a shared memory object name,
    //  and, once all processes have unmapped the object,
    //  deallocates and destroys the contents of the associated memory region."
    // The thing is here that it doesn't close the file descriptor so it may be reused.
    // To do that we call close(shm_fd) manually.
    close(shm_fd);



    /* CREATING BUFFERS INSIDE THE POOL
        TODO: move to create_wl_memory()
    */
    // Create buffer for XDG_SURFACE
    struct wl_buffer* p_wl_buffer_root = wl_shm_pool_create_buffer(p_pool, 0, WL_SURFACE_ROOT_MAX_WIDTH, WL_SURFACE_ROOT_MAX_HEIGHT, WL_SURFACE_ROOT_MAX_STRIDE, WL_SHM_FORMAT_ARGB8888);
    wl_buffer_add_listener(p_wl_buffer_root,
                           &buffer_listener,
                           &wl_state.current_root_toplevel_state.current_buffer_state); // app data send


    // Create buffer for CURSOR SURFACE
    struct wl_buffer* p_wl_buffer_cursor = wl_shm_pool_create_buffer(p_pool, WL_SURFACE_ROOT_BUFFER_SIZE, CURSOR_SURFACE_WIDTH, CURSOR_SURFACE_HEIGHT, CURSOR_SURFACE_STRIDE, WL_SHM_FORMAT_ARGB8888);
    wl_buffer_add_listener(p_wl_buffer_cursor,
                           &buffer_listener,
                           &wl_state.current_pointer_state.wl_surface_pointer_state.current_buffer_state); // app data send



    /* Create WL_SURFACE & XDG_SURFACE
        The XDG_SURFACE isn't itself a role. It's an object that provides roles.
        Currently, xdg_surface provides two roles: "toplevels" and "popups".
        In our case xdg_surface is a toplevel,
        and the WL_SURFACE is responsible for its content.
    */
    struct wl_surface* p_wl_surface_root = wl_compositor_create_surface(wl_state.current_globals_state.p_wl_compositor);
    struct xdg_surface* p_xdg_surface = xdg_wm_base_get_xdg_surface(wl_state.current_globals_state.p_xdg_wm_base, p_wl_surface_root);
    xdg_surface_add_listener(p_xdg_surface, &xdg_surface_listener, NULL);

    struct xdg_toplevel* p_xdg_toplevel = xdg_surface_get_toplevel(p_xdg_surface); // assign toplevel role
    xdg_toplevel_set_max_size(p_xdg_toplevel, WL_SURFACE_ROOT_MAX_WIDTH, WL_SURFACE_ROOT_MAX_HEIGHT);
    xdg_toplevel_set_min_size(p_xdg_toplevel, WL_SURFACE_ROOT_MIN_WIDTH, WL_SURFACE_ROOT_MIN_HEIGHT);
    xdg_toplevel_set_title(p_xdg_toplevel, "todochat");
    wl_state.current_root_toplevel_state.current_buffer_state.p_wl_buffer = p_wl_buffer_root;
    wl_state.current_root_toplevel_state.current_buffer_state.p_buffer_data_start = (struct pixel*)p_pool_data_start;
    // We attach a buffer further. When it's released allowed_to_write changes to "1"
    wl_state.current_root_toplevel_state.current_buffer_state.allowed_to_write = 0;
    wl_state.current_root_toplevel_state.p_wl_surface = p_wl_surface_root;
    wl_state.current_root_toplevel_state.width = 0;  // sets up at toplevel configure event
    wl_state.current_root_toplevel_state.height = 0; // sets up at toplevel configure event
    xdg_toplevel_add_listener(p_xdg_toplevel, &xdg_toplevel_listener,
                              &wl_state); // app data send


    // Initital commit to let the server configure xdg_surface
    wl_surface_commit(p_wl_surface_root);

    // Wait for server to configure the surface before attaching a buffer.
    // TODO: replace with wl_display_sync()
    wl_display_roundtrip(wl_state.p_wl_display);

    // When the surface is configured we can render smth on it, attach and finally,
    // show the XDG_SURFACE and its toplevel surface on the screen.
    // But we won't do this right now. We will do, however, in the next configure event, which
    // should come to us informing that it has appeared on the screen.
    surface_render((struct pixel*)p_pool_data_start, WL_SURFACE_ROOT_MAX_WIDTH, WL_SURFACE_ROOT_MAX_HEIGHT, 60, 57, 70, 255);
    struct scs_point p = {
        .scs_x = 5,
        .scs_y = 5
    };
    set_rect(wl_state.current_root_toplevel_state.current_buffer_state.p_buffer_data_start,
             p, 121, 120);
    wl_surface_attach(p_wl_surface_root, p_wl_buffer_root, 0, 0);
    wl_surface_commit(p_wl_surface_root);


    /* Get POINTER from wl_seat. Make CURSOR surface from it.
        Pointer and cursor are different things.
        Simply put, "pointer" is the fact that your mouse entering,moving,leaving,etc. on a surface,
        and "cursor" is a rendered surface on your "pointer"
    */
    // fprintf(stderr, "Surface was placed.\n");
    struct wl_pointer* p_pointer = wl_seat_get_pointer(wl_state.current_globals_state.p_wl_seat);
    struct wl_surface* p_wl_surface_pointer = wl_compositor_create_surface(wl_state.current_globals_state.p_wl_compositor);
    wl_surface_attach(p_wl_surface_pointer, p_wl_buffer_cursor, 0, 0);

    wl_state.current_pointer_state.wl_surface_pointer_state.p_wl_surface = p_wl_surface_pointer;
    wl_state.current_pointer_state.wl_surface_pointer_state.current_buffer_state.p_buffer_data_start = (struct pixel*)(p_pool_data_start + WL_SURFACE_ROOT_BUFFER_SIZE);
    wl_state.current_pointer_state.wl_surface_pointer_state.current_buffer_state.p_wl_buffer = p_wl_buffer_cursor;
    wl_state.current_pointer_state.wl_surface_pointer_state.current_buffer_state.allowed_to_write = 1;
    wl_state.current_pointer_state.wl_surface_pointer_state.width = CURSOR_SURFACE_WIDTH;
    wl_state.current_pointer_state.wl_surface_pointer_state.height = CURSOR_SURFACE_HEIGHT;
    wl_state.current_pointer_state.p_wl_surface_entered_state = &wl_state.current_root_toplevel_state;

    wl_pointer_add_listener(
        p_pointer,
        &pointer_listener,
        &wl_state.current_pointer_state // app data send
    );



    while(1){
        wl_display_dispatch(wl_state.p_wl_display);
    };

    wl_display_disconnect(wl_state.p_wl_display);
}