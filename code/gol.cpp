#include <iostream>
#include <windows.h>
#include <windowsx.h>
#include <queue>
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <shobjidl.h> // For IFileDialog
#include <iostream>
#include <fstream>
#include <cwctype>
#include "gif.h"

#include <dxgi.h>
#include <d3d11.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <ft2build.h>
#include FT_FREETYPE_H  

#define IDM_FILE_OPEN 1001
#define IDM_FILE_EXIT 1002
#define IDM_FILE_SAVE 1003
#define DLG_OK_BUTT 1
#define gpu_acceleration
#define SAFE_RELEASE(p)      { if (p) { (p)->Release(); (p)=nullptr; } }

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef POINT point;
bool operator==(point a, point b)
{
    return ((a.x == b.x) && (a.y == b.y));
}

//Colors
uint32 white = 0xffffffff;
uint32 light = 0x00f8f8a9;
uint32 black = 0x00000000;
uint32 orange = 0x00f39f80;
uint32 l_orange = 0x00ffaa80;
uint32 d_orange = 0x00ef7c51;
uint32 red = 0x00ff0000;
uint32 green = 0x0000ff00;
uint32 blue = 0x000000ff;
uint32 grey = 0x004a4a4a;
uint32 l_grey = 0x007e7e7e;
uint32 ll_grey = 0x00ef0f0f0;
uint32 l_blue = 0x008cfffb;
uint32 d_blue = 0x0000c7c0;
uint32 ld_blue = 0x007fe2de;
uint32 lg_blue = 0x00e7fffe;
uint32 ldg_blue = 0x00b4fffd;
uint32 slct_color = 0x005ac3f2;

// structs ----------------------------------

struct timer
{
    static __int64 count_freq;
    __int64 ref;
    __int64 start;
    __int64 stop;
    double dt;
    double total;
    void reset();
    void tick();
};

__int64 timer::count_freq;
void timer::reset()
{
    QueryPerformanceCounter((LARGE_INTEGER*)&ref);;
    start = ref;
    stop = ref;
    total = 0;
}

void timer::tick()
{
    __int64 counts;
    QueryPerformanceCounter((LARGE_INTEGER*)&counts);

    stop = counts;
    dt = double(stop - start) / count_freq;
    start = stop;
    total += dt;
}

struct r_point {
    double x = 0, y = 0;
};

struct r_recta
{
    double x, y, w, h;
    r_recta operator*(int k)
    {
        return r_recta{ k * x, k * y, k * w, k * h };
    }
};

struct recta
{
    int x, y, w, h;
    recta operator*(int k)
    {
        return recta{ k * x, k * y, k * w, k * h };
    }
    recta operator=(r_recta& rr)
    {
        x = int(rr.x);
        y = int(rr.y);
        w = int(rr.w);
        h = int(rr.h);
        return *this;
    }
};

struct mouse
{
    bool l_down = false, r_down = false, l_clk = false;
};

struct image
{
    uint32* pixels = NULL;
    int w = 0, h = 0;
    recta scr_zone;
};

struct character
{
    uint32* pixels;
    int w, h;
    int bearing_x, bearing_y;
    int advance;
    int max_intens = 0;
};

struct hinted_face
{
    std::unordered_map<uint16, character>* glyphs = NULL;
    int max_ascender, max_descender;
};

struct scroller
{
    timer timer;
    double ratio = 3;
    bool drag = false;
    int  y0;
    bool attrct = false;
    int dir;
    recta zone;
    recta scrler_zone;
    recta up_zone;
    recta down_zone;
};

struct txt_edit_params
{
    recta zone;
    std::wstring str;
    std::vector<int> chars_x;
    int curs_idx = 0;
    recta txt_zone;
    int dx_ref;
    uint32 max_crcs = UINT_MAX;
    int slct_idx = 0;
    int get_char_idx();
    void copy_txt(std::wstring str, FT_Face& face);
    void handle_slct();
    void draw();
    void handle_arrows(WPARAM wp);
    void add_char(uint16 my_char);
    bool handle_ldown();
};

struct opn_sav_dlg
{
    int nb_files;
    int file_h;
    std::deque<std::pair<std::wstring, bool>>  files_q;
    std::wstring curr_dir;
    bool in_root = false;
    int fslctd_idx = -1;
    int fhref;
    bool scrl_exist = false;
    
    scroller scrl;
    recta zone;
    recta exp_zone;
    recta exp_zone_wscrl;
    recta exp_zone_scrl;
    recta cancel_zone;
    recta open_zone;

    txt_edit_params txt_edit;
    std::vector<std::wstring> allowed_ext;
    
    void go_slctd_dir();
    void go_curr_dir();
    void handle_mw(int delta);
    void handle_scrl_ldblclk();
    void handle_ldown();
};

struct info_dlg
{
    recta zone;
    std::wstring txt;
    std::wstring title;
    recta ok_zone;
    int title_h;
    int txt_h;
    int char_h;
    recta txt_margins;
    recta ok_margins;
    std::vector<int> nxt_line_idxs;
    void fill_params(FT_Face& face);
    void draw_txt(FT_Face& face, const uint32& color = grey);
    void draw();
};

struct collect_dlg
{
    recta zone;
    std::wstring title;
    int title_h;
    std::vector<std::wstring> txts;
    std::vector<recta> txt_zones;
    std::vector<txt_edit_params> txt_edits;
    recta cancel_zone;
    recta ok_zone;
    void fill_params();
    int slctd_edit;
    void draw();
};

struct rect_limits
{
    uint32 left;
    uint32 top;
    uint32 right;
    uint32 bottom;
};

bool running = false;

// files ----------------------------------
const char* play_butt_fn[3]   = { "..\\images\\play_butt_0.png", 
                            "..\\images\\play_butt_1.png",
                            "..\\images\\play_butt_2.png" };
const char* pause_butt_fn[3]  = { "..\\images\\pause_butt_0.png",
                            "..\\images\\pause_butt_1.png",
                            "..\\images\\pause_butt_2.png" };
const char* reinit_butt_fn[3] = { "..\\images\\reinit_butt.png",
                            "..\\images\\reinit_butt_1.png",
                            "..\\images\\reinit_butt_2.png" };
const char* next_butt_fn[3]   = { "..\\images\\next_butt.png",
                            "..\\images\\next_butt_1.png",
                            "..\\images\\next_butt_2.png" };
const char* scrl_up_fn[2]     = { "..\\images\\scrl_up.png",
                            "..\\images\\scrl_up1.png" };
const char* scrl_down_fn[2]   = { "..\\images\\scrl_down.png",
                            "..\\images\\scrl_down1.png" };

char intens_bar_fn[]  = "..\\images\\intensity_bar.png";
char intens_slct_fn[] = "..\\images\\intensity_select.png";
char cursor_fn[]      = "..\\images\\cursor.png";
char num_letters_fn[] = "..\\images\\num_letters.png";
char hide_fn[]        = "..\\images\\hide.png";
char anti_hide_fn[]   = "..\\images\\anti_hide.png";
char folder_icon_fn[] = "..\\images\\folder.png";
char cascadia_font[] = "Cascadia.ttf";

//window handle 
HWND main_wnd;

// time calculation vars ----------------------------------
timer app_timer;
timer cursor_timer;
timer frame_timer;

// Bitmap params ----------------------------------
void* bmp_memory = NULL;
BITMAPINFO bmp_info;
int bytes_per_pixel = 4;
r_recta bmp_zone;
recta screen_zone;
recta vbmp_zone;

// frame rate params ----------------------------------
double frame_dur = 1.0f / 60; // 60fps

// zooming params ----------------------------------
recta bmp_zone_targ;
double zoom_ratio_x;
double zoom_ratio_y;
bool zoom_in = false;
double zoom_speed;
double pos_zpeed_x;
double pos_zpeed_y;
bool zooming = false;

// Grid params ----------------------------------
int cell_w    = 40;
int cell_h    = 40;
int line_w   = 6;
int nb_cell_w = 400;
int nb_cell_h = 400;
int nb_cells = nb_cell_w * nb_cell_h;
uint32* grid = new uint32[nb_cells]();
uint32* futur_grid = new uint32[nb_cells]();

// simulation params ----------------------------------
bool is_step_time = false;
timer sim_timer;
bool sim_on       = false;
double sim_freq   = 1;
double step_dur   = 0.5;
int actual_step   = 0;
int target_step   = -1;
int prv_targ_stp  = -1;
rect_limits stepping_limits, initial_limits;
std::vector<point> grid_init_stt;

// menu zone params ----------------------------------
bool go_butt_down = false;
bool plpau_down = false;
bool reinit_down = false;
bool next_down = false;
bool show_map = false;
bool int_bar_slctd = false;
bool show_cursor = false;

//  _step selection params ----------------------------------
txt_edit_params step_edit;
std::string step_str;
bool slct_mode = false;
int mouse_slct_idx = 0;
int initial_curs_x;

// gif saving params ----------------------------------
bool gif_params_collect = false;
bool gif_slct_mode = false;
bool gif_params_entrd = false;
int nb_gen = 1;
int gen_ps = 1;
std::wstring gif_path;
point gif_rg0;
point gif_rg1;

// region selection params ----------------------------------
bool mov_region = false;
point region_pos0;
int map_cell_w = 1;
int map_w = 300;
int map_h = 300;
r_point map_pos;
recta region_slct_zone;
recta bmp_region_zone;
recta sh_hide_zone;
recta map_zone = { 0,0,0,0 };

// interaction modes, may intersect ----------------------------------
bool open_file = false;
bool save_file = false;
bool gif_saving = false;
bool save_ext_err = false;
bool gif_sav_info = false;
bool f_error_dlg = false;
bool normal_mode = true;
bool go_mode = false;
bool del_mouse_lup = false;

// file opeining and saving params ----------------------------------
opn_sav_dlg opnf_dlg;
opn_sav_dlg savf_dlg;
info_dlg opn_wrong_f;
info_dlg gif_slct_info;
info_dlg ext_err_info;
collect_dlg gif_params_dlg;

// user input params ----------------------------------
mouse my_mouse;
mouse initial_mouse;
point mouse_coords;
point prev_mouse_coords;
point mouse_coords_ldown;
point mouse_coords_rdown;
point mouse_coords_rup;
point mouse_coords_lup;

// menu zone and subzones ----------------------------------
recta menu_zone;
recta actual_step_zone;
recta sim_vel_zone; 
recta go_to_stp_zone;
recta save_butt_zone;
recta open_butt_zone;
recta clear_butt_zone;
recta go_butt_zone;

// UI images ----------------------------------
image play_butt[3];
image pause_butt[3];
image reinit_butt[3];
image next_butt[3];
image scrl_up[2];
image scrl_down[2];
image intens_slct;
image intens_bar;
image my_cursor;
image num_letters;
image hide;
image anti_hide;
image folder_icon;

// num_letters sprite info ----------------------------------
int num_width;
int num_height;
int src_num_width = 16;
int src_num_height = 14;
int step_num_h;

int letter_w = 16;
int letter_h = 14;
recta mul_sign_zone = { 160, 32, 10, 10 };
recta dot_zone = { src_num_width * 10 , 0, 4, 2 };

// freetype text rendering params 
FT_Library ft;
FT_Face cascadia_face;
hinted_face face_chars[200]; 

// function prototypes ----------------------------------
// basic blitting  
void draw_bgnd_color(uint32 color);
void draw_color_to_zone(uint32 color, recta zone, recta limits);
void draw_line(point p1, point p2, int w, uint32 clr);
void draw_rect_mesh(uint32 clr, int wd, int hd);
void draw_diag_mesh(uint32 clr, int d0);
void draw_recta(recta my_rect, uint32 color, int bordr_w);
void draw_out_recta(recta my_rect, uint32 color, int bordr_w);
void draw_im(image im, bool to_blit);
void instrch_from_im(image src_im, recta src_zone, recta dest_zone, bool to_color, uint32 color, recta limits);
void draw_from_im(image src_im, recta src_zone, point pos);
void draw_num(char num, recta dest_zone);
void draw_text(const wchar_t* my_text, point pos, point char_size, bool to_color , uint32 color, recta limits); // old way using fixed sprites
void draw_text_to_zone(const wchar_t* my_text, recta dest_zone, bool to_color, uint32 color); // old way using fixed sprites

void draw_text_cntrd(FT_Face& face, const std::wstring& str, const recta& zone, const recta& margins = {0,0,0,0}, const uint32& color = grey); // using freetype
void draw_text_cntrd_fit(FT_Face& face, const std::wstring& str, const recta& zone, const recta& margins, const uint32& color = grey); // using freetype
void draw_text_ft(FT_Face& face, const std::wstring& str, const recta& zone, const recta& limits = screen_zone, const recta& margins = { 0,0,0,0 }, const int& dx_ref = 0, const uint32& color = grey); // using freetype

void update_wnd(HDC device_context, RECT* client_rect);

// map and some menu zone images and buttons drawion
void draw_sim_vel();
void draw_actual_step();
void draw_region_slct();
void draw_butt(const recta& zone, std::wstring str, int marg, uint32 clr = orange, uint32 hvr_clr = l_blue);

// simulation 
bool is_neigh_alive(int neigh_x, int neigh_y);
void one_step_ahead(); 

// grid updating 
void reinit_grid();
void clear_grid();

// opening file  
void load_from_file(std::wstring file_name);
void get_exe_dir(std::wstring& curr_dir);

// gif_saving
void save_as_gif();

// other 
int get_step_int();
point get_clk_cell(POINT m_coords);
bool in_zone(const point& m_coords, const recta& zone);
void make_inside(int& a, const int& low, const int& up);
void make_inside(double& a, const int& low, const int& up);
void make_inside(recta& zone, const recta& limits);
void make_inside(r_recta& zone, const recta& limits);
void add_constant(std::vector<int>& vec, int start_idx, int end_idx, int val);
int  get_sum(std::vector<int>& vec, int start_idx, int end_idx);
bool check_ext(const std::wstring& file_path, const std::wstring& ext);

void draw_bgnd_color(uint32 color)
{
    uint32* pixel = (uint32*)bmp_memory;
    for (int y = 0; y < screen_zone.h; y++)
        for (int x = 0; x < screen_zone.w; x++)
            pixel[y * screen_zone.w + x] = color;

}

void draw_color_to_zone(uint32 color, recta zone, recta limits = {-1,-1,INT_MAX,INT_MAX})
{
    uint32* pixel = (uint32*)bmp_memory;
    
    int y = max(0, max(zone.y, limits.y));
    int x0 = max(0, max(zone.x, limits.x));
    int y_max = min(screen_zone.h, min(zone.y + zone.h, limits.y + limits.h));
    int x_max = min(screen_zone.w, min(zone.x + zone.w, limits.x + limits.w));

    for(; y < y_max; y++)
        for (int x = x0; x < x_max; x++)
            pixel[y * screen_zone.w + x] = color;
}

void draw_recta(recta my_rect, uint32 color, int bordr_w)
{
    draw_color_to_zone(color, { my_rect.x, my_rect.y, bordr_w, my_rect.h});
    draw_color_to_zone(color, { my_rect.x + my_rect.w - bordr_w, my_rect.y, bordr_w, my_rect.h});
    draw_color_to_zone(color, { my_rect.x , my_rect.y , my_rect.w, bordr_w });
    draw_color_to_zone(color, { my_rect.x , my_rect.y + my_rect.h - bordr_w, my_rect.w , bordr_w });
}

void draw_out_recta(recta my_rect, uint32 color, int bordr_w)
{
    draw_color_to_zone(color, {my_rect.x - bordr_w, my_rect.y - bordr_w, bordr_w, my_rect.h + 2* bordr_w });
    draw_color_to_zone(color, { my_rect.x + my_rect.w, my_rect.y- bordr_w, bordr_w, my_rect.h + 2* bordr_w });
    draw_color_to_zone(color, { my_rect.x - bordr_w, my_rect.y - bordr_w, my_rect.w + 2* bordr_w, bordr_w });
    draw_color_to_zone(color, { my_rect.x - bordr_w, my_rect.y + my_rect.h, my_rect.w + 2* bordr_w, bordr_w });
}

void draw_line(point p1, point p2, int w, uint32 clr)
{
    uint32* pixel = (uint32*)bmp_memory;
    double a = double(p1.y - p2.y) / (p1.x - p2.x);   
    double b = double(p1.x * p2.y - p1.y * p2.x) / (p1.x - p2.x);
    int x0, y0;
    int x1, y1;
    bool big_slope = false;
    if (p1.x < p2.x)
    {
		x0 = p1.x;
		x1 = p2.x;
    }
    else
    {
        x0 = p2.x;
        x1 = p1.x;
    }
    if (p1.y < p2.y)
    {
        y0 = p1.y;
        y1 = p2.y;
    }
    else
    {
        y0 = p2.y;
        y1 = p1.y;
    }
    if (x1 - x0 < y1 - y0)
        big_slope = true;
    x0 = max(0, x0);
    x1 = min(screen_zone.w - 1, x1);
    y0 = max(0, y0);
    y1 = min(screen_zone.h - 1, y1);
    for (int x = x0; x <= x1; x++)
    {
        int y = a * x + b;
        if (big_slope)
        {
            for (int i = x - (w - 1) / 2; i <= x + (w - 1) / 2; i++)
            {
                if(i >= x0 && i <= x1)
                    pixel[y * screen_zone.w + i] = clr;
            }
        }
        else
        {
            for (int i = y - (w - 1) / 2; i <= y + (w - 1) / 2; i++)
            {
                if(i >= y0 && i <= y1)
                    pixel[i * screen_zone.w + x] = clr;
            }
        }
    }
}

void draw_rect_mesh(uint32 clr, int wd, int hd)
{
    uint32* pixel = (uint32*)bmp_memory;
    for (int y = 0; y < screen_zone.h; y+= hd)
        for (int x = 0; x < screen_zone.w; x++)
			pixel[y * screen_zone.w + x] = clr;

	for (int x = 0; x < screen_zone.w; x += wd)
        for (int y = 0; y < screen_zone.h; y ++)
			pixel[y * screen_zone.w + x] = clr;
}

void draw_diag_mesh(uint32 clr, int d0)
{
    int y = screen_zone.h ;
    int x = screen_zone.w ;
    int x_step = d0 * double(screen_zone.w) / screen_zone.h;
    while (x >= 0 && y >= 0)
    {
        draw_line({x, 0}, {0, y}, 1, clr);
        x -= x_step;
        y -= d0;
    }
    y = 0;
    x = 0;
    while (x < screen_zone.w && y < screen_zone.h)
    {
        draw_line({ x, screen_zone.h }, { screen_zone.w, y }, 1, clr);
        x += x_step;
        y += d0;
    }
}

void draw_im(image im, bool to_blit = false)
{
    uint32* pixel = (uint32*)bmp_memory;
    uint32 tr_mask= 0xff000000;

    int x0 = max(0, im.scr_zone.x);
    int y = max(0, im.scr_zone.y);
    int x_max = min(screen_zone.w, im.scr_zone.x + im.w);
    int y_max  = min(screen_zone.h, im.scr_zone.y + im.h);
    
    for (; y < y_max; y++)
    {
        for (int x = x0 ; x < x_max; x++)
        {
            uint32 p = im.pixels[(y - im.scr_zone.y) * im.w + x - im.scr_zone.x];
            if (p & tr_mask)
            {
                uint32 Red =   p & 0x000000ff;
                uint32 Green = p & 0x0000ff00;
                uint32 Blue =  p & 0x00ff0000;
                pixel[y * screen_zone.w + x] = (Red << 16) | Green | (Blue >> 16);
            } 
        }
    }
}

void instrch_from_im(image src_im, recta src_zone, recta dest_zone, bool to_color = false, uint32 color = black, recta limits = { -1,-1,INT_MAX,INT_MAX })
{
    uint32* pixel = (uint32*)bmp_memory;
    uint32 tr_mask = 0xff000000;
    
    int y = max(0, max(dest_zone.y, limits.y));
    int x0 = max(0, max(dest_zone.x, limits.x));
    int y_max = min(screen_zone.h, min(dest_zone.y + dest_zone.h, limits.y + limits.h));
    int x_max = min(screen_zone.w, min(dest_zone.x + dest_zone.w, limits.x + limits.w));
    double strch_y = double(src_zone.h - 1) / (dest_zone.h - 1);
    double strch_x = double(src_zone.w - 1) / (dest_zone.w - 1);
    for ( ; y < y_max ; y++)
    {
        for (int x = x0; x < x_max; x++)
        {
            int src_y = (y - dest_zone.y) * strch_y + src_zone.y;
            int src_x = (x - dest_zone.x) * strch_x + src_zone.x;

            uint32 p = src_im.pixels[src_y * src_im.w + src_x];
            if (p & tr_mask)
            {
                if(!to_color)
                {
                    uint32 Red = p & 0x000000ff;
                    uint32 Green = p & 0x0000ff00;
                    uint32 Blue = p & 0x00ff0000;
                    pixel[y * screen_zone.w + x] = (Red << 16) | Green | (Blue >> 16);
                }
                else
                    pixel[y * screen_zone.w + x] = color;

            }
        }
    }
}

