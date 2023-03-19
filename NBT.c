#include "NBT.h"
#include <stdio.h>
#include <string.h>
#include "zlib/zlib.h"
#include <stdlib.h>
#define INFO_LVL 0
#include "info/info.h"

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
		--end;
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
		gzread(f, List_push(*arr, NULL), element_size);

	return 0;
}

void NBT_internal_value_store(gzFile f, void *data, size_t size)
{
	util_endian_flip(data, (uint8_t*)(data)+size);
	gzwrite(f, data, size);
	util_endian_flip(data, (uint8_t*)(data)+size);
}


bool NBT_internal_String_store(gzFile f, char *str)
{
	uint16_t length;
	if(str)
		length=strlen(str);
	else
		length=0;

	// store next bytes into length
	NBT_internal_value_store(f, &length, sizeof(uint16_t));

	if(length){
		gzwrite(f, str, length);
	}

	return 0;
}

bool NBT_internal_Array_store(gzFile f, List l)
{
	uint16_t length;
	// store next bytes into length
	size_t size = List_size(l);
	NBT_internal_value_store(f, &size, sizeof(uint16_t));

	// read characters into string buffer
	for(size_t i=0; i<length; i++)
		gzwrite(f, List_start(l), List_end(l)-List_start(l));

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
			FATAL("Unknown Type: cannot get size!")
	}
}


const char* NBT_type_name(enum NBT_TYPE type)
{
	switch(type)
	{
		case NBT_END:
			return "End";
		case NBT_BYTE:
			return "Byte";
		case NBT_SHORT:
			return "Short";
		case NBT_INT:
			return "Int";
		case NBT_LONG:
			return "Long";
		case NBT_FLOAT:
			return "Float";
		case NBT_DOUBLE:
			return "Double";
		case NBT_STRING:
			return "String";
		case NBT_ARRAY_BYTE:
			return "byte Array";
		case NBT_LIST:
			return "List";
		case NBT_COMPOUND:
			return "Compound";
		case NBT_ARRAY_INT:
			return "int Array";
		case NBT_ARRAY_LONG:
			return "long Array";
		default:
			ERROR("Unknown Type!")
			return "Unknown Type!";
	}
}

