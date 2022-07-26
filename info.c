#include "info.h"
#include <stdlib.h>
#include <time.h>
#include <string.h>


#define INFO_GENERATE_STRING(STRING) [STRING] = "["#STRING"]",

const char *info_internal_type_ids[] = {
	INFO_FOREACH_FUNC(INFO_GENERATE_STRING)
	[FATAL] = "FATAL ERROR"
};

#define MAX_SEG_NEST 128
struct Segment
{
	clock_t start;
	const char *name;
};

struct Segment seg_data[MAX_SEG_NEST];
size_t seg_index = 0;

size_t indent = 0;


struct info_Buf
{
	char *buf;
	size_t size;
	size_t cur;
	size_t display_length;
};

#define OUT_BUF_SIZE 2048
#define PREFIX_BUF_SIZE 512

static char out_buf[OUT_BUF_SIZE];
struct info_Buf info_internal_Buf_out = {out_buf, OUT_BUF_SIZE, 0};

static char prefix_buf[PREFIX_BUF_SIZE];
struct info_Buf info_internal_Buf_prefix = {prefix_buf, PREFIX_BUF_SIZE, 0};

bool hold= false;

struct info_Msg held= {ZERO};

void info_internal_write(FILE *f, const char *str, size_t length)
{
	fwrite(str,1,length,f);
	fflush(f);
}

void info_internal_putc(FILE *f, char c)
{
	fputc(c,f);
	fflush(f);
}

FILE *info_internal_type_get_file(enum INFO_TYPE type)
{
	switch(type)
	{
		case ERROR:
		case FATAL:
			return stderr;
		default:
			return stdout;
	}
}


#define ANSI_RESET_STR "\033[0m"

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
static HANDLE Console_buffer_old, Console_buffer;

void info_internal_console_windows_ANSI_disable(){
	printf(ANSI_RESET_STR);
	HANDLE h=GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD dwMode = 0;
	GetConsoleMode(h, &dwMode);
	dwMode &= ~0x4;
	SetConsoleMode(h,dwMode);
}

void info_internal_console_windows_ANSI_enable(){
	HANDLE h=GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD dwMode = 0;
	GetConsoleMode(h, &dwMode);
	dwMode |= 0x4;
	SetConsoleMode(h,dwMode);
}
#define ENABLE_ANSI info_internal_console_windows_ANSI_enable();
#define DISABLE_ANSI info_internal_console_windows_ANSI_disable();
#else
#define ENABLE_ANSI
#define DISABLE_ANSI printf(ANSI_RESET_STR);
#endif

static void info_internal_print_n(FILE *f, int n, char c)
{
	for(; n>0; n--)
		info_internal_putc(f, c);
}

static void info_internal_indent(FILE *f, int indent)
{
	info_internal_print_n(f, indent, '\t');
}

static void info_internal_linebreak(FILE *f, int new_lines)
{
	info_internal_print_n(f, new_lines, '\n');
}

static void info_internal_flush(FILE *f, size_t indent)
{
	size_t i=0, start=0;
	const char * str = info_internal_Buf_out.buf;

	info_internal_write(f, info_internal_Buf_prefix.buf, info_internal_Buf_prefix.cur);

	while(i<info_internal_Buf_out.cur)
	{
		if(out_buf[i]=='\n')
		{
			if(start!=0)
				info_internal_print_n(f, info_internal_Buf_prefix.display_length , ' ');
			info_internal_indent(f, indent);
			info_internal_write(f,str+start,++i-start);
			start=i;
		}
		i++;
	}
	if(start!=info_internal_Buf_out.cur){
		if(start!=0)
			info_internal_print_n(f, info_internal_Buf_prefix.display_length , ' ');
		info_internal_indent(f, indent);
		info_internal_write(f,str+start,i-start);
	}

	info_internal_Buf_out.cur=0;
	info_internal_Buf_prefix.cur=0;
	info_internal_Buf_prefix.display_length=0;
	DISABLE_ANSI
}

static void info_internal_Buf_vprintf(struct info_Buf *buf, const char *fmt, va_list args)
{
	int res = vsnprintf(buf->buf+buf->cur, buf->size-buf->cur, fmt, args);
	if(res>0){
		buf->cur+=res;
		buf->display_length+=res;
	}
}

void info_internal_Buf_printf(struct info_Buf *buf, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	info_internal_Buf_vprintf(buf, fmt, args);
	va_end(args);
}

static void info_internal_vprintf(const char *fmt, va_list args)
{
	info_internal_Buf_vprintf(&info_internal_Buf_out, fmt, args);
}

void info_internal_printf(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	info_internal_vprintf(fmt, args);
	va_end(args);
}

void info_internal_Buf_vprintf_no_display(struct info_Buf *buf, const char *fmt, va_list args)
{
	size_t tmp=buf->display_length;
	info_internal_Buf_vprintf(buf, fmt, args);
	buf->display_length=tmp;
}

void info_internal_Buf_printf_no_display(struct info_Buf *buf, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	info_internal_Buf_vprintf_no_display(buf, fmt, args);
	va_end(args);
}

