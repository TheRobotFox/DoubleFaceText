#include "info/info.h"
#include "mesh.h"
#include "info/List/List.h"
#include "volume_internal.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

struct Face
{
	size_t vertex_index[4];
};

struct Mesh
{
	List vertecies;
	List faces;
};

Mesh Mesh_create()
{
	Mesh mesh = malloc(sizeof(struct Mesh));
	mesh->vertecies=List_create(sizeof(struct Vertex));
	mesh->faces=List_create(sizeof(struct Face));

	return mesh;
}

static bool _comp_vertex(void *a, void *b)
{
	return !memcmp(a,b,sizeof(struct Vertex));
}
size_t List_get_or_make(List l, void* object)
{
	uint8_t *found = List_find(l,_comp_vertex, object);
	if(found)
		return (found-(uint8_t*)List_start(l))/sizeof(struct Vertex);
	List_append(l,object);
	return List_size(l)-1;
}

void Mesh_face_add(Mesh mesh, struct Vertex a,
							  struct Vertex b,
							  struct Vertex c,
							  struct Vertex d
				  )
{
	struct Face face;
	face.vertex_index[0]=List_get_or_make(mesh->vertecies,&a);
	face.vertex_index[1]=List_get_or_make(mesh->vertecies,&b);
	face.vertex_index[2]=List_get_or_make(mesh->vertecies,&c);
	face.vertex_index[3]=List_get_or_make(mesh->vertecies,&d);

	List_append(mesh->faces, &face);

}

void Mesh_internal_face_X(Mesh mesh, int x, int y, int z)
{

	Mesh_face_add(mesh, (struct Vertex){x-.5f,y-.5f,z-0.5f},
						(struct Vertex){x-.5f,y+.5f,z-0.5f},
						(struct Vertex){x-.5f,y+.5f,z+0.5f},
						(struct Vertex){x-.5f,y-.5f,z+0.5f}
				 );
}

void Mesh_internal_face_Y(Mesh mesh, int x, int y, int z)
{

	Mesh_face_add(mesh, (struct Vertex){x-.5f,y-.5f,z-0.5f},
						(struct Vertex){x+.5f,y-.5f,z-0.5f},
						(struct Vertex){x+.5f,y-.5f,z+0.5f},
						(struct Vertex){x-.5f,y-.5f,z+0.5f}
				 );
}
void Mesh_internal_face_Z(Mesh mesh, int x, int y, int z)
{

	Mesh_face_add(mesh, (struct Vertex){x-.5f,y-.5f,z-0.5f},
						(struct Vertex){x-.5f,y+.5f,z-0.5f},
						(struct Vertex){x+.5f,y+.5f,z-0.5f},
						(struct Vertex){x+.5f,y-.5f,z-0.5f}
				 );
}

bool Mesh_from_volume(Mesh mesh, Volume volume)
{
	bool inside;

	SEG_BEGIN("Meshing")

	INFO("Calculating X faces")

	for(int z=0; z<volume->z; z++)
	{
		for(int y=0; y<volume->y; y++)
		{
			inside=false;
			for(int x=0; x<volume->x; x++)
			{
				if(inside!=*Volume_get(volume, x,y,z)){
					Mesh_internal_face_X(mesh, x,y,z);
					inside=!inside;
				}
			}
			if(inside)
				Mesh_internal_face_X(mesh, volume->x,y,z);
		}

	}

	INFO("Calculating Y faces")

	for(int x=0; x<volume->x; x++)
	{
		for(int z=0; z<volume->z; z++)
		{
			inside=false;
			for(int y=0; y<volume->y; y++)
			{
				if(inside!=*Volume_get(volume, x,y,z)){
					Mesh_internal_face_Y(mesh, x,y,z);
					inside=!inside;
				}
			}
			if(inside)
				Mesh_internal_face_Y(mesh, x, volume->y,z);
		}
	}

	INFO("Calculating Z faces")

	for(int y=0; y<volume->y; y++)
	{
		for(int x=0; x<volume->x; x++)
		{
			inside=false;
			for(int z=0; z<volume->z; z++)
			{
				if(inside!=*Volume_get(volume, x,y,z)){
					Mesh_internal_face_Z(mesh, x,y,z);
					inside=!inside;
				}
			}
			if(inside)
				Mesh_internal_face_Z(mesh, x,y,volume->z);
		}
	}
	SEG_END
	SUCCESS("Mesh generation!")
	return false;
}


