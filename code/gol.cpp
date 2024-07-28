#include <iostream>
#include <windows.h>
#include <windowsx.h>
#include <queue>
#include <vector>
#include <string>
#include <algorithm>
#include <shobjidl.h> // For IFileDialog
#include <iostream>
#include <fstream>
#include "gif.h"

#define STB_IMAGE_IMPLEMENTATION
#define IDM_FILE_OPEN 1001
#define IDM_FILE_EXIT 1002
#define IDM_FILE_SAVE 1003
#define DLG_OK_BUTT 1

#include "stb_image.h"


typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;
HWND main_wnd;
HWND h_dlg;
HWND h_v_edit;
HWND h_ng_edit;
LARGE_INTEGER PerformanceFrequencyL;
double loop_time;

char play_butt_0_fn[]  = "..\\images\\play_butt_0.png";
char play_butt_1_fn[]  = "..\\images\\play_butt_1.png";
char play_butt_2_fn[] = "..\\images\\play_butt_2.png";

char pause_butt_0_fn[] = "..\\images\\pause_butt_0.png";
char pause_butt_1_fn[] = "..\\images\\pause_butt_1.png";
char pause_butt_2_fn[] = "..\\images\\pause_butt_2.png";

char reinit_butt_fn[]   = "..\\images\\reinit_butt.png";
char reinit_butt_1_fn[] = "..\\images\\reinit_butt_1.png";
char reinit_butt_2_fn[] = "..\\images\\reinit_butt_2.png";

char next_butt_fn[]     = "..\\images\\next_butt.png";
char next_butt_1_fn[] = "..\\images\\next_butt_1.png";
char next_butt_2_fn[] = "..\\images\\next_butt_2.png";

char intens_bar_fn[]  = "..\\images\\intensity_bar.png";
char intens_slct_fn[] = "..\\images\\intensity_select.png";
char cursor_fn[]      = "..\\images\\cursor.png";
char go_butt_fn[]     = "..\\images\\go_butt.png";
char go_butt_1_fn[]   = "..\\images\\go_butt_1.png";
char go_butt_2_fn[]   = "..\\images\\go_butt_2.png";
char step_setter_fn[] = "..\\images\\step_setter.png";
char num_letters_fn[] = "..\\images\\num_letters.png";
char hide_fn[] = "..\\images\\hide.png";
char anti_hide_fn[] = "..\\images\\anti_hide.png";
char save_butt_fn[] = "..\\images\\save_butt.png";
char scrl_up_fn[] = "..\\images\\scrl_up.png";
char scrl_down_fn[] = "..\\images\\scrl_down.png";
char scrl_up1_fn[] = "..\\images\\scrl_up1.png";
char scrl_down1_fn[] = "..\\images\\scrl_down1.png";
char folder_icon_fn[] = "..\\images\\folder.png";


wchar_t *saving_fn;
wchar_t *opening_fn; 


bool running = false;

// Bitmap Info
void* bmp_memory = NULL;

int bmp_w;
int bmp_h;
double bmp_w1;
double bmp_h1;
int bmp_w1_targ;
int bmp_h1_targ;
double zoom_ratio_x;
double zoom_ratio_y;
bool zoom_in = false;
double zoom_speed;
double pos_zpeed_x;
double pos_zpeed_y;
bool zooming = false;

BITMAPINFO bmp_info;
int bytes_per_pixel = 4;

//Colors
uint32 white  = 0xffffffff;
uint32 light  = 0x00f8f8a9;
uint32 black  = 0x00000000;
uint32 orange = 0x00f39f80;
uint32 l_orange = 0x00ffaa80;
uint32 d_orange = 0x00ef7c51;
uint32 red    = 0x00ff0000;
uint32 green  = 0x0000ff00;
uint32 blue   = 0x000000ff;
uint32 grey   = 0x004a4a4a;
uint32 l_grey = 0x007e7e7e;
uint32 ll_grey = 0x00ef0f0f0;
uint32 l_blue = 0x008cfffb;
uint32 ld_blue = 0x007fe2de;
uint32 lg_blue = 0x00e7fffe;
uint32 ldg_blue = 0x00b4fffd;


uint32 slct_color = 0x005ac3f2;

// Grid info
int box_w    = 40;
int box_h    = 40;
int line_w   = 6;
int nb_box_w = 400;
int nb_box_h = 400;
int nb_boxes = nb_box_w * nb_box_h;

// num_letters sprite info
int num_width  = 16;
int num_height = 14;
int letter_w   = 16;
int letter_h   = 14;

LARGE_INTEGER begin_time, end_time, before_blit_time;

bool frame_calc_start = false;

// simulation params 
bool is_step_time = false;
double sim_timer  = 0;
bool sim_on       = false;
double sim_freq   = 1;
double step_dur   = 0.5;
int actual_step   = 0;
int target_step   = -1;
int prv_targ_stp  = -1;
int top_lim = nb_box_h;
int left_lim = nb_box_w;
int right_lim = 0;
int bottom_lim = 0;
int top_lim0;
int left_lim0;
int right_lim0;
int bottom_lim0;

// frame rate params 
double frame_dur = 0.017; // 60fps
// cursor 
double cursor_timer = 0;


// input 
bool go_butt_clkd = false;
bool go_butt_down = false;
bool plpau_down = false;
bool reinit_down = false;
bool next_down = false;
bool show_map = false;
bool sh_hide_clkd = false;
bool save_clkd = false;
bool open_clkd = false;
bool gif_saving = false;
bool gif_rg_slctd = false;
bool gif_slct_mode = false;
bool gif_params_entrd = false;
int nb_gen = 1;
int gen_ps = 1;
bool dlg_crtd = false;
std::wstring gif_path;

bool moving_started = false;
double mouse_dist_x = 0;
double mouse_dist_y = 0;
double prev_mouse_dist_x = 0;
double prev_mouse_dist_y = 0;

bool int_bar_slctd = false;
bool show_cursor = false;
std::vector<char> step_str(5);
int num_crctrs = 0;
bool slct_mode = false;
int slctd_nums[5];
int initial_slct_x = -1;
int slctd_nbr = 0;

//region selection params
bool mov_region = false;

// interaction modes, if one is true others are false
bool open_file = false;
bool save_file = false;
bool open_f_dlg = false;
bool f_error_dlg = false;
bool normal_mode = true;

bool files_extrctd = false;
int nb_files;
std::deque<std::pair<std::string, bool>>  files_q;
std::string curr_dir;
int file_h;
int fslctd_idx = -1;
double scrl_timer;
bool in_root = false;
double scrl_ratio = 3;
bool scrler_draging = false;
int scrler_y0;
bool scrler_attrctng = false;
int scrl_dir;
int f_href;
bool scrl_exist = false;

struct point {
    int x = -1, y = -1;
    bool operator==(point b)
    {
        if (b.x == x && b.y == y)
            return true;
        return false;
    }
};
point mouse_coords_region;
std::vector<point> grid_init_stt; 
point gif_rg0;
point gif_rg1;


struct r_point {
    double x = 0, y = 0;
};

r_point bmp_pos;
r_point bmp_pos_targ;

int map_box_w = 1;
int map_w = 300;
int map_h = 300;
r_point region_pos ;
r_point map_pos;

struct box
{
    bool stt = false, l_stt = false, updated = false;
};
box* grid = new box[nb_boxes];

//Mouse
struct mouse
{
	bool l_down = false, r_down = false, l_clk = false;
};
mouse my_mouse;
mouse initial_mouse;
POINT mouse_coords;
POINT prev_mouse_coords;
POINT mouse_coords_ldown;
POINT mouse_coords_rdown;
POINT mouse_coords_rup;
POINT mouse_coords_lup;

struct recta
{
    int x, y, w, h;
    recta operator*(int k)
    {
        return recta{ k * x, k * y, k * w, k * h };
    }
};

recta play_butt_zone;
recta reinit_butt_zone;
recta menu_zone ;
recta step_slct_zone ;
recta go_butt_zone;
point initial_curs_pos;
recta mul_sign_zone = { 160, 32, 10, 10 };
recta dot_zone = { num_width * 10 , 0, 4, 2 };
recta actual_step_zone;
recta sim_vel_zone; 
recta go_to_stp_zone;
recta region_slct_zone;
recta bmp_region_zone;
recta save_butt_zone;
recta open_butt_zone;
recta clear_butt_zone;
recta open_f_zone;
recta draw_f_zone;
recta scrl_zone;
recta scrler_zone;
recta openf_butt_zone;
recta cancel_butt_zone;
recta scrl_up_zone;
recta scrl_down_zone;
recta draw_f_zone_wscrl;
recta draw_f_zone_scrl;

struct image
{
    uint32* pixels = NULL;
    int w = 0, h = 0;
    point pos;
};

image play_butt_0;
image play_butt_1;
image play_butt_2;

image pause_butt_0;
image pause_butt_1;
image pause_butt_2;

image reinit_butt;
image reinit_butt_1;
image reinit_butt_2;

image next_butt;
image next_butt_1;
image next_butt_2;

image go_butt;
image go_butt_1;
image go_butt_2;

image intens_slct;
image intens_bar;
image my_cursor;
image step_setter;
image num_letters;
image hide;
image anti_hide;
image save_butt;
image scrl_up;
image scrl_down;
image scrl_up1;
image scrl_down1;
image folder_icon;

void save_as_gif();
bool zone_intersect(recta zone_1, recta zone_2);

void insert_bgnd_color(uint32 color)
{
    uint32* pixel = (uint32*)bmp_memory;
    for (int y = 0; y < bmp_h; y++)
        for (int x = 0; x < bmp_w; x++)
            pixel[y * bmp_w + x] = color;

}

void insert_color_to_zone(uint32 color, recta zone, recta limits = {-1,-1,INT_MAX,INT_MAX})
{
    uint32* pixel = (uint32*)bmp_memory;
    // check zone and limits intersection with client area, and zone and limits intersection
    if (!zone_intersect(zone, { 0,0,bmp_w,bmp_h }) || !zone_intersect(limits, {0,0,bmp_w, bmp_h}) || !zone_intersect(zone , limits))
        return;
   
    for(int y = zone.y ; y < zone.y + zone.h && y < bmp_h; y++)
    {
        if (y <= 0)
            y = 0;
        if (y >= limits.y + limits.h)
            break;
        if (y < limits.y)
            y = limits.y;
        for (int x = zone.x ; x < zone.x + zone.w && x < bmp_w; x++)
        {
            if (x <= 0)
                x = 0;
            if (x >= limits.x + limits.w)
                break;
            if (x < limits.x)
                x = limits.x;
            pixel[y * bmp_w + x] = color;
        }
    }
}

void insert_recta(recta my_rect, uint32 color, int bordr_w)
{
    insert_color_to_zone(color, { my_rect.x, my_rect.y, bordr_w, my_rect.h});
    insert_color_to_zone(color, { my_rect.x + my_rect.w - bordr_w, my_rect.y, bordr_w, my_rect.h});
    insert_color_to_zone(color, { my_rect.x , my_rect.y , my_rect.w, bordr_w });
    insert_color_to_zone(color, { my_rect.x , my_rect.y + my_rect.h - bordr_w, my_rect.w , bordr_w });
}

void insert_out_recta(recta my_rect, uint32 color, int bordr_w)
{
    insert_color_to_zone(color, {my_rect.x - bordr_w, my_rect.y - bordr_w, bordr_w, my_rect.h + 2* bordr_w });
    insert_color_to_zone(color, { my_rect.x + my_rect.w, my_rect.y- bordr_w, bordr_w, my_rect.h + 2* bordr_w });
    insert_color_to_zone(color, { my_rect.x - bordr_w, my_rect.y - bordr_w, my_rect.w + 2* bordr_w, bordr_w });
    insert_color_to_zone(color, { my_rect.x - bordr_w, my_rect.y + my_rect.h, my_rect.w + 2* bordr_w, bordr_w });
}

void insert_im(image im, bool to_blit = false)
{
    uint32* pixel = (uint32*)bmp_memory;
    uint32 tr_mask= 0xff000000;

    for (int y = im.pos.y; y < im.pos.y + im.h && y < bmp_h; y++)
    {
        if (y <= 0)
            y = 0;
        for (int x = im.pos.x ; x < im.pos.x + im.w && x < bmp_w; x++)
        {
            if (x <= 0)
                x = 0;
            uint32 p = im.pixels[(y - im.pos.y) * im.w + x - im.pos.x];
            if (p & tr_mask)
            {
                uint32 Red =   p & 0x000000ff;
                uint32 Green = p & 0x0000ff00;
                uint32 Blue =  p & 0x00ff0000;
                pixel[y * bmp_w + x] = (Red << 16) | Green | (Blue >> 16);
            } 
        }
    }
}

void instrch_from_im(image src_im, recta src_zone, recta dest_zone, bool to_color = false, uint32 color = black, recta limits = { -1,-1,INT_MAX,INT_MAX })
{
    uint32* pixel = (uint32*)bmp_memory;
    uint32 tr_mask = 0xff000000;
    if (!zone_intersect(dest_zone, { 0,0,bmp_w,bmp_h }) || !zone_intersect(limits, { 0,0,bmp_w, bmp_h }) || !zone_intersect(dest_zone, limits))
        return;
    for (int y = dest_zone.y ; y < dest_zone.y + dest_zone.h && y < bmp_h; y++)
    {
        if (y <= 0)
            y = 0;
        if (y >= limits.y + limits.h)
            break;
        if (y < limits.y)
            y = limits.y;
        for (int x = dest_zone.x ; x < dest_zone.x + dest_zone.w && x < bmp_w; x++)
        {
            if (x <= 0)
                x = 0;
            if (x <= 0)
                x = 0;
            if (x >= limits.x + limits.w)
                break;
            if (x < limits.x)
                x = limits.x;
            int src_y = (y - dest_zone.y)*(double(src_zone.h - 1)/(dest_zone.h - 1)) + src_zone.y;
            int src_x = (x - dest_zone.x)*(double(src_zone.w - 1)/(dest_zone.w - 1)) + src_zone.x;

            uint32 p = src_im.pixels[src_y * src_im.w + src_x];
            if (p & tr_mask)
            {
                if(!to_color)
                {
                    uint32 Red = p & 0x000000ff;
                    uint32 Green = p & 0x0000ff00;
                    uint32 Blue = p & 0x00ff0000;
                    pixel[y * bmp_w + x] = (Red << 16) | Green | (Blue >> 16);
                }
                else
                    pixel[y * bmp_w + x] = color;

            }
        }
    }
}

