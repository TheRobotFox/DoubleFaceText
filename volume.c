#include "mesh.h"
#include "info/info.h"
#include "volume_internal.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>
#include <signal.h>

#define THRESHOLD 128

bool* Volume_get(Volume v, size_t x, size_t y, size_t z) { return v->data[x][y]+z; }

Volume Volume_create()
{

	Volume vol = malloc(sizeof(struct Volume));
	memset(vol,0,sizeof(struct Volume));

	return vol;
}

void Volume_internal_free(Volume vol)
{

	for(int x=0; x<vol->x; x++)
	{
		for(int y=0; y<vol->y; y++)
		{
			free(vol->data[x][y]);
		}
		free(vol->data[x]);
	}
	free(vol->data);
}

bool Volume_allocate(Volume vol, int width, int height, int length)
{
	if(vol->data)
		Volume_internal_free(vol);

	vol->x = width;
	vol->y = height;
	vol->z = length;

	vol->data=malloc(width*sizeof(*vol->data));
	for(int x=0; x<width; x++)
	{
		vol->data[x]=malloc(height*sizeof(**vol->data));
		for(int y=0; y<height; y++)
		{
			vol->data[x][y]=malloc(length*sizeof(***vol->data));
			memset(vol->data[x][y],0,length);
		}
	}
	return false;
}


bool Volume_from_shadow_3(Volume vol, Image front, Image side, Image top)
{

	if( Image_get_x(front)!=Image_get_x(top) ||
			Image_get_x(side)!=Image_get_y(top) ){
		ERROR("Top shadowmap doesn't align with sides")
		return true;
	}

	if(Volume_from_shadow_2(vol, front, side))
		return true;

	bool *voxel;
	for(int z=0; z<vol->z; z++)
		for(int y=0; y<vol->y; y++)
			for(int x=0; x<vol->x; x++){
				voxel=Volume_get(vol,x,y,z);
				*voxel = *voxel && (*Image_get(top, x, z)<THRESHOLD);
			}
	return false;
};

bool Volume_from_shadow_2(Volume vol, Image front, Image side)
{
	if(Image_get_y(front)!=Image_get_y(side)){
		ERROR("Side shadowmaps missalign in height! %d, %d", Image_get_y(front), Image_get_y(side))
		return true;
	}

	int width=Image_get_x(front),
			height=Image_get_y(front),
			length=Image_get_x(side);

	INFO("Voxel Volume dimensions: X %d Y %d Z %d", width, height, length)

	Volume_allocate(vol, width, height, length);

	for(int z=0; z<vol->z; z++)
		for(int y=0; y<vol->y; y++)
			for(int x=0; x<vol->x; x++)
				*Volume_get(vol,x,y,z)= (
						(*Image_get(front, x, height-y-1)<THRESHOLD) &&
						(*Image_get(side, z, height-y-1)<THRESHOLD)
						);
	return false;
}

char *blocks_transparrent_names[] = {
	"minecraft:air",
	"minecraft:torch",
	"minecraft:redstone"
};

struct Blocks_transparrent blocks_transparrent_default = { blocks_transparrent_names, sizeof(blocks_transparrent_names)/sizeof(*blocks_transparrent_names)};

bool util_contains_string(struct Blocks_transparrent *container, char *str)
{
	for(size_t i=0; i<container->size; i++)
	{
		if(!strcmp(container->names[i], str))
			return 1;
	}
	return 0;
}

int max(int a, int b)
{
	if(a>b)
		return a;
	return b;
}