void draw_from_im(image src_im, recta src_zone, point pos)
{
    uint32* pixel = (uint32*)bmp_memory;
    uint32 tr_mask = 0xff000000;

    int x0 = max(0, pos.x);
    int y = max(0, pos.y);
    int x_max = min(screen_zone.w, pos.x + src_zone.w);
    int y_max = min(screen_zone.h, pos.y + src_zone.h);

    for (; y < y_max; y++)
    {
        for (int x = x0; x <  x_max; x++)
        {
            int src_y = y - pos.y + src_zone.y;
            int src_x = x - pos.x + src_zone.x;

            uint32 p = src_im.pixels[src_y * src_im.w + src_x];
            if (p & tr_mask)
            {
                uint32 Red = p & 0x000000ff;
                uint32 Green = p & 0x0000ff00;
                uint32 Blue = p & 0x00ff0000;
                pixel[y * screen_zone.w + x] = (Red << 16) | Green | (Blue >> 16);
            }
        }
    }
}

void draw_num(char num, recta dest_zone)
{
    int num_idx = num - '0';
    recta num_zone = { num_idx * src_num_width, 0, src_num_width, src_num_height };
    instrch_from_im(num_letters, num_zone, dest_zone);
}

bool in_zone(const point& m_coords, const recta& zone)
{
    int x = m_coords.x;
    int y = m_coords.y;
    if (x > zone.x && x < zone.x + zone.w && y > zone.y && y < zone.y + zone.h)
        return true;
    return false;
}

void draw_text(const wchar_t* my_text, point pos, point char_size, bool to_color = false, uint32 color = black, recta limits = { -1,-1,INT_MAX,INT_MAX })
{

    recta crc_src_zone = {0 , 16, letter_w, letter_h };
    recta crc_dest_zone = {0 , pos.y, char_size.x, char_size.y };
    for (int i = 0; my_text[i] != L'\0'; i++)
    {
        char c = my_text[i];
        if (c == L'.')
        {
            draw_color_to_zone(black, { pos.x + i * char_size.x + char_size.x/3 , pos.y + 5*char_size.y/6,char_size.x/3, char_size.y/6 }, limits);
        }
        else if (c != L' ')
        {
            if (islower(c))
                c = toupper(c);
            int index = c - L'A';
            crc_src_zone.x = index * letter_w;
            crc_dest_zone.x = pos.x + i * char_size.x;
            instrch_from_im(num_letters, crc_src_zone, crc_dest_zone, to_color, color, limits);
        }
    }
}

void draw_text_to_zone(const wchar_t* my_text, recta dest_zone,bool to_color = false, uint32 color = black)
{
    int crc_nbr = 0;
    while (my_text[crc_nbr] != L'\0')
        crc_nbr++;
    int dest_crc_w = dest_zone.w / crc_nbr;
    int dest_crc_h = dest_zone.h;
    
    recta crc_src_zone = {0 , 16, letter_w, letter_h };
    recta crc_dest_zone = {0 , dest_zone.y, dest_crc_w, dest_crc_h };
    for (int i = 0; i < crc_nbr; i++)
    {
        char c = my_text[i];
        if (c != L' ')
        {
            if (islower(c))
                c = toupper(c);
            int index = c - L'A';
            crc_src_zone.x = index * letter_w;
            crc_dest_zone.x = dest_zone.x + i * dest_crc_w;
            instrch_from_im(num_letters, crc_src_zone, crc_dest_zone, to_color, color);
        }
    }
}
void draw_sim_vel()
{
    point mul_sign_pos = { intens_slct.scr_zone.x + intens_slct.scr_zone.w + 1, intens_slct.scr_zone.y + intens_slct.scr_zone.h - mul_sign_zone.h };
    instrch_from_im(num_letters, mul_sign_zone, { mul_sign_pos.x, mul_sign_pos.y, 9, 9 });

    recta vel_zone = { mul_sign_pos.x + mul_sign_zone.w + 1, mul_sign_pos.y + mul_sign_zone.h - 14, 36, 14 };
    int crc_w = vel_zone.w / 3;
    draw_num(char(int(sim_freq) + '0'), { vel_zone.x, vel_zone.y, crc_w, vel_zone.h });
    draw_from_im(num_letters, dot_zone, { vel_zone.x + crc_w , vel_zone.y + vel_zone.h - dot_zone.h });
    draw_num(int(sim_freq * 10) % 10 + '0', { vel_zone.x + crc_w + dot_zone.w , vel_zone.y, crc_w, vel_zone.h });
}

void draw_actual_step()
{
    draw_text_cntrd_fit(cascadia_face, L"Actual Step", actual_step_zone, {0,-actual_step_zone.h,0,0});

    int nb_digits = 0;
    std::wstring act_step_str;
    int nbr = actual_step / pow(10, nb_digits);
    do
    {
        act_step_str = std::to_wstring(nbr % 10) + act_step_str;
        nb_digits++;
        nbr = actual_step / pow(10, nb_digits);
    } while (nbr != 0);
    int dig_init_pos_x = actual_step_zone.x + actual_step_zone.w / 2 - nb_digits * num_width / 2;
    recta str_zone = { actual_step_zone.x + actual_step_zone.w/6 , actual_step_zone.y + 3* actual_step_zone.h / 2, actual_step_zone.w - actual_step_zone.w / 3, 2.80 * actual_step_zone.h };
    draw_text_cntrd(cascadia_face, act_step_str, str_zone, {0, str_zone.h/12,0, str_zone.h/12 });
    draw_out_recta(str_zone, orange, 3);
}

void one_step_ahead()
{   
    int j0 = max(1, stepping_limits.top - 1);
    int i0 = max(1, stepping_limits.left - 1);
    int j_max = min(stepping_limits.bottom + 1, nb_cell_h - 2);
    int i_max = min(stepping_limits.right + 1, nb_cell_w - 2);
    for (int j = j0; j <= j_max; j++)
    {
        for (int i = i0; i <= i_max; i++)
        {  
            int neigh_alive = 0;
            neigh_alive += grid[(j - 1) * nb_cell_w + i - 1];
            neigh_alive += grid[j * nb_cell_w + i - 1];
            neigh_alive += grid[(j + 1) * nb_cell_w + i - 1];
            neigh_alive += grid[(j - 1) * nb_cell_w + i];
            neigh_alive += grid[(j + 1) * nb_cell_w + i];
            neigh_alive += grid[(j - 1) * nb_cell_w + i + 1];
            neigh_alive += grid[j * nb_cell_w + i + 1];
            neigh_alive += grid[(j + 1) * nb_cell_w + i + 1];

            if (grid[j * nb_cell_w + i])
            {
                if (neigh_alive < 2 || neigh_alive > 3)
                    futur_grid[j * nb_cell_w + i] = 0;
                else
                    futur_grid[j * nb_cell_w + i] = 1;
            }
            else
            {
                if (neigh_alive == 3)
                {
                    futur_grid[j * nb_cell_w + i] = 1;
                    if (i < stepping_limits.left)
                        stepping_limits.left = i;
                    if (i > stepping_limits.right)
                        stepping_limits.right = i;
                    if (j < stepping_limits.top)
                        stepping_limits.top = j;
                    if (j > stepping_limits.bottom)
                        stepping_limits.bottom = j;
                }
                else
                    futur_grid[j * nb_cell_w + i] = 0;

            }
        }
    }
    uint32* temp;
    temp = grid;
    grid = futur_grid;
    futur_grid = temp;
    actual_step++;
}

point get_clk_cell(POINT m_coords)
{
    int i = (m_coords.x/zoom_ratio_x + bmp_zone.x) / cell_w;
    int j = (m_coords.y/zoom_ratio_y + bmp_zone.y) / cell_h;
    return { i, j };
}
int get_step_int()
{
    int result = 0;
    for (int i = 0; i < step_edit.str.size(); i++)
        result += int(step_edit.str[i] - '0') * pow(10, step_edit.str.size() - i - 1);
    return result;
}

void reinit_grid()
{
    for (int i = 0; i <nb_cell_w; i++)
        for (int j = 0; j < nb_cell_h; j++)
        {
            grid[j * nb_cell_w + i] = 0;
            futur_grid[j * nb_cell_w + i] = 0;
        }
    for (auto square : grid_init_stt)
        grid[square.y * nb_cell_w + square.x] = 1;

    stepping_limits = initial_limits;
}

void collect_dlg::draw()
{
    recta title_zone = { zone.x, zone.y, zone.w,  title_h };
    draw_color_to_zone(ll_grey, zone);
    draw_out_recta(zone, d_blue, 6);
    draw_color_to_zone(l_orange, title_zone);
    draw_text_ft(cascadia_face, title, title_zone, screen_zone, { title_zone.w / 30, 0, 0, 0 });
    
    for (int i = 0; i < txts.size(); i++)
    {
        draw_text_ft(cascadia_face, txts[i], txt_zones[i]);
    }
    for(int i = 0; i < txt_edits.size(); i++)
    {
        draw_out_recta(txt_edits[i].zone, grey, 3);
    }
    draw_out_recta(ok_zone, black, 2);
    draw_butt(ok_zone, L"OK", ok_zone.w / 3);
}

void info_dlg::draw()
{
    recta title_zone = { zone.x, zone.y, zone.w,  title_h};
    draw_color_to_zone(ll_grey, zone);
    draw_out_recta(zone, d_blue, 6);
    draw_color_to_zone(l_orange, title_zone);
    draw_text_ft(cascadia_face, title, title_zone, screen_zone, {title_zone.w /30, 0, 0, 0});

    draw_txt(cascadia_face);
    draw_out_recta({ zone.x + txt_margins.x - char_h/3 , zone.y + title_h + txt_margins.y  - char_h/4, zone.w - txt_margins.x - txt_margins.w + 2*char_h/3, txt_h + char_h/2}, black, 2);
    
    draw_out_recta(ok_zone, black, 2);
    draw_butt(ok_zone, L"OK", ok_zone.w / 3);
}

void opn_sav_dlg::go_curr_dir()
{
    files_q.clear();
    if (in_root)
    {
        wchar_t buff[200];
        int actl_size = GetLogicalDriveStringsW(200, buff);
        for (int i = 0; i < actl_size; i++)
        {
            int j = i;
            while (j < 200 && buff[j])
                j++;
            files_q.push_front({ std::wstring(&buff[i], &buff[j]), true });
            i = j;
        }
    }
    else
    {
        WIN32_FIND_DATAW ffd;
        HANDLE h_find = INVALID_HANDLE_VALUE;
        h_find = FindFirstFileW(curr_dir.c_str(), &ffd);
        if (h_find == INVALID_HANDLE_VALUE)
        {
            std::cout << "have to add a dialog box mentioning that we could not open the file, and ask to retry. "; // TODO
        }
        do
        {
            if (wcscmp(ffd.cFileName, L"..") == 0 || wcscmp(ffd.cFileName, L".") == 0)
                continue;
            if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                files_q.push_front({ ffd.cFileName, true });
            else
            {
                for(int i = 0; i < allowed_ext.size(); i++)
                {
                    if (check_ext(ffd.cFileName, allowed_ext[i]))
                    {
                        files_q.push_back({ ffd.cFileName, false });
                        break;
                    }
                }
            }
        } while (FindNextFileW(h_find, &ffd));
        files_q.push_front({ L"..", true });
    }
    nb_files = files_q.size();
    // adapt zone to presence of scroll and update scrler size
    scrl.scrler_zone.y = scrl.zone.y;
    fhref = exp_zone.y;
    scrl.scrler_zone.h = scrl.zone.h;
    if (nb_files * file_h > exp_zone.h)
    {
        scrl_exist = true;
        exp_zone = exp_zone_scrl;
        scrl.scrler_zone.h = scrl.zone.h - (file_h * (nb_files + 2) - exp_zone.h) / 3;

        if (scrl.scrler_zone.h < scrl.zone.h / 30)
        {
            scrl.scrler_zone.h = scrl.zone.h / 30;
            scrl.ratio = double(file_h * (nb_files + 2) - exp_zone.h) / (scrl.zone.h - scrl.scrler_zone.h);
        }
        else
            scrl.ratio = 3;
    }
    else
    {
        scrl_exist = false;
        exp_zone = exp_zone_wscrl;
    }

    fslctd_idx = -1;
}

void opn_sav_dlg::go_slctd_dir()
{
    if (!in_root)
    {
        if (fslctd_idx == 0)
        {
            size_t bslash_idx = curr_dir.find_last_of(L"/\\", curr_dir.size() - 3);
            // no backslash found means we are in drive directory, and should go back beyond that
            if (bslash_idx == std::string::npos)
                in_root = true;
            else
                curr_dir = curr_dir.substr(0, bslash_idx + 1) + L'*';
        }
        else if (files_q[fslctd_idx].second)
        {
            curr_dir = curr_dir.substr(0, curr_dir.size() - 1) + files_q[fslctd_idx].first + L"\\*";
        }
    }
    else {
        curr_dir = files_q[fslctd_idx].first + L"*";
        in_root = false;
    }
    go_curr_dir();
}

void load_from_file(std::wstring file_name)
{
    std::wstring file_path;
    file_path = opnf_dlg.curr_dir.substr(0, opnf_dlg.curr_dir.size() - 1) + file_name;
        
    bool f_error = false;
    std::wifstream my_file;
    my_file.open(file_path);
    grid_init_stt.clear();
    if (my_file.is_open())
    {
        stepping_limits.top = nb_cell_h;
        stepping_limits.left = nb_cell_w;
        stepping_limits.right = 0;
        stepping_limits.bottom = 0;
        uint32 k;
        for (int i = 0; i < nb_cell_w; i++)
        {
            for (int j = 0; j < nb_cell_h; j++)
            {
                my_file >> k;
                if (k != 0 && k != 1)
                {
                    f_error_dlg = true;
                    draw_diag_mesh(grey, 5);
                    break;
                }
                grid[j * nb_cell_w + i] = (k != 0);
                futur_grid[j * nb_cell_w + i] = 0;

                if (k)
                {
                    if (i < stepping_limits.left)
                        stepping_limits.left = i;
                    else if (i > stepping_limits.right)
                        stepping_limits.right = i;
                    if (j < stepping_limits.top)
                        stepping_limits.top = j;
                    else if (j > stepping_limits.bottom)
                        stepping_limits.bottom = j;
                    grid_init_stt.push_back(point{ i,j });
                }
            }
            if (f_error_dlg)
                break;
        }
        stepping_limits = initial_limits;
        my_file >> k;
    }
    if (!my_file.eof())
    {
        f_error_dlg = true;
        draw_diag_mesh(grey, 5);
        stepping_limits = initial_limits;
    }
    else
    {
        normal_mode = true;
        my_cursor.scr_zone = { step_edit.txt_zone.x, step_edit.txt_zone.y , 2, step_edit.txt_zone.h };
        step_edit.chars_x.clear();
        step_edit.str.clear();
        step_edit.chars_x.push_back(my_cursor.scr_zone.x);
        step_edit.dx_ref = 0;
    }
    open_file = false;
    actual_step = 0;
    my_file.close();
}

void make_inside(recta& zone, const recta& limits)
{
    if (zone.x < limits.x)
        zone.x = limits.x;
    if (zone.x + zone.w > limits.x + limits.w)
        zone.x = limits.x + limits.w - zone.w;
    if (zone.y < limits.y)
        zone.y = limits.y;
    if (zone.y + zone.h > limits.y + limits.h)
        zone.y = limits.y + limits.h - zone.h;
}

void make_inside(r_recta& zone, const recta& limits)
{
    if (zone.x < limits.x)
        zone.x = limits.x;
    if (zone.x + zone.w > limits.x + limits.w)
        zone.x = limits.x + limits.w - zone.w;
    if (zone.y < limits.y)
        zone.y = limits.y;
    if (zone.y + zone.h > limits.y + limits.h)
        zone.y = limits.y + limits.h - zone.h;
}

// assures first tat a >= low then that a <= up, could pass through both and order is important
void make_inside(int& a, const int& low, const int& up)
{
    if (a < low)
        a = low;
    if (a > up)
        a = up;
}

void make_inside(double& a, const int& low, const int& up)
{
    if (a < low)
        a = low;
    if (a > up)
        a = up;
}

void add_constant(std::vector<int>& vec, int start_idx, int end_idx, int val)
{
    if (vec.size() == 0)
        return;
    if (start_idx < 0 || end_idx > vec.size() || end_idx <= start_idx)
        return;
    for (int i = start_idx; i < end_idx; i++)
        *(vec.begin() + i) += val;
}
int  get_sum(std::vector<int>& vec, int start_idx, int end_idx)
{
    int res = 0;
    for (int i = start_idx; i < end_idx; i++)
        res += vec[i];
    return res;
}
void load_glyphs(FT_Face& face, int h)
{
    int pixel_size = h * face->units_per_EM / face->height;
    FT_Set_Pixel_Sizes(face, 0, pixel_size);
    face_chars[h].glyphs = new std::unordered_map<uint16, character>[0xFFF]; // TODO : destroy this after finishing
    for (uint16 i = 0; i < 0xFFF; i++)
    {
        int glyph_idx = FT_Get_Char_Index(face, i);
        if (FT_Load_Glyph(face, glyph_idx, FT_LOAD_RENDER))
        {
            return;
        }
        character& cc = (*face_chars[h].glyphs)[i];
        cc.w = face->glyph->bitmap.width;
        cc.h = face->glyph->bitmap.rows;
        cc.bearing_x = face->glyph->bitmap_left;
        cc.bearing_y = face->glyph->bitmap_top;
        cc.advance = face->glyph->advance.x;
        face_chars[h].max_ascender = max(face_chars[h].max_ascender, cc.bearing_y);
        face_chars[h].max_descender = max(face_chars[h].max_descender, cc.h - cc.bearing_y);
        cc.pixels = new uint32[cc.w * cc.h];

        for (int j = 0; j < cc.h; j++)
        {
            for (int k = 0; k < cc.w; k++)
            {
                uint8 grey_lvl = face->glyph->bitmap.buffer[j * cc.w + k];
                cc.pixels[j * cc.w + k] = (grey_lvl << 16) | (grey_lvl << 8) | grey_lvl;
                cc.max_intens = max(cc.max_intens, grey_lvl);
            }
        }
    }
}

uint32 do_alpha_blending(const uint8& glvl, const uint32& bgn_pix, const uint32& color)
{
    double alpha = glvl / 255.0;
    uint8 bgn_red = bgn_pix >> 16;
    uint8 bgn_green = (bgn_pix >> 8) & 0x000000ff;
    uint8 bgn_blue = (bgn_pix & 0x000000ff);

    uint8 color_red = color >> 16;
    uint8 color_green = (color >> 8) & 0x000000ff;
    uint8 color_blue = (color & 0x000000ff);

    uint16 red   = alpha * color_red + (1 - alpha) * bgn_red;
    uint16 green = alpha * color_green + (1 - alpha) * bgn_green;
    uint16 blue  = alpha * color_blue  + (1 - alpha) * bgn_blue;
    red   = (red > 255) ? 255 : red;
    green = (green > 255) ? 255 : green;
    blue  = (blue > 255) ? 255 : blue;

    return (red << 16) | (green << 8) | blue;
}
//TODO : add margin_up effect
void draw_text_ft(FT_Face& face, const std::wstring& str, const recta& zone, const recta& limits, const recta& margins, const int& dx_ref, const uint32& color) // margins = {left, up, right, bottom}
{
    uint32* pixel = (uint32*)bmp_memory;
    int txt_h = zone.h - margins.y - margins.h;
    if (!face_chars[txt_h].glyphs)
        load_glyphs(face, txt_h);

    int d = ((zone.y + margins.y + face_chars[txt_h].max_ascender) + (zone.y + zone.h - margins.h - face_chars[txt_h].max_descender)) / 2;
    int txt_pos_x = zone.x + margins.x;
    for (int i = 0; i < str.size(); i++)
    {
        
        uint16 c = str[i];
        character cc = (*face_chars[txt_h].glyphs)[c];
        int start_x = txt_pos_x - dx_ref + cc.bearing_x;
        if (start_x > limits.x + limits.w)
            break;
        if (start_x + cc.w  < limits.x)
        {
            txt_pos_x += cc.advance / 64 ;
            continue;
        }
        int start_y = d - cc.bearing_y;

        int x0 = max(0, max(start_x, limits.x));
        int y = max(0, max(start_y, limits.y));
        int x_max = min(screen_zone.w, min(start_x + cc.w , min(limits.x + limits.w, zone.x + zone.w - margins.w)));
        int y_max = min(screen_zone.h, min(start_y + cc.h, min(limits.y + limits.h, zone.y + zone.h - margins.h)));

        for (; y < y_max; y++)
        {
            for (int x = x0; x < x_max; x++)
            {
                int x_src = x - start_x;
                int y_src = y - start_y;
                uint32 p = cc.pixels[y_src * cc.w + x_src];
                uint8 glvl = p >> 16;
                pixel[y * screen_zone.w + x] = do_alpha_blending(glvl, pixel[y * screen_zone.w + x], color);
            }
        }
        txt_pos_x += cc.advance / 64;
    }
}

