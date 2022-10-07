#include "mesh.h"
#include "info/info.h"
#include "convcluster.h"
#include "Font.font.h"
#include <string.h>

union CC_Data
{
	struct Input_Group
	{
		char *str[3];
		enum CC_STATE type[3];
	} group;
	char *str;
	Image shadow[3];
	Image img;
	Volume vol;
	NBT nbt;
	Mesh mesh;
};

// Group to shadow
bool TEXT_to_IMG(union CC_Data *data, void* _)
{
	size_t len = strlen(data->str);
	INFO("Rasterizing string '%s'[%d]", data->str, len)

	Font_Rect rect = Font_string_dimensions(&font, data->str, len, font.height);
	INFO("String dimensions %d, %d", rect.width, rect.height)

	Image img = Image_create();
	Image_from_color(img, rect.width, rect.height, (unsigned char)255);
	int printed = Font_render_string_rect(&font, data->str, len, 0, 0, font.height, f_Image_draw_rect, (void*)img);

	if(printed!=len)
		return true;

	data->img = img;
	return false;
}

bool LOAD_IMG(union CC_Data *data, void* _)
{
	bool ret=true;
	Image img = Image_create();
	ret = Image_from_file(img, data->str);
	data->img=img;
	return ret;
}

struct CC_Rule group_rules[]={
	{IN_TEXT, IMG, TEXT_to_IMG},
	{IN_IMG, IMG, LOAD_IMG},
};

bool INPUT_GROUP_to_SHADOW(union CC_Data *data, void* _)
{
	bool ret=true;
	union CC_Data out;
	Image shadow[3] = { 0 };

	struct Input_Group *g = &data->group;
	if(!g->str[0] || !g->str[1])
		return true;

	CC_Task task = CC_Task_create();
	CC_Task_rules_set(task, group_rules, sizeof(group_rules)/sizeof(*group_rules));
	CC_Task_target_set(task, &out);

	// non volatile ignores i<3 and causes SEGFAULT
	for(volatile int i=0; g->str[i] && i<3; i++)
	{
		out.str = g->str[i];

		CC_Task_state_start_set(task, g->type[i]);
		CC_Task_state_end_set(task, IMG);
		if(CC_solve(task)){
			ERROR("Could not proccess! %s", g->str[i])
				for(int x=0; x<i; x++)
					Image_free(shadow[x]);
			goto cleanup;
		}
		shadow[i]=out.img;
	}

	for(int i=0; i<3; i++)
		data->shadow[i]=shadow[i];

	ret = false;

cleanup:
	CC_Task_free(task);
	return ret;
}


// Rule functions

bool SHADOW_to_VOLUME(union CC_Data *data, void *_)
{
	bool ret = true;
	Volume vol = Volume_create();
	if(data->shadow[2]){
		ret = Volume_from_shadow_3(vol, data->shadow[0], data->shadow[1], data->shadow[2]);
		Image_free(data->shadow[2]);
	} else {
		ret = Volume_from_shadow_2(vol, data->shadow[0], data->shadow[1]);
	}

	Image_free(data->shadow[0]);
	Image_free(data->shadow[1]);
	data->vol = vol;
	return ret;
}

bool NBT_to_VOLUME(union CC_Data *data, void *_)
{
	Volume vol = Volume_create();
	bool ret = Volume_from_nbt(vol, data->nbt, NULL);
	NBT_free(data->nbt);
	data->vol=vol;
	return ret;
}

bool LOAD_NBT(union CC_Data *data, void *_)
{
	NBT nbt = NBT_create();

	bool ret = NBT_from_file(nbt, data->str);

	data->nbt=nbt;
	return ret;
}

bool VOLUME_to_MESH(union CC_Data *data, void *_)
{
	Mesh mesh = Mesh_create();
	bool ret = Mesh_from_volume(mesh, data->vol);

	Volume_free(data->vol);

	data->mesh=mesh;
	return ret;
}

bool VOLUME_to_SHADOW(union CC_Data *data, void *_)
{
	Image shadow[3];
	bool ret = Volume_to_shadow(data->vol, shadow, shadow+1, shadow+2);

	Volume_free(data->vol);

	for(int i=0; i<3; i++)
		data->shadow[i]=shadow[i];
	return ret;
}

bool NBT_OUTPUT(union CC_Data *data, void *path)
{
	ERROR("NBT output not implemented!")
	return true;
}

const char* SHADOW_names[]={
	"front",
	"side",
	"top"
};

#define NAME_BUF_SIZE 512
char name_buf[NAME_BUF_SIZE];
char name_suffix_buf[256];

bool SHADOW_OUTPUT(union CC_Data *data, void *_path)
{
	char *path=_path;
	bool ret = false;
	size_t len = strlen(path);
	for(size_t i=len-1; i>=0; i--)
	{
		if(path[i]=='.'){
			char *buf = name_suffix_buf;
			for(size_t e=i; e<=len && buf<name_suffix_buf+256; e++)
			{
				*buf=path[e];
				buf++;
			}
			path[i]=0;
			break;
		}
	}
	strcpy(name_buf, path);
	name_buf[len]='_';
	for(int i=0; i<3; i++)
	{
		if(!data->shadow[i])
			continue;

		name_buf[len+1]=0;
		snprintf(name_buf, NAME_BUF_SIZE, "%s_%s%s", path, SHADOW_names[i], name_suffix_buf);
		INFO("Exporting Shadow '%s'", SHADOW_names[i])
		ret +=Image_save(data->shadow[i], name_buf);
		Image_free(data->shadow[i]);
	}
	return ret;
}

