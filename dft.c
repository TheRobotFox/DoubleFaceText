#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"
#include <stdbool.h>

struct Volume
{
	int x,y,z;
	bool *data;
};

#define THRESHOLD 128

inline bool* volume_get(struct Volume *v, int x, int y, int z) { return data[(v->x*v->y)*z + (v->x)*y + x];}

void volume__fill(struct Volume *v, stbi_uc *A, stbi_uc *B, tbi_uc *C)
{
	for(int z=0; z<v->z; z++)
		for(int y=0; y<v->y; y++)
			for(int x=0; x<v->x; x++)
				*volume_get(v,x,y,z)= (
										(A[(v->x)*y+x]>THRESHOLD) &&
										(B[(v->z)*y+z]>THRESHOLD) &&
										(A[(v->x)*z+x]>THRESHOLD)
						);
}


struct Volume* volume_create(const char *img_A, const char *img_B, const char *img_C)
{

	int x,y,z, comp, tmp, tmp2;
	stbi_uc *x_pix, *y_pix, *z_pix;

	x_pix = stbi_load(img_A, &x, &y, &comp, 0);
	y_pix = stbi_load(img_B, &z, &tmp, &comp, 0);

	if(tmp!=y)
		exit(puts("[ERROR] Y image misalignment!"));


	z_pix = stbi_load(img_C, &tmp, &tmp2, &comp, 0);

	if(tmp!=x || tmp2 !=z)
		exit(puts("[ERROR] Z image misalignment!"));

	struct Volume *vol = malloc(sizeof(vol);
	vol->x=x;
	vol->y=y;
	vol->z=z;
	vol->data = malloc(x*y*z);

	volume__fill(vol,x_pix,y_pix,z_pix);

	return vol;
}

void volume_free(struct Volume *v)
{
	free(v->data);
	free(v);
}

int main(int argc, char* argv)
{
	if(argc == 4)
		return usage();

	void *volume = volume_create(argv[2],argv[2],argv[3]);

}