void draw_text_cntrd_fit(FT_Face& face, const std::wstring& str, const recta& zone, const recta& margins, const uint32& color)
{
    uint32* pixel = (uint32*)bmp_memory;
    int txt_h = zone.h - margins.y - margins.h;
    if (!face_chars[txt_h].glyphs)
        load_glyphs(face, txt_h);

    int txt_w = 0;
    double w_scale = 1;
    
    for (int i = 0; i < str.size(); i++)
    {
        uint16 c = str[i];
        character cc = (*face_chars[txt_h].glyphs)[c];
        txt_w += cc.advance / 64;
    }
    w_scale = double((zone.w - margins.x - margins.w)) / txt_w;

    int d = ((zone.y + margins.y + face_chars[txt_h].max_ascender) + (zone.y + zone.h - margins.h - face_chars[txt_h].max_descender)) / 2;
    double txt_pos_x = zone.x + margins.x;
    for (int i = 0; i < str.size(); i++)
    {

        uint16 c = str[i];
        character cc = (*face_chars[txt_h].glyphs)[c];
        int start_x = txt_pos_x + cc.bearing_x * w_scale;

        int start_y = d - cc.bearing_y;

        int x0 = max(0, start_x);
        int y = max(0, start_y);
        int x_max = min(screen_zone.w, txt_pos_x + (cc.bearing_x + cc.w) * w_scale);
        int y_max = min(screen_zone.h, start_y + cc.h);

        for (; y < y_max; y++)
        {
            for (int x = x0; x < x_max; x++)
            {
                int x_src = (x - start_x) / w_scale;
                int y_src = y - start_y;
                uint32 p = cc.pixels[y_src * cc.w + x_src];
                uint8 glvl = p >> 16;
                pixel[y * screen_zone.w + x] = do_alpha_blending(glvl, pixel[y * screen_zone.w + x], color);
            }
        }
        txt_pos_x += cc.advance * w_scale / 64;
    }
}

void draw_text_cntrd(FT_Face& face, const std::wstring& str, const recta& zone, const recta& margins, const uint32& color)
{
    uint32* pixel = (uint32*)bmp_memory;
    int txt_h = zone.h - margins.y - margins.h;
    if (!face_chars[txt_h].glyphs)
        load_glyphs(face, txt_h);

    int txt_w = 0;

    for (int i = 0; i < str.size(); i++)
    {
        uint16 c = str[i];
        character cc = (*face_chars[txt_h].glyphs)[c];
        txt_w += cc.advance / 64;
    }
    
    int d = ((zone.y + margins.y + face_chars[txt_h].max_ascender) + (zone.y + zone.h - margins.h - face_chars[txt_h].max_descender)) / 2;
    int txt_pos_x = zone.x + (zone.w - txt_w)/2;
    for (int i = 0; i < str.size(); i++)
    {

        uint16 c = str[i];
        character cc = (*face_chars[txt_h].glyphs)[c];
        int start_x = txt_pos_x + cc.bearing_x;
        
        int start_y = d - cc.bearing_y;
        int x0 = max(0, start_x);
        int y = max(0, start_y);
        int x_max = min(screen_zone.w, start_x + cc.w );
        int y_max = min(screen_zone.h, start_y + cc.h);

        for (; y < y_max; y++)
        {
            for (int x = x0; x < x_max; x++)
            {
                int x_src = (x - start_x);
                int y_src = y - start_y;
                uint32 p = cc.pixels[y_src * cc.w + x_src];
                uint8 glvl = p >> 16;
                pixel[y * screen_zone.w + x] = do_alpha_blending(glvl, pixel[y * screen_zone.w + x], color);
            }
        }
        txt_pos_x += cc.advance / 64;
    }
}

void info_dlg::draw_txt(FT_Face& face, const uint32& color)
{
    uint32* pixel = (uint32*)bmp_memory;
    recta txt_zone = { zone.x + txt_margins.x, zone.y + title_h + txt_margins.y, zone.w - txt_margins.x - txt_margins.w, txt_h};
    
    int txt_w = 0;
    int d = ((txt_zone.y + face_chars[char_h].max_ascender) + (txt_zone.y + char_h - face_chars[char_h].max_descender)) / 2;
    int txt_pos_x = txt_zone.x;
    int k = 0;
    for (int i = 0; i < txt.size(); i++)
    {
        if (i == nxt_line_idxs[k])
        {
            txt_pos_x = txt_zone.x;
            txt_zone.y += char_h;
            d = ((txt_zone.y + face_chars[char_h].max_ascender) + (txt_zone.y + char_h - face_chars[char_h].max_descender)) / 2;
            k++;
            i--;
            continue;
        }
        uint16 c = txt[i];
        character cc = (*face_chars[char_h].glyphs)[c];
        int start_x = txt_pos_x + cc.bearing_x;

        int start_y = d - cc.bearing_y;
        int x0 = max(0, start_x);
        int y = max(0, start_y);
        int x_max = min(screen_zone.w, start_x + cc.w);
        int y_max = min(screen_zone.h, start_y + cc.h);

        for (; y < y_max; y++)
        {
            for (int x = x0; x < x_max; x++)
            {
                int x_src = (x - start_x);
                int y_src = y - start_y;
                uint32 p = cc.pixels[y_src * cc.w + x_src];

                uint8 glvl = p >> 16;
                pixel[y * screen_zone.w + x] = do_alpha_blending(glvl, pixel[y * screen_zone.w + x], color);
            }
        }
        txt_pos_x += cc.advance / 64;
    }
}

void update_wnd(HDC device_context, RECT* client_rect)
{
    int client_w = client_rect->right - client_rect->left;
    int client_h = client_rect->bottom - client_rect->top;
    StretchDIBits(device_context,
        0, 0, client_w, client_h,
        0, 0, screen_zone.w, screen_zone.h,
        bmp_memory,
        &bmp_info,
        DIB_RGB_COLORS, SRCCOPY);
}

void opn_sav_dlg::handle_ldown()
{
    if (in_zone(mouse_coords_ldown, exp_zone))
    {
        int idx_file = ((exp_zone.y - fhref) + (mouse_coords.y - exp_zone.y)) / file_h;
        if (idx_file < nb_files)
            fslctd_idx = idx_file;
        else
            fslctd_idx = -1;
    }
    else
    {
        if (scrl_exist)
        {
            if (in_zone(mouse_coords_ldown, scrl.up_zone))
            {
                int d1 = ((exp_zone.y - fhref) / file_h) * file_h;
                int d0 = exp_zone.y - fhref;
                if (d0 == d1)
                    fhref += file_h;
                else
                    fhref += (d0 - d1);
                make_inside(fhref, exp_zone.y + exp_zone.h - (nb_files + 2) * file_h, exp_zone.y);
                scrl.scrler_zone.y = scrl.zone.y + (exp_zone.y - fhref) / scrl.ratio;
                scrl.timer.reset();
            }
            else if (in_zone(mouse_coords_ldown, scrl.down_zone))
            {
                int d1 = ((exp_zone.y - fhref + exp_zone.h) / file_h) * file_h;
                int d0 = exp_zone.y - fhref + exp_zone.h;
                if (d0 == d1)
                    fhref -= file_h;
                else
                    fhref -= file_h - (d0 - d1);
                make_inside(fhref, exp_zone.y + exp_zone.h - (nb_files + 2) * file_h, exp_zone.y);
                scrl.scrler_zone.y = scrl.zone.y + (exp_zone.y - fhref) / scrl.ratio;
                scrl.timer.reset();
            }
            else if (in_zone(mouse_coords_ldown, scrl.scrler_zone))
            {
                scrl.drag = true;
                scrl.y0 = scrl.scrler_zone.y;
            }
            else if (in_zone(mouse_coords_ldown, scrl.zone))
            {
                int dir_sign = (scrl.scrler_zone.y - mouse_coords_ldown.y > 0) ? -1 : 1;
                scrl.scrler_zone.y += dir_sign * scrl.scrler_zone.h * 0.7;
                make_inside(scrl.scrler_zone.y, scrl.zone.y, scrl.zone.y + scrl.zone.h - scrl.scrler_zone.h);
                fhref = exp_zone.y - (scrl.scrler_zone.y - scrl.zone.y) * scrl.ratio;
                scrl.timer.reset();
                scrl.attrct = true;
                scrl.dir = (scrl.scrler_zone.y - mouse_coords_ldown.y > 0) ? -1 : 1;
            }
            else 
            {
                fslctd_idx = -1;
            }
        }
        else if (!in_zone(mouse_coords_ldown, open_zone))
            fslctd_idx = -1;
    }
}
void opn_sav_dlg::handle_scrl_ldblclk()
{
    if (scrl_exist)
    {
        if (in_zone(mouse_coords_ldown, scrl.up_zone))
        {
            fhref += file_h;
            make_inside(fhref, exp_zone.y + exp_zone.h - (nb_files + 2) * file_h, exp_zone.y);
            scrl.scrler_zone.y = scrl.zone.y + (exp_zone.y - fhref) / scrl.ratio;
            scrl.timer.reset();
        }
        if (in_zone(mouse_coords_ldown, scrl.down_zone))
        {
            fhref -= file_h;
            make_inside(fhref, exp_zone.y + exp_zone.h - (nb_files + 2) * file_h, exp_zone.y);
            scrl.scrler_zone.y = scrl.zone.y + (exp_zone.y - fhref) / scrl.ratio;
            scrl.timer.reset();
        }
        if (in_zone(mouse_coords_ldown, scrl.zone) && !in_zone(mouse_coords_ldown, scrl.scrler_zone))
        {
            int dir_sign = (scrl.scrler_zone.y - mouse_coords_ldown.y > 0) ? -1 : 1;
            scrl.scrler_zone.y += dir_sign * scrl.scrler_zone.h * 0.7;
            make_inside(scrl.scrler_zone.y, scrl.zone.y, scrl.zone.y + scrl.zone.h - scrl.scrler_zone.h);
            fhref = exp_zone.y - (scrl.scrler_zone.y - scrl.zone.y) * scrl.ratio;
            scrl.timer.reset();
            scrl.attrct = true;
            scrl.dir = (scrl.scrler_zone.y - mouse_coords_ldown.y > 0) ? -1 : 1;;
        }
    }
}

void opn_sav_dlg::handle_mw(int delta)
{
    if (scrl_exist)
    {
        int dir = delta > 0 ? 1 : -1;
        fhref += dir * 2 * file_h;
        make_inside(fhref, exp_zone.y + exp_zone.h - (nb_files + 2) * file_h, exp_zone.y);
        scrl.scrler_zone.y = scrl.zone.y + (exp_zone.y - fhref) / scrl.ratio;
    }
}
bool txt_edit_params::handle_ldown()
{
    if(in_zone(mouse_coords_ldown, zone))
    {
        show_cursor = true;
        cursor_timer.reset();
        slct_mode = true;
        my_cursor.scr_zone.x = mouse_coords_ldown.x;
        curs_idx = get_char_idx();
        if (chars_x[curs_idx] - dx_ref < txt_zone.x)
            dx_ref -= txt_zone.x - (chars_x[curs_idx] - dx_ref);
        if (chars_x[curs_idx] + my_cursor.scr_zone.w - dx_ref > txt_zone.x + txt_zone.w)
            dx_ref += (chars_x[curs_idx] + my_cursor.scr_zone.w - dx_ref) - (txt_zone.x + txt_zone.w);
        my_cursor.scr_zone.x = chars_x[curs_idx] - dx_ref;
        my_cursor.scr_zone.y = txt_zone.y;
        my_cursor.scr_zone.h = txt_zone.h;
        slct_idx = curs_idx;
        return true;
    }
    show_cursor = false;
    slct_idx = curs_idx;
    return false;
}

void txt_edit_params::add_char(uint16 my_char)
{
    if (slct_idx != curs_idx)
    {
        int min_i = min(slct_idx, curs_idx);
        int max_i = max(slct_idx, curs_idx);
        str.erase(str.begin() + min_i, str.begin() + max_i);
        int width_change = chars_x[max_i] - chars_x[min_i];
        chars_x.erase(chars_x.begin() + min_i, chars_x.begin() + max_i);
        add_constant(chars_x, min_i, chars_x.size(), -width_change);
        curs_idx = min_i;
    }
    cursor_timer.reset();
    if (str.size() < max_crcs)
    {
        str.insert(str.begin() + curs_idx, 1, my_char);
        chars_x.insert(chars_x.begin() + curs_idx, 1, chars_x[curs_idx]);
        int advance = (*face_chars[txt_zone.h].glyphs)[my_char].advance / 64;
        add_constant(chars_x, curs_idx + 1, chars_x.size(), advance);
        curs_idx++;
        if (chars_x[curs_idx] + my_cursor.scr_zone.w - dx_ref > txt_zone.x + txt_zone.w)
            dx_ref += (chars_x[curs_idx] + my_cursor.scr_zone.w - dx_ref) - (txt_zone.x + txt_zone.w);
        my_cursor.scr_zone.x = chars_x[curs_idx] - dx_ref;
        slct_idx = curs_idx;
    }
}

void txt_edit_params::handle_arrows(WPARAM wp)
{
    switch (wp)
    {
    case VK_LEFT:
    {
        if (GetKeyState(VK_SHIFT) & 0xf0)
        {
            if (curs_idx > 0)
                curs_idx--;
        }
        else
        {
            curs_idx = max(curs_idx - 1, 0);
            slct_idx = curs_idx;
        }

        if (chars_x[curs_idx] - dx_ref < txt_zone.x)
            dx_ref -= txt_zone.x - (chars_x[curs_idx] - dx_ref);
        my_cursor.scr_zone.x = chars_x[curs_idx] - dx_ref;
        cursor_timer.reset();
    }break;
    case VK_RIGHT:
    {
        if (GetKeyState(VK_SHIFT) & 0xf0)
        {
            if (curs_idx < str.size())
                curs_idx++;
        }
        else
        {
            curs_idx = min(curs_idx + 1, chars_x.size() - 1);
            slct_idx = curs_idx;
        }
        if (chars_x[curs_idx] + my_cursor.scr_zone.w - dx_ref > txt_zone.x + txt_zone.w)
            dx_ref += (chars_x[curs_idx] + my_cursor.scr_zone.w - dx_ref) - (txt_zone.x + txt_zone.w);
        my_cursor.scr_zone.x = chars_x[curs_idx] - dx_ref;
        cursor_timer.reset();
    }break;
    case VK_BACK:
    {
        if (slct_idx != curs_idx)
        {
            int min_i = min(slct_idx, curs_idx);
            int max_i = max(slct_idx, curs_idx);
            str.erase(str.begin() + min_i, str.begin() + max_i);
            int width_change = chars_x[max_i] - chars_x[min_i];
            chars_x.erase(chars_x.begin() + min_i, chars_x.begin() + max_i);
            add_constant(chars_x, min_i, chars_x.size(), -width_change);
            curs_idx = min_i;
            if (chars_x[curs_idx] - dx_ref < txt_zone.x)
            {
                dx_ref -= txt_zone.x - (chars_x[curs_idx] - dx_ref) + txt_zone.w / 2;
                dx_ref = max(0, dx_ref);
            }
            slct_idx = curs_idx;
            my_cursor.scr_zone.x = chars_x[curs_idx] - dx_ref;
        }
        else
        {
            if (curs_idx > 0)
            {
                curs_idx--;
                wchar_t del_char = str[curs_idx];
                str.erase(str.begin() + curs_idx);
                chars_x.erase(chars_x.begin() + curs_idx);
                character cc = (*face_chars[txt_zone.h].glyphs)[del_char];
                int advance = (*face_chars[txt_zone.h].glyphs)[del_char].advance / 64;
                add_constant(chars_x, curs_idx, chars_x.size(), -advance);

                if (chars_x[curs_idx] - dx_ref < txt_zone.x)
                {
                    dx_ref -= txt_zone.w / 2;
                    dx_ref = max(0, dx_ref);
                }
                my_cursor.scr_zone.x = chars_x[curs_idx] - dx_ref;
                slct_idx = curs_idx;
            }
        }
        cursor_timer.reset();
    }break;
    }
}

uint16 get_char_uni(WPARAM wp)
{
    BYTE KeyState[256];
    KeyState[VK_SHIFT] = GetKeyState(VK_SHIFT);
    KeyState[VK_CONTROL] = GetKeyState(VK_CONTROL);
    KeyState[VK_MENU] = GetKeyState(VK_MENU);
    KeyState[VK_CAPITAL] = GetKeyState(VK_CAPITAL);
    wchar_t my_chars[2];
    int nb_chars = ToUnicode(wp, 0, KeyState, my_chars, 2, 0);
    return my_chars[0];
}