int min(int a, int b)
{
	if(a<b)
		return a;
	return b;
}
bool Volume_from_NBT(Volume vol, NBT nbt, struct Blocks_transparrent *blocks_transparrent)
{
	bool ret = true;

	NBT_Data size =   NBT_data(NBT_compound_get_name(NBT_data(nbt), "size"));
	NBT_Data blocks = NBT_data(NBT_compound_get_name(NBT_data(nbt), "blocks"));
	NBT_Data palette = NBT_data(NBT_compound_get_name(NBT_data(nbt), "palette"));

	if(!blocks) {
		ERROR("Could not load 'blocks'")
			return true;
	}
	if(!palette) {
		ERROR("Could not load 'palette'")
			return true;
	}
	if(!size) {
		ERROR("Could not load 'size'")
			return true;
	}
/*
	if(NBT_list_length(size)!=3 || NBT_list_type_get(size)!=NBT_INT){
		ERROR("'size' Tag malformed")
		return true;
	}

	vol->x = NBT_integer_get(NBT_list_get(size,0));
	vol->y = NBT_integer_get(NBT_list_get(size,1));
	vol->z = NBT_integer_get(NBT_list_get(size,2));
*/


	INFO("Creating transparrency Mask")

	if(!blocks_transparrent)
		blocks_transparrent = &blocks_transparrent_default;

	size_t palette_count = NBT_list_length(palette);
	bool *mask = malloc(palette_count);

	char *name;
	for(int i=0; i<palette_count; i++)
	{
		name = NBT_string_get(NBT_data(NBT_compound_get_index(NBT_list_get(palette,i),0)));
		if(name)
			mask[i] =!util_contains_string(blocks_transparrent, name);
		else
			goto cleanup2;
	}

	NBT_Data block, pos;

	size_t block_count = NBT_list_length(blocks),
		   palette_index,
		   i;

	INFO("Found %d blocks", block_count)

	//calulate actual size of structure

	int x_min=0,
	    y_min=0,
	    z_min=0,
	    x_max=0,
        y_max=0,
        z_max=0,
		x,y,z;

	for(i=0; i < block_count; i++)
	{
		block = NBT_list_get(blocks, i);
		pos = NBT_data(NBT_compound_get_name(block, "pos"));

		if(!pos){
			ERROR("Could not find block 'pos'")
			goto cleanup;
		}
		if(NBT_list_length(pos)!=3)
			goto cleanup;

		palette_index = NBT_integer_get(NBT_data(NBT_compound_get_name(block,"state")));
		if(palette_index>palette_count)
			goto cleanup;

		x=NBT_integer_get(NBT_list_get(pos,0));
		y=NBT_integer_get(NBT_list_get(pos,1));
		z=NBT_integer_get(NBT_list_get(pos,2));

		x_min = min(x_min, x);
		y_min = min(y_min, y);
		z_min = min(z_min, z);

		x_max = max(x_max, x);
		y_max = max(y_max, y);
		z_max = max(z_max, z);
	}

	vol->x=x_max-x_min+1;
	vol->y=y_max-y_min+1;
	vol->z=z_max-z_min+1;

	INFO("allocate Voxel Volume x: %d y: %d z: %d", vol->x, vol->y, vol->z);

	Volume_allocate(vol, vol->x, vol->y, vol->z);


	INFO("Writing Voxels")

	for(i=0; i<block_count; i++)
	{
		block = NBT_list_get(blocks, i);
		pos = NBT_data(NBT_compound_get_name(block, "pos"));

		if(!pos){
			ERROR("Could not find block 'pos'")
			goto cleanup;
		}
		if(NBT_list_length(pos)!=3)
			goto cleanup;

		palette_index = NBT_integer_get(NBT_data(NBT_compound_get_name(block,"state")));
		if(palette_index>palette_count)
			goto cleanup;

		x=NBT_integer_get(NBT_list_get(pos,0))-x_min;
		y=NBT_integer_get(NBT_list_get(pos,1))-y_min;
		z=NBT_integer_get(NBT_list_get(pos,2))-z_min;

		*Volume_get(vol, x, y, z)=mask[palette_index];
	}
	ret = false;
	free(mask);
	SUCCESS("Volume generation!")
	return false;

cleanup:
	ERROR("block at index '%ld' is malformed", i);
cleanup2:
	free(mask);
	return true;

}

