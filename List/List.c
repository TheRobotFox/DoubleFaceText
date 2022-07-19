#include "List.h"
#include <string.h>

struct _List{
	char *data;
	size_t size;
	size_t max;
	size_t element_size;
};

List List_create(size_t element_size)
{
	List l = malloc(sizeof(struct _List));
	l->element_size = element_size;
	l->data=malloc(l->element_size);
	l->size=0;
	l->max=1;
	return l->data ? l : NULL;
}

void List_free(List l)
{
	free(l->data);
	free(l);
}

static inline void* List_at(List l, size_t index)
{
	return l->data+(l->element_size*index);
}

void* List_get(List l, size_t index)
{
	if(index<l->size)
		return List_at(l,index);
	return NULL;
}

void* List_start(List l){return l->data;}
void* List_end(List l){return l->data+(l->size*l->element_size);}

size_t List_size(List l){return l->size;}
size_t List_capacity(List l){return l->max;}

bool List_reserve(List l, size_t capacity)
{
	if(capacity>l->max)
	{
		l->data=realloc(l->data,capacity*l->element_size);
		l->max=capacity;
	}
	return !(l->data);
}

void List_clear(List l)
{
	l->size=0;
}
void *Buff_find(char *start, char *end, size_t el_size, bool (*compare)(void*, void*), void *arg)
{
	for(; start!=end; start+=el_size)
	{
		if(compare(start,arg))
				return start;
	}
	return NULL;
}
void *List_find(List l, bool (*compare)(void*, void*), void *arg)
{
	return Buff_find(List_start(l),List_end(l),l->element_size,compare,arg);
}
void* List_append(List l, const void *element)
{
	if(l->size>=l->max)
	{
		if(List_reserve(l, l->max*2))
			return 0;
	}
	void *ptr=List_at(l,l->size);
	l->size++;
	if(element)
		memcpy(ptr,element,l->element_size);
	return ptr;
}

void* List_pop(List l)
{
	if(l->size==0)
		return NULL;
	return List_at(l, --l->size);
}

bool List_copy(List a, List b)
{
	if(a->element_size!=b->element_size)
		return 2;
	if(List_reserve(b,List_size(a)))
		return 1;
	b->size=a->size;
	memcpy(List_start(b),List_start(a),List_end(a)-List_start(a));
	return 0;
}

//void List_print(List l, void (*print)(void*))
//{
//	printf("[");
//	for(char *start=List_start(l), *end=List_end(l); start!=end; start+=l->element_size){
//		print(start);
//		if(start!=end-l->element_size)
//			printf(", ");
//	}
//	printf("]");
//}