void insert_from_im(image src_im, recta src_zone, point pos)
{
    uint32* pixel = (uint32*)bmp_memory;
    uint32 tr_mask = 0xff000000;

    for (int y = pos.y; y < pos.y + src_zone.h && y < bmp_h; y++)
    {
        if (y <= 0)
            y = 0;
        for (int x = pos.x ; x < pos.x + src_zone.w && x < bmp_w; x++)
        {
            if (x <= 0)
                x = 0;
            int src_y = y - pos.y + src_zone.y;
            int src_x = x - pos.x + src_zone.x;

            uint32 p = src_im.pixels[src_y * src_im.w + src_x];
            if (p & tr_mask)
            {
                uint32 Red = p & 0x000000ff;
                uint32 Green = p & 0x0000ff00;
                uint32 Blue = p & 0x00ff0000;
                pixel[y * bmp_w + x] = (Red << 16) | Green | (Blue >> 16);
            }
        }
    }
}

void insert_num(char num, recta dest_zone)
{
    int num_idx = num - '0';
    recta num_zone = { num_idx * num_width, 0, num_width, num_height };
    
    instrch_from_im(num_letters, num_zone, dest_zone);
}

bool in_zone(POINT m_coords, recta zone)
{
    int x = m_coords.x;
    int y = m_coords.y;
    if (x >= zone.x && x <= zone.x + zone.w && y >= zone.y && y <= zone.y + zone.h)
        return true;
    return false;
}


void insert_text(const char* my_text, point pos, point char_size, bool to_color = false, uint32 color = black, recta limits = { -1,-1,INT_MAX,INT_MAX })
{
    for (int i = 0; my_text[i] != '\0'; i++)
    {
        char c = my_text[i];

        if (c == '.')
        {
            insert_color_to_zone(black, { pos.x + i * char_size.x + char_size.x/3 , pos.y + 5*char_size.y/6,char_size.x/3, char_size.y/6 }, limits);
        }
        else if (c != ' ')
        {
            if (islower(c))
                c = toupper(c);
            int index = c - 'A';
            recta crc_src_zone = { index * letter_w, 16, letter_w, letter_h };
            recta crc_dest_zone = { pos.x + i * char_size.x, pos.y, char_size.x, char_size.y};
            instrch_from_im(num_letters, crc_src_zone, crc_dest_zone, to_color, color, limits);
        }
    }
}

void insert_text_to_zone(const char* my_text, recta dest_zone,bool to_color = false, uint32 color = black)
{
    int crc_nbr = 0;
    while (my_text[crc_nbr] != '\0')
        crc_nbr++;
    int dest_crc_w = dest_zone.w / crc_nbr;
    int dest_crc_h = dest_zone.h;
    for (int i = 0; i < crc_nbr; i++)
    {
        char c = my_text[i];

        if (c != ' ')
        {
            if (islower(c))
                c = toupper(c);
            int index = c - 'A';
            recta crc_src_zone = { index * letter_w, 16, letter_w, letter_h };
            recta crc_dest_zone = { dest_zone.x + i * dest_crc_w, dest_zone.y, dest_crc_w, dest_crc_h };
            instrch_from_im(num_letters, crc_src_zone, crc_dest_zone, to_color, color);
        }
    }
}
void insert_sim_vel()
{
    point mul_sign_pos = { intens_slct.pos.x + intens_slct.w + 1, intens_slct.pos.y + intens_slct.h - mul_sign_zone.h };
    instrch_from_im(num_letters, mul_sign_zone, { mul_sign_pos.x, mul_sign_pos.y, 9, 9 });

    recta vel_zone = { mul_sign_pos.x + mul_sign_zone.w + 1, mul_sign_pos.y - (num_height - mul_sign_zone.h), 36, 14 };
    int crc_w = vel_zone.w / 3;
    insert_num(char(int(sim_freq) + '0'), { vel_zone.x, vel_zone.y, crc_w, vel_zone.h });
    insert_from_im(num_letters, dot_zone, { vel_zone.x + crc_w , vel_zone.y + num_height - dot_zone.h });

    if (int(sim_freq * 10) % 10 == 0)
        insert_num('0', { vel_zone.x + crc_w + dot_zone.w, vel_zone.y, crc_w, vel_zone.h });
    else
        insert_num(char(int(sim_freq * 10) % 10) + '0', { vel_zone.x + crc_w + dot_zone.w , vel_zone.y, crc_w, vel_zone.h });
}

void insert_actual_step()
{
    insert_text_to_zone("actual step", actual_step_zone, true, grey);

    int nb_digits = 1;
    int dig_init_pos_x = actual_step_zone.x + actual_step_zone.w / 2 - nb_digits * num_width / 2;
    if (actual_step == 0)
        insert_num('0', { dig_init_pos_x , actual_step_zone.y + actual_step_zone.h + 20, num_width, num_height });
    else
    {
        nb_digits--;
        char act_step_str[5];
        int nbr = actual_step / pow(10, nb_digits);

        while (nbr != 0)
        {
            act_step_str[nb_digits] = nbr % 10;
            nb_digits++;
            nbr = actual_step / pow(10, nb_digits);
        }
        dig_init_pos_x = actual_step_zone.x + actual_step_zone.w / 2 - nb_digits * num_width / 2;
        for (int i = 0; i < nb_digits; i++)
            insert_num(char(act_step_str[nb_digits - i - 1]) + '0', { dig_init_pos_x + i * num_width, actual_step_zone.y + actual_step_zone.h + 20, num_width, num_height });
    }
    insert_out_recta({ dig_init_pos_x - 2, actual_step_zone.y + actual_step_zone.h + 18, nb_digits * num_width + 4, num_height + 4 }, orange, 5);

}

bool is_neigh_alive(int neigh_x, int neigh_y)
{
    if (grid[neigh_y * nb_box_w + neigh_x].updated)
        return  grid[neigh_y * nb_box_w + neigh_x].l_stt;

    return grid[neigh_y * nb_box_w + neigh_x].stt;
}

void one_step_ahead()
{
    
    for (int j = top_lim - 1; j <= bottom_lim + 1; j++)
    {
        for (int i = left_lim - 1; i <= right_lim + 1; i++)
        {

            int neigh_alive = 0;
            int neigh_x = i - 1, neigh_y = j - 1;
            neigh_alive += int(is_neigh_alive(neigh_x, neigh_y));

            neigh_x = i - 1; neigh_y = j;
            neigh_alive += int(is_neigh_alive(neigh_x, neigh_y));

            neigh_x = i - 1; neigh_y = j + 1;
            neigh_alive += int(is_neigh_alive(neigh_x, neigh_y));

            neigh_x = i; neigh_y = j - 1;
            neigh_alive += int(is_neigh_alive(neigh_x, neigh_y));

            neigh_x = i; neigh_y = j + 1;
            neigh_alive += int(is_neigh_alive(neigh_x, neigh_y));

            neigh_x = i + 1; neigh_y = j - 1;
            neigh_alive += int(is_neigh_alive(neigh_x, neigh_y));

            neigh_x = i + 1; neigh_y = j;
            neigh_alive += int(is_neigh_alive(neigh_x, neigh_y));

            neigh_x = i + 1; neigh_y = j + 1;
            neigh_alive += int(is_neigh_alive(neigh_x, neigh_y));

            grid[j * nb_box_w + i].l_stt = grid[j * nb_box_w + i].stt;
            if (grid[j * nb_box_w + i].stt)
            {
                if (neigh_alive < 2 || neigh_alive > 3)
                    grid[j * nb_box_w + i].stt = false;
            }
            else
            {
                if (neigh_alive == 3)
                {
                    grid[j * nb_box_w + i].stt = true;
                    if (i < left_lim)
                        left_lim = i;
                    if (i > right_lim)
                        right_lim = i;
                    if (j < top_lim)
                        top_lim = j;
                    if (j > bottom_lim)
                        bottom_lim = j;
                }
            }
            grid[j * nb_box_w + i].updated = true;
        }
    }

    for (int j = top_lim - 1; j <= bottom_lim + 1; j++)
    {
        for (int i = left_lim - 1; i <= right_lim + 1; i++)
        {
            grid[j * nb_box_w + i].updated = false;
        }
    }
}

bool zone_intersect(recta zone_1, recta zone_2)
{
    if (zone_1.x < zone_2.x + zone_2.w && zone_1.x + zone_1.w > zone_2.x &&
        zone_1.y < zone_2.y + zone_2.h && zone_1.y + zone_1.h > zone_2.y)
        return true;
    return false;
}

point get_clk_box(POINT m_coords)
{
    int i = (m_coords.x/zoom_ratio_x + bmp_pos.x) / box_w;
    int j = (m_coords.y/zoom_ratio_y + bmp_pos.y) / box_h;
    return { i, j };
}
int get_step_int()
{
    int result = 0;
    for (int i = 0; i < num_crctrs; i++)
        result += int(step_str[i] - '0') * pow(10, num_crctrs - i - 1);
    return result;
}

recta get_im_zone(image my_im)
{
    return recta{ my_im.pos.x, my_im.pos.y, my_im.w, my_im.h };
}

void reinit_grid()
{
    //delete[] grid;
    //grid = new box[nb_boxes];
  
    for (int i = left_lim; i <= right_lim; i++)
        for (int j = top_lim; j <= bottom_lim ; j++)
            grid[j * nb_box_w + i].stt = false;
    for (auto square : grid_init_stt)
        grid[square.y * nb_box_w + square.x].stt = true;

    top_lim = top_lim0;
    left_lim = left_lim0;
    right_lim = right_lim0;
    bottom_lim = bottom_lim0;
}

void insert_dlg_box(std::string str, std::string title, recta zone, uint32 color, uint32 hl_color)
{
    uint32 op_col = (color == black) ? white : black;
    int str_size = str.size();

    point char_size = { 11, 15};
    recta ok_zone;
    recta txt_zone;

    ok_zone.x = zone.x + 2 * zone.w / 5;
    ok_zone.w = zone.w / 5;
    
    int txt_rows;
    do
    {
        char_size.x --;
        char_size.y --;

        ok_zone.h = 2 * char_size.y;


        txt_zone.x = zone.x + char_size.x;
        txt_zone.y = zone.y + 5 * char_size.y;
        txt_zone.w = zone.w - 2*char_size.x;

        txt_rows = str_size / (txt_zone.w / char_size.x) + 1;
        txt_zone.h = txt_rows * (2*char_size.y);

        ok_zone.y = txt_zone.y + txt_zone.h + char_size.y;
        zone.h = ok_zone.y + ok_zone.h - zone.y + 2 * char_size.y;
    } while (zone.y + zone.h > bmp_h);

    recta title_zone = { zone.x + 3 * char_size.x/2, zone.y + char_size.y/2 , zone.w - 3 * char_size.x,  3*char_size.y };

    insert_color_to_zone(color, zone);
    insert_out_recta(zone, l_blue, 4);
    insert_color_to_zone(l_orange, title_zone);
    insert_text(title.c_str(), { zone.x + 2 * char_size.x, zone.y + char_size.y }, { 2 * char_size.x, 2 * char_size.y }, true, op_col);

    int i;
    int nb_chars = txt_zone.w / char_size.x;
    for (i = 0; i < txt_rows - 1; i++)
        insert_text(str.substr(i * nb_chars, nb_chars).c_str(), { txt_zone.x, txt_zone.y + i * (2*char_size.y) }, char_size, true, op_col);
    // last row of text
    int substr_size = min(nb_chars, str_size - i * nb_chars);
    insert_text(str.substr(i * nb_chars, nb_chars).c_str(), { txt_zone.x + (txt_zone.w - substr_size*char_size.x)/2, txt_zone.y + i * (2*char_size.y) }, char_size, true, op_col);
    insert_out_recta({ txt_zone.x - char_size.x / 2, txt_zone.y - char_size.y / 2, txt_zone.w + char_size.x, txt_zone.h + char_size.y }, op_col, 2);
    insert_out_recta(ok_zone, op_col, 2);
    if (in_zone(mouse_coords, ok_zone))
        insert_color_to_zone(hl_color, ok_zone);
    if (my_mouse.l_down && in_zone(mouse_coords_ldown, ok_zone))
        insert_out_recta(ok_zone, op_col, 4);
    if(my_mouse.l_clk && in_zone(mouse_coords_ldown, ok_zone) && in_zone(mouse_coords_lup, ok_zone))   
    {
        open_file = true;
        open_f_dlg = false;
        f_error_dlg = false;
        normal_mode = false;
        OutputDebugStringA("ok clicked home boys \n");
    }
    insert_text("OK", { ok_zone.x + (ok_zone.w - 2 * char_size.x) / 2, ok_zone.y + (ok_zone.h - char_size.y) / 2 }, char_size, true, op_col);

}

