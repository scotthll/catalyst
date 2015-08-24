#ifndef __CATALYST_OPTIONS_H__
#define __CATALYST_OPTIONS_H__

#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <getopt.h>

enum {
	HELP,
	VERSION,
	MAX_ARGS
};

static struct option acfg_opts[] = {
	{ .name = "help", .has_arg = 0, .val = 'h' },
	{ .name = "version", .has_arg = 0, .val = 'v' },
	{ NULL }, 
};

struct help_opt_str {
	char index;
	char *arg;
	char *help_info;
};

#define CT_ARGS_OPTS	"-l:w:hvs:d:i:0:1:2:3:4:5:6:7:8:9"

extern int ct_opt_version();
extern int ct_opt_args();
extern int ct_opt_signals();
extern int ct_opt_help();
extern int resolve_args(int argc, char ** argv);

#endif
