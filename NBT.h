#include <stdint.h>
#include "info/List/List.h"
typedef struct NBT_Tag* NBT;
typedef struct NBT_Payload* NBT_Data;

enum NBT_TYPE
{
	NBT_END, 		// 0	0
	NBT_BYTE, 		// 1	1
	NBT_SHORT,      // 2	2
	NBT_INT,        // 3	3
	NBT_LONG,       // 4	4
	NBT_FLOAT,      // 5	5
	NBT_DOUBLE,     // 6	6
	NBT_ARRAY_BYTE, // 7	7
	NBT_STRING,     // 8	8
	NBT_LIST,       // 9	9
	NBT_COMPOUND,   // 10	A
	NBT_ARRAY_INT,  // 11	B
	NBT_ARRAY_LONG  // 12	C
};

NBT NBT_create();
bool NBT_from_file(NBT nbt, const char *path);
bool NBT_to_file(NBT nbt, const char *path);

enum NBT_TYPE NBT_type_get(NBT nbt);
void NBT_type_set(NBT nbt, enum NBT_TYPE v);

const char* NBT_type_name(enum NBT_TYPE type);

NBT_Data NBT_data(NBT nbt);

int8_t NBT_byte_get(NBT_Data nbt);
int16_t NBT_short_get(NBT_Data nbt);
int32_t NBT_integer_get(NBT_Data nbt);
int64_t NBT_long_get(NBT_Data nbt);
float NBT_float_get(NBT_Data nbt);
double NBT_double_get(NBT_Data nbt);
char* NBT_string_get(NBT_Data nbt);


void NBT_byte_set(NBT_Data nbt, int8_t v);
void NBT_short_set(NBT_Data nbt, int16_t v);
void NBT_integer_set(NBT_Data nbt, int32_t v);
void NBT_long_set(NBT_Data nbt, int64_t v);
void NBT_float_set(NBT_Data nbt, float v);
void NBT_double_set(NBT_Data nbt, double v);
void NBT_string_set(NBT_Data nbt, char *v);

List NBT_array_data(NBT_Data nbt); // check if present

size_t NBT_list_length(NBT_Data nbt);

enum NBT_TYPE NBT_list_type_get(NBT_Data nbt);
void NBT_list_type_set(NBT_Data nbt, enum NBT_TYPE v);

NBT_Data NBT_list_get(NBT_Data nbt, size_t index);
void NBT_list_set(NBT_Data nbt, size_t index, NBT_Data v);

size_t NBT_compound_size(NBT_Data nbt);

NBT NBT_compound_get_name(NBT_Data nbt, char *name);
NBT NBT_compound_get_index(NBT_Data nbt, size_t index);

void NBT_compound_set_name(NBT_Data nbt, char *name, NBT v);
void NBT_compound_set_index(NBT_Data nbt, size_t index, NBT v);

void NBT_free(NBT nbt);