LRESULT CALLBACK
wnd_callback(HWND wnd,
    UINT msg,
    WPARAM wp,
    LPARAM lp)
{
    LRESULT result = 0;
    static bool just_ldbclkd = false;
    switch (msg)
    {
    case WM_LBUTTONDOWN:
    {         
        my_mouse.l_down = true;
        mouse_coords_ldown.x = GET_X_LPARAM(lp);
        mouse_coords_ldown.y = GET_Y_LPARAM(lp);
        if(normal_mode)
        {
           
            step_edit.handle_ldown();
            if (in_zone(mouse_coords_ldown, { intens_slct.scr_zone.x, intens_slct.scr_zone.y, intens_slct.scr_zone.w, intens_slct.scr_zone.h }))
                int_bar_slctd = true;
            else if (in_zone(mouse_coords_ldown, go_butt_zone))
                go_butt_down = true;
            else if (in_zone(mouse_coords_ldown, play_butt[0].scr_zone))
                plpau_down = true;
            else if (in_zone(mouse_coords_ldown, reinit_butt[0].scr_zone))
                reinit_down = true;
            else if (in_zone(mouse_coords_ldown, next_butt[0].scr_zone))
                next_down = true;
            else if (show_map && in_zone(mouse_coords_ldown, region_slct_zone))
            {
                mov_region = true;
                int last_x = bmp_region_zone.x;
                int last_y = bmp_region_zone.y;
                
                // regulating the size adjusting for ratio problms
                bmp_region_zone.h = bmp_zone.h * map_cell_w / cell_h;
                bmp_region_zone.w = bmp_zone.w * map_cell_w / cell_w;
                bmp_region_zone.x = mouse_coords_ldown.x - bmp_region_zone.w / 2;
                bmp_region_zone.y = mouse_coords_ldown.y - bmp_region_zone.h / 2;
                make_inside(bmp_region_zone, region_slct_zone);
                bmp_zone.x += (bmp_region_zone.x - last_x) * cell_w / map_cell_w;
                bmp_zone.y += (bmp_region_zone.y - last_y) * cell_w / map_cell_w;
                region_pos0.x = bmp_region_zone.x;
                region_pos0.y = bmp_region_zone.y;
            }
        }
        else if(open_file)
        {
            opnf_dlg.handle_ldown();
        }
        else if (save_file)
        {   
            savf_dlg.handle_ldown();
            if (in_zone(mouse_coords_ldown, savf_dlg.exp_zone))
            {
                if (savf_dlg.fslctd_idx != -1 && !savf_dlg.files_q[savf_dlg.fslctd_idx].second)
                {
                    savf_dlg.txt_edit.copy_txt(savf_dlg.files_q[savf_dlg.fslctd_idx].first, cascadia_face);
                    savf_dlg.txt_edit.dx_ref = 0;
                }
            }
            savf_dlg.txt_edit.handle_ldown();
        }
        else if (gif_params_collect)
        {
            for(int i = 0; i < gif_params_dlg.txt_edits.size(); i++)
            {
                if (gif_params_dlg.txt_edits[i].handle_ldown())
                {
                    gif_params_dlg.slctd_edit = i;
                    break;
                }
            }
        }

    }break;
    case WM_LBUTTONUP:
    {
        my_mouse.l_down = false;
        if (!del_mouse_lup)
        {
            mouse_coords_lup.x = GET_X_LPARAM(lp);
            mouse_coords_lup.y = GET_Y_LPARAM(lp);
            int_bar_slctd = false;
            slct_mode = false;
            go_butt_down = false;
            plpau_down = false;
            reinit_down = false;
            next_down = false;
            mov_region = false;
            if (normal_mode)
            {
                if (!in_zone(mouse_coords, menu_zone) && !in_zone(mouse_coords_ldown, map_zone) && !in_zone(mouse_coords_ldown, sh_hide_zone))
                {
                    point down_cell = get_clk_cell(mouse_coords_ldown);
                    point up_cell = get_clk_cell(mouse_coords_lup);
                    if (down_cell == up_cell)
                    {
                        int i = up_cell.x;
                        int j = up_cell.y;

                        if (!grid[j * nb_cell_w + i])
                            grid_init_stt.push_back(point{ i, j });
                        else
                            grid_init_stt.erase(std::remove(grid_init_stt.begin(), grid_init_stt.end(), point({ i,j })), grid_init_stt.end());
                        if (i != 0 && i != nb_cell_w - 1 && j != 0 && j != nb_cell_h - 1)
                            grid[j * nb_cell_w + i] = grid[j * nb_cell_w + i] ? 0 : 1;
                        if (grid[j * nb_cell_w + i])
                        {
                            if (i < stepping_limits.left)
                                stepping_limits.left = i;
                            else if (i > stepping_limits.right)
                                stepping_limits.right = i;
                            if (j < stepping_limits.top)
                                stepping_limits.top = j;
                            else if (j > stepping_limits.bottom)
                                stepping_limits.bottom = j;
                        }
                        initial_limits= stepping_limits;
                    }
                }
                else if (in_zone(mouse_coords_ldown, play_butt[0].scr_zone) && in_zone(mouse_coords_lup, play_butt[0].scr_zone))
                {
                    if (sim_on)
                        prv_targ_stp = actual_step;
                    sim_on = !sim_on;
                }
                else if (in_zone(mouse_coords_ldown, go_butt_zone) && in_zone(mouse_coords_lup, go_butt_zone))
                {
                    target_step = get_step_int();
                    if(target_step != actual_step)
                    {
                        go_mode = true;
                        normal_mode = false;
                        if (target_step < prv_targ_stp || target_step < actual_step)
                        {
                            reinit_grid();
                            actual_step = 0;
                        }
                    }
                }
                else if (in_zone(mouse_coords_ldown, reinit_butt[0].scr_zone) && in_zone(mouse_coords_lup, reinit_butt[0].scr_zone))
                {
                    sim_on = false;
                    reinit_grid();
                    actual_step = 0;
                    target_step = 0;
                }
                else if (in_zone(mouse_coords_ldown, next_butt[0].scr_zone) && in_zone(mouse_coords_lup, next_butt[0].scr_zone))
                {
                    one_step_ahead();
                }
                else if (in_zone(mouse_coords_ldown, sh_hide_zone) && in_zone(mouse_coords_lup, sh_hide_zone))
                {
                    show_map = !show_map;
                }
                else if (in_zone(mouse_coords_ldown, save_butt_zone) && in_zone(mouse_coords_lup, save_butt_zone))
                {
                    save_file = true;
                    normal_mode = false;
                    sim_on = false;
                    get_exe_dir(savf_dlg.curr_dir);
                    savf_dlg.go_curr_dir();
                    my_cursor.scr_zone = { savf_dlg.txt_edit.txt_zone.x, savf_dlg.txt_edit.txt_zone.y, 2, savf_dlg.txt_edit.txt_zone.h };
                    savf_dlg.fslctd_idx = -1;
                    savf_dlg.txt_edit.chars_x.clear();
                    savf_dlg.txt_edit.str.clear();
                    savf_dlg.txt_edit.chars_x.push_back(my_cursor.scr_zone.x);
                    savf_dlg.txt_edit.dx_ref = 0;
                    savf_dlg.txt_edit.curs_idx = savf_dlg.txt_edit.slct_idx = 0;
                    draw_diag_mesh(grey, 5);
                }
                else if (in_zone(mouse_coords_ldown, open_butt_zone) && in_zone(mouse_coords_lup, open_butt_zone))
                {
                    open_file = true;
                    normal_mode = false;
                    sim_on = false;
                    get_exe_dir(opnf_dlg.curr_dir);
                    opnf_dlg.go_curr_dir();
                    draw_diag_mesh(grey, 5);
                }
                else if (in_zone(mouse_coords_ldown, clear_butt_zone) && in_zone(mouse_coords_lup, clear_butt_zone))
                {
                    clear_grid();
                }
            }
            else if (open_file)
            {
                opnf_dlg.scrl.drag = false;
                opnf_dlg.scrl.attrct = false;

                if (in_zone(mouse_coords_ldown, opnf_dlg.cancel_zone) && in_zone(mouse_coords_lup, opnf_dlg.cancel_zone))
                {
                    open_file = false;
                    normal_mode = true;
                    opnf_dlg.fslctd_idx = -1;
                    my_cursor.scr_zone = { step_edit.txt_zone.x, step_edit.txt_zone.y, 2, step_edit.txt_zone.h };
                }
                if (opnf_dlg.fslctd_idx != -1 && in_zone(mouse_coords_ldown, opnf_dlg.open_zone) && in_zone(mouse_coords_lup, opnf_dlg.open_zone))
                {
                    if (!opnf_dlg.files_q[opnf_dlg.fslctd_idx].second)
                        load_from_file(opnf_dlg.files_q[opnf_dlg.fslctd_idx].first);
                    else
                        opnf_dlg.go_slctd_dir();
                }
            }
            else if (f_error_dlg)
            {
                if (in_zone(mouse_coords_ldown, opn_wrong_f.ok_zone) && in_zone(mouse_coords_lup, opn_wrong_f.ok_zone))
                {
                    open_file = true;
                    f_error_dlg = false;
                }
            }
            else if (save_file)
            {
                if (in_zone(mouse_coords_ldown, savf_dlg.open_zone) && in_zone(mouse_coords_lup, savf_dlg.open_zone))
                {
                    if (savf_dlg.fslctd_idx != -1)
                    {
                        if (savf_dlg.files_q[savf_dlg.fslctd_idx].second)
                            savf_dlg.go_slctd_dir();
                        else
                        {
                            std::wstring file_path = savf_dlg.curr_dir.substr(0, savf_dlg.curr_dir.size() - 1) + savf_dlg.txt_edit.str;
                            if (check_ext(savf_dlg.txt_edit.str, L".gif"))
                            {
                                gif_path = file_path;
                                gif_sav_info = true;
                                draw_diag_mesh(grey, 5);
                            }
                            else
                            {
                                std::wofstream my_file;
                                my_file.open(file_path);

                                if (my_file.is_open())
                                {
                                    for (int i = 0; i < nb_cell_w; i++)
                                        for (int j = 0; j < nb_cell_h; j++)
                                            my_file << grid[j * nb_cell_w + i] << " ";
                                }
                                my_file.close();
                                normal_mode = true;
                                my_cursor.scr_zone = { step_edit.txt_zone.x , step_edit.txt_zone.y, 2, step_edit.txt_zone.h };
                                step_edit.slct_idx = step_edit.curs_idx = 0;
                            }
                            save_file = false;
                        }
                    }
                    else
                    {
                        std::wstring file_path = savf_dlg.curr_dir.substr(0, savf_dlg.curr_dir.size() - 1) + savf_dlg.txt_edit.str;
                        if (check_ext(savf_dlg.txt_edit.str, L".gif"))
                        {
                            gif_path = file_path;
                            gif_sav_info = true;
                            draw_diag_mesh(grey, 5);
                        }
                        else if (check_ext(savf_dlg.txt_edit.str, L".txt"))
                        {
                            std::wofstream my_file;
                            my_file.open(file_path);

                            if (my_file.is_open())
                            {
                                for (int i = 0; i < nb_cell_w; i++)
                                    for (int j = 0; j < nb_cell_h; j++)
                                        my_file << grid[j * nb_cell_w + i] << " ";
                            }
                            my_file.close();
                            normal_mode = true;
                            my_cursor.scr_zone = { step_edit.txt_zone.x , step_edit.txt_zone.y, 2, step_edit.txt_zone.h };
                            step_edit.slct_idx = step_edit.curs_idx = 0;
                        }
                        else
                        {
                            save_ext_err = true;
                        }
                        save_file = false;
                    }
                }
                else if (in_zone(mouse_coords_ldown, savf_dlg.cancel_zone) && in_zone(mouse_coords_lup, savf_dlg.cancel_zone))
                {
                    save_file = false;
                    normal_mode = true;
                    my_cursor.scr_zone = { step_edit.txt_zone.x, step_edit.txt_zone.y, 2, step_edit.txt_zone.h };
                    step_edit.slct_idx = step_edit.curs_idx = 0;
                }

                savf_dlg.scrl.drag = false;
                savf_dlg.scrl.attrct = false;
            }
            else if (save_ext_err)
            {
                if (in_zone(mouse_coords_ldown, ext_err_info.ok_zone) && in_zone(mouse_coords_lup, ext_err_info.ok_zone))
                {
                    save_ext_err = false;
                    save_file = true;
                }
            }
            else if (gif_sav_info)
            {
                if (in_zone(mouse_coords_ldown, gif_slct_info.ok_zone) && in_zone(mouse_coords_lup, gif_slct_info.ok_zone))
                {
                    gif_sav_info = false;
                    gif_saving = true;
                }
            }
            else if (gif_params_collect)
            {
                if (in_zone(mouse_coords_ldown, gif_params_dlg.ok_zone) && in_zone(mouse_coords_lup, gif_params_dlg.ok_zone))
                {
                    gen_ps = std::stoi(gif_params_dlg.txt_edits[0].str);
                    nb_gen = std::stoi(gif_params_dlg.txt_edits[1].str);

                    gen_ps = gen_ps > 0 ? gen_ps : 1;
                    nb_gen = nb_gen > 0 ? nb_gen : 1;

                    save_as_gif();
                    gif_params_collect = false;
                    gif_saving = false;
                    normal_mode = true;
                }
            }
        }
        else
            del_mouse_lup = false;
        PostMessageW(wnd, WM_SETCURSOR, (WPARAM)wnd, MAKELPARAM(HTCLIENT, WM_MOUSEMOVE));
    }break;
    case WM_RBUTTONDOWN:
    {
        mouse_coords_rdown.x = GET_X_LPARAM(lp);
        mouse_coords_rdown.y = GET_Y_LPARAM(lp);
        my_mouse.r_down = true;
        if (gif_saving)
        {
            recta sh_hide_zone;
            recta map_zone = { 0,0,0,0 };
            if (show_map)
            {
                sh_hide_zone = hide.scr_zone;
                map_zone = region_slct_zone;
            }
            else
                sh_hide_zone = { anti_hide.scr_zone.x - 60, anti_hide.scr_zone.y, anti_hide.w + 60, anti_hide.h };
            if (!in_zone(mouse_coords_rdown, menu_zone) && !in_zone(mouse_coords_rdown, sh_hide_zone) && !in_zone(mouse_coords_rdown, map_zone))
            {
                gif_slct_mode = true;
            }
        }
    }break;
    case WM_RBUTTONUP:
    {
        mouse_coords_rup.x = GET_X_LPARAM(lp);
        mouse_coords_rup.y = GET_Y_LPARAM(lp);

        my_mouse.r_down = false;
        
        if (gif_saving)
        {
            if(in_zone(mouse_coords_rup, screen_zone) && in_zone(mouse_coords_rdown, screen_zone))
            {
                gif_rg0 = get_clk_cell(mouse_coords_rdown);
                gif_rg1 = get_clk_cell(mouse_coords_rup);
                point temp = gif_rg0;
                gif_rg0 = { min(temp.x, gif_rg1.x), min(temp.y, gif_rg1.y) };
                gif_rg1 = { max(temp.x, gif_rg1.x), max(temp.y, gif_rg1.y) };

                gif_slct_mode = false;
                gif_params_collect = true; 
                for(int i = 0; i < gif_params_dlg.txt_edits.size(); i++) 
                {
                    gif_params_dlg.txt_edits[i].chars_x.clear();
                    gif_params_dlg.txt_edits[i].str.clear();
                    gif_params_dlg.txt_edits[i].chars_x.push_back(gif_params_dlg.txt_edits[i].txt_zone.x);
                    gif_params_dlg.txt_edits[i].dx_ref = 0;
                }
                HCURSOR h_curs = LoadCursor(NULL, IDC_WAIT);
                SetCursor(h_curs);
            }
        }
    }break;

    case WM_LBUTTONDBLCLK:
    {
        my_mouse.l_down = true;
        if(normal_mode)
        {
            if (in_zone(mouse_coords_lup, { intens_slct.scr_zone.x, intens_slct.scr_zone.y, intens_slct.scr_zone.w, intens_slct.scr_zone.h }))
                int_bar_slctd = true;
            else if (in_zone(mouse_coords_ldown, go_butt_zone))
                go_butt_down = true;
            else if (in_zone(mouse_coords_ldown, play_butt[0].scr_zone))
                plpau_down = true;
            else if (in_zone(mouse_coords_ldown, reinit_butt[0].scr_zone))
                reinit_down = true;
            else if (in_zone(mouse_coords_ldown, next_butt[0].scr_zone))
                next_down = true;
            else if (in_zone(mouse_coords_lup, step_edit.zone))
            {
                my_cursor.scr_zone.x = step_edit.chars_x.back() - step_edit.dx_ref;
                step_edit.slct_idx = 0;
                step_edit.curs_idx = step_edit.str.size();
            }
        }
        else if (open_file )
        {
            if (in_zone(mouse_coords, opnf_dlg.exp_zone))
            {
                int idx_f = (mouse_coords.y - opnf_dlg.fhref) / opnf_dlg.file_h;
                if (idx_f < opnf_dlg.nb_files)
                {
                    if (!opnf_dlg.files_q[idx_f].second)
                    {
                        load_from_file(opnf_dlg.files_q[idx_f].first);
                        del_mouse_lup = true;
                    }
                    else
                    {
                        opnf_dlg.fslctd_idx = idx_f;
                        opnf_dlg.go_slctd_dir();
                    }
                }
            }
            else 
                opnf_dlg.handle_scrl_ldblclk();
        }
        else if (save_file)
        {
            if (in_zone(mouse_coords, savf_dlg.exp_zone))
            {
                int idx_f = (mouse_coords.y - savf_dlg.fhref) / savf_dlg.file_h;
                if (idx_f < savf_dlg.nb_files)
                {
                    if (savf_dlg.files_q[idx_f].second)
                    {
                        savf_dlg.fslctd_idx = idx_f;
                        savf_dlg.go_slctd_dir();
                    }
                    else
                    {
                        std::wstring file_path = savf_dlg.curr_dir.substr(0, savf_dlg.curr_dir.size() - 1) + savf_dlg.txt_edit.str;
                        if (check_ext(savf_dlg.txt_edit.str, L".gif"))
                        {
                            gif_path = file_path;
                            gif_sav_info = true;
                            draw_diag_mesh(grey, 5);
                        }
                        else
                        {
                            std::wofstream my_file;
                            my_file.open(file_path);

                            if (my_file.is_open())
                            {
                                for (int i = 0; i < nb_cell_w; i++)
                                    for (int j = 0; j < nb_cell_h; j++)
                                        my_file << grid[j * nb_cell_w + i] << " ";
                            }
                            my_file.close();
                            normal_mode = true;
                            my_cursor.scr_zone = { step_edit.txt_zone.x , step_edit.txt_zone.y, 2, step_edit.txt_zone.h };
                            step_edit.slct_idx = step_edit.curs_idx = 0;
                            del_mouse_lup = true;
                        }
                        save_file = false;
                    }
                }
            }
            else if(in_zone(mouse_coords_ldown, savf_dlg.txt_edit.zone))
            {
                int i;
                for (i = savf_dlg.txt_edit.curs_idx; i >= 0; i--)
                {
                    if (savf_dlg.txt_edit.str[i] == L'.' || savf_dlg.txt_edit.str[i] == L' ')
                        break;
                }
                savf_dlg.txt_edit.slct_idx = i+1;
                for (i = savf_dlg.txt_edit.curs_idx; i <savf_dlg.txt_edit.str.size(); i++)
                {
                    if (savf_dlg.txt_edit.str[i] == L'.' || savf_dlg.txt_edit.str[i] == L' ')
                        break;
                }
                savf_dlg.txt_edit.curs_idx = i;
                my_cursor.scr_zone.x = savf_dlg.txt_edit.chars_x[savf_dlg.txt_edit.curs_idx] - savf_dlg.txt_edit.dx_ref;
            }
        }
        else if (gif_params_collect)
        {
            for(int i = 0; i < gif_params_dlg.txt_edits.size(); i++)
            {
                if (in_zone(mouse_coords_lup, gif_params_dlg.txt_edits[i].zone))
                {
                    my_cursor.scr_zone.x = gif_params_dlg.txt_edits[i].chars_x.back() - gif_params_dlg.txt_edits[i].dx_ref;
                    gif_params_dlg.txt_edits[i].slct_idx = 0;
                    gif_params_dlg.txt_edits[i].curs_idx = gif_params_dlg.txt_edits[i].str.size();
                    break;
                }
            }
        }
            
    }break;
    
    case WM_MOUSEWHEEL:
    {
        int delta = int(GET_WHEEL_DELTA_WPARAM(wp));
        if (open_file)
        {
            opnf_dlg.handle_mw(delta);
        }
        else if (save_file)
        {
            savf_dlg.handle_mw(delta);
        }
        else if(normal_mode)
        {
            bool zoom_chng_dir = false;
            if (zoom_in && delta < 0 || !zoom_in && delta>0)
                zoom_chng_dir = true;
            int new_w1_targ = zoom_chng_dir ? bmp_zone.w - delta : bmp_zone_targ.w - delta;
            if (zoom_chng_dir)
                zooming = false;
            if (new_w1_targ > 8 * cell_w) // setting a limit for zoom in  
            {
                int new_cell_w = cell_w * screen_zone.w / new_w1_targ;
                if (new_cell_w > 6) // setting a limit for zoom out 
                {
                    if (!zooming)
                    {
                        bmp_zone_targ = bmp_zone;
                        zooming = true;
                    }
                    zoom_in = delta > 0 ? true : false;
                    bmp_zone_targ.w = new_w1_targ;
                    bmp_zone_targ.x = mouse_coords.x * (bmp_zone.w - bmp_zone_targ.w) / screen_zone.w + bmp_zone.x;
                    make_inside(bmp_zone_targ.x, 0, nb_cell_w* cell_w - bmp_zone_targ.w);

                    bmp_zone_targ.h = screen_zone.h * bmp_zone_targ.w / screen_zone.w;
                    bmp_zone_targ.y = mouse_coords.y * (bmp_zone.h - bmp_zone_targ.h) / screen_zone.h + bmp_zone.y;
                    make_inside(bmp_zone_targ.y, 0, nb_cell_h* cell_h - bmp_zone_targ.h);

                    zoom_speed = std::abs(int(bmp_zone_targ.w) - bmp_zone.w) * 3.5;
                    pos_zpeed_x = zoom_speed * std::abs(bmp_zone_targ.x - bmp_zone.x) / std::abs(bmp_zone_targ.w - bmp_zone.w);
                    pos_zpeed_y = zoom_speed * std::abs(bmp_zone_targ.y - bmp_zone.y) / std::abs(bmp_zone_targ.w - bmp_zone.w);
                }
            }
        }

    }break;
            
    case WM_KEYDOWN:
    {
        if(show_cursor)
        {
            uint16 my_char = get_char_uni(wp);

            if(normal_mode)
            {
                if (!slct_mode)
                {
                    if (std::iswdigit(my_char))
                        step_edit.add_char(my_char);
                    step_edit.handle_arrows(wp);
                }
            }
            else if (save_file)
            {
                if (!slct_mode)
                {
                    if (iswprint(my_char))
                        savf_dlg.txt_edit.add_char(my_char);
                    savf_dlg.txt_edit.handle_arrows(wp);
                }
            }
            else if (gif_params_collect)
            {
                if (std::iswdigit(my_char))
                    gif_params_dlg.txt_edits[gif_params_dlg.slctd_edit].add_char(my_char);
                gif_params_dlg.txt_edits[gif_params_dlg.slctd_edit].handle_arrows(wp);
            }
        }
        else if (open_file)
        {
            switch (wp)
            {
                case VK_UP:
                {
                    opnf_dlg.fslctd_idx = (opnf_dlg.fslctd_idx - 1 >= 0) ? opnf_dlg.fslctd_idx - 1 : 0;

                    if (opnf_dlg.fhref + opnf_dlg.fslctd_idx * opnf_dlg.file_h < opnf_dlg.exp_zone.y)
                        opnf_dlg.fhref = opnf_dlg.exp_zone.y - opnf_dlg.fslctd_idx * opnf_dlg.file_h;
                    else if (opnf_dlg.fhref + (opnf_dlg.fslctd_idx+1) * opnf_dlg.file_h > opnf_dlg.exp_zone.y + opnf_dlg.exp_zone.h)
                        opnf_dlg.fhref = opnf_dlg.exp_zone.y +opnf_dlg.exp_zone.h - (opnf_dlg.fslctd_idx +1)* opnf_dlg.file_h;

                    make_inside(opnf_dlg.fhref, opnf_dlg.exp_zone.y + opnf_dlg.exp_zone.h - (opnf_dlg.nb_files + 2) * opnf_dlg.file_h, opnf_dlg.exp_zone.y);
                    opnf_dlg.scrl.scrler_zone.y = opnf_dlg.scrl.zone.y + (opnf_dlg.exp_zone.y - opnf_dlg.fhref) / opnf_dlg.scrl.ratio;

                }break;
                case VK_DOWN:
                {
                    opnf_dlg.fslctd_idx = (opnf_dlg.fslctd_idx + 1 <= opnf_dlg.nb_files - 1) ? opnf_dlg.fslctd_idx + 1 : opnf_dlg.nb_files - 1;

                    if (opnf_dlg.fhref + opnf_dlg.fslctd_idx * opnf_dlg.file_h < opnf_dlg.exp_zone.y)
                        opnf_dlg.fhref = opnf_dlg.exp_zone.y - opnf_dlg.fslctd_idx * opnf_dlg.file_h;
                    else if (opnf_dlg.fhref + (opnf_dlg.fslctd_idx + 1) * opnf_dlg.file_h > opnf_dlg.exp_zone.y + opnf_dlg.exp_zone.h)
                        opnf_dlg.fhref = opnf_dlg.exp_zone.y + opnf_dlg.exp_zone.h - (opnf_dlg.fslctd_idx + 1) * opnf_dlg.file_h;
                    
                    make_inside(opnf_dlg.fhref, opnf_dlg.exp_zone.y + opnf_dlg.exp_zone.h - (opnf_dlg.nb_files + 2) * opnf_dlg.file_h, opnf_dlg.exp_zone.y);
                    opnf_dlg.scrl.scrler_zone.y = opnf_dlg.scrl.zone.y + (opnf_dlg.exp_zone.y - opnf_dlg.fhref) / opnf_dlg.scrl.ratio;

                }break;
                case VK_ESCAPE:
                {
                    open_file = false;
                    normal_mode = true;
                    opnf_dlg.fslctd_idx = -1;
                    my_cursor.scr_zone = { step_edit.txt_zone.x, step_edit.txt_zone.y, 2, step_edit.txt_zone.h };

                }break;
                case VK_RETURN:
                {
                    if(opnf_dlg.fslctd_idx != -1)
                    {
                        if (!opnf_dlg.files_q[opnf_dlg.fslctd_idx].second)
                            load_from_file(opnf_dlg.files_q[opnf_dlg.fslctd_idx].first);
                        else
                            opnf_dlg.go_slctd_dir();
                    }
                }break;
                case VK_BACK:
                {
                    if(!opnf_dlg.in_root)
                    {
                        opnf_dlg.fslctd_idx = 0;
                        opnf_dlg.go_slctd_dir();
                    }
                }break;
            }
        }
        else if (save_file)
        {
            switch (wp)
            {
                case VK_UP:
                {
                    savf_dlg.fslctd_idx = (savf_dlg.fslctd_idx - 1 >= 0) ? savf_dlg.fslctd_idx - 1 : 0;
                    if (!savf_dlg.files_q[savf_dlg.fslctd_idx].second)
                        savf_dlg.txt_edit.copy_txt(savf_dlg.files_q[savf_dlg.fslctd_idx].first, cascadia_face);

                    if (savf_dlg.fhref + savf_dlg.fslctd_idx * savf_dlg.file_h < savf_dlg.exp_zone.y)
                        savf_dlg.fhref = savf_dlg.exp_zone.y - savf_dlg.fslctd_idx * savf_dlg.file_h;
                    else if (savf_dlg.fhref + (savf_dlg.fslctd_idx + 1) * savf_dlg.file_h > savf_dlg.exp_zone.y + savf_dlg.exp_zone.h)
                        savf_dlg.fhref = savf_dlg.exp_zone.y + savf_dlg.exp_zone.h - (savf_dlg.fslctd_idx + 1) * savf_dlg.file_h;

                    make_inside(savf_dlg.fhref, savf_dlg.exp_zone.y + savf_dlg.exp_zone.h - (savf_dlg.nb_files + 2) * savf_dlg.file_h, savf_dlg.exp_zone.y);
                    savf_dlg.scrl.scrler_zone.y = savf_dlg.scrl.zone.y + (savf_dlg.exp_zone.y - savf_dlg.fhref) / savf_dlg.scrl.ratio;

                }break;
                case VK_DOWN:
                {
                    savf_dlg.fslctd_idx = (savf_dlg.fslctd_idx + 1 <= savf_dlg.nb_files - 1) ? savf_dlg.fslctd_idx + 1 : savf_dlg.nb_files - 1;
                    if (!savf_dlg.files_q[savf_dlg.fslctd_idx].second)
                        savf_dlg.txt_edit.copy_txt(savf_dlg.files_q[savf_dlg.fslctd_idx].first, cascadia_face);

                    if (savf_dlg.fhref + savf_dlg.fslctd_idx * savf_dlg.file_h < savf_dlg.exp_zone.y)
                        savf_dlg.fhref = savf_dlg.exp_zone.y - savf_dlg.fslctd_idx * savf_dlg.file_h;
                    else if (savf_dlg.fhref + (savf_dlg.fslctd_idx + 1) * savf_dlg.file_h > savf_dlg.exp_zone.y + savf_dlg.exp_zone.h)
                        savf_dlg.fhref = savf_dlg.exp_zone.y + savf_dlg.exp_zone.h - (savf_dlg.fslctd_idx + 1) * savf_dlg.file_h;

                    make_inside(savf_dlg.fhref, savf_dlg.exp_zone.y + savf_dlg.exp_zone.h - (savf_dlg.nb_files + 2) * savf_dlg.file_h, savf_dlg.exp_zone.y);
                    savf_dlg.scrl.scrler_zone.y = savf_dlg.scrl.zone.y + (savf_dlg.exp_zone.y - savf_dlg.fhref) / savf_dlg.scrl.ratio;

                }break;
                case VK_ESCAPE:
                {
                    save_file = false;
                    normal_mode = true;
                    savf_dlg.fslctd_idx = -1;
                    my_cursor.scr_zone = { step_edit.txt_zone.x, step_edit.txt_zone.y, 2, step_edit.txt_zone.h };
                    step_edit.slct_idx = step_edit.curs_idx = 0;
                }break;
                case VK_RETURN:
                {
                    if (savf_dlg.fslctd_idx != -1)
                    {
                        if (savf_dlg.files_q[savf_dlg.fslctd_idx].second)
                            savf_dlg.go_slctd_dir();
                    }
                }break;
                case VK_BACK:
                {
                    if (!savf_dlg.in_root)
                    {
                        savf_dlg.fslctd_idx = 0;
                        savf_dlg.go_slctd_dir();
                    }
                }break;
            }
        }
        else if (f_error_dlg)
        {
            switch(wp)
            {
                case VK_RETURN:
                {
                    f_error_dlg = false;
                    open_file = true;
                }break;
            }
        }
    }break;

    case WM_SETCURSOR:
    {
        result = TRUE;
        HCURSOR h_curs;
        h_curs = LoadCursor(NULL, IDC_ARROW);

        if (normal_mode)
        {
            if (in_zone(mouse_coords, step_edit.zone))
                h_curs = LoadCursor(NULL, IDC_IBEAM);
            else if (my_mouse.l_down)
                    h_curs = LoadCursor(NULL, IDC_SIZEALL);

        }
        else if(save_file)
        { 
            if (in_zone(mouse_coords, savf_dlg.txt_edit.zone))
                h_curs = LoadCursor(NULL, IDC_IBEAM);
        }
        else if (gif_params_collect)
        {
            for(int i = 0; i < gif_params_dlg.txt_edits.size(); i++)
                if (in_zone(mouse_coords, gif_params_dlg.txt_edits[i].zone))
                    h_curs = LoadCursor(NULL, IDC_IBEAM);
        }
        SetCursor(h_curs);
    }break;
    case WM_PAINT:
    {
        PAINTSTRUCT Paint;
        HDC DeviceContext = BeginPaint(wnd, &Paint);

        int X = Paint.rcPaint.left;
        int Y = Paint.rcPaint.top;
        int Width = Paint.rcPaint.right - Paint.rcPaint.left;
        int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;

        RECT ClientRect;
        GetClientRect(wnd, &ClientRect);
        update_wnd(DeviceContext, &ClientRect);
        EndPaint(wnd, &Paint);
    } break;
    case WM_CLOSE:
    {
        running = false;
    } break;

    case WM_ACTIVATEAPP:
    {
    } break;

    case WM_DESTROY:
    {
        running = false;
    } break;

    default:
    {
        result = DefWindowProc(wnd, msg, wp, lp);
    } break;
    }
    return(result);
}

