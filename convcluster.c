#include "convcluster.h"
#define INFO_LVL 5
#include "info.h"
#include <stdlib.h>
#include <string.h>

#define MAX_PATH_LENGTH 512

struct CC_Task
{
	enum CC_STATE start;
	union CC_Data *input;
	enum CC_STATE end;
	void *additional;

	union CC_Data *data;

	struct CC_Context
	{
		struct CC_Rule *rules;
		int rules_count;
		enum CC_STATE previous[MAX_PATH_LENGTH];
		int indexes[MAX_PATH_LENGTH];
		int path_length;
		int depth;
		int load_depth;
	} context;

	bool valid;

};

typedef struct CC_Context* CC_Context;

struct CC_internal_step
{

	enum CC_STATE in;
	enum CC_STATE out;
};

#if INFO_LVL>4
static const char* CC_internal_STATE_names[] = {
	FOREACH_FUNC(GENERATE_STRING)
	"internal_state_counter",
	"Unknown"
};
#endif

CC_Task CC_Task_create()
{
	CC_Task task = malloc(sizeof(struct CC_Task));
	memset(task, 0, sizeof(struct CC_Task));
	return task;
}

bool CC_Task_target_set(CC_Task task, union CC_Data *data)
{
	task->data=data;
	return false;
}

bool CC_Task_state_start_set(CC_Task task, enum CC_STATE start)
{
	if(start>COUNT){
		start=UNKNOWN;
		return true;
	}
	task->valid=false;
	task->start=start;
	return false;
}
bool CC_Task_state_end_set(CC_Task task, enum CC_STATE end)
{
	if(end>COUNT){
		end=UNKNOWN;
		return true;
	}
	task->valid=false;
	task->end=end;
	return false;
}

bool CC_Task_additional_set(CC_Task task, void *data)
{
	task->additional = data;
	return false;
}

bool CC_Task_rules_set(CC_Task task, struct CC_Rule *rules, int rules_count)
{
	task->valid=0;
	task->context.rules = rules;
	task->context.rules_count = rules_count;
	return false;
}

void CC_Task_free(CC_Task task)
{
	free(task);
}

bool CC_internal_is_previous_state(CC_Context context, enum CC_STATE state)
{
	for(int i=0; i<context->depth; i++)
	{
		if(state==context->previous[i])
			return true;
	}
	return false;
}

bool CC_internal_context_state_set(CC_Context context, enum CC_STATE state)
{
	// set up context for next level
	if(context->depth<0)
		return true;
	context->previous[context->depth]=state;
	return false;
}

bool CC_internal_context_index_set(CC_Context context, int index)
{
	context->indexes[context->depth] = index;
	return false;
}


#if INFO_LVL > 4
void CC_internal_error_path(CC_Context context, int index)
{
	PRINT(" [")
	for(int i=0; i<context->depth; i++)
	{
		if(i==index)
			PRINT("{")
		PRINT(CC_internal_STATE_names[context->previous[i]])
		if(i==index)
			PRINT("}")

		if(i<context->depth-1)
			PRINT(", ")
	}
	PRINT("]")
}

#endif

bool CC_internal_solve(struct CC_internal_step task, CC_Context context)
{
	if(context->depth>=MAX_PATH_LENGTH-1)
		return true;

	CC_internal_context_state_set(context, task.in);

	// set start index if it has to be loaded
	int start=0;
	if(context->depth<context->load_depth){
		start=context->indexes[context->depth];
	}

	context->depth++;

	// look for direct path first
	for(int i=start; i<context->rules_count; i++)
	{
		if(context->rules[i].in==task.in && context->rules[i].out==task.out){
				HOLD
				INFO("Found conversion path! depth: %d", context->depth)
				#if INFO_LVL > 4
					CC_internal_error_path(context, -1);
				#endif
				RELEASE
				context->path_length=context->depth;
				// push index for potential continuation
				context->depth--;
				CC_internal_context_index_set(context, i);
				return false;
		}

	}

	// try all subpaths
	for(int i=start; i<context->rules_count; i++)
	{
		if(context->rules[i].in==task.in && !CC_internal_is_previous_state(context, context->rules[i].out)){
			struct CC_internal_step child = { context->rules[i].out, task.out };
			if(!CC_internal_solve(child, context)){

				context->depth--;
				// push index for potential continuation
				CC_internal_context_index_set(context, i);
				return false;
			}
		}
	}

	context->depth--;
	return true;
}

void CC_internal_continue(CC_Context context)
{
	// setup context to continue search after current path
	// force loops to load startvalue
	context->load_depth=context->depth+1;

	// skip last path
	context->indexes[context->depth]++;
}


bool CC_solve(CC_Task task)
{
	task->context.depth=0;
	task->context.load_depth=0;

	// if gaol already reached
	if(task->start==task->end)
		goto success;
	struct CC_internal_step step = { task->start, task->end };

	if(!task->valid){

		#if INFO_LVL > 4
			INFO("Searching conversion Path! %s -> %s", CC_internal_STATE_names[task->start], CC_internal_STATE_names[task->end])
		#else
			INFO("Searching conversion Path!")
		#endif

		search:

		if(CC_internal_solve(step, &task->context)){
			ERROR("Cannot find conversion path!")
			return true;
		}
	}

	SEG_BEGIN("Excecute conversion path")
	for(int i=0; i<task->context.path_length; i++)
	{
		if(task->context.rules[task->context.indexes[i]].convert(task->data, task->additional)){
			HOLD
			INFO("Path exceution failed!")
#if INFO_LVL > 4
			CC_internal_error_path(&task->context, i);
#endif
			RELEASE
			task->valid=false;
			CC_internal_continue(&task->context);
			SEG_END
			INFO("Continue search!")
			goto search;
		}
	}
	SEG_END

success:
	task->valid=true;

	SUCCESS("Conversion was successful!")
	return false;
}

