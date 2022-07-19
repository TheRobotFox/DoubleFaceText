#include "mesh.h"
#include <stdio.h>

int usage(const char *name)
{
	return	printf("Usage: %s [IMG_A] [IMG_B] [IMG_C]", name);
}

int main(int argc, char **argv)
{

	Volume vol = Volume_create();
	puts("Created Volume");
	bool res = Volume_from_nbt(vol, argv[1], NULL);
	if(!res)
	puts("Success!");
//	if(argc != 4)
//		return usage(argv[0]);
//
//	Volume volume = Volume_create(argv[1],argv[2],argv[3]);
//
	Mesh mesh = Mesh_create();
	Mesh_from_volume(mesh,vol);
	Mesh_save_obj(mesh, "out.obj");
	Mesh_save_stl(mesh, "out.stl");

	Mesh_free(mesh);

	Image a, b, c;
	Volume_to_shadow(vol, &a, &b, &c);
	Image_save(a, "A.png");
	Image_save(b, "B.jpg");
	Image_save(c, "C.bmp");

	Volume_free(vol);
}
