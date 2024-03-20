
struct Volume
{
	int x,y,z;
	bool ***data;
};

bool* Volume_get(Volume v, size_t x, size_t y, size_t z);

