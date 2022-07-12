#include "mesh.h"
#include <stdio.h>

int usage(const char *name)
{
	return	printf("Usage: %s [IMG_A] [IMG_B] [IMG_C]", name);
}

int main(int argc, char **argv)
{
	if(argc != 4)
		return usage(argv[0]);

	Volume volume = Volume_create(argv[1],argv[2],argv[3]);

	Mesh mesh = Mesh_create();
	Mesh_from_volume(mesh,volume);
	//struct Vertex a = {-1.0f,0,-1.0f},
	//							b = {1.0f, 0, -1.0f},
	//							c = {1.0f, 0, 1.0f},
	//							d = {-1.0f, 0, 1.0f};
	//Mesh_face_add(mesh,a,b,c,d);
	Mesh_save_obj(mesh, "out.obj");
	Mesh_save_stl(mesh, "out.stl");
	Mesh_free(mesh);
	Volume_free(volume);

}