void draw_region_slct()
{
    draw_color_to_zone(black, region_slct_zone);
    uint32* pixel = (uint32*)bmp_memory;
    
    for (int i = region_slct_zone.x; i < region_slct_zone.x + region_slct_zone.w; i++)
    {
        for (int j = region_slct_zone.y; j < region_slct_zone.y + region_slct_zone.h; j++)
        {
            int i0 = (i - region_slct_zone.x + int(map_pos.x)) / map_cell_w;
            int j0 = (j - region_slct_zone.y + int(map_pos.y)) / map_cell_w;

            if (grid[j0 * nb_cell_w + i0])
                pixel[j * screen_zone.w + i] = light;
            else
                pixel[j * screen_zone.w + i] = black;
        }
    }

    draw_out_recta(region_slct_zone, l_blue, 3);
    draw_out_recta({ bmp_region_zone.x + 1, bmp_region_zone.y + 1, bmp_region_zone.w - 2, bmp_region_zone.h - 2 }, orange, 2);
}

void draw_butt(const recta& zone, std::wstring str, int marg, uint32 clr, uint32 hvr_clr)
{
    if (in_zone(mouse_coords, zone))
    {
        draw_color_to_zone(hvr_clr, zone);
        if (my_mouse.l_down)
            draw_out_recta(zone, black, 2);
    }
    else
        draw_color_to_zone(clr, zone);
    draw_text_cntrd_fit(cascadia_face, str, zone, { marg, 0, marg, 0 });
}

void clear_grid()
{
    for (int i = 0; i < nb_cell_w; i++)
        for (int j = 0; j < nb_cell_h; j++)
        {
            grid[j * nb_cell_w + i] = 0;
            futur_grid[j * nb_cell_w + i] = 0;
        }
    grid_init_stt.clear();
    actual_step = target_step = 0;
}

bool check_ext(const std::wstring& file_path, const std::wstring& ext) {
    size_t dot_pos = file_path.find_last_of(L'.');
    if (file_path.length() - dot_pos != ext.size())
        return false;
    for (int i = 1; i < ext.size(); i++)
        if (ext[i] != file_path[dot_pos + i])
            return false;
    return true; 
}

void save_as_gif()
{
    int nbw = gif_rg1.x - gif_rg0.x + 3;
    int nbh = gif_rg1.y - gif_rg0.y + 3;
    int gbxw = cell_w * zoom_ratio_x;
    int width = nbw * gbxw;
    int height = nbh * gbxw;

    std::vector<uint8_t> gif_frame(width * height * 4);

    reinit_grid();
    int delay;
    delay = 100.0 / gen_ps >= 10 ? int(100.0 / gen_ps) : 10;
    GifWriter g;
    GifBegin(&g, gif_path.c_str(), width, height, delay);
    for(int k =0; k <= nb_gen; k++)
    {
        for (int i = 0; i < width; i++)
        {
            for (int j = 0; j < height; j++)
            {
                int pix_addr = j * width + i;
                if ((i % gbxw == 0) || (i % gbxw == 1) || (j % gbxw == 0) || (j % gbxw == 1))
                {
                    gif_frame[pix_addr * 4] = 0;
                    gif_frame[pix_addr * 4 + 1] = 0;
                    gif_frame[pix_addr * 4 + 2] = 0;
                    gif_frame[pix_addr * 4 + 3] = 0;
                }
                else
                {
                    int bx_i = i / gbxw + gif_rg0.x - 1;
                    int bx_j = j / gbxw + gif_rg0.y - 1;

                    if (grid[bx_j * nb_cell_w + bx_i])
                    {
                        gif_frame[pix_addr * 4] = 127;
                        gif_frame[pix_addr * 4 + 1] = 127;
                        gif_frame[pix_addr * 4 + 2] = 127;
                        gif_frame[pix_addr * 4 + 3] = 127;
                    }
                    else
                    {
                        gif_frame[pix_addr * 4] = 255;
                        gif_frame[pix_addr * 4 + 1] = 255;
                        gif_frame[pix_addr * 4 + 2] = 255;
                        gif_frame[pix_addr * 4 + 3] = 255;
                    }
                }
            }
        }
        GifWriteFrame(&g, gif_frame.data(), width, height, delay);
        one_step_ahead();
    }
    GifEnd(&g);
    reinit_grid();
}

int txt_edit_params::get_char_idx()
{
    auto lower = std::upper_bound(chars_x.begin(), chars_x.end() - 1, my_cursor.scr_zone.x + dx_ref);
    int idx = std::distance(chars_x.begin(), lower);
    if (idx == 0)
        return 0;
    if (idx == chars_x.size())
        return idx - 1;
    if (my_cursor.scr_zone.x + dx_ref <(chars_x[idx - 1] + chars_x[idx]) / 2)
    {
        idx--;
    }
    return idx;
}

void txt_edit_params::copy_txt(std::wstring strr, FT_Face& face)
{
    if (!face_chars[txt_zone.h].glyphs)
        load_glyphs(face, txt_zone.h);
    str.resize(strr.size());
    chars_x.resize(1);
    for (int i = 0; i < strr.size(); i++)
    {
        wchar_t c = strr[i];
        str[i] = c;
        character& cc = (*face_chars[txt_zone.h].glyphs)[c];
        chars_x.push_back(chars_x[i] + cc.advance / 64);
    }
}

void txt_edit_params::draw()
{
    if (curs_idx != slct_idx)
    {
        int min_i = min(slct_idx, curs_idx);
        int max_i = max(slct_idx, curs_idx);
        recta slcted_zone = { chars_x[min_i] - dx_ref, my_cursor.scr_zone.y, chars_x[max_i] - chars_x[min_i], my_cursor.scr_zone.h };
        draw_color_to_zone(slct_color, slcted_zone, txt_zone);
    }
    // draw user text
    draw_text_ft(cascadia_face, str, txt_zone, txt_zone, { 0,0,0,0 }, dx_ref);
}

void txt_edit_params::handle_slct()
{
    // selection in txt_zone
    if (slct_mode)
    {
        while (mouse_coords.x + dx_ref > (chars_x[curs_idx] + chars_x[curs_idx + 1]) / 2 && curs_idx + 1 < chars_x.size())
            curs_idx++;
        while (mouse_coords.x + dx_ref < (chars_x[curs_idx] + chars_x[curs_idx - 1]) / 2 && curs_idx > 0)
            curs_idx--;
        if (chars_x[curs_idx] - dx_ref < txt_zone.x)
            dx_ref -= txt_zone.x - (chars_x[curs_idx] - dx_ref);
        if (chars_x[curs_idx] + my_cursor.scr_zone.w - dx_ref > txt_zone.x + txt_zone.w)
            dx_ref += (chars_x[curs_idx] + my_cursor.scr_zone.w - dx_ref) - (txt_zone.x + txt_zone.w);
    }
}

void get_exe_dir(std::wstring& curr_dir)
{
    wchar_t fn[MAX_PATH];
    GetModuleFileNameW(NULL, fn, MAX_PATH);
    
    if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
    {
        std::cout << "file path too long\n";
        return;
    }
    wchar_t* last_bslash = wcsrchr(fn, L'\\');

    wcsncpy_s(fn, fn, last_bslash - fn + 1);
    last_bslash[1] = L'*';
    last_bslash[2] = L'\0';
    curr_dir = fn;
}

void info_dlg::fill_params(FT_Face& face)
{
    char_h = 5*title_h / 12;
    txt_margins = {char_h/2, char_h/2, char_h/2, char_h/2 };
    ok_margins = { 0,char_h / 2,0,char_h / 2 };
    if (!face_chars[char_h].glyphs)
        load_glyphs(face, char_h);
    int nb_rows = 1;
    int txt_w = 0;
    int i = 0;
    int txt_max_w = zone.w - txt_margins.x - txt_margins.w;
    int j = 0;
    while (true)
    {
        while (txt_w < txt_max_w && i < txt.size())
        {
            if (txt[i] == L'\n')
            {
                j = i;
                txt_w = txt_max_w;
                break;
            }
            if (txt[i] == L' ')
                j = i;
            character& cc = (*face_chars[char_h].glyphs)[txt[i]];
            txt_w += cc.advance / 64;
            i++;
        }
        if (txt_w >= txt_max_w)
        {
            if (!nxt_line_idxs.empty() && nxt_line_idxs.back() == j + 1)
                i--;
            else
                i = j + 1;
            nb_rows++;
            txt_w = 0;
            nxt_line_idxs.push_back(i);
        }
        else
            break;
    }
    txt_h = nb_rows * char_h;
    // ok_zone 
    ok_zone.w = zone.w / 5;
    ok_zone.x = zone.x + (zone.w - ok_zone.w) / 2;
    ok_zone.y = zone.y + title_h + txt_margins.y + txt_h + txt_margins.h + ok_margins.y;
    ok_zone.h = 2 * title_h / 3;
    // zone height
    zone.h = title_h + txt_margins.y + txt_h + txt_margins.h + ok_margins.y + ok_zone.h + ok_margins.h;
}

void collect_dlg::fill_params()
{
    ok_zone.w = zone.w / 5;
    ok_zone.x = zone.x + (zone.w - ok_zone.w) / 2;
    ok_zone.h = 2 * title_h / 3;
    zone.h = title_h + txt_edits.size() * (3*title_h/2) + 2*ok_zone.h;
    ok_zone.y = zone.y + zone.h -  2 * ok_zone.h;
}

