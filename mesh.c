#include "info/info.h"
#include "mesh.h"
#include "info/List/List.h"
#include "volume_internal.h"
#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <float.h>
#include <string.h>
#include <errno.h>

IMPLEMENT_LIST(Vertex)

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
	Mesh mesh = (Mesh)malloc(sizeof(struct Mesh));
	mesh->vertecies=List_create(sizeof(struct Vertex));
	mesh->faces=List_create(sizeof(struct Face));

	return mesh;
}

static bool _comp_vertex(Vertex *a, Vertex *b)
{
	return !memcmp(a,b,sizeof(Vertex));
}
size_t List_at_or_make(List l, void* object)
{
	Vertex *found = LIST(Vertex, find)(l,_comp_vertex, object);
	if(found)
		return (found-(Vertex*)List_start(l));
	List_push(l,object);
	return List_size(l)-1;
}

void Mesh_face_add(Mesh mesh, struct Vertex a,
							  struct Vertex b,
							  struct Vertex c,
							  struct Vertex d
				  )
{
	struct Face face;
	face.vertex_index[0]=List_at_or_make(mesh->vertecies,&a);
	face.vertex_index[1]=List_at_or_make(mesh->vertecies,&b);
	face.vertex_index[2]=List_at_or_make(mesh->vertecies,&c);
	face.vertex_index[3]=List_at_or_make(mesh->vertecies,&d);

	List_push(mesh->faces, &face);

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

static Vector Vector_add(Vector a, Vector b)
{
	return (Vector){a.x+b.x, a.y+b.y, a.z+b.z};
}

static Vector Vector_sub(Vector a, Vector b)
{
	return (Vector){a.x-b.x, a.y-b.y, a.z-b.z};
}

static Vector Vector_cross(Vector a, Vector b)
{
	return (Vector){a.y*b.z - b.y*a.z, a.z*b.x - b.z*a.x, a.x*b.y - b.x*a.y};
}
static Vector Vector_scale(Vector a, float s)
{
	return (Vector){a.x*s, a.y*s, a.z*s};
}

static float Vector_dot(Vector a, Vector b)
{
	return a.x*b.x+a.y*b.y+a.z*b.z;
}
static float Vector_length(Vector v)
{
	return sqrt(v.x*v.x+v.y*v.y+v.z*v.z);
}

/* static float calulate_z(float x, float y, Trig t) */
/* { */
/* 	Vector gs = {x,y,0}; */
/* 	Vector s = t[0]; */

/* 	Vector a = Vector_sub(t[1], s); */
/* 	Vector b = Vector_sub(t[2], s); */
/* 	Vector n = Vector_cross(a,b); */

/* 	// colinear */
/* 	if(fabsf(n.z)<0.001f) */
/* 		// maybe check for identicallity */
/* 		return NAN; */

/* 	 return (Vector_dot(s, n)-Vector_dot(gs,n))/n.z; */
/* } */

bool Vector_equals(Vector a, Vector b)
{
	return a.x == b.x && a.y == b.y && a.z==b.z;
}

float Vector_angle(Vector a, Vector b)
{
	return (Vector_dot(a,b))/Vector_length(a)/Vector_length(b);
}

// from blackpawn

static bool Line_same_side(Vector a, Vector b, Vector p, Vector r)
{
	Vector line = Vector_sub(a,b);
	return Vector_dot(Vector_cross(line, Vector_sub(p,a)),Vector_cross(line, Vector_sub(r, a)))>=0;
}
static bool inside_trig(Vector p, Trig t)
{
	return Line_same_side(t[0], t[1], p, t[2]) &&
		   Line_same_side(t[1], t[2], p, t[0]) &&
		   Line_same_side(t[0], t[2], p, t[1]);
}

static float Trig_intersect(Trig t, Vector pos, Vector norm_direction)
{
	Vector a = Vector_sub(t[1], t[0]);
	Vector b = Vector_sub(t[2], t[0]);

	Vector n = Vector_cross(a,b);

	float direction_dot = Vector_dot(norm_direction, n);
	if(fabsf(direction_dot)<0.001f)
		return NAN;

	float dist = (Vector_dot(Vector_sub(t[0], pos), n))/direction_dot;
	Vector plane_hit = Vector_add(pos, Vector_scale(norm_direction, dist));

	return inside_trig(plane_hit, t) ? dist : NAN;
}

List Mesh_intersects(List trigs, Vector pos, Vector direction)
{
	List intersections = List_create(sizeof(float));
	Vector direction_n = Vector_scale(direction, Vector_length(direction));

	for(Trig *start = List_start(trigs),
			 *end   = List_end(trigs); start!=end; start++)
	{
		float a = Trig_intersect(*start, pos, direction_n);
		if(!isnan(a))
			List_push(intersections, &a);
	}
	return intersections;
}


struct Dimensions Mesh_dimensions(List trigs)
{
	union coord min={{FLT_MAX,FLT_MAX,FLT_MAX}};
	union coord max={{-FLT_MAX,-FLT_MAX,-FLT_MAX}};

	for(Trig *start=List_start(trigs),
			 *end = List_end(trigs); start!=end; start++)
	{
		for(size_t p=0; p<3; p++){
			union coord v = {(*start)[p]};
			for(size_t d=0; d<3; d++)
			{
				if(v.a[d]<min.a[d])
					min.a[d]=v.a[d];

				if(v.a[d]>max.a[d])
					max.a[d]=v.a[d];
			}
		}
	}
	return (struct Dimensions){min, max};
}

float Coord_remap(float a, struct Dimensions size, size_t d)
{
	return (a-size.min.a[d])/(size.max.a[d]-size.min.a[d]);
}

void print_vex(void *a)
{
	Trig *t=a;
	PRINT("[%f,%f,%f]", *t[0], *t[1], *t[2])
}
void print_length(List a)
{
	INFO("Chunk has %d Items", List_size(a));
	//List_foreach(a, print_vex);
	puts("");
}

List Mesh_to_chunks(List trigs, size_t res[3])
{

	struct Dimensions mesh_dimensions = Mesh_dimensions(trigs),
					  tmp;

	List chunks = List_create(sizeof(List));
	List_resize(chunks, res[0]*res[1]*res[2]);
	for(size_t i=0; i<res[0]*res[1]*res[2]; i++)
		List_GET_REF(List, chunks, i)=List_create(sizeof(Trig));

	for(Trig *trig_start=List_start(trigs),
			 *trig_end = List_end(trigs);
			trig_start!=trig_end; trig_start++)
	{
		// get Triangle dimensions (X and Y)
		tmp.min = mesh_dimensions.max;
		tmp.max = mesh_dimensions.min;

		for(size_t p=0; p<3; p++){
			union coord v = {(*trig_start)[p]};

			for(size_t d=0; d<3; d++)
			{
				if(v.a[d]<tmp.min.a[d])
					tmp.min.a[d]=v.a[d];

				if(v.a[d]>tmp.max.a[d])
					tmp.max.a[d]=v.a[d];
			}
		}
		// Calculate chunk span
		size_t start[3];
		size_t end[3];
		for(size_t d=0; d<3; d++)
		{
			start[d] = (Coord_remap(tmp.min.a[d], mesh_dimensions, d)-.1)*(res[d]-1);
			end[d] = (Coord_remap(tmp.max.a[d], mesh_dimensions, d)+.1)*(res[d]-1);
			/* printf("%d: %d %d\n", d, start[d], end[d]); */
		}

		// write triangle to chunks
		for(size_t z=start[2]; z<=end[2]; z++)
		{
			for(size_t y=start[1]; y<=end[1]; y++)
			{
				for(size_t x=start[0]; x<=end[0]; x++)
				{
					List *c = List_at(chunks, z*res[1]*res[0]+y*res[0]+x);
					List_push(*c, trig_start);
				}
			}
		}
	}
	LIST(List, forward)(chunks, print_length);
	return chunks;
}

void Chunks_free(List chunks)
{
	LIST(List, forward)(chunks, List_free);
	List_free(chunks);
}


bool Mesh_to_slices(List trigs, size_t res[3])
{
	//INFO("Creating Chunks")
	//List chunks = Mesh_to_chunks(trigs, (size_t[3]){10, 10, 1});
	struct Dimensions size = Mesh_dimensions(trigs);
	INFO("Mesh Dimensions %f <= x <= %f\n%f <= y <= %f\n%f <= z <= %f",
							size.min.v.x, size.max.v.x,
							size.min.v.y, size.max.v.y,
							size.min.v.z, size.max.v.z)

	char path[] = "Sec_.png";

	INFO("Drawing Slices")
	for(size_t slice=0; slice<res[2]; slice++)
	{
		Image img = Image_create();
		Image_from_color(img, res[0], res[1], 0);

		for(float x=size.min.a[0]; x<size.max.a[0]; x+=(size.max.a[0]-size.min.a[0])/res[0])
		{
			float xx = Coord_remap(x, size, 0);

			Vector pos = {x,size.min.v.y+slice*(size.max.v.y-size.min.v.y)/(res[2]-1),0};
			List hits = Mesh_intersects(trigs, pos, (Vector){0,0,1}); //List_GET_REF(List, chunks, (float)slice*10.0f/res[2]+xx*10));
			for(size_t i=0; i<List_size(hits); i++)
			{
				float norm_distance = Coord_remap(List_GET_REF(float, hits, i), size, 2);
				if(norm_distance>=1.0f)
					norm_distance=1.0f;
				if(norm_distance<0)
					norm_distance=0;

				size_t zz = norm_distance*(res[1]-1);
				*Image_get(img, xx*res[0], zz) = 255;
			}
			List_free(hits);
		}
		path[3]='a'+slice;
		Image_save(img, path);
		Image_free(img);
	}

	//Chunks_free(chunks);
	return true;
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
	if(fread(&count, sizeof(count), 1, f)!=1) ERROR("fread failed!")

	List l = List_create(sizeof(Trig));
	List_reserve(l, count);

	struct Triangle_data data;
	for(int i=0; i<count; i++)
	{
	if(fread(&data, sizeof(data), 1, f)!=1) ERROR("fread failed!")
		List_push(l, (Trig*)&data.vertecies);
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
			trig1.vertecies[0]=List_GET_REF(struct Vertex, mesh->vertecies,f->vertex_index[0]);
			trig1.vertecies[1]=List_GET_REF(struct Vertex, mesh->vertecies,f->vertex_index[1]);
			trig1.vertecies[2]=List_GET_REF(struct Vertex, mesh->vertecies,f->vertex_index[2]);
			fwrite(&trig1,50, 1, file);

			trig2.vertecies[0]=List_GET_REF(struct Vertex, mesh->vertecies,f->vertex_index[2]);
			trig2.vertecies[1]=List_GET_REF(struct Vertex, mesh->vertecies,f->vertex_index[3]);
			trig2.vertecies[2]=List_GET_REF(struct Vertex, mesh->vertecies,f->vertex_index[0]);
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