// Wavefront Obj
// ASCII
// Vertex data: v [x] [y] [z] \n
// Face data  : f [v1 index]  [v2 index][v3 index]\n | 1 indexed

bool Mesh_save_obj(Mesh mesh, const char *path)
{
	INFO("Saving Mesh at '%s' as Wavefront-Obj", path)
	FILE *file = fopen(path, "w");

	if(file){

		INFO("Faces: %d Vertecies: %d", List_size(mesh->faces), List_size(mesh->vertecies))

		// Write Vertex data
		for(struct Vertex *v = List_start(mesh->vertecies),
				*e = List_end(mesh->vertecies);
				v != e; 	v++)
			fprintf(file, "v %.3f %.3f %.3f\n", v->x, v->y, v->z);

		// Write Face data 1 indexed
		for(struct Face *f = List_start(mesh->faces),
				*e = List_end(mesh->faces);
				f != e; 	f++)
			fprintf(file, "f %ld %ld %ld %ld\n", f->vertex_index[0]+1,
											     f->vertex_index[1]+1,
											     f->vertex_index[2]+1,
											     f->vertex_index[3]+1
				   );

		SUCCESS("exported! %s -> wrote %lu bytes", path, ftell(file))
		fclose(file);
		return false;
	}
	return true;
}

// STL
// Binary
// [Header]<80b> ; unused
// [num_triangles]<4b>
// [Triangle_data]...

#pragma pack(push,1)

struct Triangle_data
{
	float normals[3]; // unused
	struct Vertex vertecies[3];
	uint16_t additional_data; // unused
};

#pragma pack(pop)

List Mesh_read_stl(const char *path)
{
	INFO("Reading STL from: %s", path);

	FILE *f=fopen(path, "rb");

	if(f==0){
		ERROR("Could not open file: %s", strerror(errno))
		return NULL;
	}

	fseek(f, 80, SEEK_SET);
	unsigned int count;
	fread(&count, sizeof(count), 1, f);

	List l = List_create(sizeof(Trig));
	List_reserve(l, count);

	struct Triangle_data data;
	for(int i=0; i<count; i++)
	{
		fread(&data, sizeof(data), 1, f);
		List_append(l, (Trig*)&data.vertecies);
	}

	fclose(f);
	INFO("Read %d Triangles!", List_size(l))
	return l;
}

bool Mesh_save_stl(Mesh mesh, const char *path)
{
	INFO("Saving Mesh at '%s' as STL", path)
	FILE *file = fopen(path, "wb");
	if(file){

		// Write empty header
		uint8_t header[80] = "header unused. Soo yeah... guess I just put some promo here UwU -NoHamster 2022";
		fwrite(header, 1, 80, file);

		uint32_t num_of_trig = 2*List_size(mesh->faces);
		fwrite(&num_of_trig, 4, 1, file);

		INFO("Triangles: %u", num_of_trig)
		// Tessellate Quad with two Triangles
		struct Triangle_data trig1 = {0},
												 trig2 = {0};

		for(struct Face *f = List_start(mesh->faces),
				*e = List_end(mesh->faces); f != e; f++)
		{
			trig1.vertecies[0]=*(struct Vertex*)List_get(mesh->vertecies,f->vertex_index[0]);
			trig1.vertecies[1]=*(struct Vertex*)List_get(mesh->vertecies,f->vertex_index[1]);
			trig1.vertecies[2]=*(struct Vertex*)List_get(mesh->vertecies,f->vertex_index[2]);
			fwrite(&trig1,50, 1, file);

			trig2.vertecies[0]=*(struct Vertex*)List_get(mesh->vertecies,f->vertex_index[2]);
			trig2.vertecies[1]=*(struct Vertex*)List_get(mesh->vertecies,f->vertex_index[3]);
			trig2.vertecies[2]=*(struct Vertex*)List_get(mesh->vertecies,f->vertex_index[0]);
			fwrite(&trig2,50, 1, file);
		}

		SUCCESS("exported! %s -> wrote %lu bytes", path, ftell(file))
		fclose(file);
		return false;
	}
	return true;
}

void Mesh_free(Mesh mesh)
{
	List_free(mesh->vertecies);
	List_free(mesh->faces);
	free(mesh);
}
