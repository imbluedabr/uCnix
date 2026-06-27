#pragma once

#define SHELL_BUILTIN_COUNT 7
#define BUILTIN_COUNT 9

extern char line_buff[32];

int builtin_exit(int argc, char** argv);
int builtin_pwd(int argc, char** argv);
int builtin_cd(int argc, char** argv); 
int builtin_cat(int argc, char** argv); 
int builtin_ls(int argc, char** argv);
int builtin_uname(int argc, char** argv);
int builtin_kill(int argc, char** argv);

int builtin_sh(int argc, char** argv);
int builtin_getty(int argc, char** argv);

char* strip_path(char* str); 
int get_builtin_index(const char* name); 
int execute_builtin(int index, int argc, char** argv);

