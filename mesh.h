#pragma once


//TODO: Smooth Slices
//TODO: Chunking
//TODO: fix voxelization
//TODO: Speedup Meshing (still) -> by obviously redoing at_or_make

typedef struct Mesh* Mesh;

#include "mesh_internal.h"

typedef	struct Vertex Trig[3];

#include "volume.h"

Mesh Mesh_create();
bool Mesh_from_volume(Mesh mesh, Volume volume);
void Mesh_face_add(Mesh mesh, struct Vertex a,
							  struct Vertex b,
							  struct Vertex c,
							  struct Vertex d
				  );
bool Mesh_save_obj(Mesh mesh, const char *path);
bool Mesh_save_stl(Mesh mesh, const char *path);
void Mesh_free(Mesh mesh);

typedef struct Vertex Vector;

union coord{
	struct Vertex v;
	float a[3];
};
struct Dimensions
{
	union coord min, max;
};

List Mesh_read_stl(const char *path);
List Mesh_to_chunks(List trigs, size_t res[3]);

float Coord_remap(float a, struct Dimensions size, size_t d);
bool Mesh_to_slices(List trigs, size_t res[3]);
List Mesh_intersects(List trigs, Vector pos, Vector direction);
float Coord_remap(float a, struct Dimensions size, size_t d);
struct Dimensions Mesh_dimensions(List trigs);
