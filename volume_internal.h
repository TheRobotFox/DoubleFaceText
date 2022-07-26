
struct Volume
{
	int x,y,z;
	bool ***data;
};

bool* Volume_get(Volume v, int x, int y, int z);

