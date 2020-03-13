#include "tool.h"
#include <getopt.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void rebuild_usage()
{
	puts("Usage: dbtool <-r|--rebuild> <-s|--srouce> <sourcefile> <-d|--destination> <destinationfile>");
	exit(0);
}

int rebuild_main(int argc, char *argv[])
{
	option options[] = 
	{ 
		{ "source",      required_argument, NULL, 's' },
		{ "destination", required_argument, NULL, 'd' },
		{ 0,             0,                 0,     0  }
	};
	char *src = NULL, *dst = NULL;
	for(int c; (c = getopt_long(argc, argv, "s:d:", options, NULL)) != -1;)
	{
		switch(c)
		{
		case 's' : src = strdup(optarg); break;
		case 'd' : dst = strdup(optarg); break;
		default  : rebuild_usage();
		}
	}
	if(!src || !dst) rebuild_usage();
	lcore::PageRebuild rebuild(dst, src);
	size_t corrupt_count;
	printf("rebuilding %s...\n", src);
	printf("rebuild %ld items", rebuild.action(&corrupt_count));
	if(corrupt_count) printf(" with %ld corrupt items\n", corrupt_count );
	else printf("\n");
	return 0;
}

void monitor_usage()
{
	puts("Usage: dbtool <-m|--monitor> <-d|--dbfile> <dbfilepath>");
	exit(0);
}

int monitor_main(int argc, char *argv[])
{
	option options[] = 
	{ 
		{ "dbfile",      required_argument, NULL, 'd' },
		{ 0,             0,                 0,     0  }
	};
	char *dbfile = NULL;
	for(int c; (c = getopt_long(argc, argv, "d:", options, NULL)) != -1;)
	{
		switch(c)
		{
		case 'd' : dbfile = strdup(optarg); break;
		default  : monitor_usage();
		}
	}
	if(!dbfile) monitor_usage();
	lcore::PageMonitor monitor(dbfile);
	{
		time_t last_check;
		const lcore::Performance *p = monitor.performance_init(&last_check);
		fprintf(stdout, "Performance at %ld / %s", last_check, ctime(&last_check));
		p->dump();
	}
	while(1)
	{
		if(monitor.monitor())
		{
			time_t t_org, t_cur, t_new;
			const lcore::Performance *p1 = monitor.performance_from_begin(&t_org, &t_new);
			const lcore::Performance *p2 = monitor.performance_from_checkpoint(&t_cur, &t_new);
			fprintf(stdout, "Performance from begin (%ld ~ %ld) elapsed %ld seconds:\n", t_org, t_new, t_new - t_org );
			p1->dump(1.0 * (t_new - t_org));
			fprintf(stdout, "Performance from checkpoint (%ld ~ %ld) elapsed %ld seconds:\n", t_cur, t_new, t_new - t_cur );
			p2->dump(1.0 * (t_new - t_cur));
		}
		sleep(1);
	}
	return 0;
}

void usage()
{
	printf("compile time: %s %s\n", __DATE__, __TIME__);
	puts("Usage: dbtool [-v|--version] [-r|--rebuild] [-m|--monitor] ....");
	exit(0);
}

void version()
{
	printf("compile time: %s %s\n", __DATE__, __TIME__);
	exit(0);
}

int main(int argc, char *argv[])
{
	option options[] = 
	{
		{ "version",     no_argument,       NULL, 'v' },
		{ "rebuild",     no_argument,       NULL, 'r' },
		{ "monitor",     no_argument,       NULL, 'm' },
		{ 0,             0,                 0,     0  }
	};

	for(int c; (c = getopt_long(argc, argv, "vrm", options, NULL)) != -1;)
	{
		switch(c)
		{
		case 'r': return rebuild_main(argc, argv);
		case 'm': return monitor_main(argc, argv);
		case 'v': version();
		default : usage();
		}
	}
	usage();
	return 0;
}
