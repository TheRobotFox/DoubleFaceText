#include "img.h"
#include "img_internal.h"
#include "info/info.h"
#include <string.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

Image Image_create()
{
	Image img = malloc(sizeof(struct Image));
	memset(img,0,sizeof(struct Image));
	return img;
}

bool Image_from_file(Image img, const char *path)
{

	if(img->data)
		stbi_image_free(img->data);

	int comp;
	img->data = stbi_load(path, &img->x, &img->y, &comp, 1);
	return !img->data;
}

bool Image_from_color(Image img, int x, int y, unsigned char col)
{

	img->x=x;
	img->y=y;
	img->data=malloc(img->x*img->y);
	memset(img->data, 255, img->x*img->y);
	return 0;
}

Image Image_load(const char *path)
{
	Image img = Image_create();
	if(Image_from_file(img, path))
		return img;
	free(img);
	return NULL;
}

int Image_get_x(Image img){return img->x;}
int Image_get_y(Image img){return img->y;}

unsigned char *Image_get(Image img, int x, int y)
{
	return img->data+img->x*y+x;
}

bool Image_save(Image img, const char * path)
{
	char indicator=0;
	for(size_t i=1; i<strlen(path); i++)
	{
		if(path[i-1]=='.')
			indicator = path[i];
	}

	stbi_flip_vertically_on_write(true);

	switch(indicator)
	{
	case 'b':
	case 'B':
		INFO("Saving image '%s' as Bitmap", path);
		return !stbi_write_bmp(path, img->x, img->y, 1, img->data);
	case 'j':
	case 'J':
		INFO("Saving image '%s' as Jpeg", path);
		return !stbi_write_jpg(path, img->x, img->y, 1, img->data, 50);
	default:
		INFO("Could not detect Image format!")
	case 'p':
	case 'P':
		INFO("Saving image '%s' as PNG", path);
		return !stbi_write_png(path, img->x, img->y, 1, img->data, img->x);
	}
}

void Image_free(Image img)
{
	stbi_image_free(img->data);
	free(img);
}
