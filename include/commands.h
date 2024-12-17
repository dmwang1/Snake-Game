#ifndef __COMMANDS_H__
#define __COMMANDS_H__

#include "ff.h"
#include "lcd.h"
#include "tty.h"
#include "commands.h"
#include <string.h>
#include <stdio.h>

struct commands_t {
    const char *cmd;
    void      (*fn)(int argc, char *argv[]);
};

void command_shell(void);
void my_command_shell(void);
void parse_command(char *input);
void execute_command(const char *cmd, const char *args);
void print_error(FRESULT res, const char *msg);
void print_file_info(FILINFO *info);
void list_files(const char *path);
void make_directory(const char *path);
void remove_file(const char *path);
void draw_line(int x1, int y1, int x2, int y2);
void draw_rectangle(int x, int y, int w, int h);
void clear_screen(void);
void set_date(WORD year, BYTE month, BYTE day);
void advance_fattime(void);
const char *get_fattime_string(DWORD fattime);

#endif /* __COMMANDS_H_ */
