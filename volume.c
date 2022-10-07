#include "volume.h"
#include "info/info.h"
#include "volume_internal.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define THRESHOLD 128

bool* Volume_get(Volume v, int x, int y, int z) { return v->data[x][y]+z; }

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

	INFO("Voxel Volume dimensions: X %d Y %d Z %d\n", width, height, length)

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
	"minecraft:torch"
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

bool Volume_from_nbt(Volume vol, NBT nbt, struct Blocks_transparrent *blocks_transparrent)
{
	bool ret = true;

	NBT_Data size =   NBT_data_get(NBT_compound_get_name(NBT_data_get(nbt), "size"));
	NBT_Data blocks = NBT_data_get(NBT_compound_get_name(NBT_data_get(nbt), "blocks"));
	NBT_Data palette = NBT_data_get(NBT_compound_get_name(NBT_data_get(nbt), "palette"));

	if(blocks && size && palette){
		if(NBT_list_length(size)==3 && NBT_list_type_get(size)==NBT_INT){

			vol->x = NBT_integer_get(NBT_list_get(size,0));
			vol->y = NBT_integer_get(NBT_list_get(size,1));
			vol->z = NBT_integer_get(NBT_list_get(size,2));
			INFO("allocate Voxel Volume x: %d y: %d z: %d", vol->x, vol->y, vol->z);

			Volume_allocate(vol, vol->x, vol->y, vol->z);


			INFO("Creating transparrency Mask")

			if(!blocks_transparrent)
				blocks_transparrent = &blocks_transparrent_default;

			size_t palette_count = NBT_list_length(palette);
			bool *mask = malloc(palette_count);

			char *name;
			for(int i=0; i<palette_count; i++)
			{
				name = NBT_string_get(NBT_data_get(NBT_compound_get_index(NBT_list_get(palette,i),0)));
				if(name)
					mask[i] =!util_contains_string(blocks_transparrent, name);
				else
					goto cleanup;
			}

			INFO("Writing Voxels")

			size_t block_count = NBT_list_length(blocks),
						 palette_index;
			NBT_Data block, pos;
			for(size_t i=0; i<block_count; i++)
			{
				block = NBT_list_get(blocks, i);
				pos = NBT_data_get(NBT_compound_get_index(block, 0));

				if(NBT_list_length(pos)!=3)
					goto cleanup;

				palette_index = NBT_integer_get(NBT_data_get(NBT_compound_get_index(block,1)));
				if(palette_index>palette_count)
					goto cleanup;

				*Volume_get(vol, NBT_integer_get(NBT_list_get(pos,0)),
								 NBT_integer_get(NBT_list_get(pos,1)),
								 NBT_integer_get(NBT_list_get(pos,2)))=mask[palette_index];
			}
			ret = false;
			free(mask);
			SUCCESS("Volume generation!")
			goto end;

		cleanup:
			free(mask);
			goto error;
		}
	}
error:
	ERROR("NBT does not contain valid structure!")
end:
	return ret;

}

bool Volume_to_shadow(Volume vol, Image *front, Image *side, Image *top)
{
	*front = Image_create();
	Image_from_color(*front, vol->x, vol->y, 255);
	*side  = Image_create();
	Image_from_color(*side, vol->z, vol->y, 255);
	*top   = Image_create();
	Image_from_color(*top, vol->x, vol->z, 255);

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


void Volume_free(Volume v)
{
	Volume_internal_free(v);
	free(v);
}
