#pragma once
#include "img.h"
#include "NBT.h"
#include <stdlib.h>


struct Vertex
{
	float x,y,z;
};

typedef	struct Vertex Trig[3];

typedef struct Volume* Volume;

Volume Volume_create();

bool Volume_from_shadow_3(Volume vol, Image front, Image side, Image top);
bool Volume_from_shadow_2(Volume vol, Image front, Image side);

struct Blocks_transparrent
{
	char **names;
	size_t size;
};

bool Volume_from_NBT(Volume vol, NBT nbt, struct Blocks_transparrent *blocks_transparrent);

bool Volume_to_shadow(Volume vol, Image *front, Image *side, Image *top);

bool Volume_to_NBT(Volume vol, NBT nbt, char *material_name);

bool Volume_from_mesh(Volume vol, Trig *mesh, size_t length, size_t res[3]);

void Volume_free(Volume v);
