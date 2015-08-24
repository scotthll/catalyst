#include "ct_options.h"

static struct option acfg_opts[] = {
	{ .name = "help", .has_arg = 0, .val = 'h' },
	{ .name = "version", .has_arg = 0, .val = 'v' },
	{ NULL }, 
};

int ct_opt_version()
{
	printf("Catalyst Library -- Options \n");
	printf("    version: %d.%d.%d\n", main_version, stable_version, patch_version);
	return 0;
}

int ct_opt_args()
{
	int i = 0;
	int print_len = 0;
	for (i = 0; i < MAX_ARGS; i++)
	{
		print_len = 0;
		//print_len = printf("  --%s | -%c", acfg_opts[i].name, acfg_opts[i].val);
		print_len = printf("  -%c | --%s", acfg_opts[i].val, acfg_opts[i].name);
		if (acfg_opts[i].has_arg != 0)
		{
			print_len += printf(" %s", acfg_opts_str[i].arg);
		}
		int k = 0;
		int last = 0;
		if (print_len < 40)
		{
			last = 40 - print_len;
		}
		else
		{
			last = 1;
		}
		for (k = 0; k < last; k++)
		{
			printf(" ");
		}
		printf(" %s", acfg_opts_str[i].help_info);
		printf("\n");
	}
	return 0;
}

int ct_opt_signals()
{
	printf("\n");
	printf("  SIGUSR1 is for WAN UP       (kill -s USR1 `pidof acfgm`)\n");
	printf("  SIGUSR2 is for WAN DOWN     (kill -s USR2 `pidof acfgm`)\n");
	return 0;
}

int ct_opt_help()
{
	int rc = 0;
	ct_opt_version();
	printf("\n");
	ct_opt_args();
//	ct_opt_signals();
	return rc;
}

int resolve_args(int argc, char ** argv)
{
	int rc = 0;
	int opt = 0;
	while ((opt = getopt_long(argc, argv, ACFGM_ARGS_OPTS,
				acfg_opts, NULL)) != -1)
	{
		switch (opt)
		{
			case 'h':
				ct_opt_help();
				exit(1);
				break;
			case 'v':
				ct_opt_version();
				exit(2);
				break;
			/*
			 case 'xxxxx':
				break;
			*/
			default:
				break;
		}
	}	
	return rc;
}