void openf_change_dir(int idx_f)
{
    if (!files_q[idx_f].second)
    {
        std::string file_path;
        file_path = curr_dir.substr(0, curr_dir.size() - 1) + files_q[idx_f].first;
        
        if (file_path.compare(file_path.size() - 4, 4, ".txt") != 0)
        {
            open_file = false;
            open_f_dlg = true;
            normal_mode = false;
        }
        else
        {
            bool f_error = false;
            std::wifstream my_file;
            my_file.open(file_path);
            grid_init_stt.clear();
            if (my_file.is_open())
            {
                int k;
                for (int i = 0; i < nb_box_w; i++)
                {
                    for (int j = 0; j < nb_box_h; j++)
                    {
                        my_file >> k;
                        if (k != 0 && k != 1)
                        {
                            f_error_dlg = true;
                            break;
                        }
                        grid[j * nb_box_w + i].stt = (k != 0);
                        if (k)
                            grid_init_stt.push_back(point{ i,j });
                    }
                    if (f_error_dlg)
                        break;
                }
                my_file >> k;
            }
            if (!my_file.eof())
                f_error_dlg = true;
            else
                normal_mode = true;
            open_file = false;
            my_file.close();
            // so the LBUTTON_UP wouldn't be processed
            PeekMessage(0, 0, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE);
            my_mouse.l_down = false;
        }
    }
    else
    {
        if (!in_root)
        {
            if (idx_f == 0)
            {
                size_t bslash_idx = curr_dir.find_last_of("/\\", curr_dir.size() - 3);
                // no backslash found means we are in drive directory, and should go back beyond that
                if (bslash_idx == std::string::npos)
                    in_root = true;
                else
                    curr_dir = curr_dir.substr(0, bslash_idx + 1) + '*';
            }
            else if (files_q[idx_f].second)
            {
                curr_dir = curr_dir.substr(0, curr_dir.size() - 1) + files_q[idx_f].first + "\\*";
            }
        }
        else {
            curr_dir = files_q[idx_f].first + "*";
            in_root = false;
        }

        //extract names of files when dir change
        files_q.clear();
        if (in_root)
        {
            char buff[200];
            int actl_size = GetLogicalDriveStrings(200, buff);
            files_q.clear();
            for (int i = 0; i < actl_size; i++)
            {
                int j = i;
                while (j < 200 && buff[j])
                    j++;
                files_q.push_front({ std::string(&buff[i], &buff[j]), true });
                i = j;
            }
        }
        else
        {
            WIN32_FIND_DATA ffd;
            HANDLE h_find = INVALID_HANDLE_VALUE;
            h_find = FindFirstFile(curr_dir.c_str(), &ffd);
            if (h_find == INVALID_HANDLE_VALUE)
            {
                std::cout << "have to add a dialog box mentioning that we could not open the file, and ask to retry. ";
            }
            do
            {
                if (strcmp(ffd.cFileName, "..") == 0 || strcmp(ffd.cFileName, ".") == 0)
                    continue;
                if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                    files_q.push_front({ ffd.cFileName, true });
                else
                    files_q.push_back({ ffd.cFileName, false });
            } while (FindNextFile(h_find, &ffd));
            files_q.push_front({ "..", true });
        }
        nb_files = files_q.size();
        // adapt zone to presence of scroll and update scrler size
        scrler_zone.y = scrl_zone.y;
        f_href = draw_f_zone.y;
        scrler_zone.h = scrl_zone.h;
        if (nb_files * file_h > draw_f_zone.h)
        {
            scrl_exist = true;
            draw_f_zone = draw_f_zone_scrl;
            scrler_zone.h = scrl_zone.h - (file_h * (nb_files + 2) - draw_f_zone.h) / 3;

            if (scrler_zone.h < scrl_zone.h / 30)
            {
                scrler_zone.h = scrl_zone.h / 30;
                scrl_ratio = double(file_h * (nb_files + 2) - draw_f_zone.h) / (scrl_zone.h - scrler_zone.h);
            }
            else
                scrl_ratio = 3;
        }
        else
        {
            scrl_exist = false;
            draw_f_zone = draw_f_zone_wscrl;
        }

        fslctd_idx = -1;
    }
}

void update_wnd(HDC device_context, RECT* client_rect)
{
    //OutputDebugStringA("window update \n");

    int client_w = client_rect->right - client_rect->left;
    int client_h = client_rect->bottom - client_rect->top;
    StretchDIBits(device_context,
        0, 0, client_w, client_h,
        0, 0, bmp_w, bmp_h,
        bmp_memory,
        &bmp_info,
        DIB_RGB_COLORS, SRCCOPY);
}

