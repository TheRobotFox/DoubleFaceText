#include "NBT.h"
#include <stdio.h>
#include <string.h>
#include "zlib/zlib.h"
#include <stdlib.h>

struct NBT_List
{
	enum NBT_TYPE type;
	List data;
};

struct NBT_Payload
{
	union{
		int8_t b;
		int16_t s;
		int32_t i;
		int64_t l;
		float f;
		double d;
		char *str;
		struct NBT_List list;
		List array;
	};
};


struct NBT_Tag
{
	enum NBT_TYPE type;
	char *name;
	struct NBT_Payload data;
};

void util_endian_flip(uint8_t *start, uint8_t *end)
{
	uint8_t tmp;
	while(start<end)
	{
		end--;
		tmp = *start;
		*start = *end;
		*end = tmp;
		start++;
	}
}

// read [size] bytes from file and flip endianess
void NBT_internal_value_load(void *data, size_t size, gzFile f)
{
	gzread(f, data, size);
	util_endian_flip(data, (uint8_t*)(data)+size);
}


bool NBT_internal_String_load(char **str, gzFile f)
{
	uint16_t length;
	// load next bytes into length
	NBT_internal_value_load(&length, sizeof(uint16_t), f);

	if(length){
		*str = malloc(length+1);
		gzread(f, *str, length);
		(*str)[length]=0;
	}else{
		*str=NULL;
	}

	return 0;
}

bool NBT_internal_Array_load(List *arr, size_t element_size, gzFile f)
{
	uint16_t length;
	// load next bytes into length
	NBT_internal_value_load(&length, sizeof(uint16_t), f);

	*arr = List_create(element_size);
	List_reserve(*arr, length);

	// read characters into string buffer
	for(size_t i=0; i<length; i++)
		gzread(f, List_append(*arr, NULL), element_size);

	return 0;
}

size_t NBT_internal_sizeof(enum NBT_TYPE type)
{
	switch(type)
	{
		case NBT_BYTE:
			return sizeof(uint8_t);
		case NBT_SHORT:
			return sizeof(int16_t);
		case NBT_INT:
			return sizeof(int32_t);
		case NBT_LONG:
			return sizeof(int64_t);
		case NBT_FLOAT:
			return sizeof(float);
		case NBT_DOUBLE:
			return sizeof(double);

		case NBT_STRING:
			return sizeof(char*);
		case NBT_ARRAY_BYTE:
		case NBT_LIST:
		case NBT_COMPOUND:
		case NBT_ARRAY_INT:
		case NBT_ARRAY_LONG:
			return sizeof(List);
		default:
			exit(puts("[FATAL ERROR] Unknown Type: cannot get size!"));
	}
}

enum NBT_TYPE NBT_Tag_load(NBT head, gzFile f);

bool NBT_internal_Tag_body_load(struct NBT_Payload *data, enum NBT_TYPE type, gzFile f)
{
	bool ret=false;

	switch(type)
	{
		// load single values
		case NBT_BYTE:
			data->b = gzgetc(f);
			break;

		case NBT_SHORT:
		case NBT_INT:
		case NBT_LONG:
		case NBT_FLOAT:
		case NBT_DOUBLE:
			NBT_internal_value_load(&data->d, NBT_internal_sizeof(type), f);
			break;

		case NBT_ARRAY_BYTE:
			NBT_internal_Array_load(&data->array, sizeof(uint8_t), f);
			break;

		case NBT_STRING:
			NBT_internal_String_load(&data->str, f);
			break;

		case NBT_LIST:
			{
				// List header : [element_type<byte> length<int>]
				struct NBT_List *list = &data->list;
				int32_t length;

				list->type = gzgetc(f);
				NBT_internal_value_load(&length, sizeof(int32_t), f);

				// if list not empty load elements, else zero
				if(list->type && length){
					size_t type_size = NBT_internal_sizeof(list->type);
					list->data = List_create(type_size);
					List_reserve(list->data, length);

					for(size_t i=0; i<length; i++)
						NBT_internal_Tag_body_load(List_append(list->data, NULL), list->type, f);

				}else{
					list->data=NULL;
				}
			} break;
		case NBT_COMPOUND:
			{
				data->array = List_create(sizeof(struct NBT_Tag));

				struct NBT_Tag element;
				while((NBT_Tag_load(&element, f) != NBT_END))
				{
					List_append(data->array, &element);
				}

			} break;
		case NBT_ARRAY_INT:
			NBT_internal_Array_load(&data->array, sizeof(int32_t), f);

			break;

		case NBT_ARRAY_LONG:
			NBT_internal_Array_load(&data->array, sizeof(int64_t), f);
			break;

		default:
			puts("[ERROR] Unnkown Type: could not load!");
			ret = true;
	}
	return ret;
}