void util_array_print(List array, enum NBT_TYPE type)
{

	PRINT("= {")
	uint8_t *start = List_start(array), *end = List_end(array);
	while(start<end)
	{
		switch(type)
		{
			case NBT_ARRAY_BYTE:
				PRINT("%d", *start)
				start+=NBT_internal_sizeof(NBT_BYTE);
				break;
			case NBT_ARRAY_INT:
				PRINT("%d", *(int*)start)
				start+=NBT_internal_sizeof(NBT_INT);
				break;
			case NBT_ARRAY_LONG:
				PRINT("%ld", *(long*)start)
				start+=NBT_internal_sizeof(NBT_LONG);
				break;
			default:
				ERROR("Unknown Type!")
				return;
		}

		if(start<end)
			PRINT(", ")
	}
	PRINT("}\n")
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
			INFO("Loading Byte= %d", data->b)
			break;

		case NBT_SHORT:
			NBT_internal_value_load(&data->s, sizeof(data->s), f);
			INFO("Loading Short= %s", data->s)
			break;
		case NBT_INT:
			NBT_internal_value_load(&data->i, sizeof(data->i), f);
			INFO("Loading Int= %i", data->i)
			break;
		case NBT_LONG:
			NBT_internal_value_load(&data->l, sizeof(data->l), f);
			INFO("Loading Long= %l", data->l)
			break;
		case NBT_FLOAT:
			NBT_internal_value_load(&data->f, sizeof(data->l), f);
			INFO("Loading Float= %f", data->f)
			break;
		case NBT_DOUBLE:
			NBT_internal_value_load(&data->d, sizeof(data->d), f);
			INFO("Loading Double= %f", data->d)
			break;

		case NBT_ARRAY_BYTE:
			NBT_internal_Array_load(&data->array, sizeof(uint8_t), f);
			HOLD
			INFO("Loading %s[%d]= ", NBT_type_name(NBT_ARRAY_BYTE), List_size(data->array))
#if INFO_LVL > 4
			util_array_print(data->array,type);
#endif
			RELEASE
			break;

		case NBT_STRING:
			NBT_internal_String_load(&data->str, f);
			INFO("Loading String[%d]= \"%s\"", data->str ? 0 : strlen(data->str), data->str ? "" : data->str);
			break;

		case NBT_LIST:
			{
				// List header : [element_type<byte> length<int>]
				struct NBT_List *list = &data->list;
				int32_t length;

				list->type = gzgetc(f);
				NBT_internal_value_load(&length, sizeof(int32_t), f);

				HOLD
				INFO("Loading List<%d>[%d]", (char)type, length)
				RELEASE

				// if list not empty load elements, else zero
				if(list->type && length){
					size_t type_size = NBT_internal_sizeof(list->type);
					list->data = List_create(type_size);
					List_reserve(list->data, length);

					INDENT(1)
					for(size_t i=0; i<length; i++)
						NBT_internal_Tag_body_load(List_push(list->data, NULL), list->type, f);

					INDENT(-1)
				}else{
					list->data=NULL;
				}
			} break;
		case NBT_COMPOUND:
			{
				data->array = List_create(sizeof(struct NBT_Tag));
				INFO("Loading Compound")
				INDENT(1)
				struct NBT_Tag element;
				while((NBT_Tag_load(&element, f) != NBT_END))
				{
					List_push(data->array, &element);
				}
				INDENT(-1)

			} break;
		case NBT_ARRAY_INT:
			NBT_internal_Array_load(&data->array, sizeof(int32_t), f);
			HOLD
			INFO("Loading %s[%d]", NBT_type_name(type), List_size(data->array))
#if INFO_LVL > 4
			util_array_print(data->array,type);
#endif
			RELEASE

			break;

		case NBT_ARRAY_LONG:
			NBT_internal_Array_load(&data->array, sizeof(int64_t), f);
			HOLD
			INFO("Loading %s[%d]= ", NBT_type_name(type), List_size(data->array))
#if INFO_LVL > 4
			util_array_print(data->array,type);
#endif
			RELEASE
			break;

		default:
			ERROR("Unkown Type: could not load!")
			ret = true;
	}
	return ret;
}

void NBT_Tag_store(gzFile f, NBT head);

