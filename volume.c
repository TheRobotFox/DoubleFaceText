#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "volume.h"
#include <stdio.h>
#include <stdlib.h>


#define THRESHOLD 128

bool* Volume_get(Volume v, int x, int y, int z) { return v->data+(v->x*v->y)*z + (v->x)*y + x;}

void Volume_internal_fill(Volume v,unsigned char *A, unsigned char *B, unsigned char *C)
{
	for(int z=0; z<v->z; z++)
		for(int y=0; y<v->y; y++)
			for(int x=0; x<v->x; x++)
				*Volume_get(v,x,y,z)= (
										(A[v->x*y+x]<THRESHOLD) &&
										(B[v->z*y+z]<THRESHOLD) &&
										(C[v->x*z+x]<THRESHOLD)
						);
}


Volume Volume_create(const char *img_A, const char *img_B, const char *img_C)
{

	int x,y,z, comp, tmp, tmp2;
	unsigned char *x_pix, *y_pix, *z_pix;

	x_pix = stbi_load(img_A, &x, &y, &comp, 1);
	y_pix = stbi_load(img_B, &z, &tmp, &comp, 1);

	if(tmp!=y)
		exit(puts("[ERROR] Y image misalignment!"));


	z_pix = stbi_load(img_C, &tmp, &tmp2, &comp, 1);

	if(tmp!=x || tmp2 !=z)
		exit(puts("[ERROR] Z image misalignment!"));

	printf("X: %d Y: %d Z: %d\n",x,y,z);

	Volume vol = malloc(sizeof(struct Volume));
	vol->x=x;
	vol->y=y;
	vol->z=z;
	vol->data = malloc(x*y*z);

	Volume_internal_fill(vol,x_pix,y_pix,z_pix);

	stbi_image_free(x_pix);
	stbi_image_free(y_pix);
	stbi_image_free(z_pix);
	return vol;
}


void Volume_free(Volume v)
{
	free(v->data);
	free(v);
}
