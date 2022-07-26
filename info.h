#include <stdarg.h>
#include <stdnoreturn.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

//TODO : multifile streams
//TODO : fix ansi ->hmmm.png
//TODO : restructure maybe

void info_indent(int n);
void info_seg_start(const char *func_name, const char *name);
void info_seg_end(const char *func_name);
void info_printf(const char *fmt, ...);
void info_handle_fatal(void);
void info_prefix_offset(void);

struct info_Msg;
void info_Msg_create(struct info_Msg msg, const char *fmt, ...);


struct info_ANSI
{
	bool enable;
	uint8_t r,g,b;
	bool reset;
};

#define INFO_COLOR(R,G,B) (struct info_ANSI){1, R, G, B, 0}
#define INFO_ANSI_RESET (struct info_ANSI){1,0,0,0,1}
// Thanks to https://stackoverflow.com/questions/9907160/how-to-convert-enum-names-to-string-in-c
#define INFO_FOREACH_FUNC(FUNC) 	    \
        FUNC(INFO)     		    \
        FUNC(SUCCESS)	        \
        FUNC(ERROR)      	    \
        FUNC(SEG)     	        \

#define INFO_GENERATE_ENUM(ENUM) ENUM,
#define INFO_GENERATE_ANSI(ENUM) [ENUM] = (struct info_ANSI){0},

enum INFO_TYPE
{
	ZERO,
	INFO_FOREACH_FUNC(INFO_GENERATE_ENUM)
	FATAL
};


static struct info_ANSI info_types_ANSI[] = {
	// set defaults
	INFO_FOREACH_FUNC(INFO_GENERATE_ANSI)

	[INFO] = INFO_COLOR(0, 200, 255),
	[SEG] = INFO_COLOR(200, 255, 0),
	[ERROR] = INFO_COLOR(200, 70, 70),
	[FATAL] = INFO_COLOR(255, 0, 0),
	[SUCCESS] = INFO_COLOR(0, 255, 0),
};

struct info_Msg
{
	enum INFO_TYPE type;
	const char *func_name;
	void (*handle)(void);
	size_t new_lines;
	struct info_ANSI ANSI;

	// set internally
	size_t indent;
	const char *fmt;
	va_list args;
};


#ifndef INFO_LVL
	#define INFO_LVL 4
#endif

#define INFO_TIMESTAMP


#if INFO_LVL>3
	void info_release(void);
	void info_hold(void);
	#define INFO(...) info_Msg_create((struct info_Msg){INFO, __FUNCTION__, NULL, 1, {NULL}}, __VA_ARGS__);
	#define SEG_BEGIN(x) info_seg_start(__FUNCTION__, x);
	#define SEG_END info_seg_end(__FUNCTION__);
	#define INDENT(n) info_indent(n);
	#define HOLD info_hold();
	#define RELEASE info_release();
	#define PRINT(...)  info_printf(__VA_ARGS__);
	#define PREFIX_OFFSET info_prefix_offset();
#else
	#define INFO(...) ;
	#define SEG_BEGIN(x) ;
	#define SEG_END ;
	#define INDENT(n) ;
	#define HOLD ;
	#define RELEASE ;
	#define PRINT(...) ;
	#define PREFIX_OFFSET ;
#endif

#if INFO_LVL>2
	#define SUCCESS(...) info_Msg_create((struct info_Msg){SUCCESS, __FUNCTION__, NULL, 2, info_types_ANSI[SUCCESS]}, __VA_ARGS__);
#else
	#define SUCCESS(...) ;
#endif

#if INFO_LVL>1
	#define ERROR(...) info_Msg_create((struct info_Msg){ERROR, __FUNCTION__, NULL, 2, info_types_ANSI[ERROR]}, __VA_ARGS__);
#else
	#define ERROR(...) ;
#endif

#if INFO_LVL>0
	#define FATAL(...) info_Msg_create((struct info_Msg){FATAL, __FUNCTION__, info_handle_fatal, 2, info_types_ANSI[FATAL]}, __VA_ARGS__);
#else
	#define FATAL(...) info_handle_fatal();
#endif