void update_back_buffer()
{
    if (normal_mode || gif_saving)
    {
        // grid drawing 
        draw_bgnd_color(black);
        int i = 1;
        int line_x = 0;
        while (line_x < screen_zone.w)
        {
            line_x = ((int(bmp_zone.x / cell_w) + i) * cell_w - bmp_zone.x) * zoom_ratio_x;
            draw_color_to_zone(white, { line_x, 0, 2, screen_zone.h });
            i++;
        }
        i = 1;
        int line_y = 0;
        while (line_y < screen_zone.h)
        {
            line_y = ((int(bmp_zone.y / cell_w) + i) * cell_w - bmp_zone.y) * zoom_ratio_y;
            draw_color_to_zone(white, { 0,line_y, screen_zone.w, 2 });
            i++;
        }

        // updating the state of bitmap
        for (int i = int(bmp_zone.x / cell_w); i < int((bmp_zone.x + bmp_zone.w) / cell_w) + 1; i++)
        {
            for (int j = int(bmp_zone.y / cell_h); j < int((bmp_zone.y + bmp_zone.h) / cell_h) + 1; j++)
            {
                recta zone = { i * cell_w, j * cell_h , cell_w, cell_h };

                int scr_cell_x = (zone.x - bmp_zone.x) * zoom_ratio_x;
                int scr_cell_y = (zone.y - bmp_zone.y) * zoom_ratio_y;
                int scr_cell_w = int((zone.x + cell_w - bmp_zone.x) * zoom_ratio_x) - scr_cell_x - 2;
                int scr_cell_h = int((zone.y + cell_h - bmp_zone.y) * zoom_ratio_y) - scr_cell_y - 2;

                recta in_bmp_zone = { scr_cell_x + 2, scr_cell_y + 2, scr_cell_w,  scr_cell_h };
                if (grid[j * nb_cell_w + i])
                    draw_color_to_zone(light, in_bmp_zone);
            }
        }

        // menu zone drawing 
        draw_color_to_zone(ll_grey, menu_zone);
        draw_out_recta({ menu_zone.x, menu_zone.y, menu_zone.w, menu_zone.h }, ld_blue, 4);
        draw_out_recta({ menu_zone.x , menu_zone.y , menu_zone.w , menu_zone.h }, grey, 2);

        draw_out_recta(intens_bar.scr_zone, black, 2);
        draw_out_recta(step_edit.zone, grey, 3);
        //draw_text_to_zone(, , true, grey);
        //draw_text_to_zone("simulation velocity", sim_vel_zone, true, grey);
        draw_text_cntrd_fit(cascadia_face, L"Go To Step", go_to_stp_zone, { 0, -go_to_stp_zone.h, 0, 0 });
        draw_text_cntrd_fit(cascadia_face, L"Simulation Velocity", sim_vel_zone, { 0, -sim_vel_zone.h, 0, 0 });
        draw_butt(save_butt_zone, L"SAVE", save_butt_zone.w / 16);
        draw_butt(open_butt_zone, L"OPEN", open_butt_zone.w / 16);
        draw_butt(clear_butt_zone, L"CLEAR", clear_butt_zone.w / 16);
        draw_butt(go_butt_zone, L"GO", go_butt_zone.w * 7 / 24);

        draw_actual_step();

        // map drawing
        if (show_map)
        {
            draw_region_slct();
            draw_im(hide);
        }
        else
        {
            draw_im(anti_hide);
            int pw = 60;
            recta panel_zone = { anti_hide.scr_zone.x - pw, anti_hide.scr_zone.y, pw, anti_hide.h };
            draw_color_to_zone(ld_blue, panel_zone);
            draw_text_to_zone(L"map", { panel_zone.x + 3, panel_zone.y + 3, panel_zone.w - 6, panel_zone.h - 6 }, true, black);
        }

        // play, pause, reinit, go, next buttons
        if (sim_on)
        {
            if (!plpau_down)
            {
                if (in_zone(mouse_coords, pause_butt[0].scr_zone))
                    instrch_from_im(pause_butt[1], { 0,0,pause_butt[0].w, pause_butt[0].h }, pause_butt[0].scr_zone);
                else
                    instrch_from_im(pause_butt[0], { 0,0,pause_butt[0].w, pause_butt[0].h }, pause_butt[0].scr_zone);
            }
            else
                instrch_from_im(pause_butt[2], { 0,0,pause_butt[0].w, pause_butt[0].h }, pause_butt[0].scr_zone);
        }
        else
        {
            if (!plpau_down)
            {
                if (in_zone(mouse_coords, play_butt[0].scr_zone))
                    instrch_from_im(play_butt[1], { 0,0,play_butt[0].w, play_butt[0].h }, play_butt[0].scr_zone);
                else
                    instrch_from_im(play_butt[0], { 0,0,play_butt[0].w, play_butt[0].h }, play_butt[0].scr_zone);
            }
            else
                instrch_from_im(play_butt[2], { 0,0,play_butt[0].w, play_butt[0].h }, play_butt[0].scr_zone);
        }

        if (!reinit_down)
            if (in_zone(mouse_coords, reinit_butt[0].scr_zone))
                instrch_from_im(reinit_butt[1], { 0,0,reinit_butt[0].w, reinit_butt[0].h }, reinit_butt[0].scr_zone);
            else
                instrch_from_im(reinit_butt[0], { 0,0,reinit_butt[0].w, reinit_butt[0].h }, reinit_butt[0].scr_zone);
        else
            instrch_from_im(reinit_butt[2], { 0,0,reinit_butt[0].w, reinit_butt[0].h }, reinit_butt[0].scr_zone);
        if (!next_down)
            if (in_zone(mouse_coords, next_butt[0].scr_zone))
                instrch_from_im(next_butt[1], { 0,0,next_butt[0].w, next_butt[0].h }, next_butt[0].scr_zone);
            else
                instrch_from_im(next_butt[0], { 0,0,next_butt[0].w, next_butt[0].h }, next_butt[0].scr_zone);
        else
            instrch_from_im(next_butt[2], { 0,0,next_butt[0].w, next_butt[0].h }, next_butt[0].scr_zone);

        draw_color_to_zone(slct_color, { intens_bar.scr_zone.x, intens_bar.scr_zone.y, intens_slct.scr_zone.x - intens_bar.scr_zone.x , intens_bar.scr_zone.h });
        instrch_from_im(intens_slct, { 0,0,intens_slct.w, intens_slct.h }, intens_slct.scr_zone);
        draw_sim_vel();
        if (normal_mode)
        {
            step_edit.draw();
        }
        if (gif_saving)
        {
            if (gif_slct_mode)
            {
                int x = mouse_coords.x;
                int y = mouse_coords.y;
                int x0 = mouse_coords_rdown.x;
                int y0 = mouse_coords_rdown.y;
                int mi_x = min(x, x0), mi_y = min(y, y0), ma_x = max(x, x0), ma_y = max(y, y0);
                draw_recta({ mi_x, mi_y, ma_x - mi_x + 1, ma_y - mi_y + 1 }, l_blue, 2);
            }
            else if (gif_params_collect)
            {
                draw_diag_mesh(grey, 5);
                gif_params_dlg.draw();
                for (int i = 0; i < gif_params_dlg.txt_edits.size(); i++)
                {
                    gif_params_dlg.txt_edits[i].draw();
                }
            }
        }
    }
    else if (open_file)
    {
        // draw bakgnd colors and outlines
        draw_color_to_zone(ll_grey, opnf_dlg.zone);
        draw_out_recta(opnf_dlg.zone, l_grey, 4);
        draw_out_recta(opnf_dlg.exp_zone, black, 2);

        // draw open cancel buttons
        draw_butt(opnf_dlg.open_zone, L"OPEN", opnf_dlg.open_zone.w / 5);
        draw_butt(opnf_dlg.cancel_zone, L"CANCEL", opnf_dlg.cancel_zone.w / 10);
        if (my_mouse.l_down)
        {
            if (in_zone(mouse_coords_ldown, opnf_dlg.cancel_zone) && in_zone(mouse_coords, opnf_dlg.cancel_zone))
            {
                draw_out_recta(opnf_dlg.cancel_zone, black, 4);
            }
            if (in_zone(mouse_coords_ldown, opnf_dlg.open_zone) && in_zone(mouse_coords, opnf_dlg.open_zone))
            {
                draw_out_recta(opnf_dlg.open_zone, black, 4);
            }
        }

        // hovered and selected file hilighting
        if (in_zone(mouse_coords, opnf_dlg.exp_zone))
        {
            int idx_hilit = ((opnf_dlg.exp_zone.y - opnf_dlg.fhref) + (mouse_coords.y - opnf_dlg.exp_zone.y)) / opnf_dlg.file_h;
            if (idx_hilit < opnf_dlg.nb_files)
                draw_color_to_zone(lg_blue, { opnf_dlg.exp_zone.x, opnf_dlg.fhref + idx_hilit * opnf_dlg.file_h, opnf_dlg.exp_zone.w, opnf_dlg.file_h }, opnf_dlg.exp_zone);
        }
        if (opnf_dlg.fslctd_idx != -1)
        {
            draw_color_to_zone(ldg_blue, { opnf_dlg.exp_zone.x, opnf_dlg.fhref + opnf_dlg.fslctd_idx * opnf_dlg.file_h, opnf_dlg.exp_zone.w, opnf_dlg.file_h }, opnf_dlg.exp_zone);
        }

        // draw file names
        int icon_w = opnf_dlg.file_h;
        for (int i = (opnf_dlg.exp_zone.y - opnf_dlg.fhref) / opnf_dlg.file_h; i < opnf_dlg.nb_files; i++)
        {
            int row_y = opnf_dlg.fhref + i * opnf_dlg.file_h;
            if (row_y > opnf_dlg.exp_zone.y + opnf_dlg.exp_zone.h)
                break;
            if (opnf_dlg.files_q[i].second)
                instrch_from_im(folder_icon, { 0,0,folder_icon.w, folder_icon.h }, { opnf_dlg.exp_zone.x, row_y, icon_w, opnf_dlg.file_h }, false, black, opnf_dlg.exp_zone);
            draw_text_ft(cascadia_face, opnf_dlg.files_q[i].first, { opnf_dlg.exp_zone.x + icon_w, row_y, opnf_dlg.exp_zone.w - icon_w, opnf_dlg.file_h }, opnf_dlg.exp_zone, { 0, opnf_dlg.file_h * 11 / 36, 0, opnf_dlg.file_h * 11 / 36 });
        }

        // add scroll bar when necessary
        if (opnf_dlg.scrl_exist)
        {
            draw_out_recta(opnf_dlg.scrl.zone, black, 2);
            if (opnf_dlg.scrl.drag)
                draw_color_to_zone(grey, opnf_dlg.scrl.scrler_zone);
            else
                draw_color_to_zone(l_grey, opnf_dlg.scrl.scrler_zone);

            draw_out_recta(opnf_dlg.scrl.up_zone, black, 2);
            draw_out_recta(opnf_dlg.scrl.down_zone, black, 2);
            int up_ind = in_zone(mouse_coords, opnf_dlg.scrl.up_zone) ? 1 : 0;
            instrch_from_im(scrl_up[up_ind], { 0,0,scrl_up[up_ind].w, scrl_up[up_ind].h }, opnf_dlg.scrl.up_zone);
            int down_ind = in_zone(mouse_coords, opnf_dlg.scrl.down_zone) ? 1 : 0;
            instrch_from_im(scrl_down[down_ind], { 0,0,scrl_down[down_ind].w, scrl_down[down_ind].h }, opnf_dlg.scrl.down_zone);
        }
    }
    else if (f_error_dlg)
    {
        opn_wrong_f.draw();
    }
    else if (save_file)
    {
        // draw bakgnd colors and outlines
        draw_color_to_zone(ll_grey, savf_dlg.zone);
        draw_out_recta(savf_dlg.zone, l_grey, 4);
        draw_out_recta(savf_dlg.exp_zone, black, 2);
        draw_out_recta(savf_dlg.txt_edit.zone, black, 2);
        // draw open cancel buttons
        draw_butt(savf_dlg.open_zone, L"SAVE", savf_dlg.open_zone.w / 5);
        draw_butt(savf_dlg.cancel_zone, L"CANCEL", savf_dlg.cancel_zone.w / 10);

        // handling left mouse down
        if (my_mouse.l_down)
        {
            if (in_zone(mouse_coords_ldown, savf_dlg.cancel_zone) && in_zone(mouse_coords, savf_dlg.cancel_zone))
            {
                draw_out_recta(savf_dlg.cancel_zone, black, 4);
            }
            if (in_zone(mouse_coords_ldown, savf_dlg.open_zone) && in_zone(mouse_coords, savf_dlg.open_zone))
            {
                draw_out_recta(savf_dlg.open_zone, black, 4);
            }
        }

        // hovered and selected file hilighting
        if (in_zone(mouse_coords, savf_dlg.exp_zone))
        {
            int idx_hilit = ((savf_dlg.exp_zone.y - savf_dlg.fhref) + (mouse_coords.y - savf_dlg.exp_zone.y)) / savf_dlg.file_h;
            if (idx_hilit < savf_dlg.nb_files)
                draw_color_to_zone(lg_blue, { savf_dlg.exp_zone.x, savf_dlg.fhref + idx_hilit * savf_dlg.file_h, savf_dlg.exp_zone.w, savf_dlg.file_h }, savf_dlg.exp_zone);
        }
        if (savf_dlg.fslctd_idx != -1)
        {
            draw_color_to_zone(ldg_blue, { savf_dlg.exp_zone.x, savf_dlg.fhref + savf_dlg.fslctd_idx * savf_dlg.file_h, savf_dlg.exp_zone.w, savf_dlg.file_h }, savf_dlg.exp_zone);
        }

        // draw file names
        int icon_w = savf_dlg.file_h;
        for (int i = (savf_dlg.exp_zone.y - savf_dlg.fhref) / savf_dlg.file_h; i < savf_dlg.nb_files; i++)
        {
            int row_y = savf_dlg.fhref + i * savf_dlg.file_h;
            if (row_y > savf_dlg.exp_zone.y + savf_dlg.exp_zone.h)
                break;
            if (savf_dlg.files_q[i].second)
                instrch_from_im(folder_icon, { 0,0,folder_icon.w, folder_icon.h }, { savf_dlg.exp_zone.x, row_y , icon_w, savf_dlg.file_h - savf_dlg.file_h * 17 / 80 }, false, black, savf_dlg.exp_zone);
            draw_text_ft(cascadia_face, savf_dlg.files_q[i].first, { savf_dlg.exp_zone.x + icon_w, row_y, savf_dlg.exp_zone.w - icon_w, savf_dlg.file_h }, savf_dlg.exp_zone, { 0, savf_dlg.file_h * 17 / 80, 0, savf_dlg.file_h * 17 / 80 });
        }

        // add scroll bar when necessary
        if (savf_dlg.scrl_exist)
        {
            draw_out_recta(savf_dlg.scrl.zone, black, 2);
            if (savf_dlg.scrl.drag)
                draw_color_to_zone(grey, savf_dlg.scrl.scrler_zone);
            else
                draw_color_to_zone(l_grey, savf_dlg.scrl.scrler_zone);

            draw_out_recta(savf_dlg.scrl.up_zone, black, 2);
            draw_out_recta(savf_dlg.scrl.down_zone, black, 2);
            int up_ind = in_zone(mouse_coords, savf_dlg.scrl.up_zone) ? 1 : 0;
            instrch_from_im(scrl_up[up_ind], { 0,0,scrl_up[up_ind].w, scrl_up[up_ind].h }, savf_dlg.scrl.up_zone);
            int down_ind = in_zone(mouse_coords, savf_dlg.scrl.down_zone) ? 1 : 0;
            instrch_from_im(scrl_down[down_ind], { 0,0,scrl_down[down_ind].w, scrl_down[down_ind].h }, savf_dlg.scrl.down_zone);
        }
        savf_dlg.txt_edit.draw();
    }
    else if (save_ext_err)
    {
        ext_err_info.draw();
    }
    else if (gif_sav_info)
    {
        gif_slct_info.draw();
    }

    if (show_cursor)
    {
        if (cursor_timer.total <= 0.8)
        {
            instrch_from_im(my_cursor, { 0,0,my_cursor.w, my_cursor.h }, my_cursor.scr_zone, false, black);
        }
        if (cursor_timer.total >= 1.6)
            cursor_timer.reset();
    }
}

