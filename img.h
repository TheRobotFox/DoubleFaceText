#include <stdbool.h>
#include "NoFont/Font_Driver.h"

typedef struct Image* Image;

Image Image_create();

Image Image_load(const char *path);

bool Image_from_file(Image img, const char *path);
bool Image_from_color(Image img, int x, int y, unsigned char col);
bool Image_from_text(const char *text);

int Image_get_x(Image img);
int Image_get_y(Image img);

unsigned char *Image_get(Image img, int x, int y);

int f_Image_draw_rect(Font_Rect *rect, void *arg);

bool Image_save(Image img, const char *path);
void Image_free(Image img);