// NBT
// TAG: { byte TYPE, Name, Playload }
// Name: { uint16_t length, char[length] }
// Playload: { header<TYPE>, data, TAG<END> }

enum NBT_TYPE NBT_Tag_load(NBT head, gzFile f)
{
	if(( head->type=gzgetc(f) )){
		NBT_internal_String_load(&head->name, f);
		NBT_internal_Tag_body_load(&head->data, head->type, f);
	}
	return head->type;
}


NBT NBT_load(const char *path)
{
	NBT root =0;
	// uncompress nbt gzip container with zlib
	gzFile nbt_file = gzopen(path, "rb");
	if(nbt_file){

		// load content into NBT_Tag structure
		root = malloc(sizeof(struct NBT_Tag));
		NBT_Tag_load(root, nbt_file);

		gzclose(nbt_file);
		return root;
	}else{

		free(root);
		return NULL;
	}
};

enum NBT_TYPE NBT_type_get(NBT nbt) 	{	return nbt->type;}
int8_t NBT_byte_get(NBT_Data nbt) 		{	return nbt->b;}
int16_t NBT_short_get(NBT_Data nbt) 	{	return nbt->s;}
int32_t NBT_integer_get(NBT_Data nbt) {	return nbt->i;}
int64_t NBT_long_get(NBT_Data nbt) 		{	return nbt->l;}
float NBT_float_get(NBT_Data nbt) 		{	return nbt->f;}
double NBT_double_get(NBT_Data nbt) 	{	return nbt->d;}
List NBT_array_get(NBT_Data nbt) 			{	return nbt->array;}
char *NBT_string_get(NBT_Data nbt)    { return nbt->str;}

NBT_Data NBT_data_get(NBT nbt)
{
	if(nbt)
		return &nbt->data;
	return NULL;
}

size_t NBT_list_length(NBT_Data nbt)
{
	if(nbt)
		if(nbt->list.data)
			return List_size(nbt->list.data);
	return 0;
}

enum NBT_TYPE NBT_list_type_get(NBT_Data nbt)
{
	if(nbt)
		return nbt->list.type;
	return 0;
}

NBT_Data NBT_list_get(NBT_Data nbt, size_t index)
{
	if(nbt && nbt->list.data)
		return List_get(nbt->list.data, index);
	return NULL;
}

size_t NBT_compound_size(NBT_Data nbt)
{
	return List_size(nbt->array);
}

static bool cmp_nbt(void *a, void *b)
{
	NBT element = a;
	if ( element->name==NULL )
		return 0;
	return !strcmp(element->name, b);
}

NBT NBT_compound_get_name(NBT_Data nbt, char *name)
{
	if(nbt)
		return List_find(nbt->array, cmp_nbt, name);
	return NULL;
}

NBT NBT_compound_get_index(NBT_Data nbt, size_t index)
{
	return List_get(nbt->array, index);
}

void NBT_internal_free(NBT nbt);

void NBT_internal_data_free(NBT_Data data, enum NBT_TYPE type)
{
	switch(type)
	{
		case NBT_BYTE:
		case NBT_SHORT:
		case NBT_INT:
		case NBT_LONG:
		case NBT_FLOAT:
		case NBT_DOUBLE:
			break;

		case NBT_STRING:
			if(data->str)
				free(data->str);
			break;
		case NBT_ARRAY_BYTE:
		case NBT_ARRAY_INT:
		case NBT_ARRAY_LONG:
			List_free(data->array);
			break;

		case NBT_LIST:
		{
			struct NBT_List *list=&data->list;
			if(list->data){
				void *element;
				while(( element = List_pop(list->data) ))
					NBT_internal_data_free(element,  list->type);
				List_free(list->data);
			}
			break;
		}

		case NBT_COMPOUND:
		{
			NBT element;
			while(( element = List_pop(data->array) ))
				NBT_internal_free(element);
			List_free(data->array);
			break;
		}

		default:
			printf("[ERROR] Unknown type: %d", type);
	}
}

void NBT_internal_free(NBT nbt)
{
	if(nbt->name)
		free(nbt->name);
	NBT_internal_data_free(&nbt->data, nbt->type);
}

void NBT_free(NBT nbt)
{
	NBT_internal_free(nbt);
	free(nbt);
};