void info_internal_Buf_ANSI_reset(struct info_Buf *buf)
{
	info_internal_Buf_printf_no_display(buf, ANSI_RESET_STR);
}

void info_internal_Buf_ANSI_set(struct info_Buf *buf, struct info_ANSI *ansi)
{
	ENABLE_ANSI
	if(ansi->reset)
		info_internal_Buf_ANSI_reset(buf);
	else
		info_internal_Buf_printf_no_display(buf, "\033[38;2;%d;%d;%dm", ansi->r, ansi->g, ansi->b);
}

void info_printf(const char *fmt, ...)
{

	va_list args;
	va_start(args, fmt);
	info_internal_vprintf(fmt, args);
	va_end(args);
	if(!hold && held.type == ZERO)
		info_internal_flush(stdout, indent);
}


static void info_internal_Msg_flush(struct info_Msg *msg)
{
	FILE * f = info_internal_type_get_file(msg->type);
	info_internal_flush(f, msg->indent);
	DISABLE_ANSI
	info_internal_linebreak(f, msg->new_lines);
	if(msg->handle)
		msg->handle();
}

static void info_internal_prefix(enum INFO_TYPE type)
{

#ifdef INFO_TIMESTAMP
	time_t time_rel;
	struct tm *time_struct;
	time(&time_rel);
	time_struct = gmtime(&time_rel);
	info_internal_Buf_printf(&info_internal_Buf_prefix, "[%d:%.2d:%.2d]", time_struct->tm_hour, time_struct->tm_min, time_struct->tm_sec);
	//info_internal_printf("[%d:%.2d:%.2d]", time_struct->tm_hour, time_struct->tm_min, time_struct->tm_sec);
#endif

	if(info_types_ANSI[type].enable)
		info_internal_Buf_ANSI_set(&info_internal_Buf_prefix, info_types_ANSI + type);

	info_internal_Buf_printf(&info_internal_Buf_prefix, " %-7s ", info_internal_type_ids[type]);
	//info_internal_printf("%-7s ", info_internal_type_ids[msg->type]);

	info_internal_Buf_ANSI_reset(&info_internal_Buf_prefix);
}

void info_prefix_offset(void)
{
	info_internal_prefix(INFO);
	memset(info_internal_Buf_prefix.buf, ' ', info_internal_Buf_prefix.cur);
}


static void info_internal_Msg_write(struct info_Msg *msg)
{
	info_internal_prefix(msg->type);

	if(msg->ANSI.enable)
		info_internal_Buf_ANSI_set(&info_internal_Buf_out, &msg->ANSI);

	if(msg->func_name)
		info_internal_printf("%s: ", msg->func_name);
	info_internal_vprintf(msg->fmt,msg->args);
}

void info_hold(void)
{
	hold=true;
}

void info_internal_release(void)
{

	if(held.type!=ZERO){
		info_internal_Msg_flush(&held);
		held.type=ZERO;
	}
	if(info_internal_Buf_out.cur)
		info_internal_flush(stdout, indent);
}

void info_release()
{
	info_internal_release();
	hold=false;
}

static void info_internal_Msg_handle(struct info_Msg *msg)
{
	info_internal_release();

	// write msg into out_buf
	info_internal_Msg_write(msg);

	if(hold) {
		held = *msg;
		hold=false;
	} else {
		info_internal_Msg_flush(msg);
	}
}

void info_Msg_create(struct info_Msg msg, const char *fmt, ...)
{
	va_start(msg.args, fmt);

	msg.fmt=fmt;
	msg.indent=indent;

	info_internal_Msg_handle(&msg);
	va_end(msg.args);
}


void info_indent(int n)
{
	indent+=n;
	if(indent<0 || n==0)
		indent=0;
}

void info_handle_fatal(void)
{
	exit(1);
}

#define INFO_SEG_MSG (struct info_Msg){SEG, NULL, NULL, 1, info_types_ANSI[SEG]}

void info_seg_start(const char *func_name, const char *name)
{
	HOLD
	info_Msg_create(INFO_SEG_MSG, "Segment ");

	if(name)
		PRINT("'%s' ", name);
	PRINT("started!");

	if(seg_index>=MAX_SEG_NEST){
		PRINT("Cannot record time, MAX_SEG_NEST (%d) reached", MAX_SEG_NEST)
		goto end;
	}

	seg_data[seg_index].start=clock();
	seg_data[seg_index].name=name;

	end:
	RELEASE
	seg_index++;
	info_indent(1);
}

void info_seg_end(const char *func_name)
{
	seg_index--;
	info_indent(-1);

	HOLD
	info_Msg_create(INFO_SEG_MSG, "Segment ");
	if(seg_index<MAX_SEG_NEST){
		if(seg_data[seg_index].name)
			PRINT("'%s' ", seg_data[seg_index].name);
	}
	PRINT("has ended");
	if(seg_index<MAX_SEG_NEST)
		PRINT(" %gms", (double)(clock()-seg_data[seg_index].start)/CLOCKS_PER_SEC*1000);

	PRINT("!")
	RELEASE
}

