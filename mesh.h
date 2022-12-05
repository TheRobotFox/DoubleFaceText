#include "volume.h"

typedef struct Mesh* Mesh;

struct Vertex
{
	float x,y,z;
};

typedef	struct Vertex Trig[3];

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

bool Trig_to_Volume(List trigs:wq