LRESULT CALLBACK
dialog_callback(HWND wnd,
    UINT msg,
    WPARAM wp,
    LPARAM lp)
{
    LRESULT result = 0;
    switch (msg)
    {
    case WM_COMMAND:
    {
        switch (wp) {
        case DLG_OK_BUTT:
            char a[6];
            char b[6];
            GetWindowText(h_v_edit, a, 5);
            GetWindowText(h_ng_edit, b, 5);
            gen_ps = atoi(a);
            nb_gen = atoi(b);

            gen_ps = gen_ps > 0 ? gen_ps : 1;
            nb_gen = nb_gen > 0 ? nb_gen : 1;

            save_as_gif();
            gif_rg_slctd = false;
            gif_saving = false;
            dlg_crtd = false;
            EnableWindow(main_wnd, true);
            DestroyWindow(wnd);
        }
    }break;
    case WM_CLOSE: case WM_DESTROY:
    {
        gif_rg_slctd = false;
        gif_saving = false;
        dlg_crtd = false;
        EnableWindow(main_wnd, true);
        DestroyWindow(wnd);
    } break;

    default:
    {
        result = DefWindowProc(wnd, msg, wp, lp);
    } break;

    }
    return result;
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
        mouse_dist_x = 0;
        mouse_dist_y = 0;

        if (in_zone(mouse_coords_ldown, step_slct_zone))
        {
            show_cursor = true;
            cursor_timer = 0;
            slct_mode = true;
            if (slctd_nbr > 0)
                slctd_nbr = 0;
            int block = (mouse_coords_ldown.x - initial_curs_pos.x + num_width / 2) / num_width;
            if (block <= num_crctrs)
                my_cursor.pos.x = initial_curs_pos.x + block * num_width;
            else
                my_cursor.pos.x = initial_curs_pos.x + num_crctrs * num_width;
            initial_slct_x = my_cursor.pos.x;
        }
        else
        {
            show_cursor = false;
            slctd_nbr = 0;
        }
        if (in_zone(mouse_coords_ldown, { intens_slct.pos.x, intens_slct.pos.y, intens_slct.w, intens_slct.h }))
            int_bar_slctd = true;
        if (in_zone(mouse_coords_ldown, go_butt_zone))
            go_butt_down = true;
        if (in_zone(mouse_coords_ldown, play_butt_zone))
            plpau_down = true;
        if (in_zone(mouse_coords_ldown, reinit_butt_zone))
            reinit_down = true;
        if (in_zone(mouse_coords_ldown, get_im_zone(next_butt)))
            next_down = true;
        if (show_map && in_zone(mouse_coords_ldown, region_slct_zone))
        {
            mov_region = true;
            int last_x = region_pos.x;
            int last_y = region_pos.y;

            region_pos.x = mouse_coords_ldown.x - bmp_region_zone.w / 2;
            region_pos.y = mouse_coords_ldown.y - bmp_region_zone.h / 2;
            if (region_pos.x <= region_slct_zone.x)
                region_pos.x = region_slct_zone.x + 1;
            if (region_pos.x + bmp_region_zone.w >= region_slct_zone.x + region_slct_zone.w)
                region_pos.x = region_slct_zone.x + region_slct_zone.w - bmp_region_zone.w - 1;
            if (region_pos.y <= region_slct_zone.y)
                region_pos.y = region_slct_zone.y + 1;
            if (region_pos.y + bmp_region_zone.h >= region_slct_zone.y + region_slct_zone.h)
                region_pos.y = region_slct_zone.y + region_slct_zone.h - bmp_region_zone.h - 1;
            
            bmp_pos.x += (region_pos.x - last_x) * box_w / map_box_w;
            bmp_pos.y += (region_pos.y - last_y) * box_w / map_box_w;

        }
        if(open_file)
        {
            if(scrl_exist)
            {
                if (in_zone(mouse_coords_ldown, scrl_up_zone))
                {
                    int d1 = ((draw_f_zone.y - f_href) / file_h) * file_h;
                    int d0 = draw_f_zone.y - f_href;
                    if (d0 == d1)
                        f_href += file_h;
                    else
                        f_href += (d0 - d1);

                    if (f_href > draw_f_zone.y)
                        f_href = draw_f_zone.y;
                    if (f_href < draw_f_zone.y + draw_f_zone.h - (nb_files + 2) * file_h)
                        f_href = draw_f_zone.y + draw_f_zone.h - (nb_files + 2) * file_h;
                    scrler_zone.y = scrl_zone.y + (draw_f_zone.y - f_href) / scrl_ratio;
                    scrl_timer = 0;
                }
                if (in_zone(mouse_coords_ldown, scrl_down_zone))
                {
                    int d1 = ((draw_f_zone.y - f_href + draw_f_zone.h) / file_h) * file_h;
                    int d0 = draw_f_zone.y - f_href + draw_f_zone.h;
                    if (d0 == d1)
                        f_href -= file_h;
                    else
                        f_href -= file_h - (d0 - d1);

                    if (f_href > draw_f_zone.y)
                        f_href = draw_f_zone.y;
                    if (f_href < draw_f_zone.y + draw_f_zone.h - (nb_files + 2) * file_h)
                        f_href = draw_f_zone.y + draw_f_zone.h - (nb_files + 2) * file_h;
                    scrler_zone.y = scrl_zone.y + (draw_f_zone.y - f_href) / scrl_ratio;
                    scrl_timer = 0;
                }
                if (in_zone(mouse_coords_ldown, scrler_zone))
                {
                    scrler_draging = true;
                    scrler_y0 = scrler_zone.y;
                }
                if (in_zone(mouse_coords_ldown, scrl_zone) && !in_zone(mouse_coords_ldown, scrler_zone))
                {
                    int dir_sign = (scrler_zone.y - mouse_coords_ldown.y > 0) ? -1 : 1;
                    scrler_zone.y += dir_sign * scrler_zone.h * 0.7;
                    if (scrler_zone.y + scrler_zone.h > scrl_zone.y + scrl_zone.h)
                        scrler_zone.y = scrl_zone.y + scrl_zone.h - scrler_zone.h;
                    if (scrler_zone.y < scrl_zone.y)
                        scrler_zone.y = scrl_zone.y;
                    f_href = draw_f_zone.y - (scrler_zone.y - scrl_zone.y) * scrl_ratio;
                    scrl_timer = 0;
                    scrler_attrctng = true;
                    scrl_dir = (scrler_zone.y - mouse_coords_ldown.y > 0) ? -1 : 1;
                }
            }
            if (in_zone(mouse_coords_ldown, draw_f_zone))
            {
                int idx_file = ((draw_f_zone.y - f_href) + (mouse_coords.y - draw_f_zone.y)) / file_h;
                if (idx_file < nb_files)
                    fslctd_idx = idx_file;
                else
                    fslctd_idx = -1;
            }
            else
            {
                if (scrl_exist)
                {
                    if(!in_zone(mouse_coords_ldown, scrl_zone) && !in_zone(mouse_coords_ldown, scrl_up_zone) && !in_zone(mouse_coords_ldown, scrl_down_zone))
                        fslctd_idx = -1;
                }
                else
                    fslctd_idx = -1;
            }
        }

    }break;
    case WM_LBUTTONUP:
    {
        mouse_coords_lup.x = GET_X_LPARAM(lp);
        mouse_coords_lup.y = GET_Y_LPARAM(lp);
        //OutputDebugStringA("MOUSE_l_UP \n");
        my_mouse.l_clk = true;
        my_mouse.l_down = false;
        int_bar_slctd = false;
        slct_mode = false;
        go_butt_down = false;
        plpau_down = false;
        reinit_down = false;
        next_down = false;
        mov_region = false;

        if (open_file)
        {
            scrler_draging = false;
            scrler_attrctng = false;
        }
        PostMessageW(wnd, WM_SETCURSOR, (WPARAM)wnd, MAKELPARAM(HTCLIENT, WM_MOUSEMOVE));
        
    }break;
    case WM_RBUTTONDOWN:
    {

        mouse_coords_rdown.x = GET_X_LPARAM(lp);
        mouse_coords_rdown.y = GET_Y_LPARAM(lp);
        my_mouse.r_down = true;
        recta sh_hide_zone;
        recta map_zone = { 0,0,0,0 };
        if (show_map)
        {
            sh_hide_zone = get_im_zone(hide);
            map_zone = region_slct_zone;
        }
        else
            sh_hide_zone = { anti_hide.pos.x - 60, anti_hide.pos.y, anti_hide.w + 60, anti_hide.h };
        if (gif_saving && !in_zone(mouse_coords_rdown, menu_zone) && !in_zone(mouse_coords_rdown, sh_hide_zone) && !in_zone(mouse_coords_rdown, map_zone))
        {
            gif_slct_mode = true;
        }
    }break;
    case WM_RBUTTONUP:
    {
        mouse_coords_rup.x = GET_X_LPARAM(lp);
        mouse_coords_rup.y = GET_Y_LPARAM(lp);

        my_mouse.r_down = false;

        if (gif_saving && in_zone(mouse_coords_rup, { 0,0,bmp_w,bmp_h }) && in_zone(mouse_coords_rdown, { 0,0,bmp_w,bmp_h }))
        {
            gif_rg0 = get_clk_box(mouse_coords_rdown);
            gif_rg1 = get_clk_box(mouse_coords_rup);
            point temp = gif_rg0;
            gif_rg0 = {min(temp.x, gif_rg1.x), min(temp.y, gif_rg1.y)};
            gif_rg1 = { max(temp.x, gif_rg1.x), max(temp.y, gif_rg1.y) };

            gif_slct_mode = false;
            gif_rg_slctd = true;
            HCURSOR h_curs = LoadCursor(NULL, IDC_WAIT);
            SetCursor(h_curs);
        }
    }break;

    case WM_LBUTTONDBLCLK:
    {
        my_mouse.l_down = true;
        if (in_zone(mouse_coords_lup, { intens_slct.pos.x, intens_slct.pos.y, intens_slct.w, intens_slct.h }))
            int_bar_slctd = true;
        if (in_zone(mouse_coords_ldown, go_butt_zone))
            go_butt_down = true;
        if (in_zone(mouse_coords_ldown, play_butt_zone))
            plpau_down = true;
        if (in_zone(mouse_coords_ldown, reinit_butt_zone))
            reinit_down = true;
        if (in_zone(mouse_coords_ldown, get_im_zone(next_butt)))
            next_down = true;
        if (in_zone(mouse_coords_lup, step_slct_zone))
        {
            my_cursor.pos.x = initial_curs_pos.x + num_crctrs * num_width;
            slctd_nbr = num_crctrs;
            for (int i = 0; i < slctd_nbr; i++)
                slctd_nums[i] = i;
        }
        if (open_file )
        {
            if(in_zone(mouse_coords, draw_f_zone))
            {
                int idx_f = (mouse_coords.y - f_href) / file_h;
                if (idx_f < nb_files)
                    openf_change_dir(idx_f);
            }

            if(scrl_exist)
            {
                if (in_zone(mouse_coords_ldown, scrl_up_zone))
                {
                    f_href += file_h;

                    if (f_href > draw_f_zone.y)
                        f_href = draw_f_zone.y;
                    if (f_href < draw_f_zone.y + draw_f_zone.h - (nb_files + 2) * file_h)
                        f_href = draw_f_zone.y + draw_f_zone.h - (nb_files + 2) * file_h;
                    scrler_zone.y = scrl_zone.y + (draw_f_zone.y - f_href) / scrl_ratio;
                    scrl_timer = 0;
                }
                if (in_zone(mouse_coords_ldown, scrl_down_zone))
                {
                    f_href -= file_h;

                    if (f_href > draw_f_zone.y)
                        f_href = draw_f_zone.y;
                    if (f_href < draw_f_zone.y + draw_f_zone.h - (nb_files + 2) * file_h)
                        f_href = draw_f_zone.y + draw_f_zone.h - (nb_files + 2) * file_h;
                    scrler_zone.y = scrl_zone.y + (draw_f_zone.y - f_href) / scrl_ratio;
                    scrl_timer = 0;
                }
                if (in_zone(mouse_coords_ldown, scrl_zone) && !in_zone(mouse_coords_ldown, scrler_zone))
                {
                    int dir_sign = (scrler_zone.y - mouse_coords_ldown.y > 0) ? -1 : 1;
                    scrler_zone.y += dir_sign * scrler_zone.h * 0.7;
                    if (scrler_zone.y + scrler_zone.h > scrl_zone.y + scrl_zone.h)
                        scrler_zone.y = scrl_zone.y + scrl_zone.h - scrler_zone.h;
                    if (scrler_zone.y < scrl_zone.y)
                        scrler_zone.y = scrl_zone.y;
                    f_href = draw_f_zone.y - (scrler_zone.y - scrl_zone.y) * scrl_ratio;
                    scrl_timer = 0;
                    scrler_attrctng = true;
                    scrl_dir = (scrler_zone.y - mouse_coords_ldown.y > 0) ? -1 : 1;;
                }
            }
        }
            
    }break;
    
    case WM_MOUSEWHEEL:
    {
        int delta = int(GET_WHEEL_DELTA_WPARAM(wp));
        if (open_file)
        {
            if(scrl_exist)
            {
                int dir = delta > 0 ? 1 : -1;
                f_href += dir * 2 * file_h;

                if (f_href > draw_f_zone.y)
                    f_href = draw_f_zone.y;
                if (f_href < draw_f_zone.y + draw_f_zone.h - (nb_files + 2) * file_h)
                    f_href = draw_f_zone.y + draw_f_zone.h - (nb_files + 2) * file_h;
                scrler_zone.y = scrl_zone.y + (draw_f_zone.y - f_href) / scrl_ratio;
            }
        }
        else
        {
            bool zoom_chng_dir = false;
            if (zoom_in && delta < 0 || !zoom_in && delta>0)
                zoom_chng_dir = true;
            double new_w1_targ = zoom_chng_dir ? bmp_w1 - delta : bmp_w1_targ - delta;
            if (zoom_chng_dir)
                zooming = false;
            if (new_w1_targ > 8 * box_w) // setting a limit for zoom in  
            {
                int new_box_w = box_w * bmp_w / new_w1_targ;
                if (new_box_w > 12) // setting a limit for zoom out 
                {
                    if (!zooming)
                    {
                        bmp_pos_targ = bmp_pos;
                        zooming = true;
                    }
                    zoom_in = delta > 0 ? true : false;
                    bmp_w1_targ = new_w1_targ;

                    bmp_pos_targ.x = mouse_coords.x * (bmp_w1 - bmp_w1_targ) / bmp_w + bmp_pos.x;

                    if (bmp_pos_targ.x + bmp_w1_targ >= nb_box_w * box_w)
                        bmp_pos_targ.x = nb_box_w * box_w - bmp_w1_targ;
                    if (bmp_pos_targ.x < 0)
                        bmp_pos_targ.x = 0;

                    bmp_h1_targ = bmp_h * bmp_w1_targ / bmp_w;
                    bmp_pos_targ.y = mouse_coords.y * (bmp_h1 - bmp_h1_targ) / bmp_h + bmp_pos.y;
                    if (bmp_pos_targ.y + bmp_h1_targ >= nb_box_h * box_h)
                        bmp_pos_targ.y = nb_box_h * box_h - bmp_h1_targ;
                    if (bmp_pos_targ.y < 0)
                        bmp_pos_targ.y = 0;
                    zoom_speed = std::abs(bmp_w1_targ - bmp_w1) * 3.5;
                    pos_zpeed_x = zoom_speed * std::abs(bmp_pos_targ.x - bmp_pos.x) / std::abs(bmp_w1_targ - bmp_w1);
                    pos_zpeed_y = zoom_speed * std::abs(bmp_pos_targ.y - bmp_pos.y) / std::abs(bmp_w1_targ - bmp_w1);

                }
            }
        }

    }break;
            
    case WM_KEYDOWN:
    {
        if(show_cursor)
        {
            if (slctd_nbr > 0)
            {
                char my_num = MapVirtualKeyW(wp, MAPVK_VK_TO_CHAR);
                if ((my_num >= '0' && my_num <= '9'))
                {
                    if (my_cursor.pos.x > initial_curs_pos.x + slctd_nums[0] * num_width)
                        my_cursor.pos.x -= slctd_nbr * num_width;
                    for (int i = 0; i < slctd_nbr; i++)
                    {
                        step_str.erase(step_str.begin() + slctd_nums[i]);
                        num_crctrs--;
                    }
                    slctd_nbr = 0;
                    cursor_timer = 0;

                    int index = (my_cursor.pos.x - initial_curs_pos.x) / num_width;
                    step_str.insert(step_str.begin() + index, my_num);
                    num_crctrs++;
                    my_cursor.pos.x += num_width;
                }
            }
            else if (num_crctrs < 5)
            {
                char my_num;
                if (wp >= '0' && wp <= '9')
                    my_num = wp;
                else
                    my_num = MapVirtualKeyW(wp, MAPVK_VK_TO_CHAR);

                if (my_num >= '0' && my_num <= '9')
                {
                    int index = (my_cursor.pos.x - initial_curs_pos.x) / num_width;
                    step_str.insert(step_str.begin() + index, my_num);
                    num_crctrs++;
                    my_cursor.pos.x += num_width;
                }
                //num_queue.push(my_num);
            }
            switch (wp)
            {
                case VK_LEFT:
                {
                    if (!slct_mode && slctd_nbr > 0)
                    {
                        slctd_nbr = 0;
                    }
                    else if (my_cursor.pos.x - initial_curs_pos.x > 0)
                    {
                        my_cursor.pos.x -= num_width;
                        cursor_timer = 0;
                    }
                }break;
                case VK_RIGHT:
                {
                    if (!slct_mode && slctd_nbr > 0)
                    {
                        slctd_nbr = 0;
                    }
                    else if ((my_cursor.pos.x - initial_curs_pos.x) / num_width < num_crctrs)
                    {
                        my_cursor.pos.x += num_width;
                        cursor_timer = 0;
                    }
                }break;
                case VK_BACK:
                {
                    if (!slct_mode && slctd_nbr > 0)
                    {
                        if (my_cursor.pos.x > initial_curs_pos.x + slctd_nums[0] * num_width)
                            my_cursor.pos.x -= slctd_nbr * num_width;
                        for (int i = 0; i < slctd_nbr; i++)
                        {
                            step_str.erase(step_str.begin() + slctd_nums[i]);
                            num_crctrs--;
                        }
                        slctd_nbr = 0;
                        cursor_timer = 0;

                    }
                    else
                    {
                        int index = (my_cursor.pos.x - initial_curs_pos.x) / num_width;
                        if (index > 0)
                        {
                            step_str.erase(step_str.begin() + index - 1);
                            num_crctrs--;
                            my_cursor.pos.x -= num_width;
                            cursor_timer = 0;
                        }
                    }
                }break;
                case VK_RETURN:
                {
                    go_butt_clkd = true;
                    show_cursor = false;
                }break;
            }
        }
        else if (open_file)
        {
            switch (wp)
            {
                case VK_UP:
                {
                    fslctd_idx = (fslctd_idx - 1 >= 0) ? fslctd_idx - 1 : 0;

                    if (f_href + fslctd_idx * file_h < draw_f_zone.y)
                        f_href = draw_f_zone.y - fslctd_idx * file_h;
                    else if (f_href + (fslctd_idx+1) * file_h > draw_f_zone.y + draw_f_zone.h)
                        f_href = draw_f_zone.y +draw_f_zone.h - (fslctd_idx +1)* file_h;

                    if (f_href < draw_f_zone.y + draw_f_zone.h - (nb_files + 2) * file_h)
                        f_href = draw_f_zone.y + draw_f_zone.h - (nb_files + 2) * file_h;
                    if (f_href > draw_f_zone.y)
                        f_href = draw_f_zone.y;
                    scrler_zone.y = scrl_zone.y + (draw_f_zone.y - f_href) / scrl_ratio;

                }break;
                case VK_DOWN:
                {
                    fslctd_idx = (fslctd_idx + 1 <= nb_files - 1) ? fslctd_idx + 1 : nb_files - 1;

                    if (f_href + fslctd_idx * file_h < draw_f_zone.y)
                        f_href = draw_f_zone.y - fslctd_idx * file_h;
                    else if (f_href + (fslctd_idx + 1) * file_h > draw_f_zone.y + draw_f_zone.h)
                        f_href = draw_f_zone.y + draw_f_zone.h - (fslctd_idx + 1) * file_h;
                    
                    if (f_href < draw_f_zone.y + draw_f_zone.h - (nb_files + 2) * file_h)
                        f_href = draw_f_zone.y + draw_f_zone.h - (nb_files + 2) * file_h;
                    if (f_href > draw_f_zone.y)
                        f_href = draw_f_zone.y;
                    scrler_zone.y = scrl_zone.y + (draw_f_zone.y - f_href) / scrl_ratio;

                }break;
                case VK_ESCAPE:
                {
                    open_file = false;
                    normal_mode = true;
                    fslctd_idx = -1;

                }break;
                case VK_RETURN:
                {
                    if(fslctd_idx != -1)
                        openf_change_dir(fslctd_idx);
                }break;
                case VK_BACK:
                {
                    if(!in_root)
                        openf_change_dir(0);
                }break;
            }
        }
        else if (open_f_dlg || f_error_dlg)
        {
            switch(wp)
            {
                case VK_RETURN:
                {
                    open_f_dlg = false;
                    f_error_dlg = false;
                    open_file = true;
                }break;
            }
        }
    }break;

    case WM_SETCURSOR:
    {
        if (LOWORD(lp) == HTCLIENT)
        {
            result = TRUE;
            if (normal_mode)
            {
                HCURSOR h_curs;
                if (in_zone(mouse_coords, step_slct_zone))
                {
                    h_curs = LoadCursor(NULL, IDC_IBEAM);
                }
                else
                {

                    if (my_mouse.l_down)
                        h_curs = LoadCursor(NULL, IDC_SIZEALL);
                    else
                        h_curs = LoadCursor(NULL, IDC_ARROW);
                }
                SetCursor(h_curs);
            }

        }
        else {
            my_mouse = initial_mouse;
        }
    }
        break;
    case WM_PAINT:
    {
        //OutputDebugStringA("WM_PAINT\n");
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
        //OutputDebugStringA("WM_ACTIVATEAPP\n");
    } break;

    case WM_DESTROY:
    {
        running = false;
    } break;

    default:
    {
        //OutputDebugStringA("WM_DEFAULT\n");
        result = DefWindowProc(wnd, msg, wp, lp);
    } break;
    }

    return(result);
}

void insert_region_slct()
{
    insert_color_to_zone(black, region_slct_zone);
    uint32* pixel = (uint32*)bmp_memory;
    
    for (int i = region_slct_zone.x; i < region_slct_zone.x + region_slct_zone.w; i++)
    {
        for (int j = region_slct_zone.y; j < region_slct_zone.y + region_slct_zone.h; j++)
        {
            int i0 = (i - region_slct_zone.x + int(map_pos.x)) / map_box_w;
            int j0 = (j - region_slct_zone.y + int(map_pos.y)) / map_box_w;

            if (grid[j0 * nb_box_w + i0].stt)
                pixel[j * bmp_w + i] = light;
            else
                pixel[j * bmp_w + i] = black;
        }
    }


    //int i = 1;
    //int line_x = 0;
    //while (line_x < map_w + region_slct_zone.x)
    //{
    //    line_x = (int(map_pos.x / map_box_w) + i) * map_box_w - map_pos.x + region_slct_zone.x;
    //    insert_color_to_zone(white, { line_x, region_slct_zone.y, 1, region_slct_zone.h });
    //    i++;
    //}
    //i = 1;
    //int line_y = 0;
    //while (line_y < map_w + region_slct_zone.y)
    //{
    //    line_y = (int(map_pos.y / map_box_w) + i) * map_box_w - map_pos.y + region_slct_zone.y;
    //    insert_color_to_zone(white, { region_slct_zone.x,line_y, region_slct_zone.w, 1 });
    //    i++;
    //}

    insert_out_recta(region_slct_zone, l_blue, 3);
    insert_out_recta({ bmp_region_zone.x + 2, bmp_region_zone.y + 2, bmp_region_zone.w - 4, bmp_region_zone.h - 4 }, orange, 2);
}

void insert_save_butt()
{
    if(in_zone(mouse_coords, save_butt_zone))
        insert_color_to_zone(l_blue, save_butt_zone);
    else
        insert_color_to_zone(orange, save_butt_zone);
    insert_text_to_zone("save", { save_butt_zone.x + 4 , save_butt_zone.y + 6,save_butt_zone.w - 8,save_butt_zone.h - 12 }, true, grey);
}

