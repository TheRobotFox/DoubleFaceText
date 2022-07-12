#include <stdbool.h>

struct Volume
{
	int x,y,z;
	bool *data;
};
typedef struct Volume* Volume;

bool* Volume_get(Volume v, int x, int y, int z);
Volume Volume_create(const char *img_A, const char *img_B, const char *img_C);
void Volume_free(Volume v);
