#include "img.h"
#include <stdlib.h>

typedef struct Volume* Volume;

Volume Volume_create();

bool Volume_from_shadow_3(Volume vol, Image front, Image side, Image top);
bool Volume_from_shadow_2(Volume vol, Image front, Image side);

struct Blocks_transparrent
{
	char **names;
	size_t size;
};

bool Volume_from_nbt(Volume vol, const char *path, struct Blocks_transparrent *blocks_transparrent);

bool Volume_to_shadow(Volume vol, Image *front, Image *side, Image *top);

bool* Volume_get(Volume v, int x, int y, int z);

bool Volume_to_nbt(Volume vol, const char *path);

void Volume_free(Volume v);