bool MESH_OUTPUT(union CC_Data *data, void *_path)
{
	char *path = _path;
	size_t len = strlen(path);
	char id = 0;
	for(int i=0; i<len-1; i++)
	{
		if(path[i]=='.'){
			id=path[i+1];
			break;
		}
	}

	bool ret=true;
	switch(id)
	{
		case 's':
		case 'S':
			ret = Mesh_save_stl(data->mesh, path);
			break;
		default:
			INFO("Could not detect file type!")
		case 'o':
		case 'O':
			ret = Mesh_save_obj(data->mesh, path);
	}
	Mesh_free(data->mesh);
	return ret;
}

struct CC_Rule main_rules[]={
	{INPUT_GROUP, SHADOW, INPUT_GROUP_to_SHADOW},
	{SHADOW, VOLUME, SHADOW_to_VOLUME},
	{NBT_VOL, VOLUME, NBT_to_VOLUME},
	{IN_NBT, NBT_VOL, LOAD_NBT},
	{VOLUME, MESH, VOLUME_to_MESH},
	{VOLUME, SHADOW, VOLUME_to_SHADOW},
	{NBT_VOL, NBT_OUT, NBT_OUTPUT},
	{SHADOW, SHADOW_OUT, SHADOW_OUTPUT},
	{MESH, MESH_OUT, MESH_OUTPUT},
	//{VOLUME, NBT, VOLUME_to_NBT}
};


const char *tag_names[] = {
	"FROM", // 1-3 inputs
	"TO", 	// 1+
	"AS" 	// if present 1?[TO]
};

const char *input_names[] = {
	"IMG",
	"TEXT",
	"NBT",
};

const char *output_names[] = {
	"SHADOW",
	"NBT",
	"MESH"
};

enum CC_STATE DFT_state_input_get(const char *str)
{
	if(!strcmp("IMG", str))
		return IN_IMG;
	if(!strcmp("TEXT", str))
		return IN_TEXT;
	if(!strcmp("NBT", str))
		return IN_NBT;
	ERROR("Unknown input_type \"%s\"", str)
	return UNKNOWN;
}
enum CC_STATE DFT_state_output_get(const char *str)
{
	if(!strcmp("SHADOW", str))
		return SHADOW_OUT;
	if(!strcmp("NBT", str))
		return NBT_OUT;
	if(!strcmp("MESH", str))
		return MESH_OUT;
	ERROR("Unknown output_type \"%s\"", str)
	return UNKNOWN;
}
void usage(const char *name)
{
	HOLD
	INFO("%s [INPUT] %s [INPUT TYPE] %s [OUTPUT TYPE] (%s [OUTPUT])\n", name, tag_names[0], tag_names[1], tag_names[2]);
	PRINT("'INPUT'       : {IMG_PATH or TEXT,...}[3] or NBT\n")
	PRINT("'INPUT TYPE'  : INPUT_TYPE[i] = INPUT[i].type eg. IMG/TEXT/NBT\n")
	PRINT("'OUTPUT TYPE' : INPUT -> {OUTPUT_TYPE, ...}\n")
	PRINT("'OUTPUT'      : OUTPUT[i] = OUTPUT_TYPE[i].out_path")
	RELEASE
}

#define ARGS_START 1

int main(int argc, char **argv)
{

	if(argc<8){
		ERROR("Invalid number of arguments")
		usage(argv[0]);
		return 1;
	}

	int tags[3] = {0};

	INFO("Parsing arguments")

	for(int tag=0; tag<3; tag++)
	{
		for(int i=2*(1+tag); i<argc; i++)
		{
			if(strcmp(argv[i], tag_names[tag]))
					continue;
			tags[tag] = i;
			goto found;
		}

		ERROR("Could not find '%s' Tag!", tag_names[tag])
		usage(argv[0]);
		return 1;

		found:
		continue;
	}

	INFO("Checking input");

	int input_count = tags[0]-ARGS_START;
	if(input_count<1 || input_count>3){
		ERROR("'INPUT' tag malformed!", tag_names[0])
		usage(argv[0]);
		return 1;
	}

	int input_type_count = tags[1]-tags[0]-1;
	if(input_type_count != input_count){
		ERROR("'INPUT TYPE' Tag needs to map 'INPUT'. Found %d inputs but %d type descriptors", input_count, input_type_count)
		usage(argv[0]);
		return 1;
	}

	int output_type_count = tags[2]-tags[1]-1;

	int output_count = argc - tags[2]-1;
	if(output_type_count != output_count){
		ERROR("'OUTPUT TYPE' Tag needs to map 'OUTPUT'")
		usage(argv[0]);
		return 1;
	}

	INFO("Preparing Task")

	union CC_Data input = {0};
	CC_Task task = CC_Task_create();
	CC_Task_rules_set(task, main_rules, sizeof(main_rules)/sizeof(main_rules[0]));

	if(input_count==1){
		input.str=argv[ARGS_START];
		CC_Task_state_start_set(task, DFT_state_input_get(argv[tags[0]+1]));
	}else{
		struct Input_Group *group=&input.group;
		CC_Task_state_start_set(task, INPUT_GROUP);
		for(int i=0; i<tags[0]-1; i++){
			group->str[i] = argv[i+ARGS_START];
			group->type[i] = DFT_state_input_get(argv[tags[0]+i+1]);
		}
	}

	union CC_Data tmp;

	SEG_BEGIN("Run Task")
	for(int i=0; i<output_type_count; i++)
	{
		tmp=input;
		CC_Task_target_set(task, &tmp);
		CC_Task_state_end_set(task, DFT_state_output_get(argv[tags[1]+i+1]));
		CC_Task_additional_set(task, argv[tags[2]+i+1]);
		CC_solve(task);
	}
	SEG_END

		CC_Task_free(task);

}
