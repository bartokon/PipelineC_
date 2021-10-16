// https://github.com/JulianKemmerer/PipelineC/discussions/22
/*
(C) 2021 Victor Suarez Rovere <suarezvictor@gmail.com>
LICENSE: GPL 3.0

Compile like:
(TODO: make compatible with Verilator --main_cpp make file based flow)

# From pipelinec repo:
$ ./src/pipelinec ./examples/arty/src/vga/test_pattern_modular.c --sim_comb --verilator --out_dir ~/pipelinec_output
# ^ can swap out test_pattern_modular.c for other vga example files.

# From current dir with main.cpp
$ verilator -cc ~/pipelinec_output/top/top.v
$ clang++ -O3 -I. -I/usr/share/verilator/include main.cpp -o vtop -lSDL2
# OR
$ clang++ -O3 -I. -I/usr/local/share/verilator/include -I$HOME/pipelinec_output/verilator main.cpp -o vtop -lSDL2 -I/usr/include/SDL2

# Run
$ ./vtop
*/
  
#include <SDL2/SDL.h>
#include "bitregs.h"

#define USE_VERILATOR

#ifndef USE_VERILATOR

#define WIRE_WRITE(a,b,c)
#define WIRE_READ(a,b,c)
#define DEBUG_OUTPUT_DECL(a,b)
#define MAIN_MHZ(a,b)
#define SKIP_DEBUG_OUTPUT

// VGA pattern generation specifics
uint4_t debug_vga_red;
uint4_t debug_vga_green;
uint4_t debug_vga_blue;
uint1_t debug_vsync;
uint1_t debug_hsync;
typedef int app_to_vga_t_array_1_t, app_to_pmod_jc_t_array_1_t, app_to_pmod_jb_t_array_1_t;

#pragma clang diagnostic push
//#pragma clang diagnostic ignored "-Wparentheses" //for & and | operator precedence warning
#include "../../arty/src/vga/test_pattern_modular.c"
#pragma clang diagnostic pop

#else
// screen dimensions
// (make sure match what was used in PipelineC code - TODO combine)
#define FRAME_WIDTH 1280
#define FRAME_HEIGHT 720
#define V_POL 1
#define H_POL 1
#define clk clk_74p25
#define _vga_red vga_red_DEBUG_OUTPUT_MAIN_return_output
#define _vga_green vga_green_DEBUG_OUTPUT_MAIN_return_output
#define _vga_blue vga_blue_DEBUG_OUTPUT_MAIN_return_output
#define _vsync vsync_DEBUG_OUTPUT_MAIN_return_output
#define _hsync hsync_DEBUG_OUTPUT_MAIN_return_output

#include "verilated.cpp" //verilator library

//#include "obj_dir/Vtop__ALL.cpp" //generated by verilator
// Some larger examples dont produce a _ALL.cpp?
/*
#include "obj_dir/Vtop___024root__DepSet_heccd7ead__0.cpp"
#include "obj_dir/Vtop___024root__DepSet_heccd7ead__0__Slow.cpp"
#include "obj_dir/Vtop___024root__DepSet_heccd7ead__1.cpp"
#include "obj_dir/Vtop___024root__DepSet_heccd7ead__1__Slow.cpp"
#include "obj_dir/Vtop___024root__DepSet_heccd7ead__2.cpp"
#include "obj_dir/Vtop___024root__DepSet_heccd7ead__2__Slow.cpp"
#include "obj_dir/Vtop___024root__DepSet_heccd7ead__3.cpp"
#include "obj_dir/Vtop___024root__DepSet_heccd7ead__4.cpp"
#include "obj_dir/Vtop___024root__Slow.cpp"
#include "obj_dir/Vtop__Syms.cpp"
#include "obj_dir/Vtop.cpp"
*/

#include "obj_dir/Vtop.cpp"
#include "obj_dir/Vtop___024root__DepSet_heccd7ead__0.cpp"
#include "obj_dir/Vtop___024root__DepSet_heccd7ead__0__Slow.cpp"
#include "obj_dir/Vtop___024root__DepSet_heccd7ead__1.cpp"
#include "obj_dir/Vtop___024root__DepSet_heccd7ead__1__Slow.cpp"
#include "obj_dir/Vtop___024root__DepSet_heccd7ead__2.cpp"
#include "obj_dir/Vtop___024root__DepSet_heccd7ead__3.cpp"
#include "obj_dir/Vtop___024root__DepSet_heccd7ead__4.cpp"
#include "obj_dir/Vtop___024root__DepSet_heccd7ead__5.cpp"
#include "obj_dir/Vtop___024root__Slow.cpp"
#include "obj_dir/Vtop__Syms.cpp"


#endif