bool Volume_to_NBT(Volume vol, NBT nbt, char *material_name)
{
	NBT tmp = NBT_create();
	// Compound
	NBT_type_set(nbt, NBT_COMPOUND);

	// 		DataVersion
	INFO("creating 'DataVersion'")
	NBT data_version = NBT_create();
	{
		NBT_type_set(data_version, NBT_INT);
		NBT_integer_set(NBT_data(data_version), 3105);
	}
	// 		size
	INFO("creating 'size' -> %d | %d | %d", vol->x, vol->y, vol->z)
	NBT size = NBT_create();
	{
		NBT_type_set(size, NBT_LIST);
		NBT_list_type_set(NBT_data(size), NBT_INT);

		NBT_integer_set(NBT_data(tmp), vol->x);
		NBT_list_set(NBT_data(size), 0, NBT_data(tmp));
		NBT_integer_set(NBT_data(tmp), vol->y);
		NBT_list_set(NBT_data(size), 1, NBT_data(tmp));
		NBT_integer_set(NBT_data(tmp), vol->z);
		NBT_list_set(NBT_data(size), 2, NBT_data(tmp));
	}
	// 		blocks
	INFO("writing block data");
	NBT blocks = NBT_create();
	{
		NBT_type_set(blocks, NBT_LIST);
		NBT_list_type_set(NBT_data(blocks), NBT_COMPOUND);

		size_t blocks_index=0;

		for(int z=0; z<vol->z; z++)
		{
			for(int y=0; y<vol->y; y++)
			{
				for(int x=0; x<vol->x; x++)
				{
					if(*Volume_get(vol, x, y, z)){

						NBT block = NBT_create();
						NBT_type_set(block, NBT_COMPOUND);

	// 			pos
						NBT pos = NBT_create();
						NBT_type_set(pos, NBT_LIST);
						NBT_list_type_set(NBT_data(pos), NBT_INT);

						NBT_integer_set(NBT_data(tmp), x);
						NBT_list_set(NBT_data(pos), 0, NBT_data(tmp));
						NBT_integer_set(NBT_data(tmp), y);
						NBT_list_set(NBT_data(pos), 1, NBT_data(tmp));
						NBT_integer_set(NBT_data(tmp), z);
						NBT_list_set(NBT_data(pos), 2, NBT_data(tmp));
						NBT_compound_set_name(NBT_data(block), "pos", pos);

	// 			palette
						NBT palette = NBT_create();
						NBT_type_set(palette, NBT_INT);
						NBT_integer_set(NBT_data(palette), 0);

						NBT_compound_set_name(NBT_data(block), "state", palette);

						//INFO("appending block")
						NBT_list_set(NBT_data(blocks), blocks_index++, NBT_data(block));

						free(block);
						free(palette);
						free(pos);
					}

				}
			}
		}
		INFO("Wrote %d blocks", NBT_list_length(NBT_data(blocks)))
	}
	// 		palette
	INFO("creating 'palette' using '%s'", material_name)
	NBT palette = NBT_create();
	{
		NBT_type_set(palette, NBT_LIST);
		NBT_list_type_set(NBT_data(palette), NBT_COMPOUND);

		NBT material = NBT_create();
		NBT_type_set(material, NBT_COMPOUND);

		NBT material_tag = NBT_create();
		NBT_type_set(material_tag, NBT_STRING);
		NBT_string_set(NBT_data(material_tag), material_name);

		NBT_compound_set_name(NBT_data(material), "Name", material_tag);

		NBT_list_set(NBT_data(palette), 0, NBT_data(material));
		free(material);
		free(material_tag);
	}

	INFO("Filling NBT")
	NBT_compound_set_name(NBT_data(nbt), "DataVersion", data_version);
	NBT_compound_set_name(NBT_data(nbt), "size", size);
	NBT_compound_set_name(NBT_data(nbt), "blocks", blocks);
	NBT_compound_set_name(NBT_data(nbt), "palette", palette);

	// cleanup
	free(tmp);
	free(data_version);
	free(size);
	free(blocks);
	free(palette);

	return false;
}
bool Volume_to_shadow(Volume vol, Image *front, Image *side, Image *top)
{
	*front = Image_create();
	Image_from_color(*front, vol->x, vol->y, 255);
	*side  = Image_create();
	Image_from_color(*side, vol->z, vol->y, 255);
	*top   = Image_create();
	Image_from_color(*top, vol->x, vol->z, 255);

	INFO("Voxel resolution: %dx%dx%d", vol->x, vol->y, vol->z)
	for(int y=0; y<vol->y; y++)
	{
		for(int x=0; x<vol->x; x++)
		{
			for(int z=0; z<vol->z; z++)
			{
				if(*Volume_get(vol,x,y,z)){
					*Image_get(*front,x,y)=0;
					*Image_get(*side,z,y)=0;
					*Image_get(*top,x,z)=0;
				}
			}
		}
	}
	return false;
}