void insert_open_butt()
{
    if (in_zone(mouse_coords, open_butt_zone))
        insert_color_to_zone(l_blue, open_butt_zone);
    else
        insert_color_to_zone(orange, open_butt_zone);
    insert_text_to_zone("open", { open_butt_zone.x + 4 , open_butt_zone.y + 6,open_butt_zone.w - 8,open_butt_zone.h - 12 }, true, grey);
}
void insert_clear_butt()
{
    if (in_zone(mouse_coords, clear_butt_zone))
        insert_color_to_zone(l_blue, clear_butt_zone);
    else
        insert_color_to_zone(orange, clear_butt_zone);
    insert_text_to_zone("clear", { clear_butt_zone.x + 4 , clear_butt_zone.y + 6,clear_butt_zone.w - 8,clear_butt_zone.h - 12 }, true, grey);
}

void clear_grid()
{
    for (int i = 0; i < nb_box_w; i++)
        for (int j = 0; j < nb_box_h; j++)
            grid[j * nb_box_w + i].stt = false;
    grid_init_stt.clear();
}

std::wstring f_ext(const std::wstring& file_path) {
    size_t dot_pos = file_path.find_last_of(L'.');
    if (dot_pos != std::wstring::npos && dot_pos < file_path.length() - 1) 
        return file_path.substr(dot_pos); 
    return L""; 
}