void update_app_params()
{
    // get mouse input
    prev_mouse_coords = mouse_coords;
    if (GetCursorPos(&mouse_coords))
        if (ScreenToClient(main_wnd, &mouse_coords))
        {
        }
        else
            OutputDebugStringA("Error while getting mouse coor rel to client area\n");
    else
        OutputDebugStringA("Error while getting mouse coor rel to screen\n");

    if (mouse_coords.x > screen_zone.w || mouse_coords.x < 0 || mouse_coords.y < 0 || mouse_coords.y > screen_zone.h)
        my_mouse = initial_mouse;

    if (show_map)
    {
        sh_hide_zone = hide.scr_zone;
        map_zone = region_slct_zone;
    }
    else
    {
        sh_hide_zone = { anti_hide.scr_zone.x - 60, anti_hide.scr_zone.y, anti_hide.w + 60, anti_hide.h };
    }

    if (normal_mode || gif_saving)
    {
        // grid and region_select dragging
        if (my_mouse.l_down && !zooming)
        {
            if (mov_region)
            {
                if (in_zone(mouse_coords, region_slct_zone))
                {
                    // regulating the size adjusting for ratio problms
                    bmp_region_zone.h = std::ceil(bmp_zone.h * float(map_cell_w) / cell_h);
                    bmp_region_zone.w = std::ceil(bmp_zone.w * float(map_cell_w) / cell_w);

                    // update region position
                    bmp_region_zone.x = region_pos0.x + mouse_coords.x - mouse_coords_ldown.x;
                    bmp_region_zone.y = region_pos0.y + mouse_coords.y - mouse_coords_ldown.y;

                    // assert limits and change map_pos accordingly
                    if (bmp_region_zone.x <= region_slct_zone.x)
                    {
                        if (map_pos.x > 0)
                            map_pos.x = max(0, map_pos.x - (region_slct_zone.x - bmp_region_zone.x));
                        bmp_region_zone.x = region_slct_zone.x;
                    }
                    if (bmp_region_zone.x + bmp_region_zone.w >= region_slct_zone.x + region_slct_zone.w)
                    {
                        if (map_pos.x + map_w < nb_cell_w * map_cell_w)
                            map_pos.x = min(nb_cell_w * map_cell_w - map_w, map_pos.x + bmp_region_zone.x + bmp_region_zone.w - (region_slct_zone.x + region_slct_zone.w));
                        bmp_region_zone.x = region_slct_zone.x + region_slct_zone.w - bmp_region_zone.w;
                    }
                    if (bmp_region_zone.y <= region_slct_zone.y)
                    {
                        if (map_pos.y > 0)
                            map_pos.y = max(0, map_pos.y - region_slct_zone.y - bmp_region_zone.y);
                        bmp_region_zone.y = region_slct_zone.y;
                    }
                    if (bmp_region_zone.y + bmp_region_zone.h >= region_slct_zone.y + region_slct_zone.h)
                    {
                        if (map_pos.y + map_h < nb_cell_h * map_cell_w)
                        {
                            map_pos.y = min(nb_cell_h * map_cell_w - map_h, map_pos.y + bmp_region_zone.y + bmp_region_zone.h - (region_slct_zone.y + region_slct_zone.h));
                        }
                        bmp_region_zone.y = region_slct_zone.y + region_slct_zone.h - bmp_region_zone.h;
                    }

                    // update bitmap_pos and assert limits
                    bmp_zone.x = (bmp_region_zone.x + map_pos.x - region_slct_zone.x) * int(cell_w / map_cell_w);
                    bmp_zone.y = (bmp_region_zone.y + map_pos.y - region_slct_zone.y) * int(cell_w / map_cell_w);
                    make_inside(bmp_zone, vbmp_zone);

                }
            }
            else if (!in_zone(mouse_coords_ldown, menu_zone) && !in_zone(mouse_coords_ldown, map_zone) && !in_zone(mouse_coords_ldown, sh_hide_zone))
            {
                bmp_zone.x -= (mouse_coords.x - prev_mouse_coords.x) / zoom_ratio_x;
                bmp_zone.y -= (mouse_coords.y - prev_mouse_coords.y) / zoom_ratio_y;
                make_inside(bmp_zone, vbmp_zone);
            }
        }

        // handling zoom smoothness
        if (bmp_zone.w != bmp_zone_targ.w || bmp_zone.h != bmp_zone_targ.h)
        {
            int zoom_sign = zoom_in ? -1 : 1;
            bmp_zone.w += zoom_sign * zoom_speed * frame_dur;
            bmp_zone.h = (bmp_zone.w / screen_zone.w) * screen_zone.h;

            bmp_zone.x += -zoom_sign * pos_zpeed_x * frame_dur;
            bmp_zone.y += -zoom_sign * pos_zpeed_y * frame_dur;
            if (zoom_in)
            {
                if (bmp_zone.w < bmp_zone_targ.w)
                {
                    bmp_zone.w = bmp_zone_targ.w;
                    bmp_zone.x = bmp_zone_targ.x;
                    zooming = false;
                }
                if (bmp_zone.h < bmp_zone_targ.h)
                {
                    bmp_zone.h = bmp_zone_targ.h;
                    bmp_zone.y = bmp_zone_targ.y;
                    zooming = false;
                }
            }
            else
            {
                if (bmp_zone.w > bmp_zone_targ.w)
                {
                    bmp_zone.w = bmp_zone_targ.w;
                    bmp_zone.x = bmp_zone_targ.x;
                    zooming = false;

                }
                if (bmp_zone.h > bmp_zone_targ.h)
                {
                    bmp_zone.h = bmp_zone_targ.h;
                    bmp_zone.y = bmp_zone_targ.y;
                    zooming = false;
                }
            }
            zoom_ratio_x = double(screen_zone.w) / bmp_zone.w;
            zoom_ratio_y = double(screen_zone.h) / bmp_zone.h;
            make_inside(bmp_zone, vbmp_zone);
        }

        // change region_pos after zoom and dragging handled
        if (!mov_region)
        {
            bmp_region_zone.x = bmp_zone.x * map_cell_w / cell_w - map_pos.x + region_slct_zone.x;
            bmp_region_zone.y = bmp_zone.y * map_cell_w / cell_w - map_pos.y + region_slct_zone.y;
            if (bmp_region_zone.x <= region_slct_zone.x)
            {
                if (map_pos.x > 0)
                    map_pos.x = max(0, map_pos.x - (region_slct_zone.x - bmp_region_zone.x));
                bmp_region_zone.x = region_slct_zone.x;
            }
            if (bmp_region_zone.x + bmp_region_zone.w >= region_slct_zone.x + region_slct_zone.w)
            {
                if (map_pos.x + map_w < nb_cell_w * map_cell_w)
                    map_pos.x = min(nb_cell_w * map_cell_w - map_w, map_pos.x + bmp_region_zone.x + bmp_region_zone.w - (region_slct_zone.x + region_slct_zone.w));
                bmp_region_zone.x = region_slct_zone.x + region_slct_zone.w - bmp_region_zone.w;
            }
            if (bmp_region_zone.y <= region_slct_zone.y)
            {
                if (map_pos.y > 0)
                    map_pos.y = max(0, map_pos.y - region_slct_zone.y - bmp_region_zone.y);
                bmp_region_zone.y = region_slct_zone.y;
            }
            if (bmp_region_zone.y + bmp_region_zone.h >= region_slct_zone.y + region_slct_zone.h)
            {
                if (map_pos.y + map_h < nb_cell_h * map_cell_w)
                    map_pos.y = min(nb_cell_h * map_cell_w - map_h, map_pos.y + bmp_region_zone.y + bmp_region_zone.h - (region_slct_zone.y + region_slct_zone.h));
                bmp_region_zone.y = region_slct_zone.y + region_slct_zone.h - bmp_region_zone.h;
            }
            if (bmp_zone.x == nb_cell_w * cell_w - bmp_zone.w && int(bmp_zone.w) % cell_w != 0)
                bmp_region_zone.w = bmp_zone.w * map_cell_w / cell_w + 1;
            else
                bmp_region_zone.w = bmp_zone.w * map_cell_w / cell_w;
            if (bmp_zone.y == nb_cell_h * cell_h - bmp_zone.h && int(bmp_zone.h) % cell_h != 0)
                bmp_region_zone.h = bmp_zone.h * map_cell_w / cell_h + 1;
            else
                bmp_region_zone.h = bmp_zone.h * map_cell_w / cell_h;
        }

        if (normal_mode)
        {
            if (int_bar_slctd)
            {
                intens_slct.scr_zone.x = mouse_coords.x - intens_slct.scr_zone.w / 2;
                if (mouse_coords.x >= intens_bar.scr_zone.x + intens_bar.scr_zone.w)
                    intens_slct.scr_zone.x = intens_bar.scr_zone.x + intens_bar.scr_zone.w - intens_slct.scr_zone.w / 2;
                if (mouse_coords.x <= intens_bar.scr_zone.x)
                    intens_slct.scr_zone.x = intens_bar.scr_zone.x - intens_slct.scr_zone.w / 2;
            }
            sim_freq = double(5 * (intens_slct.scr_zone.x + intens_slct.scr_zone.w / 2 - intens_bar.scr_zone.x)) / intens_bar.scr_zone.w + 1;
            step_dur = 1.0 / sim_freq;

            // mouse activation of grid cells + menu zone interaction
            if (my_mouse.r_down)
            {
                if (!in_zone(mouse_coords, menu_zone) && !in_zone(mouse_coords, map_zone) && !in_zone(mouse_coords, sh_hide_zone))
                {
                    static int last_i;
                    static int last_j;
                    point act_cell = get_clk_cell(mouse_coords);
                    int i = act_cell.x;
                    int j = act_cell.y;
                    if (i != last_i || j != last_j)
                    {
                        if (i != 0 && i != nb_cell_w - 1 && j != 0 && j != nb_cell_h - 1)
                        {
                            if (!grid[j * nb_cell_w + i])
                                grid_init_stt.push_back(point{ i, j });
                            else
                                grid_init_stt.erase(std::remove(grid_init_stt.begin(), grid_init_stt.end(), point({ i,j })), grid_init_stt.end());
                            if (i != 0 && i != nb_cell_w - 1 && j != 0 && j != nb_cell_h - 1)
                                grid[j * nb_cell_w + i] = grid[j * nb_cell_w + i] ? 0 : 1;
                            if (grid[j * nb_cell_w + i])
                            {
                                if (i < stepping_limits.left)
                                    stepping_limits.left = i;
                                else if (i > stepping_limits.right)
                                    stepping_limits.right = i;
                                if (j < stepping_limits.top)
                                    stepping_limits.top = j;
                                else if (j > stepping_limits.bottom)
                                    stepping_limits.bottom = j;
                            }
                        }
                        initial_limits = stepping_limits;
                    }
                    last_i = i;
                    last_j = j;
                }
            }
            step_edit.handle_slct();
            my_cursor.scr_zone.x = step_edit.chars_x[step_edit.curs_idx] - step_edit.dx_ref;

            // play button click = simulation on => stepping ahead with given velocity
            if (sim_on)
            {
                if (sim_timer.total >= step_dur)
                {
                    sim_timer.reset();
                    one_step_ahead();
                }
            }
            if (actual_step == target_step)
            {
                prv_targ_stp = target_step;
                sim_on = false;
            }
        }
        if (gif_saving)
        {
            if (gif_params_collect)
            {
                for (int i = 0; i < gif_params_dlg.txt_edits.size(); i++)
                {
                    gif_params_dlg.txt_edits[i].handle_slct();
                }
                txt_edit_params& temp = gif_params_dlg.txt_edits[gif_params_dlg.slctd_edit];
                my_cursor.scr_zone.x = temp.chars_x[temp.curs_idx] - temp.dx_ref;
            }
        }
    }
    else if (open_file) // show the open file window
    {
        // handling left mouse down
        if (my_mouse.l_down)
        {
            if (opnf_dlg.scrl_exist)
            {
                if (opnf_dlg.scrl.drag)
                {
                    opnf_dlg.scrl.scrler_zone.y = opnf_dlg.scrl.y0 + mouse_coords.y - mouse_coords_ldown.y;
                    opnf_dlg.fhref = opnf_dlg.exp_zone.y - (opnf_dlg.scrl.scrler_zone.y - opnf_dlg.scrl.zone.y) * opnf_dlg.scrl.ratio;
                }
                else if (opnf_dlg.scrl.timer.total > 0.3)
                {
                    if (in_zone(mouse_coords, opnf_dlg.scrl.up_zone) && in_zone(mouse_coords_ldown, opnf_dlg.scrl.up_zone))
                    {
                        opnf_dlg.fhref += opnf_dlg.file_h;
                        opnf_dlg.scrl.scrler_zone.y = opnf_dlg.scrl.zone.y + (opnf_dlg.exp_zone.y - opnf_dlg.fhref) / opnf_dlg.scrl.ratio;
                    }
                    else if (in_zone(mouse_coords, opnf_dlg.scrl.down_zone) && in_zone(mouse_coords_ldown, opnf_dlg.scrl.down_zone))
                    {
                        opnf_dlg.fhref -= opnf_dlg.file_h;
                        opnf_dlg.scrl.scrler_zone.y = opnf_dlg.scrl.zone.y + (opnf_dlg.exp_zone.y - opnf_dlg.fhref) / opnf_dlg.scrl.ratio;
                    }
                    else if (opnf_dlg.scrl.attrct)
                    {
                        if (in_zone(mouse_coords, opnf_dlg.scrl.zone) && !in_zone(mouse_coords, opnf_dlg.scrl.scrler_zone))
                        {
                            int new_dir = (opnf_dlg.scrl.scrler_zone.y - mouse_coords.y > 0) ? -1 : 1;
                            if (new_dir == opnf_dlg.scrl.dir)
                            {
                                opnf_dlg.scrl.scrler_zone.y += opnf_dlg.scrl.dir * opnf_dlg.scrl.scrler_zone.h * 0.7;
                                opnf_dlg.fhref = opnf_dlg.exp_zone.y - (opnf_dlg.scrl.scrler_zone.y - opnf_dlg.scrl.zone.y) * opnf_dlg.scrl.ratio;
                            }
                        }
                    }
                }
                make_inside(opnf_dlg.scrl.scrler_zone.y, opnf_dlg.scrl.zone.y, opnf_dlg.scrl.zone.y + opnf_dlg.scrl.zone.h - opnf_dlg.scrl.scrler_zone.h);
                make_inside(opnf_dlg.fhref, opnf_dlg.exp_zone.y + opnf_dlg.exp_zone.h - (opnf_dlg.nb_files + 2) * opnf_dlg.file_h, opnf_dlg.exp_zone.y);
            }
        }
    }
    else if (save_file)
    {
        // handling left mouse down
        if (my_mouse.l_down)
        {
            if (savf_dlg.scrl_exist)
            {
                if (savf_dlg.scrl.drag)
                {
                    savf_dlg.scrl.scrler_zone.y = savf_dlg.scrl.y0 + mouse_coords.y - mouse_coords_ldown.y;
                    savf_dlg.fhref = savf_dlg.exp_zone.y - (savf_dlg.scrl.scrler_zone.y - savf_dlg.scrl.zone.y) * savf_dlg.scrl.ratio;
                }
                else if (savf_dlg.scrl.timer.total > 0.3)
                {
                    if (in_zone(mouse_coords, savf_dlg.scrl.up_zone) && in_zone(mouse_coords_ldown, savf_dlg.scrl.up_zone))
                    {
                        savf_dlg.fhref += savf_dlg.file_h;
                        savf_dlg.scrl.scrler_zone.y = savf_dlg.scrl.zone.y + (savf_dlg.exp_zone.y - savf_dlg.fhref) / savf_dlg.scrl.ratio;
                    }
                    else if (in_zone(mouse_coords, savf_dlg.scrl.down_zone) && in_zone(mouse_coords_ldown, savf_dlg.scrl.down_zone))
                    {
                        savf_dlg.fhref -= savf_dlg.file_h;
                        savf_dlg.scrl.scrler_zone.y = savf_dlg.scrl.zone.y + (savf_dlg.exp_zone.y - savf_dlg.fhref) / savf_dlg.scrl.ratio;
                    }
                    else if (savf_dlg.scrl.attrct)
                    {
                        if (in_zone(mouse_coords, savf_dlg.scrl.zone) && !in_zone(mouse_coords, savf_dlg.scrl.scrler_zone))
                        {
                            int new_dir = (savf_dlg.scrl.scrler_zone.y - mouse_coords.y > 0) ? -1 : 1;
                            if (new_dir == savf_dlg.scrl.dir)
                            {
                                savf_dlg.scrl.scrler_zone.y += savf_dlg.scrl.dir * savf_dlg.scrl.scrler_zone.h * 0.7;
                                savf_dlg.fhref = savf_dlg.exp_zone.y - (savf_dlg.scrl.scrler_zone.y - savf_dlg.scrl.zone.y) * savf_dlg.scrl.ratio;
                            }
                        }
                    }
                }
                make_inside(savf_dlg.scrl.scrler_zone.y, savf_dlg.scrl.zone.y, savf_dlg.scrl.zone.y + savf_dlg.scrl.zone.h - savf_dlg.scrl.scrler_zone.h);
                make_inside(savf_dlg.fhref, savf_dlg.exp_zone.y + savf_dlg.exp_zone.h - (savf_dlg.nb_files + 2) * savf_dlg.file_h, savf_dlg.exp_zone.y);
            }
        }
        savf_dlg.txt_edit.handle_slct();
        my_cursor.scr_zone.x = savf_dlg.txt_edit.chars_x[savf_dlg.txt_edit.curs_idx] - savf_dlg.txt_edit.dx_ref;
    }
	else if (go_mode)
	{
#ifdef gpu_acceleration
        ID3D11Device* pdevice;
        D3D_FEATURE_LEVEL feat_lvl;
        ID3D11DeviceContext* pimmediate_context;
        HRESULT hr = D3D11CreateDevice(NULL,
            D3D_DRIVER_TYPE_HARDWARE,
            NULL,
            D3D11_CREATE_DEVICE_DEBUG,
            NULL,
            0,
            D3D11_SDK_VERSION,
            &pdevice,
            &feat_lvl,
            &pimmediate_context
        );
        assert(hr == S_OK);
        assert(feat_lvl == D3D_FEATURE_LEVEL_11_0);

        std::ifstream fin("compute_sh.o", std::ios::binary);
        fin.seekg(0, std::ios_base::end);
        int size = (int)fin.tellg();
        std::vector<char> compute_sh(size);
        fin.seekg(0, std::ios_base::beg);
        fin.read(compute_sh.data(), size);
        fin.close();

        ID3D11ComputeShader* comp_sh;
        hr = pdevice->CreateComputeShader(compute_sh.data(), size, 0, &comp_sh);
        assert(hr == S_OK);

        // buffers description and creation. 
        D3D11_BUFFER_DESC data_buff_bd;
        data_buff_bd.ByteWidth = nb_cells * 4;
        data_buff_bd.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
        data_buff_bd.StructureByteStride = 4;
        data_buff_bd.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
        data_buff_bd.Usage = D3D11_USAGE_DEFAULT;
        data_buff_bd.CPUAccessFlags = 0;
        D3D11_SUBRESOURCE_DATA data_sbr;
        data_sbr.pSysMem = grid;

        ID3D11Buffer* first_buff;
        hr = pdevice->CreateBuffer(&data_buff_bd, &data_sbr, &first_buff);
        assert(hr == S_OK);
        ID3D11Buffer* second_buff;
        hr = pdevice->CreateBuffer(&data_buff_bd, nullptr, &second_buff);
        assert(hr == S_OK);
        
        //limits buffer creation : this is for the new limits of calculation when stepping ahead with CPU
        D3D11_BUFFER_DESC limits_buff_desc;
        limits_buff_desc.ByteWidth = 16;
        limits_buff_desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
        limits_buff_desc.StructureByteStride = 4;
        limits_buff_desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
        limits_buff_desc.Usage = D3D11_USAGE_DEFAULT;
        limits_buff_desc.CPUAccessFlags = 0;

        uint32 limits[4] = { stepping_limits.left, stepping_limits.top, stepping_limits.right, stepping_limits.bottom};
        ID3D11Buffer* limits_buff = nullptr;
        D3D11_SUBRESOURCE_DATA limits_sbr;
        limits_sbr.pSysMem = &limits;
        pdevice->CreateBuffer(&limits_buff_desc, &limits_sbr, &limits_buff);
        assert(hr == S_OK);

        // unordered access views creation 
        D3D11_UNORDERED_ACCESS_VIEW_DESC buff_vdesc;
        buff_vdesc.Format = DXGI_FORMAT_UNKNOWN;
        buff_vdesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
        buff_vdesc.Buffer.NumElements = nb_cells;
        buff_vdesc.Buffer.FirstElement = 0;
        buff_vdesc.Buffer.Flags = 0;

        ID3D11UnorderedAccessView* first_uav;
        hr = pdevice->CreateUnorderedAccessView(first_buff, &buff_vdesc, &first_uav);
        assert(hr == S_OK);
        ID3D11UnorderedAccessView* second_uav;
        hr = pdevice->CreateUnorderedAccessView(second_buff, &buff_vdesc, &second_uav);
        assert(hr == S_OK);

        buff_vdesc.Buffer.NumElements = 4;
        ID3D11UnorderedAccessView* limits_uav;
        hr = pdevice->CreateUnorderedAccessView(limits_buff, &buff_vdesc, &limits_uav);
        assert(hr == S_OK);

        // constant buffer holds a switch to use result as input for next dispath operation 
        D3D11_BUFFER_DESC cbuff_desc;
        cbuff_desc.ByteWidth = 16;
        cbuff_desc.MiscFlags = 0;
        cbuff_desc.StructureByteStride = 0;
        cbuff_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        cbuff_desc.Usage = D3D11_USAGE_DYNAMIC;
        cbuff_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        struct my_cbuff
        {
            uint32 grid_w;
            uint32 grid_h;
            uint32 grps_per_dim;
            uint32 calc_switch;
        } cbuff_data;

        ID3D11Buffer* switch_buff = nullptr;
        pdevice->CreateBuffer(&cbuff_desc, nullptr, &switch_buff);
        assert(hr == S_OK);

        // run the compute shader 
        int nb_groups = ceil(nb_cells /32.0f);
        int grps_per_dim = ceil(cbrt(nb_groups));

        cbuff_data.calc_switch = 0;
        cbuff_data.grid_w = nb_cell_w;
        cbuff_data.grid_h = nb_cell_h;
        cbuff_data.grps_per_dim = grps_per_dim;

        ID3D11UnorderedAccessView* uavs[3] = { first_uav, second_uav, limits_uav };
        pimmediate_context->CSSetShader(comp_sh, nullptr, 0);
        pimmediate_context->CSSetUnorderedAccessViews(0, 3, uavs, nullptr);
        pimmediate_context->CSSetConstantBuffers(0, 1, &switch_buff);
        D3D11_MAPPED_SUBRESOURCE mapped_sbr;
        while(actual_step != target_step)
        {
            pimmediate_context->Map(switch_buff, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_sbr);
            memcpy(mapped_sbr.pData, &cbuff_data, sizeof(my_cbuff));
            pimmediate_context->Unmap(switch_buff, 0);
            pimmediate_context->Dispatch(grps_per_dim, grps_per_dim, grps_per_dim);
            actual_step++;
            cbuff_data.calc_switch = cbuff_data.calc_switch ? 0 : 1;
        }
        // copy the result back to the grid 
        D3D11_BUFFER_DESC staging_bd;
        ID3D11Buffer* output_buff;
        if(cbuff_data.calc_switch)
            output_buff = second_buff;
        else
            output_buff = first_buff;
        output_buff->GetDesc(&staging_bd);
        staging_bd.Usage = D3D11_USAGE_STAGING;
        staging_bd.BindFlags = 0;
        staging_bd.MiscFlags = 0;
        staging_bd.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

        ID3D11Buffer* staging_buff = nullptr;
        hr = pdevice->CreateBuffer(&staging_bd, NULL, &staging_buff);
        assert(hr == S_OK);
        pimmediate_context->CopyResource(staging_buff, output_buff);
        pimmediate_context->Map(staging_buff, 0, D3D11_MAP_READ, 0, &mapped_sbr);
        memcpy(grid, mapped_sbr.pData, nb_cell_h * nb_cell_w * 4);
        pimmediate_context->Unmap(staging_buff, 0);

        // gettin the new limits back 
        limits_buff->GetDesc(&staging_bd);
        staging_bd.Usage = D3D11_USAGE_STAGING;
        staging_bd.BindFlags = 0;
        staging_bd.MiscFlags = 0;
        staging_bd.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

        hr = pdevice->CreateBuffer(&staging_bd, NULL, &staging_buff);
        assert(hr == S_OK);
        pimmediate_context->CopyResource(staging_buff, limits_buff);
        pimmediate_context->Map(staging_buff, 0, D3D11_MAP_READ, 0, &mapped_sbr);
        memcpy(&stepping_limits, mapped_sbr.pData, 16);
        pimmediate_context->Unmap(staging_buff, 0);

        // clearing and releasing everything
        pimmediate_context->CSSetShader(nullptr, nullptr, 0);
        ID3D11UnorderedAccessView* uav_null[2] = { nullptr, nullptr };
        pimmediate_context->CSSetUnorderedAccessViews(0, 2, uav_null, nullptr);
        ID3D11Buffer* cb_null[1] = { nullptr };
        pimmediate_context->CSSetConstantBuffers(0, 1, cb_null);

        SAFE_RELEASE(first_buff);
        SAFE_RELEASE(second_buff);
        SAFE_RELEASE(first_uav);
        SAFE_RELEASE(second_uav);
        SAFE_RELEASE(comp_sh);
        SAFE_RELEASE(pimmediate_context);
        SAFE_RELEASE(pdevice);
        SAFE_RELEASE(switch_buff);
#else
        while (actual_step != target_step)
            one_step_ahead();
#endif
        normal_mode = true;
        go_mode = false;
        prv_targ_stp = target_step;
    }
}