typedef struct Vertex Vector;

static Vector Vector_sub(Vector a, Vector b)
{
	return (Vector){a.x-b.x, a.y-b.y, a.z-b.z};
}

static Vector Vector_cross(Vector a, Vector b)
{
	return (Vector){a.y*b.z - b.y*a.z, a.z*b.x - b.z*a.x, a.x*b.y - b.x*a.y};
}
float Vector_dot(Vector a, Vector b)
{
	return a.x*b.x+a.y*b.y+a.z*b.z;
}
float Vector_length(Vector v)
{
	return sqrt(v.x*v.x+v.y*v.y+v.z*v.z);
}

static float collision_distance(float x, float y, Trig t)
{
	Vector gs = {x,y,0};
	Vector s = t[0];

	Vector a = Vector_sub(t[1], s);
	Vector b = Vector_sub(t[2], s);
	Vector n = Vector_cross(a,b);

	// colinear
	if(fabsf(n.z)<0.02)
		// maybe check for identicallity
		return NAN;

	 return -(Vector_dot(s, n)-Vector_dot(gs,n))/n.z;
}

float Vector_angle(Vector a, Vector b)
{
	return (Vector_dot(a,b))/Vector_length(a)/Vector_length(b);
}

static bool inside_trig(Vector p, Trig t)
{
	float a = asinf(Vector_angle(Vector_sub(t[1],t[0]), Vector_sub(p,t[0])));
	float b = asinf(Vector_angle(Vector_sub(t[1],t[0]), Vector_sub(t[2], t[0])));

	if(b/a>1 || b/a <0)
		return false;


	a = asinf(Vector_angle(Vector_sub(t[0],t[2]), Vector_sub(p,t[2])));
	b = asinf(Vector_angle(Vector_sub(t[0],t[2]), Vector_sub(t[1], t[2])));

	if(b/a>1 || b/a <0)
		return false;

	a = Vector_angle(Vector_sub(t[2],t[1]), Vector_sub(p,t[1]));
	b = Vector_angle(Vector_sub(t[2],t[1]), Vector_sub(t[0], t[1]));

	if(b/a>1 || b/a <0)
		return false;

	return true;
}

bool cmp(void *a, void *b) { return *(int*)a<*(int*)b;}
void print(void *a) { printf("%d, ", *(size_t*)a);}