bool NBT_internal_Tag_body_store(gzFile f, struct NBT_Payload *data, enum NBT_TYPE type)
{
	bool ret=false;
	HOLD
	INFO("Storing ")
	switch(type)
	{
		// load single values
		case NBT_BYTE:
			gzputc(f, data->b);
			INFO("Byte= %d", data->b)
			break;

		case NBT_SHORT:
			NBT_internal_value_store(f, &data->s, sizeof(data->s));
			INFO("Short= %s", data->s)
			break;
		case NBT_INT:
			NBT_internal_value_store(f, &data->i, sizeof(data->i));
			INFO("Int= %i", data->i)
			break;
		case NBT_LONG:
			NBT_internal_value_store(f, &data->l, sizeof(data->l));
			INFO("Long= %l", data->l)
			break;
		case NBT_FLOAT:
			NBT_internal_value_store(f, &data->f, sizeof(data->l));
			INFO("Float= %f", data->f)
			break;
		case NBT_DOUBLE:
			NBT_internal_value_store(f, &data->d, sizeof(data->d));
			INFO("Double= %f", data->d)
			break;

		case NBT_ARRAY_BYTE:
			NBT_internal_Array_store(f, data->array);
			INFO("%s[%d]= ", NBT_type_name(NBT_ARRAY_BYTE), List_size(data->array))
#if INFO_LVL > 4
			util_array_print(data->array,type);
#endif
			break;

		case NBT_STRING:
			INFO("String[%d]= \"%s\"", data->str ? 0 : strlen(data->str), data->str ? "" : data->str);
			NBT_internal_String_store(f, data->str);
			break;

		case NBT_LIST:
			{
				// List header : [element_type<byte> length<int>]
				struct NBT_List *list = &data->list;
				int32_t length;
				if(list->data)
					length=List_size(list->data);
				else
					length=0;

				gzputc(f, list->type);
				NBT_internal_value_store(f, &length, sizeof(int32_t));

				INFO("List<%d>[%d]", (char)type, length)

				// if list not empty load elements, else zero
				if(list->type && length){
					size_t type_size = NBT_internal_sizeof(list->type);

					INDENT(1)
					for(uint8_t *start=List_start(list->data),
							*end=List_end(list->data);
							start!=end; start+=type_size)
					{
						NBT_internal_Tag_body_store(f, (struct NBT_Payload*)start, list->type);
					}
					INDENT(-1)
				}
			} break;
		case NBT_COMPOUND:
			{
				INFO("Compound")
				INDENT(1)
				for(NBT start=List_start(data->array),
					    end=List_end(data->array);
						start!=end; start++)
				{
					NBT_Tag_store(f, start);
				}
				gzputc(f, 0);
				INDENT(-1)

			} break;
		case NBT_ARRAY_INT:
			NBT_internal_Array_store(f, data->array);
			INFO("%s[%d]", NBT_type_name(type), List_size(data->array))
#if INFO_LVL > 4
			util_array_print(data->array,type);
#endif

			break;

		case NBT_ARRAY_LONG:
			NBT_internal_Array_store(f, data->array);
			INFO("%s[%d]= ", NBT_type_name(type), List_size(data->array))
#if INFO_LVL > 4
			util_array_print(data->array,type);
#endif
			RELEASE
			break;

		default:
			ERROR("Unkown Type: could not load!")
			ret = true;
	}
	RELEASE
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

void NBT_Tag_store(gzFile f, NBT head)
{
	gzputc(f, (int)head->type);
	NBT_internal_String_store(f, head->name);
	NBT_internal_Tag_body_store(f, &head->data, head->type);
}

enum NBT_TYPE NBT_type_get(NBT nbt) 	{	return nbt->type;}
int8_t NBT_byte_get(NBT_Data nbt) 		{	return nbt->b;}
int16_t NBT_short_get(NBT_Data nbt) 	{	return nbt->s;}
int32_t NBT_integer_get(NBT_Data nbt) 	{	return nbt->i;}
int64_t NBT_long_get(NBT_Data nbt) 		{	return nbt->l;}
float NBT_float_get(NBT_Data nbt) 		{	return nbt->f;}
double NBT_double_get(NBT_Data nbt) 	{	return nbt->d;}
List NBT_array_get(NBT_Data nbt) 		{	return nbt->array;}
char *NBT_string_get(NBT_Data nbt) 		{ return nbt->str;}

void NBT_type_set(NBT nbt, enum NBT_TYPE v) 	{ nbt->type=v;}
void NBT_byte_set(NBT_Data nbt, int8_t v) 		{ nbt->b=v;}
void NBT_short_set(NBT_Data nbt, int16_t v) 	{ nbt->s=v;}
void NBT_integer_set(NBT_Data nbt, int32_t v) 	{ nbt->i=v;}
void NBT_long_set(NBT_Data nbt, int64_t v) 		{ nbt->l=v;}
void NBT_float_set(NBT_Data nbt, float v) 		{ nbt->f=v;}
void NBT_double_set(NBT_Data nbt, double v) 	{ nbt->d=v;}
void NBT_array_set(NBT_Data nbt, List v) 		{ nbt->array=v;}

void NBT_string_set(NBT_Data nbt, char *v)
{
	if(!v)
		nbt->str=NULL;
	else{
		size_t len=strlen(v)+1;
		nbt->str=malloc(len);
		memcpy(nbt->str, v, len);
	}
}

NBT_Data NBT_data(NBT nbt)
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
void NBT_list_type_set(NBT_Data nbt, enum NBT_TYPE v)
{
	if(nbt->list.type!=v){
		if(nbt->list.data)
			List_free(nbt->list.data);

		nbt->list.data=List_create(NBT_internal_sizeof(v));
		nbt->list.type=v;
	}
}

NBT_Data NBT_list_get(NBT_Data nbt, size_t index)
{
	if(nbt && nbt->list.data)
		return List_get(nbt->list.data, index);
	return NULL;
}

void NBT_list_set(NBT_Data nbt, size_t index, NBT_Data v)
{
	if(nbt && nbt->list.data){
		if(index>=List_size(nbt->list.data))
			List_resize(nbt->list.data, index+1);
		memcpy(List_get(nbt->list.data, index), v, NBT_internal_sizeof(nbt->list.type));
	}
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

void NBT_compound_set_name(NBT_Data nbt, char *name, NBT v)
{
	if(nbt){
		size_t len = strlen(name)+1;
		v->name=malloc(len);
		memcpy(v->name, name, len);

		NBT e=NULL;

		if(nbt->array)
			e = List_find(nbt->array, cmp_nbt, name);
		else
			nbt->array=List_create(sizeof(struct NBT_Tag));

		if(e)
			*e=*v;
		else
			e = List_push(nbt->array, v);
	}
}

void NBT_compound_set_index(NBT_Data nbt, size_t index, NBT v)
{
	if(nbt){
		if(!nbt->array)
			nbt->array=List_create(sizeof(struct NBT_Tag));

		if(index>=List_size(nbt->array))
			List_resize(nbt->array, index+1);

		*(NBT)List_get(nbt->array, index)=*v;
	}
}

void NBT_internal_free(NBT nbt);

void NBT_internal_data_free(NBT_Data data, enum NBT_TYPE type)
{
	switch(type)
	{
		case NBT_END:
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
			ERROR("Unknown type: %d", type)
	}
}

void NBT_internal_free(NBT nbt)
{
	if(nbt->name)
		free(nbt->name);
	NBT_internal_data_free(&nbt->data, nbt->type);
}


NBT NBT_create()
{
	NBT root = malloc(sizeof(struct NBT_Tag));
	memset(root, 0, sizeof(struct NBT_Tag));
	return root;
}

bool NBT_from_file(NBT root, const char *path)
{
	NBT_internal_free(root);
	// uncompress nbt gzip container with zlib
	gzFile nbt_file = gzopen(path, "rb");
	if(nbt_file){

		size_t indent = INDENT(0);
		INDENT(-indent)
		MODE(format_structured)

		PRINT("Loading %s", path)
		INDENT(1)

		// load content into NBT_Tag structure
		NBT_Tag_load(root, nbt_file);

		MODE(format_default)
		INDENT(-1)
		INDENT(indent)

		gzclose(nbt_file);
		return false;
	}
	ERROR("Could not open file '%s'", path);

	return true;
};

bool NBT_to_file(NBT root, const char *path)
{
	// uncompress nbt gzip container with zlib
	gzFile nbt_file = gzopen(path, "wb");
	if(nbt_file){

		size_t indent = INDENT(0);
		INDENT(-indent)
		MODE(format_structured)

		PRINT("Writing %s", path)
		INDENT(1)

		// load content into NBT_Tag structure
		NBT_Tag_store(nbt_file, root);

		MODE(format_default)
		INDENT(-1)
		INDENT(indent)

		gzclose(nbt_file);
		return false;
	}
	ERROR("Could not open file '%s'", path);

	return true;
}

void NBT_free(NBT nbt)
{
	NBT_internal_free(nbt);
	free(nbt);
};

