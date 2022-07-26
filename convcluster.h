#include <stdbool.h>

// Thanks to https://stackoverflow.com/questions/9907160/how-to-convert-enum-names-to-string-in-c
#define FOREACH_FUNC(FUNC) 	    \
        FUNC(SHADOW)  		    \
        FUNC(NBT_VOL)      	    \
        FUNC(MESH) 		        \
		FUNC(IMG)               \
		FUNC(VOLUME)	       	\
						       	\
        FUNC(IN_NBT) 	       	\
        FUNC(IN_IMG) 	       	\
        FUNC(IN_TEXT) 		    \
        FUNC(INPUT_GROUP)	    \
								\
        FUNC(SHADOW_OUT)  	    \
        FUNC(NBT_OUT)      	    \
        FUNC(MESH_OUT) 	        \

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,


enum CC_STATE
{
	FOREACH_FUNC(GENERATE_ENUM)
	COUNT,
	UNKNOWN
};

// implement yourself
union CC_Data;

typedef struct CC_Task* CC_Task;

typedef bool (*CC_F_Convert)(union CC_Data*, void*);

struct CC_Rule
{
	enum CC_STATE in;
	enum CC_STATE out;
	CC_F_Convert convert;
};

CC_Task CC_Task_create();
bool CC_Task_target_set(CC_Task task, union CC_Data *data);
bool CC_Task_state_start_set(CC_Task task, enum CC_STATE start);
bool CC_Task_state_end_set(CC_Task task, enum CC_STATE end);
bool CC_Task_additional_set(CC_Task task, void *data);
bool CC_Task_rules_set(CC_Task task, struct CC_Rule *rules, int rules_count);
void CC_Task_free(CC_Task task);

bool CC_solve(CC_Task task);
