#include "mesh.h"
#include "List/List.h"
#include "volume_internal.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>

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

void Mesh_face_add(Mesh mesh,
										struct Vertex a,
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

	Mesh_face_add(mesh,
								(struct Vertex){x-.5f,y-.5f,z-0.5f},
								(struct Vertex){x-.5f,y+.5f,z-0.5f},
								(struct Vertex){x-.5f,y+.5f,z+0.5f},
								(struct Vertex){x-.5f,y-.5f,z+0.5f}
								);
}

void Mesh_internal_face_Y(Mesh mesh, int x, int y, int z)
{

	Mesh_face_add(mesh,
								(struct Vertex){x-.5f,y-.5f,z-0.5f},
								(struct Vertex){x+.5f,y-.5f,z-0.5f},
								(struct Vertex){x+.5f,y-.5f,z+0.5f},
								(struct Vertex){x-.5f,y-.5f,z+0.5f}
								);
}
void Mesh_internal_face_Z(Mesh mesh, int x, int y, int z)
{

	Mesh_face_add(mesh,
								(struct Vertex){x-.5f,y-.5f,z-0.5f},
								(struct Vertex){x-.5f,y+.5f,z-0.5f},
								(struct Vertex){x+.5f,y+.5f,z-0.5f},
								(struct Vertex){x+.5f,y-.5f,z-0.5f}
								);
}

void Mesh_from_volume(Mesh mesh, Volume volume)
{
	bool inside;

	puts("Calculating X faces");

	bool *data = volume->data;
	for(int z=0; z<volume->z; z++)
	{
		for(int y=0; y<volume->y; y++)
		{
			inside=false;
			for(int x=0; x<volume->x; x++)
			{
				if(inside!=*data++){
					Mesh_internal_face_X(mesh, x,y,z);
					inside=!inside;
				}
			}
			if(inside)
				Mesh_internal_face_X(mesh, volume->x,y,z);
		}

	}

	puts("Calculating Y faces");
	data = volume->data;
	int row = volume->x;
	int layer = row*volume->y;
	int cube = layer*volume->z;

	for(int x=0; x<volume->x; x++)
	{
		for(int z=0; z<volume->z; z++)
		{
			inside=false;
			for(int y=0; y<volume->y; y++)
			{
				if(inside!=*data){
					Mesh_internal_face_Y(mesh, x,y,z);
					inside=!inside;
				}
				data+=row;
			}
			if(inside)
				Mesh_internal_face_Y(mesh, x, volume->y,z);
		}
		data-=cube;
		data++;
	}

	puts("Calculating Z faces");
	data = volume->data;

	for(int y=0; y<volume->y; y++)
	{
		for(int x=0; x<volume->x; x++)
		{
			inside=false;
			for(int z=0; z<volume->z; z++)
			{
				if(inside!=*data){
					Mesh_internal_face_Z(mesh, x,y,z);
					inside=!inside;
				}
				data+=layer;
			}
			data-=cube;
			data++;
			if(inside)
				Mesh_internal_face_Z(mesh, x,y,volume->z);
		}
	}

}

// Wavefront Obj
// ASCII
// Vertex data: v [x] [y] [z] \n
// Face data  : f [v1 index]  [v2 index][v3 index]\n | 1 indexed

bool Mesh_save_obj(Mesh mesh, const char *path)
{
	FILE *file = fopen(path, "w");

	if(file){

		// Write Vertex data 
		for(struct Vertex *v = List_start(mesh->vertecies),
				*e = List_end(mesh->vertecies);
				v != e; 	v++)
			fprintf(file, "v %.3f %.3f %.3f\n", v->x, v->y, v->z);

		// Write Face data 1 indexed
		for(struct Face *f = List_start(mesh->faces),
				*e = List_end(mesh->faces);
				f != e; 	f++)
			fprintf(file, "f %ld %ld %ld %ld\n", f->vertex_index[0]+1
																		 , f->vertex_index[1]+1
																		 , f->vertex_index[2]+1
																		 , f->vertex_index[3]+1
																		 );
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

bool Mesh_save_stl(Mesh mesh, const char *path)
{
	FILE *file = fopen(path, "wb");
	if(file){

		// Write empty header
		uint8_t header[80] = "DoubleFaceText";
		fwrite(header, 1, 80, file);

		uint32_t num_of_trig = 2*List_size(mesh->faces);
		fwrite(&num_of_trig, 4, 1, file);

		printf("Triangles: %d -> %d", num_of_trig, sizeof(struct Triangle_data));
		// Tessellate Quad with two Triangles
		struct Triangle_data trig1 = {0},
												 trig2 = {0};

		for(struct Face *f = List_start(mesh->faces),
				*e = List_end(mesh->faces);
				f != e; 	f++)
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