bool Volume_from_mesh(Volume vol, Trig *mesh, size_t length, size_t res[3])
{
	Volume_allocate(vol, res[0], res[1], res[2]);

	union coord{
		struct Vertex v;
		float a[3];
	};

	INFO("Calculating Mesh dimensions!")
	union coord min={{FLT_MAX,FLT_MAX,FLT_MAX}};
	union coord max={{-FLT_MAX,-FLT_MAX,-FLT_MAX}};

	for(size_t i=0; i < length; i++)
	{
		for(size_t p=0; p<3; p++){
			union coord v = {mesh[i][p]};
			for(size_t j=0; j<3; j++)
			{
				if(v.a[j]<min.a[j])
					min.a[j]=v.a[j];

				if(v.a[j]>max.a[j])
					max.a[j]=v.a[j];
			}
		}
	}
	INFO("Mesh dimensions:\n%f <= x <= %f\n%f <= y <= %f\n%f <= z <= %f", min.v.x, max.v.x, min.v.y, max.v.y, min.v.z, max.v.z)
	INFO("Voxel resolution: %dx%dx%d", vol->x, vol->y, vol->z)

	INFO("write chunks!")
	List *chunks = malloc(sizeof(List)*res[0]*res[1]);

	for(size_t i=0; i<res[0]*res[1]; i++)
		chunks[i]=List_create(sizeof(size_t));

	for(size_t i=0; i<length; i++)
	{
		// get Triangle dimensions (X and Y)
		union coord trig_max={{-FLT_MAX,-FLT_MAX,-FLT_MAX}};
		union coord trig_min={{FLT_MAX,FLT_MAX,FLT_MAX}};;

		for(size_t p=0; p<3; p++){
			union coord v = {mesh[i][p]};

			for(size_t j=0; j<2; j++)
			{
				if(v.a[p]<trig_min.a[j])
					trig_min.a[j]=v.a[j];

				if(v.a[p]>trig_max.a[j])
					trig_max.a[j]=v.a[j];
			}
		}
		// Calculate chunk span
		size_t start[2];
		size_t end[2];
		for(size_t d=0; d<2; d++)
		{
			start[d] = (trig_min.a[d]-min.a[d])/(max.a[d]-min.a[d])*(res[d]);
			end[d] = (trig_max.a[d]-min.a[d])/(max.a[d]-min.a[d])*(res[d]);
		}
		//printf("V: %f,%f,%f %f,%f,%f %f,%f,%f\nx: %d,%d y: %d,%d\n", mesh[i][0].x, mesh[i][0].y, mesh[i][0].z, mesh[i][1].x, mesh[i][1].y, mesh[i][1].z, mesh[i][2].x, mesh[i][2].y, mesh[i][2].z, start[0], end[0], start[1], end[1]);
		// write triangle to chunks
		for(size_t y=start[1]; y<end[1]; y++)
		{
			for(size_t x=start[0]; x<end[0]; x++)
			{
				List c = chunks[y*res[0]+x];
				List_append(c, &i);
			}
		}
	}

	INFO("Calculate Collisions")
	for(size_t y=0; y<res[1]; y++)
	{
		for(size_t x=0; x<res[0]; x++)
		{
			// get current chunk
			float fx = (float)x/res[0]*max.v.x+min.v.x;
			float fy = (float)y/res[1]*max.v.y+min.v.y;

			List chunk = chunks[y*res[0]+x];
			List collisions = List_create(sizeof(size_t));

			for(size_t *t=List_start(chunk); t<(size_t*)List_end(chunk); t++)
			{
				// Calculate collision
				float fz = collision_distance(fx,fy, mesh[*t]);
				if(!isnan(fz)){
					if(inside_trig((Vector){fx,fy,fz}, mesh[*t])){
						//printf("Collision at %f\n", fz);
						// rescale z
						size_t z = (fz-min.v.z)/(max.v.z-min.v.z)*(res[2]);
						List_append(collisions, &z);
					}
				}
			}

			// sort collisions
			List_sort(collisions, cmp);
			//List_foreach(collisions, print);
			//printf("\n");

			// write collisions
			size_t coll_index=0;
			for(size_t z=0; z<res[2] && coll_index<List_size(collisions); z++)
			{
				if(*(size_t*)List_get(collisions, coll_index)==z)
					coll_index++;

				*Volume_get(vol,x,y,z)=coll_index%2;
			}

			List_free(collisions);
		}
	}

	for(size_t i=0; i<res[0]*res[1]; i++)
		List_free(chunks[i]);
	free(chunks);

	SUCCESS("Voxelisation done!")
	return false;
}

void Volume_free(Volume v)
{
	Volume_internal_free(v);
	free(v);
}