void save_as_gif()
{
    int nbw = gif_rg1.x - gif_rg0.x + 3;
    int nbh = gif_rg1.y - gif_rg0.y + 3;
    int gbxw = box_w * zoom_ratio_x;
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

                    if (grid[bx_j * nb_box_w + bx_i].stt)
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

void save_config(HWND wnd)
{
    IFileDialog* pfd;
    HRESULT hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
    std::wstring file_path;
    if (SUCCEEDED(hr)) {
        COMDLG_FILTERSPEC filterSpec[] = { { L"Text Files", L"*.txt" }, {L"Gif Files", L"*.gif"}};
        hr = pfd->SetFileTypes(ARRAYSIZE(filterSpec), filterSpec);
        hr = pfd->SetDefaultExtension(L"txt");
        if (FAILED(hr)) {
            return;
        }

        while(true)
        {
            hr = pfd->Show(wnd);
            if (hr == HRESULT_FROM_WIN32(ERROR_CANCELLED)) {
                // User canceled or closed the dialog box
                pfd->Release();
                return; // Exit without showing any message
            }
            else if (FAILED(hr)) {
                // Handle other errors
                pfd->Release();
                return;
            }
            IShellItem* pItem;
            hr = pfd->GetResult(&pItem);
            if (SUCCEEDED(hr)) {
                PWSTR pszFilePath;
                hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
                if (SUCCEEDED(hr)) {
                    file_path = std::wstring(pszFilePath);
                    CoTaskMemFree(pszFilePath);
                    if (f_ext(file_path.c_str()) == L".gif")
                    {
                        gif_path = file_path;
                        gif_saving = true;
                        MessageBoxW(wnd, L"Use mouse right button \n to select the region you want to save as gif.", L"INFO", MB_OK | MB_ICONINFORMATION);
                        break;
                    }
                    else if (f_ext(file_path.c_str()) == L".txt")
                    {
                        std::wofstream my_file;
                        my_file.open(file_path);

                        if (my_file.is_open())
                        {
                            for (int i = 0; i < nb_box_w; i++)
                                for (int j = 0; j < nb_box_h; j++)
                                    my_file << int(grid[j * nb_box_w + i].stt) << " ";
                        }
                        my_file.close();
                        break;
                    }
                    else if (f_ext(file_path.c_str()) != L".txt")
                        MessageBoxW(wnd, L"Please enter a file with a .txt or .gif extension .", L"Invalid File Type", MB_OK | MB_ICONERROR);
                }
                pItem->Release();
            }
        }

        pfd->Release();
    }

}

//
//void init_from_file(HWND wnd)
//{
//    // Create an instance of IFileDialog
//    IFileDialog* pfd;
//    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
//    std::wstring file_path;
//
//    if (SUCCEEDED(hr))
//    {
//        while(true)
//        {
//            COMDLG_FILTERSPEC filterSpec[] = { L"Text Files", L"*.txt" };
//            hr = pfd->SetFileTypes(ARRAYSIZE(filterSpec), filterSpec);
//            hr = pfd->SetDefaultExtension(L"txt");
//            if (FAILED(hr)) {
//                return;
//            }
//
//            hr = pfd->Show(wnd);
//            if (hr == HRESULT_FROM_WIN32(ERROR_CANCELLED)) {
//                pfd->Release();
//                return; 
//            }
//            else if (FAILED(hr)) {
//                pfd->Release();
//                return;
//            }
//            if (SUCCEEDED(hr))
//            {
//                IShellItem* pItem;
//                hr = pfd->GetResult(&pItem);
//                if (SUCCEEDED(hr))
//                {
//                    PWSTR pszFilePath;
//                    hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
//                    if (SUCCEEDED(hr))
//                    {
//                        file_path = pszFilePath;
//                        CoTaskMemFree(pszFilePath);
//                        if (f_ext(file_path.c_str()) != L".txt")
//                            MessageBoxW(NULL, L"Please choose a file with a .txt extension.", L"Invalid File Type", MB_OK | MB_ICONERROR);
//                        else
//                            break;
//                    }
//                    pItem->Release();
//                }
//            }
//            pfd->Release();
//        }
//    }
//
//    std::wifstream my_file;
//    my_file.open(file_path.c_str());
//    if (my_file.is_open())
//    {
//        int k;
//        for (int i = 0; i < nb_box_w; i++)
//            for (int j = 0 ; j < nb_box_h; j++)
//            {
//                my_file >> k;
//                grid[j * nb_box_w + i].stt = (k != 0);
//            }
//    }
//    my_file.close();
//}
void get_curr_dir(std::string& curr_dir)
{
    char fn[MAX_PATH];
    GetModuleFileName(NULL, fn, MAX_PATH);
    
    if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
    {
        std::cout << "file path too long\n";
        return;
    }
    char* last_bslash = strrchr(fn, '\\');

    strncpy(fn, fn, last_bslash - fn + 1);
    last_bslash[1] = '*';
    last_bslash[2] = '\0';
    curr_dir = fn;
}
int CALLBACK
WinMain(HINSTANCE Instance,
    HINSTANCE PrevInstance,
    LPSTR CommandLine,
    int ShowCode)
    {
    bmp_w = 0.8*GetSystemMetrics(SM_CXSCREEN);
    bmp_h = 0.8*GetSystemMetrics(SM_CYSCREEN);
    bmp_w1 = bmp_w;
    bmp_h1 = bmp_h;
    bmp_w1_targ = bmp_w1;
    bmp_h1_targ = bmp_h1;
    zoom_ratio_x = bmp_w / bmp_w1;
    zoom_ratio_y = bmp_h / bmp_h1;
    bmp_pos = { double(nb_box_w * box_w - bmp_w) / 2, double(nb_box_h * box_h - bmp_h) / 2 };

    // get current directory and fill the files_q
    get_curr_dir(curr_dir);
    WIN32_FIND_DATA ffd;
    HANDLE h_find = INVALID_HANDLE_VALUE;
    h_find = FindFirstFile(curr_dir.c_str(), &ffd);
    if (h_find == INVALID_HANDLE_VALUE)
    {
        std::cout << "have to add a dialog box mentioning that we could not open the file, and ask to retry. ";
    }
    do
    {
        if (strcmp(ffd.cFileName, ".") == 0 || strcmp(ffd.cFileName, "..") == 0)
            continue;
        if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            files_q.push_front({ ffd.cFileName, true });
        else
            files_q.push_back({ ffd.cFileName, false });
    } while (FindNextFile(h_find, &ffd));
    files_q.push_front({ "..", true });
    nb_files = files_q.size();

    QueryPerformanceFrequency(&PerformanceFrequencyL);
    int64 PerformanceFrequency = PerformanceFrequencyL.QuadPart;

    // Create the window class gor the Editor
    WNDCLASS wnd_class = {};
    wnd_class.lpfnWndProc = wnd_callback;
    wnd_class.hInstance = Instance;
    wnd_class.lpszClassName = "wnd_class";
    wnd_class.style = CS_DBLCLKS;

    if (RegisterClassA(&wnd_class))
    {
        // register dialog window
        WNDCLASS dialog_wnd = {};
        dialog_wnd.lpfnWndProc = dialog_callback;
        dialog_wnd.hInstance = Instance;
        dialog_wnd.lpszClassName = "dialog_class";
        RegisterClassA(&dialog_wnd);

        RECT client_area_rect = { 0, 0, bmp_w, bmp_h};
        AdjustWindowRect(&client_area_rect, WS_CAPTION | WS_MINIMIZEBOX | WS_VISIBLE , false);
        HWND wnd =
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

        if (wnd)
        {
            // Attach the menu to the window
            // load images
            main_wnd = wnd;
            play_butt_0.pixels = (uint32*)stbi_load(play_butt_0_fn, &play_butt_0.w, &play_butt_0.h, NULL, 0);
            play_butt_1.pixels = (uint32*)stbi_load(play_butt_1_fn, &play_butt_1.w, &play_butt_1.h, NULL, 0);
            play_butt_2.pixels = (uint32*)stbi_load(play_butt_2_fn, &play_butt_2.w, &play_butt_2.h, NULL, 0);

            pause_butt_0.pixels = (uint32*)stbi_load(pause_butt_0_fn, &pause_butt_0.w, &pause_butt_0.h, NULL, 0);
            pause_butt_1.pixels = (uint32*)stbi_load(pause_butt_1_fn, &pause_butt_1.w, &pause_butt_1.h, NULL, 0);
            pause_butt_2.pixels = (uint32*)stbi_load(pause_butt_2_fn, &pause_butt_2.w, &pause_butt_2.h, NULL, 0);

            reinit_butt.pixels = (uint32*)stbi_load(reinit_butt_fn, &reinit_butt.w, &reinit_butt.h, NULL, 0);
            reinit_butt_1.pixels = (uint32*)stbi_load(reinit_butt_1_fn, &reinit_butt_1.w, &reinit_butt_1.h, NULL, 0);
            reinit_butt_2.pixels = (uint32*)stbi_load(reinit_butt_2_fn, &reinit_butt_2.w, &reinit_butt_2.h, NULL, 0);

            next_butt.pixels = (uint32*)stbi_load(next_butt_fn, &next_butt.w, &next_butt.h, NULL, 0);
            next_butt_1.pixels = (uint32*)stbi_load(next_butt_1_fn, &next_butt_1.w, &next_butt_1.h, NULL, 0);
            next_butt_2.pixels = (uint32*)stbi_load(next_butt_2_fn, &next_butt_2.w, &next_butt_2.h, NULL, 0);

            intens_slct.pixels = (uint32*)stbi_load(intens_slct_fn, &intens_slct.w, &intens_slct.h, NULL, 0);
            intens_bar.pixels = (uint32*)stbi_load(intens_bar_fn, &intens_bar.w, &intens_bar.h, NULL, 0);

            my_cursor.pixels = (uint32*)stbi_load(cursor_fn, &my_cursor.w, &my_cursor.h, NULL, 0);

            go_butt.pixels = (uint32*)stbi_load(go_butt_fn, &go_butt.w, &go_butt.h, NULL, 0);
            go_butt_1.pixels = (uint32*)stbi_load(go_butt_1_fn, &go_butt_1.w, &go_butt_1.h, NULL, 0);
            go_butt_2.pixels = (uint32*)stbi_load(go_butt_2_fn, &go_butt_2.w, &go_butt_2.h, NULL, 0);
            num_letters.pixels = (uint32*)stbi_load(num_letters_fn, &num_letters.w, &num_letters.h, NULL, 0);
            hide.pixels = (uint32*)stbi_load(hide_fn, &hide.w, &hide.h, NULL, 0);
            anti_hide.pixels = (uint32*)stbi_load(anti_hide_fn, &anti_hide.w, &anti_hide.h, NULL, 0);
            save_butt.pixels = (uint32*)stbi_load(save_butt_fn, &save_butt.w, &save_butt.h, NULL, 0);

            scrl_up.pixels = (uint32*)stbi_load(scrl_up_fn, &scrl_up.w, &scrl_up.h, NULL, 0);
            scrl_down.pixels = (uint32*)stbi_load(scrl_down_fn, &scrl_down.w, &scrl_down.h, NULL, 0);
            scrl_up1.pixels = (uint32*)stbi_load(scrl_up1_fn, &scrl_up1.w, &scrl_up1.h, NULL, 0);
            scrl_down1.pixels = (uint32*)stbi_load(scrl_down1_fn, &scrl_down1.w, &scrl_down1.h, NULL, 0);
            folder_icon.pixels = (uint32*)stbi_load(folder_icon_fn, &folder_icon.w, &folder_icon.h, NULL, 0);

            // set different zones and positions
            menu_zone = { int(0.005 * bmp_w),int(0.005 * bmp_h),int(0.48 * bmp_w),int(0.13*bmp_h)}; // 800, 120 
            pause_butt_0.pos = { menu_zone.x, menu_zone.y};
            pause_butt_1.pos = pause_butt_0.pos;
            pause_butt_2.pos = pause_butt_0.pos;
            play_butt_0.pos  = pause_butt_0.pos;
            play_butt_1.pos  = pause_butt_0.pos;
            play_butt_2.pos = pause_butt_0.pos;

            next_butt.pos = { play_butt_0.pos.x + (play_butt_0.w - next_butt.w)/2, play_butt_0.pos.y + play_butt_0.h + menu_zone.h/24 };
            next_butt_1.pos = next_butt.pos;
            next_butt_2.pos = next_butt.pos;

            reinit_butt.pos   = { play_butt_0.pos.x + play_butt_0.w + 1, play_butt_0.pos.y };
            reinit_butt_1.pos = reinit_butt.pos;
            reinit_butt_2.pos = reinit_butt.pos;

            intens_bar.pos = { reinit_butt.pos.x + reinit_butt.w + menu_zone.w/20, int(menu_zone.h*7.0/12) };
            intens_slct.pos = { intens_bar.pos.x - intens_slct.w / 2 , menu_zone.h/3 };
            
            play_butt_zone = { play_butt_0.pos.x, play_butt_0.pos.y, play_butt_0.w, play_butt_0.h };
            reinit_butt_zone = { reinit_butt.pos.x, reinit_butt.pos.y, reinit_butt.w, reinit_butt.h };
            step_slct_zone = { intens_bar.pos.x + intens_bar.w + int(menu_zone.w * 3.0 / 40), int(menu_zone.w * 9.0 / 160), 5 * num_width + 4, num_height + int(menu_zone.h * 2.0 / 15) };
            go_butt.pos = { step_slct_zone.x + (step_slct_zone.w - go_butt.w)/2, step_slct_zone.y + step_slct_zone.h + int(menu_zone.h*1/12) };
            go_butt_1.pos = go_butt.pos;
            go_butt_2.pos = go_butt.pos;
            
            go_butt_zone = { go_butt.pos.x, go_butt.pos.y, go_butt.w, go_butt.h };
            go_to_stp_zone = { step_slct_zone.x - menu_zone.w/100, step_slct_zone.y - int(menu_zone.h * 25.0/120), step_slct_zone.w + int(menu_zone.w * 20.0 /800), int(menu_zone.h * 15.0 /120) };
            actual_step_zone = { step_slct_zone.x + step_slct_zone.w + int(menu_zone.w * 1.0 /20), go_to_stp_zone.y, int(menu_zone.w * 150.0 /800), int(menu_zone.h * 13.0 /120) };
            
            save_butt_zone = { actual_step_zone.x + actual_step_zone.w + int(menu_zone.w * 10.0 /800), actual_step_zone.y + int(menu_zone.h * 5.0 /120), int(menu_zone.w * 60.0 /800), int(menu_zone.h * 40.0 /120) };
            open_butt_zone = { actual_step_zone.x + actual_step_zone.w + int(menu_zone.w * 10.0 /800), actual_step_zone.y + save_butt_zone.h + int(menu_zone.h * 10.0 /120), int(menu_zone.w * 60.0 /800), int(menu_zone.h * 40.0 /120) };
            clear_butt_zone = { reinit_butt.pos.x + (reinit_butt.w - next_butt.w) / 2, next_butt.pos.y, next_butt.w, next_butt.h };

            sim_vel_zone = { intens_bar.pos.x - int(menu_zone.w * 8.0 /800), go_to_stp_zone.y , intens_bar.w + int(menu_zone.w * 20.0 /800), int(menu_zone.h * 15.0 /120) };
            region_slct_zone = { bmp_w - map_w- int(menu_zone.w * 20.0 /800), bmp_h - map_h- int(menu_zone.h * 20.0 /120), map_w, map_h};
            map_pos = { double(nb_box_w * map_box_w - map_w) / 2 , double(nb_box_h * map_box_w - map_h) / 2 };

            region_pos.x = bmp_pos.x * map_box_w / box_w - map_pos.x + region_slct_zone.x;
            region_pos.y = bmp_pos.y * map_box_w / box_w - map_pos.y + region_slct_zone.y;
            bmp_region_zone.x = region_pos.x;
            bmp_region_zone.y = region_pos.y;
            bmp_region_zone.w = int(bmp_w1)*map_box_w / box_w;
            bmp_region_zone.h = int(bmp_h1) * map_box_w / box_h;

            hide.pos.x = region_slct_zone.x + region_slct_zone.w - hide.w + int(menu_zone.w * 3.0/800);
            hide.pos.y = region_slct_zone.y - hide.h - int(menu_zone.h * 3.0/120);
            anti_hide.pos.x = region_slct_zone.x + region_slct_zone.w - hide.w;
            anti_hide.pos.y = region_slct_zone.y + region_slct_zone.h - hide.h;

            my_cursor.pos = {step_slct_zone.x + 2, step_slct_zone.y + (step_slct_zone.h - my_cursor.h)/2};
            initial_curs_pos = my_cursor.pos;

            open_f_zone = { bmp_w / 3, bmp_h / 5, bmp_w / 3, 3 * bmp_h / 5 };
            draw_f_zone_wscrl = { open_f_zone.x + open_f_zone.w / 20, open_f_zone.y + open_f_zone.h / 20, 18 * open_f_zone.w / 20, 16 * open_f_zone.h / 20 };
            draw_f_zone = draw_f_zone_wscrl;
            scrler_zone = { draw_f_zone.x + draw_f_zone.w - draw_f_zone.w / 16, draw_f_zone.y + draw_f_zone.h / 16 + 2, draw_f_zone.w / 16, draw_f_zone.h - 2* draw_f_zone.h / 16 - 4};
            scrl_zone = scrler_zone;
            scrl_up_zone = { scrl_zone.x, draw_f_zone.y, scrl_zone.w, draw_f_zone.h / 16 };
            scrl_down_zone = { scrl_zone.x, scrl_zone.y + scrl_zone.h + 2, scrl_zone.w, draw_f_zone.h - scrl_zone.h - scrl_up_zone.h - 4};
            draw_f_zone_scrl = { open_f_zone.x + open_f_zone.w / 20, open_f_zone.y + open_f_zone.h / 20, 18 * open_f_zone.w / 20 - scrler_zone.w - 2, 16 * open_f_zone.h / 20 };
            openf_butt_zone = { open_f_zone.x + 4 * open_f_zone.w / 6 , open_f_zone.y + 18 * open_f_zone.h / 20, open_f_zone.w / 6, open_f_zone.h / 20 };
            cancel_butt_zone = { open_f_zone.x + 2 * open_f_zone.w / 6, open_f_zone.y + 18 * open_f_zone.h / 20, open_f_zone.w / 6, open_f_zone.h / 20 };

            file_h = draw_f_zone.h / 12;
            f_href = draw_f_zone.y;
            // allocate memory buffer representing the UI 

            if (bmp_memory)
            {
                VirtualFree(bmp_memory, 0, MEM_RELEASE);
            }

            bmp_info.bmiHeader.biSize = sizeof(bmp_info.bmiHeader);
            bmp_info.bmiHeader.biWidth = bmp_w;
            bmp_info.bmiHeader.biHeight = -bmp_h;
            bmp_info.bmiHeader.biPlanes = 1;
            bmp_info.bmiHeader.biBitCount = 32;
            bmp_info.bmiHeader.biCompression = BI_RGB;
                
            int bmp_mem_size = (bmp_w * bmp_h) * bytes_per_pixel;
            bmp_memory = VirtualAlloc(0, bmp_mem_size, MEM_COMMIT, PAGE_READWRITE);

            // Main Loop
            while (running)
            {

                QueryPerformanceCounter(&begin_time);

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
                // get mouse input
                prev_mouse_coords = mouse_coords;
                if (GetCursorPos(&mouse_coords))
                    if (ScreenToClient(wnd, &mouse_coords))
                    {
                    }
                    else
                        OutputDebugStringA("Error while getting mouse coor rel to client area\n");
                else
                    OutputDebugStringA("Error while getting mouse coor rel to screen\n");
               
                if (normal_mode)
                {
                    recta sh_hide_zone;
                    recta map_zone = { 0,0,0,0 };
                    if (show_map)
                    {
                        sh_hide_zone = get_im_zone(hide);
                        map_zone = region_slct_zone;
                    }
                    else
                        sh_hide_zone = { anti_hide.pos.x - 60, anti_hide.pos.y, anti_hide.w + 60, anti_hide.h };

                    // update bitmap position
                    if (my_mouse.l_down && !zooming)
                    {
                        if (mov_region)
                        {
                            if (in_zone(mouse_coords, region_slct_zone))
                            {
                                bmp_pos.x += (mouse_coords.x - prev_mouse_coords.x) * box_w / map_box_w;
                                bmp_pos.y += (mouse_coords.y - prev_mouse_coords.y) * box_w / map_box_w;

                                if (bmp_pos.x < 0)
                                    bmp_pos.x = 0;
                                if (bmp_pos.x + bmp_w1 > nb_box_w * box_w)
                                    bmp_pos.x = nb_box_w * box_w - bmp_w1;
                                if (bmp_pos.y < 0)
                                    bmp_pos.y = 0;
                                if (bmp_pos.y + bmp_h1 > nb_box_h * box_h)
                                    bmp_pos.y = nb_box_h * box_h - bmp_h1;

                                region_pos.x = bmp_pos.x * map_box_w / box_w - map_pos.x + region_slct_zone.x;
                                region_pos.y = bmp_pos.y * map_box_w / box_w - map_pos.y + region_slct_zone.y;

                                if (region_pos.x <= region_slct_zone.x)
                                {
                                    map_pos.x -= region_slct_zone.x - int(region_pos.x);
                                    region_pos.x = region_slct_zone.x;
                                }
                                if (region_pos.x + bmp_region_zone.w >= region_slct_zone.x + region_slct_zone.w)
                                {
                                    map_pos.x += int(region_pos.x) + bmp_region_zone.w - region_slct_zone.x - region_slct_zone.w;
                                    region_pos.x = region_slct_zone.x + region_slct_zone.w - bmp_region_zone.w;
                                }
                                if (region_pos.y <= region_slct_zone.y)
                                {
                                    map_pos.y -= region_slct_zone.y - int(region_pos.y);
                                    region_pos.y = region_slct_zone.y;
                                }
                                if (region_pos.y + bmp_region_zone.h >= region_slct_zone.y + region_slct_zone.h)
                                {
                                    map_pos.y += int(region_pos.y) + bmp_region_zone.h - region_slct_zone.y - region_slct_zone.h;
                                    region_pos.y = region_slct_zone.y + region_slct_zone.h - bmp_region_zone.h;
                                }

                                if (region_pos.x == region_slct_zone.x)
                                    bmp_pos.x -= 2 * box_w / map_box_w;
                                if (region_pos.x == region_slct_zone.x + region_slct_zone.w - bmp_region_zone.w)
                                    bmp_pos.x += 2 * box_w / map_box_w;
                                if (region_pos.y == region_slct_zone.y)
                                    bmp_pos.y -= 2 * box_w / map_box_w;
                                if (region_pos.y == region_slct_zone.y + region_slct_zone.h - bmp_region_zone.h)
                                    bmp_pos.y += 2 * box_w / map_box_w;

                                if (bmp_pos.x < 0)
                                    bmp_pos.x = 0;
                                if (bmp_pos.x + bmp_w1 > nb_box_w * box_w)
                                    bmp_pos.x = nb_box_w * box_w - bmp_w1;
                                if (bmp_pos.y < 0)
                                    bmp_pos.y = 0;
                                if (bmp_pos.y + bmp_h1 > nb_box_h * box_h)
                                    bmp_pos.y = nb_box_h * box_h - bmp_h1;

                                if (map_pos.x < 0)
                                    map_pos.x = 0;
                                if (map_pos.x >= nb_box_w * map_box_w - map_w)
                                    map_pos.x = nb_box_w * map_box_w - map_w;
                                if (map_pos.y < 0)
                                    map_pos.y = 0;
                                if (map_pos.y >= nb_box_h * map_box_w + map_h)
                                    map_pos.y = nb_box_h * map_box_w - map_h;

                                bmp_region_zone.x = int(region_pos.x);
                                bmp_region_zone.y = int(region_pos.y);
                            }
                        }
                        else if (!in_zone(mouse_coords_ldown, menu_zone) && !in_zone(mouse_coords_ldown, map_zone) && !in_zone(mouse_coords_ldown, sh_hide_zone))
                        {
                            bmp_pos.x -= (mouse_coords.x - prev_mouse_coords.x) / zoom_ratio_x;
                            bmp_pos.y -= (mouse_coords.y - prev_mouse_coords.y) / zoom_ratio_y;
                            if (bmp_pos.x < 0)
                                bmp_pos.x = 0;
                            if (bmp_pos.x + bmp_w1 > nb_box_w * box_w)
                                bmp_pos.x = nb_box_w * box_w - bmp_w1;
                            if (bmp_pos.y < 0)
                                bmp_pos.y = 0;
                            if (bmp_pos.y + bmp_h1 > nb_box_h * box_h)
                                bmp_pos.y = nb_box_h * box_h - bmp_h1;
                        }
                    }

                    // handling mouse clicks
                    if (!gif_saving)
                    {
                        if (my_mouse.r_down)
                        {
                            if (!in_zone(mouse_coords, menu_zone) && !in_zone(mouse_coords, map_zone) && !in_zone(mouse_coords, sh_hide_zone))
                            {
                                static int last_i;
                                static int last_j;
                                point act_box = get_clk_box(mouse_coords);
                                int i = act_box.x;
                                int j = act_box.y;
                                if (i != last_i || j != last_j)
                                {
                                    if (i != 0 && i != nb_box_w - 1 && j != 0 && j != nb_box_h - 1)
                                    {
                                        if (!grid[j * nb_box_w + i].stt)
                                            grid_init_stt.push_back(point{ i, j });
                                        else
                                            grid_init_stt.erase(std::remove(grid_init_stt.begin(), grid_init_stt.end(), point({ i,j })), grid_init_stt.end());
                                        if (i != 0 && i != nb_box_w - 1 && j != 0 && j != nb_box_h - 1)
                                            grid[j * nb_box_w + i].stt = !grid[j * nb_box_w + i].stt;
                                        if (grid[j * nb_box_w + i].stt)
                                        {
                                            if (i < left_lim)
                                                left_lim = i;
                                            if (i > right_lim)
                                                right_lim = i;
                                            if (j < top_lim)
                                                top_lim = j;
                                            if (j > bottom_lim)
                                                bottom_lim = j;
                                        }
                                    }
                                    top_lim0 = top_lim;
                                    left_lim0 = left_lim;
                                    right_lim0 = right_lim;
                                    bottom_lim0 = bottom_lim;
                                }
                                last_i = i;
                                last_j = j;
                            }
                        }
                        else if (my_mouse.l_clk)
                        {
                            if (!in_zone(mouse_coords, menu_zone) && !in_zone(mouse_coords_ldown, map_zone) && !in_zone(mouse_coords_ldown, sh_hide_zone))
                            {
                                //OutputDebugStringA("MOUSE CLICKED HANDLING \n");
                                point down_box = get_clk_box(mouse_coords_ldown);
                                point up_box = get_clk_box(mouse_coords_lup);
                                if (down_box == up_box)
                                {
                                    //OutputDebugStringA("DOWN BOX + UP BOX \n");
                                    int i = up_box.x;
                                    int j = up_box.y;

                                    if (!grid[j * nb_box_w + i].stt)
                                        grid_init_stt.push_back(point{ i, j });
                                    else
                                        grid_init_stt.erase(std::remove(grid_init_stt.begin(), grid_init_stt.end(), point({ i,j })), grid_init_stt.end());
                                    if (i != 0 && i != nb_box_w - 1 && j != 0 && j != nb_box_h - 1)
                                        grid[j * nb_box_w + i].stt = !grid[j * nb_box_w + i].stt;
                                    if (grid[j * nb_box_w + i].stt)
                                    {
                                        if (i < left_lim)
                                            left_lim = i;
                                        if (i > right_lim)
                                            right_lim = i;
                                        if (j < top_lim)
                                            top_lim = j;
                                        if (j > bottom_lim)
                                            bottom_lim = j;
                                    }
                                    top_lim0 = top_lim;
                                    left_lim0 = left_lim;
                                    right_lim0 = right_lim;
                                    bottom_lim0 = bottom_lim;
                                }
                            }
                            else if (in_zone(mouse_coords_ldown, play_butt_zone) && in_zone(mouse_coords_lup, play_butt_zone))
                            {
                                if (sim_on)
                                    prv_targ_stp = actual_step;
                                sim_on = !sim_on;
                            }
                            else if (in_zone(mouse_coords_ldown, go_butt_zone) && in_zone(mouse_coords_lup, go_butt_zone))
                            {
                                go_butt_clkd = true;
                            }
                            else if (in_zone(mouse_coords_ldown, reinit_butt_zone) && in_zone(mouse_coords_lup, reinit_butt_zone))
                            {
                                sim_on = false;
                                reinit_grid();
                                actual_step = 0;
                            }
                            else if (in_zone(mouse_coords_ldown, get_im_zone(next_butt)) && in_zone(mouse_coords_lup, get_im_zone(next_butt)))
                            {
                                target_step = actual_step + 1;
                                sim_on = true;
                                step_dur = 0;
                            }
                            else if ((in_zone(mouse_coords_ldown, get_im_zone(hide)) && in_zone(mouse_coords_lup, get_im_zone(hide)) && show_map) ||
                                ((in_zone(mouse_coords_ldown, get_im_zone(anti_hide)) || in_zone(mouse_coords_ldown, sh_hide_zone)) &&
                                    (in_zone(mouse_coords_lup, get_im_zone(anti_hide)) || in_zone(mouse_coords_lup, sh_hide_zone)) && !show_map))
                            {
                                show_map = !show_map;
                            }
                            else if (in_zone(mouse_coords_ldown, save_butt_zone) && in_zone(mouse_coords_lup, save_butt_zone))
                            {
                                save_config(wnd);
                            }
                            else if (in_zone(mouse_coords_ldown, open_butt_zone) && in_zone(mouse_coords_lup, open_butt_zone))
                            {
                                open_file = true;
                                normal_mode = false;
                            }
                            else if (in_zone(mouse_coords_ldown, clear_butt_zone) && in_zone(mouse_coords_lup, clear_butt_zone))
                            {
                                clear_grid();
                            }
                        }
                    }
                    // normal simulation mode (<=> target_step == -1)
                    if (target_step == -1)
                    {
                        // handling zoom smoothness
                        if (bmp_w1 != bmp_w1_targ || bmp_h1 != bmp_h1_targ)
                        {
                            int zoom_sign = zoom_in ? -1 : 1;
                            bmp_w1 += zoom_sign * zoom_speed * loop_time;
                            bmp_h1 = (bmp_w1 / bmp_w) * bmp_h;

                            bmp_pos.x += -zoom_sign * pos_zpeed_x * loop_time;
                            bmp_pos.y += -zoom_sign * pos_zpeed_y * loop_time;
                            if (zoom_in)
                            {
                                if (bmp_w1 < bmp_w1_targ)
                                {
                                    bmp_w1 = bmp_w1_targ;
                                    bmp_pos.x = bmp_pos_targ.x;
                                    zooming = false;
                                }
                                if (bmp_h1 < bmp_h1_targ)
                                {
                                    bmp_h1 = bmp_h1_targ;
                                    bmp_pos.y = bmp_pos_targ.y;
                                    zooming = false;
                                }
                            }
                            else
                            {
                                if (bmp_w1 > bmp_w1_targ)
                                {
                                    bmp_w1 = bmp_w1_targ;
                                    bmp_pos.x = bmp_pos_targ.x;
                                    zooming = false;

                                }
                                if (bmp_h1 > bmp_h1_targ)
                                {
                                    bmp_h1 = bmp_h1_targ;
                                    bmp_pos.y = bmp_pos_targ.y;
                                    zooming = false;
                                }
                            }
                            zoom_ratio_x = double(bmp_w) / bmp_w1;
                            zoom_ratio_y = double(bmp_h) / bmp_h1;

                            if (bmp_pos.x < 0)
                                bmp_pos.x = 0;
                            if (bmp_pos.x + bmp_w1 > nb_box_w * box_w)
                                bmp_pos.x = nb_box_w * box_w - bmp_w1;
                            if (bmp_pos.y < 0)
                                bmp_pos.y = 0;
                            if (bmp_pos.y + bmp_h1 > nb_box_h * box_h)
                                bmp_pos.y = nb_box_h * box_h - bmp_h1;

                        }

                        // grid inserting 
                        insert_bgnd_color(black);
                        int i = 1;
                        int line_x = 0;
                        while (line_x < bmp_w)
                        {
                            line_x = ((int(bmp_pos.x / box_w) + i) * box_w - bmp_pos.x) * zoom_ratio_x;
                            insert_color_to_zone(white, { line_x, 0, 2, bmp_h });
                            i++;
                        }
                        i = 1;
                        int line_y = 0;
                        while (line_y < bmp_h)
                        {
                            line_y = ((int(bmp_pos.y / box_w) + i) * box_w - bmp_pos.y) * zoom_ratio_y;
                            insert_color_to_zone(white, { 0,line_y, bmp_w, 2 });
                            i++;
                        }

                        // updating the state of bitmap
                        for (int i = int(bmp_pos.x / box_w); i < int((bmp_pos.x + bmp_w1) / box_w) + 1; i++)
                        {
                            for (int j = int(bmp_pos.y / box_h); j < int((bmp_pos.y + bmp_h1) / box_h) + 1; j++)
                            {
                                recta zone = { i * box_w, j * box_h , box_w, box_h };
                                recta bmp_zone = { int(bmp_pos.x), int(bmp_pos.y), bmp_w1, bmp_h1 };

                                int scr_box_x = (zone.x - bmp_pos.x) * zoom_ratio_x;
                                int scr_box_y = (zone.y - bmp_pos.y) * zoom_ratio_y;
                                int scr_box_w = int((zone.x + box_w - bmp_pos.x) * zoom_ratio_x) - scr_box_x - 2;
                                int scr_box_h = int((zone.y + box_h - bmp_pos.y) * zoom_ratio_y) - scr_box_y - 2;

                                recta in_bmp_zone = { scr_box_x + 2, scr_box_y + 2, scr_box_w,  scr_box_h };
                                if (grid[j * nb_box_w + i].stt)
                                    insert_color_to_zone(light, in_bmp_zone);
                                else
                                    insert_color_to_zone(black, in_bmp_zone);
                            }
                        }

                        // handling simulation velocity setting
                        if (int_bar_slctd)
                        {
                            intens_slct.pos.x = mouse_coords.x - intens_slct.w / 2;
                            if (mouse_coords.x >= intens_bar.pos.x + intens_bar.w)
                                intens_slct.pos.x = intens_bar.pos.x + intens_bar.w - intens_slct.w / 2;
                            if (mouse_coords.x <= intens_bar.pos.x)
                                intens_slct.pos.x = intens_bar.pos.x - intens_slct.w / 2;
                        }
                        sim_freq = double(5 * (intens_slct.pos.x + intens_slct.w / 2 - intens_bar.pos.x)) / intens_bar.w + 1;
                        step_dur = 1.0 / sim_freq;

                        if (!mov_region)
                        {
                            bmp_region_zone.x = bmp_pos.x * map_box_w / box_w - map_pos.x + region_slct_zone.x;
                            bmp_region_zone.y = bmp_pos.y * map_box_w / box_w - map_pos.y + region_slct_zone.y;
                            if (bmp_region_zone.x <= region_slct_zone.x)
                                bmp_region_zone.x = region_slct_zone.x;
                            if (bmp_region_zone.x + bmp_region_zone.w >= region_slct_zone.x + region_slct_zone.w)
                                bmp_region_zone.x = region_slct_zone.x + region_slct_zone.w - bmp_region_zone.w;
                            if (bmp_region_zone.y <= region_slct_zone.y)
                                bmp_region_zone.y = region_slct_zone.y;
                            if (bmp_region_zone.y + bmp_region_zone.h >= region_slct_zone.y + region_slct_zone.h)
                                bmp_region_zone.y = region_slct_zone.y + region_slct_zone.h - bmp_region_zone.h;
                            region_pos.x = bmp_region_zone.x;
                            region_pos.y = bmp_region_zone.y;

                        }
                        bmp_region_zone.w = bmp_w1 * map_box_w / box_w;
                        bmp_region_zone.h = bmp_h1 * map_box_w / box_h;
                    }

                    if (gif_saving)
                    {
                        if (my_mouse.l_clk && in_zone(mouse_coords_ldown, reinit_butt_zone) && in_zone(mouse_coords_lup, reinit_butt_zone))
                        {
                            sim_on = false;
                            reinit_grid();
                            actual_step = 0;
                        }

                        if (!gif_rg_slctd)
                        {
                            if (gif_slct_mode)
                            {
                                int x = mouse_coords.x;
                                int y = mouse_coords.y;
                                int x0 = mouse_coords_rdown.x;
                                int y0 = mouse_coords_rdown.y;


                                int mi_x = min(x, x0), mi_y = min(y, y0), ma_x = max(x, x0), ma_y = max(y, y0);
                                insert_recta({ mi_x, mi_y, ma_x - mi_x + 1, ma_y - mi_y + 1 }, l_blue, 2);
                            }
                        }
                        else if (!dlg_crtd)
                        {
                            RECT wr;
                            GetWindowRect(wnd, &wr);
                            int wx = wr.left, wy = wr.top, ww = wr.right - wr.left, wh = wr.bottom - wr.top;
                            int dw = 300, dh = 180;

                            RECT car = { 0, 0, dw, dh };
                            AdjustWindowRect(&car, WS_CAPTION | WS_MINIMIZEBOX | WS_VISIBLE, false);
                            h_dlg = CreateWindowExA(0, dialog_wnd.lpszClassName, "GIf Parameters", WS_VISIBLE | WS_OVERLAPPEDWINDOW, wx + (ww - dw) / 2, wy + (wh - dh) / 2,
                                car.right - car.left, car.bottom - car.top, wnd, NULL, NULL, NULL);
                            h_v_edit = CreateWindowExA(0, "EDIT", "1", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER, 180, 20, 100, 20, h_dlg, (HMENU)DLG_OK_BUTT, NULL, NULL);
                            h_ng_edit = CreateWindowExA(0, "EDIT", "1", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER, 180, 90, 100, 20, h_dlg, (HMENU)DLG_OK_BUTT, NULL, NULL);

                            CreateWindowExA(0, "STATIC", "Velocity (Gen/S) ", WS_VISIBLE | WS_CHILD | SS_CENTER, 20, 20, 160, 20, h_dlg, (HMENU)DLG_OK_BUTT, NULL, NULL);
                            CreateWindowExA(0, "STATIC", "Number Of Generations ", WS_VISIBLE | WS_CHILD | SS_CENTER, 20, 90, 160, 20, h_dlg, (HMENU)DLG_OK_BUTT, NULL, NULL);

                            CreateWindowExA(0, "BUTTON", "OK", WS_VISIBLE | WS_CHILD, 150, 130, 100, 40, h_dlg, (HMENU)DLG_OK_BUTT, NULL, NULL);
                            EnableWindow(main_wnd, false);
                            dlg_crtd = true;
                        }
                    }
                    if (show_map)
                    {
                        insert_region_slct();
                        insert_im(hide);
                    }
                    else
                    {
                        insert_im(anti_hide);
                        int pw = 60;
                        recta panel_zone = { anti_hide.pos.x - pw, anti_hide.pos.y, pw, anti_hide.h };
                        insert_color_to_zone(ld_blue, panel_zone);
                        insert_text_to_zone("map", { panel_zone.x + 3, panel_zone.y + 3, panel_zone.w - 6, panel_zone.h - 6 }, true, black);
                    }

                    // menu zone inserting 
                    insert_color_to_zone(ll_grey, menu_zone);
                    insert_out_recta({ menu_zone.x + 2, menu_zone.y + 2, menu_zone.w - 4, menu_zone.h - 4 }, ld_blue, 4);
                    insert_out_recta({ menu_zone.x + 4, menu_zone.y + 4, menu_zone.w - 8, menu_zone.h - 8 }, grey, 2);

                    insert_im(intens_bar);
                    insert_out_recta(step_slct_zone, grey, 3);
                    insert_im(go_butt);
                    insert_text_to_zone("go to step", go_to_stp_zone, true, grey);
                    insert_text_to_zone("simulation velocity", sim_vel_zone, true, grey);
                    insert_save_butt();
                    insert_open_butt();
                    insert_clear_butt();
                    insert_actual_step();

                    if (!sim_on || target_step != -1)
                        if (!plpau_down)
                            if (in_zone(mouse_coords, play_butt_zone))
                                insert_im(play_butt_1);
                            else
                                insert_im(play_butt_0);
                        else
                            insert_im(play_butt_2);
                    else
                        if (!plpau_down)
                            if (in_zone(mouse_coords, play_butt_zone))
                                insert_im(pause_butt_1);
                            else
                                insert_im(pause_butt_0);
                        else
                            insert_im(pause_butt_2);

                    if (!reinit_down)
                        if (in_zone(mouse_coords, reinit_butt_zone))
                            insert_im(reinit_butt_1);
                        else
                            insert_im(reinit_butt);
                    else
                        insert_im(reinit_butt_2);
                    if (!next_down)
                        if (in_zone(mouse_coords, get_im_zone(next_butt)))
                            insert_im(next_butt_1);
                        else
                            insert_im(next_butt);
                    else
                        insert_im(next_butt_2);
                    if (!go_butt_down)
                        if (in_zone(mouse_coords, go_butt_zone))
                            insert_im(go_butt_1);
                        else
                            insert_im(go_butt);
                    else
                        insert_im(go_butt_2);

                    insert_color_to_zone(slct_color, { intens_bar.pos.x + 2, intens_bar.pos.y + 1, intens_slct.pos.x - intens_bar.pos.x + 2, 4 });
                    insert_im(intens_slct);
                    insert_sim_vel();
                    if (my_mouse.l_down && !gif_saving)
                    {
                        if (in_zone(mouse_coords_ldown, save_butt_zone))
                            insert_out_recta(save_butt_zone, black, 2);
                        if (in_zone(mouse_coords_ldown, open_butt_zone))
                            insert_out_recta(open_butt_zone, black, 2);
                        if (in_zone(mouse_coords_ldown, clear_butt_zone))
                            insert_out_recta(clear_butt_zone, black, 2);
                    }
                    // play button click = simulation on => stepping ahead with given velocity
                    if (sim_on)
                    {
                        if (sim_timer >= step_dur)
                            is_step_time = false;
                        if (!is_step_time)
                        {
                            sim_timer = 0;
                            is_step_time = true;
                            one_step_ahead();
                            actual_step++;
                        }
                    }
                    // handle go button click
                    if (go_butt_clkd && !gif_saving)
                    {
                        target_step = get_step_int();

                        if (target_step > prv_targ_stp)
                        {
                            sim_on = true;
                            step_dur = 0;
                        }
                        else if (target_step < prv_targ_stp || target_step < actual_step)
                        {
                            sim_on = true;
                            step_dur = 0;
                            reinit_grid();
                            actual_step = 0;
                        }
                        else if (actual_step < target_step)
                        {
                            sim_on = true;
                            step_dur = 0;
                        }
                    }
                    // handle selected elements in step setting zone 
                    if (slct_mode)
                    {
                        if (mouse_coords.x - initial_slct_x >= 0)
                        {
                            int first_char_idx = (initial_slct_x - initial_curs_pos.x) / num_width;
                            if (mouse_coords.x - initial_curs_pos.x <= num_crctrs * num_width)
                                slctd_nbr = (mouse_coords.x - initial_slct_x + num_width / 2) / num_width;
                            else
                                slctd_nbr = (initial_curs_pos.x + num_crctrs * num_width - initial_slct_x + num_width / 2) / num_width;

                            for (int i = 0; i < slctd_nbr; i++)
                                slctd_nums[i] = first_char_idx + i;
                        }
                        else
                        {
                            int first_char_idx = (initial_slct_x - initial_curs_pos.x) / num_width - 1;
                            if (mouse_coords.x >= initial_curs_pos.x)
                                slctd_nbr = (initial_slct_x - mouse_coords.x + num_width / 2) / num_width;
                            else
                                slctd_nbr = (initial_slct_x - initial_curs_pos.x + num_width / 2) / num_width;

                            for (int i = 0; i < slctd_nbr; i++)
                                slctd_nums[slctd_nbr - i - 1] = first_char_idx - i;

                        }
                    }
                    for (int i = 0; i < slctd_nbr; i++)
                    {
                        recta num_zone = { initial_curs_pos.x + slctd_nums[i] * num_width, initial_curs_pos.y + 2, num_width, num_height };
                        insert_color_to_zone(slct_color, num_zone);
                    }

                    // handle cursor position 
                    if (my_mouse.l_down && in_zone(mouse_coords_ldown, step_slct_zone))
                    {
                        int block = (mouse_coords.x - initial_curs_pos.x + num_width / 2) / num_width;
                        if (block <= 0)
                            my_cursor.pos.x = initial_curs_pos.x;
                        else if (block <= num_crctrs)
                            my_cursor.pos.x = initial_curs_pos.x + block * num_width;
                        else
                            my_cursor.pos.x = initial_curs_pos.x + num_crctrs * num_width;
                    }

                    // blit set step
                    static point pos = { initial_curs_pos.x, initial_curs_pos.y + (my_cursor.h - num_height) / 2 };
                    for (int i = 0; i < num_crctrs; i++)
                    {
                        insert_num(step_str[i], { pos.x, pos.y, num_width, num_height });
                        pos.x += num_width;

                    }
                    pos = { initial_curs_pos.x, initial_curs_pos.y + (my_cursor.h - num_height) / 2 };

                    if (show_cursor)
                    {
                        if (cursor_timer <= 0.8)
                        {
                            insert_im(my_cursor);
                        }
                        if (cursor_timer >= 1.6)
                            cursor_timer = 0;
                    }
                }
                else if(open_file) // show the open file window
                {
                    // insert bakgnd colors and outlines
                    insert_color_to_zone(white, open_f_zone);
                    insert_out_recta(open_f_zone, l_grey, 4);
                    insert_out_recta(draw_f_zone, black, 2);

                    // insert open cancel buttons
                    insert_out_recta(openf_butt_zone, grey, 3);
                    insert_out_recta(cancel_butt_zone, grey, 3);
                    if (in_zone(mouse_coords, openf_butt_zone))
                        insert_color_to_zone(orange, openf_butt_zone);
                    else if (in_zone(mouse_coords, cancel_butt_zone))
                        insert_color_to_zone(orange, cancel_butt_zone);
                    insert_text("open", { openf_butt_zone.x + openf_butt_zone.w / 10 ,openf_butt_zone.y + openf_butt_zone.h / 10 }, { 12, 8 * openf_butt_zone.h / 10 }, true, black, openf_butt_zone);
                    insert_text("cancel", { cancel_butt_zone.x + cancel_butt_zone.w / 10, cancel_butt_zone.y + cancel_butt_zone.h / 10 }, { 12, 8 * cancel_butt_zone.h / 10 }, true, black, cancel_butt_zone);

                    // handling left mouse down
                    if (my_mouse.l_down)
                    {
                        if (scrl_exist)
                        {
                            if (scrler_draging)
                            {
                                scrler_zone.y = scrler_y0 + mouse_coords.y - mouse_coords_ldown.y;
                                f_href = draw_f_zone.y - (scrler_zone.y - scrl_zone.y) * scrl_ratio;
                            }
                            else if (scrl_timer > 0.3)
                            {
                                if (in_zone(mouse_coords, scrl_up_zone) && in_zone(mouse_coords_ldown, scrl_up_zone))
                                {
                                    f_href += file_h;
                                    scrler_zone.y = scrl_zone.y + (draw_f_zone.y - f_href) / scrl_ratio;
                                }
                                else if (in_zone(mouse_coords, scrl_down_zone) && in_zone(mouse_coords_ldown, scrl_down_zone))
                                {
                                    
                                    f_href -= file_h;
                                    scrler_zone.y = scrl_zone.y + (draw_f_zone.y - f_href) / scrl_ratio;
                                }
                                else if (scrler_attrctng)
                                {
                                    if (in_zone(mouse_coords, scrl_zone) && !in_zone(mouse_coords, scrler_zone))
                                    {
                                        int new_dir = (scrler_zone.y - mouse_coords.y > 0) ? -1 : 1;
                                        if (new_dir == scrl_dir)
                                        {
                                            scrler_zone.y += scrl_dir * scrler_zone.h * 0.7;
                                            f_href = draw_f_zone.y - (scrler_zone.y - scrl_zone.y) * scrl_ratio;
                                        }
                                    }
                                }
                            }

                            if (scrler_zone.y < scrl_zone.y)
                                scrler_zone.y = scrl_zone.y;
                            if (scrler_zone.y + scrler_zone.h > scrl_zone.y + scrl_zone.h)
                                scrler_zone.y = scrl_zone.y + scrl_zone.h - scrler_zone.h;

                            if (f_href > draw_f_zone.y)
                                f_href = draw_f_zone.y;
                            if (f_href < draw_f_zone.y + draw_f_zone.h - (nb_files + 2) * file_h)
                                f_href = draw_f_zone.y + draw_f_zone.h - (nb_files + 2) * file_h;
                        }
                        else if (in_zone(mouse_coords_ldown, cancel_butt_zone) && in_zone(mouse_coords, cancel_butt_zone))
                        {
                            insert_out_recta(cancel_butt_zone, black, 4);
                        }
                        else if (in_zone(mouse_coords_ldown, openf_butt_zone) && in_zone(mouse_coords, openf_butt_zone))
                        {
                            insert_out_recta(openf_butt_zone, black, 4);
                        }

                    }
                    // handling left mouse click
                    if (my_mouse.l_clk)
                    {
                        if (in_zone(mouse_coords_ldown, cancel_butt_zone) && in_zone(mouse_coords_lup, cancel_butt_zone))
                        {
                            open_file = false;
                            normal_mode = true;
                            fslctd_idx = -1;
                        }
                        if (fslctd_idx != -1 && in_zone(mouse_coords_ldown, openf_butt_zone) && in_zone(mouse_coords_lup, openf_butt_zone))
                            openf_change_dir(fslctd_idx);
                    }

                    // hovered and selected file hilighting
                    if (in_zone(mouse_coords, draw_f_zone))
                    {
                        int idx_hilit = ((draw_f_zone.y - f_href) + (mouse_coords.y - draw_f_zone.y)) / file_h;
                        if (idx_hilit < nb_files)
                            insert_color_to_zone(lg_blue, { draw_f_zone.x, f_href + idx_hilit * file_h, draw_f_zone.w, file_h }, draw_f_zone);
                    }
                    if (fslctd_idx != -1)
                    {
                        insert_color_to_zone(ldg_blue, { draw_f_zone.x, f_href + fslctd_idx * file_h, draw_f_zone.w, file_h }, draw_f_zone);
                    }

                    // insert file names
                    point char_size = { 10, 5 * file_h / 10 };
                    int icon_w = file_h;
                    point file_txt_pos = { draw_f_zone.x + icon_w , -1 };
                    int txty_off = (file_h - char_size.y) / 2;
                    for (int i = (draw_f_zone.y - f_href) / file_h; i < nb_files; i++)
                    {
                        int row_y = f_href + i * file_h;
                        if (row_y > draw_f_zone.y + draw_f_zone.h)
                            break;
                        file_txt_pos.y = row_y + txty_off;
                        if (files_q[i].second)
                            instrch_from_im(folder_icon, { 0,0,folder_icon.w, folder_icon.h }, { draw_f_zone.x, row_y, icon_w, file_h }, false, black, draw_f_zone);
                        insert_text(files_q[i].first.c_str(), file_txt_pos, char_size, false, black, draw_f_zone);
                    }

                    // add scroll bar when necessary
                    if (scrl_exist)
                    {
                        insert_out_recta(scrl_zone, black, 2);
                        if(scrler_draging)
                            insert_color_to_zone(grey, scrler_zone);
                        else
                            insert_color_to_zone(l_grey, scrler_zone);
    
                        insert_out_recta(scrl_up_zone, black, 2);
                        insert_out_recta(scrl_down_zone, black, 2);
                        if (in_zone(mouse_coords, scrl_up_zone))
                            instrch_from_im(scrl_up1, { 0,0,scrl_up1.w, scrl_up1.h }, scrl_up_zone);
                        else
                            instrch_from_im(scrl_up, { 0,0,scrl_up.w, scrl_up.h }, scrl_up_zone);
                        if (in_zone(mouse_coords, scrl_down_zone))
                            instrch_from_im(scrl_down1, { 0,0,scrl_down1.w, scrl_down1.h }, scrl_down_zone);
                        else
                            instrch_from_im(scrl_down, { 0,0,scrl_down.w, scrl_down.h }, scrl_down_zone);
                    }

                }
                else if (open_f_dlg)
                {
                    insert_dlg_box("Please choose a valid .txt file !", "File Type Error", { open_f_zone.x + open_f_zone.w / 10, open_f_zone.y + open_f_zone.h / 10, 8 * open_f_zone.w / 10,0 }, ll_grey, orange);
                }
                else if (f_error_dlg)
                {
                    insert_dlg_box("The chosen file contains incompatible data. Please choose a valid file.", "Error", { open_f_zone.x + open_f_zone.w / 10, open_f_zone.y + open_f_zone.h / 10, 8 * open_f_zone.w / 10, 0 }, ll_grey, orange);
                }
                my_mouse.l_clk = false;
                go_butt_clkd = false;
                save_clkd = false;
                open_clkd = false;
                
                double frame_time = frame_dur;
                if (frame_calc_start)
                {
                    QueryPerformanceCounter(&before_blit_time);
                    frame_time = double(before_blit_time.QuadPart - end_time.QuadPart) / double(PerformanceFrequency);
                }
                else
                    frame_calc_start = true;

                if (frame_time < frame_dur && target_step == -1)
                    Sleep((frame_dur - frame_time) * 1000);
                               
                HDC device_context = GetDC(wnd);
                RECT client_rect;
                GetClientRect(wnd, &client_rect);
                update_wnd(device_context, &client_rect);
                ReleaseDC(wnd, device_context);
                
                if(actual_step == target_step)
                {
                    prv_targ_stp = target_step;
                    target_step = -1;
                    sim_on = false;
                }
                
                QueryPerformanceCounter(&end_time);
                loop_time = double(end_time.QuadPart - begin_time.QuadPart) / double(PerformanceFrequency);
                scrl_timer += loop_time;
                
                sim_timer += loop_time;
                cursor_timer += loop_time;
               
            }
        }
    }
    else
    {
    }

    return(0);
}