void init_everything()
{
    QueryPerformanceFrequency((LARGE_INTEGER*)&timer::count_freq);
    bmp_zone.w = screen_zone.w;
    bmp_zone.h = screen_zone.h;
    bmp_zone_targ.w = bmp_zone.w;
    bmp_zone_targ.h = bmp_zone.h;
    zoom_ratio_x = double(screen_zone.w) / bmp_zone.w;
    zoom_ratio_y = double(screen_zone.h) / bmp_zone.h;
    bmp_zone.x = double(nb_cell_w * cell_w - screen_zone.w) / 2;
    bmp_zone.y = double(nb_cell_h * cell_h - screen_zone.h) / 2;
    vbmp_zone = { 0, 0, nb_cell_w * cell_w, nb_cell_h * cell_h };

    stepping_limits.top = nb_cell_h;
    stepping_limits.left = nb_cell_w;
    stepping_limits.right = 0;
    stepping_limits.bottom = 0;
    initial_limits = stepping_limits;

    // load images
    for (int i = 0; i < 3; i++)
    {
        play_butt[i].pixels = (uint32*)stbi_load(play_butt_fn[i], &play_butt[i].w, &play_butt[i].h, NULL, 0);
        pause_butt[i].pixels = (uint32*)stbi_load(pause_butt_fn[i], &pause_butt[i].w, &pause_butt[i].h, NULL, 0);
        reinit_butt[i].pixels = (uint32*)stbi_load(reinit_butt_fn[i], &reinit_butt[i].w, &reinit_butt[i].h, NULL, 0);
        next_butt[i].pixels = (uint32*)stbi_load(next_butt_fn[i], &next_butt[i].w, &next_butt[i].h, NULL, 0);
    }
    for (int i = 0; i < 2; i++)
    {
        scrl_up[i].pixels = (uint32*)stbi_load(scrl_up_fn[i], &scrl_up[i].w, &scrl_up[i].h, NULL, 0);
        scrl_down[i].pixels = (uint32*)stbi_load(scrl_down_fn[i], &scrl_down[i].w, &scrl_down[i].h, NULL, 0);
    }
    intens_slct.pixels = (uint32*)stbi_load(intens_slct_fn, &intens_slct.w, &intens_slct.h, NULL, 0);
    intens_bar.pixels = (uint32*)stbi_load(intens_bar_fn, &intens_bar.w, &intens_bar.h, NULL, 0);
    my_cursor.pixels = (uint32*)stbi_load(cursor_fn, &my_cursor.w, &my_cursor.h, NULL, 0);
    num_letters.pixels = (uint32*)stbi_load(num_letters_fn, &num_letters.w, &num_letters.h, NULL, 0);
    hide.pixels = (uint32*)stbi_load(hide_fn, &hide.w, &hide.h, NULL, 0);
    anti_hide.pixels = (uint32*)stbi_load(anti_hide_fn, &anti_hide.w, &anti_hide.h, NULL, 0);
    folder_icon.pixels = (uint32*)stbi_load(folder_icon_fn, &folder_icon.w, &folder_icon.h, NULL, 0);

    // initialise freetype, only one face font is used 
    FT_Init_FreeType(&ft); // TODO: handle errors 
    FT_New_Face(ft, cascadia_font, 0, &cascadia_face); // TODO: handle erros 

    // set different zones and positions
    menu_zone = { 4,4,int(0.60 * screen_zone.w),int(0.15 * screen_zone.h) };
    num_width = menu_zone.w / 60;
    num_height = menu_zone.h / 4;

    pause_butt[0].scr_zone = { menu_zone.x, menu_zone.y, menu_zone.w / 12, 15 * menu_zone.h / 24 };
    play_butt[0].scr_zone = pause_butt[0].scr_zone;
    next_butt[0].scr_zone = { play_butt[0].scr_zone.x + menu_zone.w / 96, play_butt[0].scr_zone.y + play_butt[0].scr_zone.h + menu_zone.h / 24, menu_zone.w / 16,  7 * menu_zone.h / 24 };
    reinit_butt[0].scr_zone = { play_butt[0].scr_zone.x + menu_zone.w / 12 , play_butt[0].scr_zone.y, menu_zone.w / 12, 15 * menu_zone.h / 24 };
    for (int i = 1; i < 3; i++)
    {
        pause_butt[i].scr_zone = pause_butt[0].scr_zone;
        play_butt[i].scr_zone = play_butt[0].scr_zone;
        next_butt[i].scr_zone = next_butt[0].scr_zone;
        reinit_butt[i].scr_zone = reinit_butt[0].scr_zone;
    }

    intens_bar.scr_zone = { menu_zone.x + 2 * menu_zone.w / 12 + menu_zone.w / 80 + menu_zone.w / 50, menu_zone.y + 3 * menu_zone.h / 5, menu_zone.w / 3 - menu_zone.w / 40 - 2 * num_width - menu_zone.w / 50, menu_zone.h / 20 };
    intens_slct.scr_zone = { intens_bar.scr_zone.x - menu_zone.w / 80, intens_bar.scr_zone.y + intens_bar.scr_zone.h / 2 - menu_zone.h / 4, menu_zone.w / 40, menu_zone.h / 2 };

    step_edit.zone = { menu_zone.x + 6 * menu_zone.w / 12 + menu_zone.w / 32, menu_zone.y + menu_zone.h / 3 , menu_zone.w * 5 / 48, num_height + 6 };

    go_butt_zone = { step_edit.zone.x , step_edit.zone.y + step_edit.zone.h + menu_zone.h / 12, step_edit.zone.w, menu_zone.h / 4 };

    go_to_stp_zone = { menu_zone.x + 6 * menu_zone.w / 12 + menu_zone.w / 48, step_edit.zone.y - menu_zone.h * 25 / 120, 3 * menu_zone.w / 24, menu_zone.h * 15 / 120 };
    actual_step_zone = { menu_zone.x + 8 * menu_zone.w / 12 + menu_zone.w / 48, go_to_stp_zone.y, menu_zone.w / 6 , menu_zone.h * 15 / 120 };

    save_butt_zone = { menu_zone.x + 10 * menu_zone.w / 12 + menu_zone.w / 16, actual_step_zone.y + menu_zone.h * 5 / 120, menu_zone.w / 12, menu_zone.h * 40 / 120 };
    open_butt_zone = { menu_zone.x + 10 * menu_zone.w / 12 + menu_zone.w / 16, actual_step_zone.y + save_butt_zone.h + menu_zone.h * 10 / 120, menu_zone.w / 12, menu_zone.h * 40 / 120 };
    clear_butt_zone = { reinit_butt[0].scr_zone.x + menu_zone.w / 96, reinit_butt[0].scr_zone.y + reinit_butt[0].scr_zone.h + menu_zone.h / 24, menu_zone.w / 16, 7 * menu_zone.h / 24 };

    sim_vel_zone = { intens_bar.scr_zone.x - menu_zone.w * 8 / 800, go_to_stp_zone.y , intens_bar.scr_zone.w + menu_zone.w * 20 / 800, menu_zone.h * 15 / 120 };
    region_slct_zone = { screen_zone.w - map_w - menu_zone.w * 20 / 800, screen_zone.h - map_h - menu_zone.h * 20 / 120, map_w, map_h };
    map_pos = { double(nb_cell_w * map_cell_w - map_w) / 2 , double(nb_cell_h * map_cell_w - map_h) / 2 };

    bmp_region_zone.x = bmp_zone.x * int(map_cell_w / cell_w) - map_pos.x + region_slct_zone.x;
    bmp_region_zone.y = bmp_zone.y * int(map_cell_w / cell_w) - map_pos.y + region_slct_zone.y;
    bmp_region_zone.w = int(bmp_zone.w) * map_cell_w / cell_w;
    bmp_region_zone.h = int(bmp_zone.h) * map_cell_w / cell_h;

    hide.scr_zone.x = region_slct_zone.x + region_slct_zone.w - hide.w + menu_zone.w * 3 / 800;
    hide.scr_zone.y = region_slct_zone.y - hide.h - menu_zone.h * 3.0 / 120;
    anti_hide.scr_zone.x = region_slct_zone.x + region_slct_zone.w - hide.w;
    anti_hide.scr_zone.y = region_slct_zone.y + region_slct_zone.h - hide.h;

    opnf_dlg.zone = { screen_zone.w / 3, screen_zone.h / 5, screen_zone.w / 3, 3 * screen_zone.h / 5 };
    opnf_dlg.exp_zone_wscrl = { opnf_dlg.zone.x + opnf_dlg.zone.w / 20, opnf_dlg.zone.y + opnf_dlg.zone.h / 20, 18 * opnf_dlg.zone.w / 20, 16 * opnf_dlg.zone.h / 20 };
    opnf_dlg.exp_zone = opnf_dlg.exp_zone_wscrl;
    opnf_dlg.scrl.scrler_zone = { opnf_dlg.exp_zone.x + opnf_dlg.exp_zone.w - opnf_dlg.exp_zone.w / 16, opnf_dlg.exp_zone.y + opnf_dlg.exp_zone.h / 16 + 2, opnf_dlg.exp_zone.w / 16, opnf_dlg.exp_zone.h - 2 * opnf_dlg.exp_zone.h / 16 - 4 };
    opnf_dlg.scrl.zone = opnf_dlg.scrl.scrler_zone;
    opnf_dlg.scrl.up_zone = { opnf_dlg.scrl.zone.x, opnf_dlg.exp_zone.y, opnf_dlg.scrl.zone.w, opnf_dlg.exp_zone.h / 16 };
    opnf_dlg.scrl.down_zone = { opnf_dlg.scrl.zone.x, opnf_dlg.scrl.zone.y + opnf_dlg.scrl.zone.h + 2, opnf_dlg.scrl.zone.w, opnf_dlg.exp_zone.h - opnf_dlg.scrl.zone.h - opnf_dlg.scrl.up_zone.h - 4 };
    opnf_dlg.exp_zone_scrl = opnf_dlg.exp_zone_wscrl;
    opnf_dlg.exp_zone_scrl.w -= opnf_dlg.scrl.scrler_zone.w + 2;
    opnf_dlg.open_zone = { opnf_dlg.zone.x + 9 * opnf_dlg.zone.w / 12 , opnf_dlg.zone.y + 18 * opnf_dlg.zone.h / 20, opnf_dlg.zone.w / 6, opnf_dlg.zone.h / 20 };
    opnf_dlg.cancel_zone = { opnf_dlg.zone.x + 6 * opnf_dlg.zone.w / 12, opnf_dlg.zone.y + 18 * opnf_dlg.zone.h / 20, opnf_dlg.zone.w / 6, opnf_dlg.zone.h / 20 };
    opnf_dlg.file_h = opnf_dlg.exp_zone.h / 10;
    opnf_dlg.fhref = opnf_dlg.exp_zone.y;

    savf_dlg.zone = opnf_dlg.zone;
    savf_dlg.exp_zone_wscrl = opnf_dlg.exp_zone_wscrl;
    savf_dlg.exp_zone_wscrl.h = 13 * savf_dlg.zone.h / 20;
    savf_dlg.exp_zone = savf_dlg.exp_zone_wscrl;
    savf_dlg.scrl.scrler_zone = { savf_dlg.exp_zone.x + savf_dlg.exp_zone.w - savf_dlg.exp_zone.w / 16, savf_dlg.exp_zone.y + savf_dlg.exp_zone.h / 16 + 2, savf_dlg.exp_zone.w / 16, savf_dlg.exp_zone.h - 2 * savf_dlg.exp_zone.h / 16 - 4 };
    savf_dlg.scrl.zone = savf_dlg.scrl.scrler_zone;
    savf_dlg.scrl.up_zone = { savf_dlg.scrl.zone.x, savf_dlg.exp_zone.y, savf_dlg.scrl.zone.w, savf_dlg.exp_zone.h / 16 };
    savf_dlg.scrl.down_zone = { savf_dlg.scrl.zone.x, savf_dlg.scrl.zone.y + savf_dlg.scrl.zone.h + 2, savf_dlg.scrl.zone.w, savf_dlg.exp_zone.h - savf_dlg.scrl.zone.h - savf_dlg.scrl.up_zone.h - 4 };
    savf_dlg.exp_zone_scrl = savf_dlg.exp_zone_wscrl;
    savf_dlg.exp_zone_scrl.w -= savf_dlg.scrl.scrler_zone.w + 2;
    savf_dlg.open_zone = opnf_dlg.open_zone;
    savf_dlg.cancel_zone = opnf_dlg.cancel_zone;
    savf_dlg.file_h = savf_dlg.exp_zone.h / 10;
    savf_dlg.fhref = savf_dlg.exp_zone.y;
    savf_dlg.txt_edit.zone = { savf_dlg.exp_zone.x, savf_dlg.exp_zone.y + savf_dlg.exp_zone.h + savf_dlg.zone.h / 20, savf_dlg.exp_zone.w, savf_dlg.file_h };
    savf_dlg.txt_edit.txt_zone = { savf_dlg.txt_edit.zone.x + savf_dlg.txt_edit.zone.w / 40  , savf_dlg.txt_edit.zone.y + savf_dlg.file_h / 8 , savf_dlg.exp_zone.w - savf_dlg.exp_zone.w / 20, 3 * savf_dlg.file_h / 4 };

    opnf_dlg.allowed_ext.push_back(L".txt");
    savf_dlg.allowed_ext.push_back(L".txt");
    savf_dlg.allowed_ext.push_back(L".gif");

    opn_wrong_f.txt = L"The chosen file contains incompatible data! Please choose a valid file.";
    opn_wrong_f.title = L"Error";
    opn_wrong_f.zone = { opnf_dlg.zone.x + opnf_dlg.zone.w / 10, opnf_dlg.zone.y + opnf_dlg.zone.h / 10, 8 * opnf_dlg.zone.w / 10, 0 };
    opn_wrong_f.title_h = opnf_dlg.zone.w / 12;
    opn_wrong_f.fill_params(cascadia_face);

    gif_slct_info.txt = L"Hold mouse right button and select the region you want to save as gif.";
    gif_slct_info.title = L"INFO";
    gif_slct_info.zone = { savf_dlg.zone.x + savf_dlg.zone.w / 10, savf_dlg.zone.y + savf_dlg.zone.h / 10, 8 * opnf_dlg.zone.w / 10,0 };
    gif_slct_info.title_h = savf_dlg.zone.w / 12;
    gif_slct_info.fill_params(cascadia_face);
    
    ext_err_info.txt = L"The extension of the file should be \".txt\" or \".gif\"";
    ext_err_info.title = L"File Type Error";
    ext_err_info.zone = { savf_dlg.zone.x + savf_dlg.zone.w / 10, savf_dlg.zone.y + savf_dlg.zone.h / 10, 8 * opnf_dlg.zone.w / 10,0 };
    ext_err_info.title_h = savf_dlg.zone.w / 12;
    ext_err_info.fill_params(cascadia_face);
    
    gif_params_dlg.zone = { savf_dlg.zone.x + savf_dlg.zone.w / 10, savf_dlg.zone.y + savf_dlg.zone.h / 10, 8 * opnf_dlg.zone.w / 10,0 };
    gif_params_dlg.title = L"Parameters";
    gif_params_dlg.title_h = savf_dlg.zone.w / 12;
    gif_params_dlg.txts.push_back(L"Velocity (generation/second) : ");
    gif_params_dlg.txts.push_back(L"Number of generations : ");
    gif_params_dlg.txt_zones.push_back({ gif_params_dlg.zone.x + gif_params_dlg.zone.w / 20, gif_params_dlg.zone.y + 3 * gif_params_dlg.title_h / 2, 2 * gif_params_dlg.zone.w / 3, 5 * gif_params_dlg.title_h / 12 });
    gif_params_dlg.txt_zones.push_back({ gif_params_dlg.zone.x + gif_params_dlg.zone.w / 20, gif_params_dlg.zone.y + 5 * gif_params_dlg.title_h / 2, 2 * gif_params_dlg.zone.w / 3, 5 * gif_params_dlg.title_h / 12 });
    
    txt_edit_params temp;
    temp.zone = { gif_params_dlg.zone.x + 14 * gif_params_dlg.zone.w / 20, gif_params_dlg.zone.y + 3 * gif_params_dlg.title_h / 2, gif_params_dlg.zone.w / 4, gif_params_dlg.title_h / 2 };
    temp.txt_zone = { temp.zone.x + temp.zone.w / 20, temp.zone.y + temp.zone.h / 8  , temp.zone.w - temp.zone.w / 10, 3 * temp.zone.h / 4 };
    gif_params_dlg.txt_edits.push_back(temp);
    temp.zone.y += gif_params_dlg.title_h;
    temp.txt_zone.y += gif_params_dlg.title_h;
    gif_params_dlg.txt_edits.push_back(temp);

    gif_params_dlg.fill_params();

    // step_edit params 
    step_edit.txt_zone = { step_edit.zone.x + step_edit.zone.w / 10, step_edit.zone.y + step_edit.zone.h / 8, step_edit.zone.w - step_edit.zone.w / 5,  3 * step_edit.zone.h / 4 };
    my_cursor.scr_zone = { step_edit.txt_zone.x, step_edit.txt_zone.y , 2, step_edit.txt_zone.h };
    step_edit.chars_x.push_back(my_cursor.scr_zone.x);
    step_edit.dx_ref = 0;
    step_edit.curs_idx = 0;
    step_edit.max_crcs = 5;

    // Load the glyphs for already known sizes 
    load_glyphs(cascadia_face, opnf_dlg.file_h);
    load_glyphs(cascadia_face, savf_dlg.txt_edit.txt_zone.h);
    load_glyphs(cascadia_face, step_edit.txt_zone.h);
    for (int i = 0; i < gif_params_dlg.txt_edits.size(); i++)
        load_glyphs(cascadia_face, gif_params_dlg.txt_edits[i].txt_zone.h);

    // allocate memory buffer representing the UI 
    if (bmp_memory)
    {
        VirtualFree(bmp_memory, 0, MEM_RELEASE);
    }

    bmp_info.bmiHeader.biSize = sizeof(bmp_info.bmiHeader);
    bmp_info.bmiHeader.biWidth = screen_zone.w;
    bmp_info.bmiHeader.biHeight = -screen_zone.h;
    bmp_info.bmiHeader.biPlanes = 1;
    bmp_info.bmiHeader.biBitCount = 32;
    bmp_info.bmiHeader.biCompression = BI_RGB;

    int bmp_mem_size = (screen_zone.w * screen_zone.h) * bytes_per_pixel;
    bmp_memory = VirtualAlloc(0, bmp_mem_size, MEM_COMMIT, PAGE_READWRITE);

    frame_timer.reset();
    app_timer.reset();
    cursor_timer.reset();
}

int CALLBACK
WinMain(HINSTANCE Instance,
    HINSTANCE PrevInstance,
    LPSTR CommandLine,
    int ShowCode)
{
    screen_zone.w = 0.8 * GetSystemMetrics(SM_CXSCREEN);
    screen_zone.h = 0.8 * GetSystemMetrics(SM_CYSCREEN);
    // Create the window class gor the Editor
    WNDCLASSA wnd_class = {};
    wnd_class.lpfnWndProc = wnd_callback;
    wnd_class.hInstance = Instance;
    wnd_class.lpszClassName = "wnd_class";
    wnd_class.style = CS_DBLCLKS;

    if (RegisterClassA(&wnd_class))
    {
        RECT client_area_rect = { 0, 0, screen_zone.w, screen_zone.h};
        AdjustWindowRect(&client_area_rect, WS_CAPTION | WS_MINIMIZEBOX | WS_VISIBLE , false);
        main_wnd =
            CreateWindowExA(
                0,
                wnd_class.lpszClassName,
                "GOL",
                WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE ,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                client_area_rect.right - client_area_rect.left,
                client_area_rect.bottom - client_area_rect.top,
                0,
                0,
                Instance,
                0);

        running = true;

        if (main_wnd)
        {
            init_everything();
            // Main Loop
            while (running)
            {
                MSG Message;
                while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
                {
                    if (Message.message == WM_QUIT)
                    {
                        running = false;
                    }

                    TranslateMessage(&Message);
                    DispatchMessageA(&Message);
                }

                update_app_params();
                update_back_buffer();

                // respecting the frame rate
                frame_timer.tick();
                if (frame_timer.dt < frame_dur )
                    Sleep((frame_dur - frame_timer.dt) * 1000);

                HDC device_context = GetDC(main_wnd);
                RECT client_rect;
                GetClientRect(main_wnd, &client_rect);
                update_wnd(device_context, &client_rect);
                ReleaseDC(main_wnd, device_context);
                
                opnf_dlg.scrl.timer.tick();
                sim_timer.tick();
                cursor_timer.tick();
            }
        }
    }
    else
    {
    }
    return(0);
}