bool fb_init(unsigned hres, unsigned vres);
void fb_update();
bool fb_should_quit();
void fb_deinit();
struct pixel_t { uint8_t a, b, g, r; };
pixel_t pixelbuf[FRAME_HEIGHT][FRAME_WIDTH];
inline void fb_setpixel(pixel_t *p, uint8_t r, uint8_t g, uint8_t b) { p->a = 0xFF; p->b = b; p->g = g; p->r = r; }
inline void fb_setpixel(unsigned x, unsigned y, uint8_t r, uint8_t g, uint8_t b) { fb_setpixel(&pixelbuf[y][x], r, g, b); }
inline uint64_t higres_ticks() { return SDL_GetPerformanceCounter(); }
inline uint64_t higres_ticks_freq() { return SDL_GetPerformanceFrequency(); }

int main()
{
#ifdef USE_VERILATOR
    Vtop* g_top = new Vtop;
#endif
    if(!fb_init(FRAME_WIDTH, FRAME_HEIGHT))
        return 1;

    uint64_t frame = 0;

    uint16_t x = 0, y = 0;
    unsigned hsync_width = FRAME_WIDTH, vsync_height = FRAME_HEIGHT; //initially out of range
    pixel_t *p = NULL;

    fb_update();
    uint64_t t0 = higres_ticks();

    uint4_t testval;
    for(;;)
    {
       if(x == 0 && y == 0 && fb_should_quit())
           break;

#ifdef USE_VERILATOR
       bool vs = g_top->_vsync;
       bool hs = g_top->_hsync;
#else
       bool vs = debug_vsync;
       bool hs = debug_hsync;
#endif
       if(vs == V_POL)
       {
           if(y != 0)
           {
             hsync_width = x;
             vsync_height = y - FRAME_HEIGHT;
             x = 0; y = 0;
             p = &pixelbuf[0][0];
             fb_update();
             ++frame;
           }
       }
       else if(hs == H_POL)
       {
           if(x != 0)
           {
              ++y;
              x = 0;
              if(y >= vsync_height && y < FRAME_HEIGHT + vsync_height)
                p = &pixelbuf[y-vsync_height][0];
              else
                p = NULL;
           }
       }
       else
       {
         if(p != NULL && x >= hsync_width && x < FRAME_WIDTH+hsync_width)
         {
#ifdef USE_VERILATOR
           uint8_t r = g_top->_vga_red;
           uint8_t g = g_top->_vga_green;
           uint8_t b = g_top->_vga_blue;
#else
           uint8_t r = debug_vga_red;
           uint8_t g = debug_vga_green;
           uint8_t b = debug_vga_blue;
#endif
           fb_setpixel(p++, r << 4, g << 4, b << 4);
         }
         ++x; //16 bit value that can wrap around if no hsync pulses comes
       }

#ifdef USE_VERILATOR
       g_top->clk = 0;
       g_top->eval();
       g_top->clk = 1;
       g_top->eval();
#else
#if 1
       app();
#else
       //reaches 107 FPS:
       debug_hsync=(x==FRAME_WIDTH) ? H_POL:!H_POL;
       debug_vsync=(y==FRAME_HEIGHT) ? V_POL:!V_POL;
       debug_vga_red = x+y+frame;
#endif
#endif
    }

    float elapsed = float(higres_ticks()-t0)/higres_ticks_freq();
    printf("FPS: %.1f\n", frame/elapsed);

#ifdef USE_VERILATOR
    g_top->final();
#endif
    fb_deinit();
    return 0;
}

//code adapted from https://projectf.io/posts/verilog-sim-verilator-sdl/

SDL_Window*   sdl_window   = NULL;
SDL_Renderer* sdl_renderer = NULL;
SDL_Texture*  sdl_texture  = NULL;

bool fb_init(unsigned hres, unsigned vres)
{
    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL init failed.\n");
        return false;
    }

    sdl_window = SDL_CreateWindow("Verilated Framebuffer", SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, hres, vres, SDL_WINDOW_SHOWN);
    if (!sdl_window) {
        printf("Window creation failed: %s\n", SDL_GetError());
        return false;
    }

    sdl_renderer = SDL_CreateRenderer(sdl_window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!sdl_renderer) {
        printf("Renderer creation failed: %s\n", SDL_GetError());
        return false;
    }

    sdl_texture = SDL_CreateTexture(sdl_renderer, SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET, hres, vres);
    if (!sdl_texture) {
        printf("Texture creation failed: %s\n", SDL_GetError());
        return false;
    }
    return true;
}

bool fb_should_quit()
{
    SDL_Event e;
    if (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            return true;
        }
    }
    return false;
}

void fb_update()
{
    SDL_UpdateTexture(sdl_texture, NULL, pixelbuf, FRAME_WIDTH*sizeof(pixel_t));
    SDL_RenderClear(sdl_renderer);
    SDL_RenderCopy(sdl_renderer, sdl_texture, NULL, NULL);
    SDL_RenderPresent(sdl_renderer);
}

void fb_deinit()
{
    SDL_DestroyTexture(sdl_texture);
    SDL_DestroyRenderer(sdl_renderer);
    SDL_DestroyWindow(sdl_window);
    SDL_Quit();
}
