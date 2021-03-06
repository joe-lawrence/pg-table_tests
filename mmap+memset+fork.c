#include <stdio.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

int flag = MAP_FIXED;
int do_fork = 0;
int demo = 0;
int debug_print = 0;
int i_from = 30;
int i_till = 39;
int map_size = 4096;
int set_size = 4096;

void usage(){
	printf("Usage: ./mmap+memset+fork [OPTION]\n");	
	printf("Tests virtual adress space updates with fork and mmap options\n\n");
	printf("-d, --debug		prints out the PID and address touched by memset\n");
	printf("-k, --keystroke		same as debug, but requires a char to be entered to show each line\n");
	printf("-h, --help              prints out help message\n");
	printf("-f, --fork_child        causes program to fork a child process\n");
	printf("-b, --begin             mmaps and memsets from 2^(arg)\n");
	printf("-e, --end               mmaps and memsets till 2^(arg)\n");
	printf("-m, --map_size          mmaps given size\n");
	printf("-n, --set_size          memsets given size\n");
	printf("-u, --map_populate      sets MAP_POPULATE flag in mmap\n");
	printf("-a, --map_anonymous     sets MAP_ANONYMOUS flag in mmap\n");
	printf("-s, --map_shared        sets MAP_SHARED flag in mmap\n");
	printf("-p, --map_private       sets MAP_PRIVATE flag in mmap\n");
	exit(EXIT_FAILURE);
}


void parse_options (int argc, char *argv[]){
	int long_index = 0;
	int option;
	struct option long_options [] =
	{
		{"debug", no_argument, 0, 'd'},
		{"keystroke", no_argument, 0, 'k'},
		{"help", no_argument, 0, 'h'},
		{"map_shared", no_argument, 0, 's'},
		{"map_private", no_argument, 0, 'p'},
		{"fork_child", no_argument, 0, 'f'},
		{"begin", required_argument, 0, 'b'},
		{"end", required_argument, 0, 'e'},
		{"map_size", required_argument, 0, 'm'},
		{"set_size", required_argument, 0, 'n'},
		{"map_populate", required_argument, 0, 'u'},
		{"map_anonymous", required_argument, 0, 'a'},
		{0,0,0,0}
	};

	while ((option = getopt_long(argc,argv,"dkhspfb:e:m:n:",long_options, &long_index)) != -1){
		switch (option){

			case 'd' :
				debug_print = 1;
				break;
			case 'f' :
				do_fork = 1;
				break;

			case 'k' :
				demo = 1;
				debug_print = 1;
				break;
			case 'h':
				usage();
			case 's':
				flag |= MAP_SHARED;
				break;
			case 'p':
				flag |= MAP_PRIVATE;
				break;
			case 'b':
				i_from = atoi (optarg);
				break;
			case 'e':
				i_till = atoi (optarg);
				break;
			case 'm':
				map_size = atoi (optarg);
				break;
			case 'n':
				set_size = atoi (optarg);
				break;
			case 'u':
				flag |= MAP_POPULATE;
				break;
			case 'a':
				flag |= MAP_ANONYMOUS;
				break;

			default:
				usage();
		}
	}

}

int main(int argc, char *argv[]){

	int i;
	void * ptr;
	void * addr;
	void ** array;
	int pid;
	unsigned char write_pattern;


	parse_options (argc,argv);

	array = malloc (sizeof(*array)*(i_till-i_from));
	
	if (!array) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}	


	for (i = i_from; i < i_till; i++){
		addr = (void*)((unsigned long long) 1<<i);
		ptr= mmap(addr, map_size, PROT_READ | PROT_WRITE,
				flag, -1, 0);
		array[i - i_from] = ptr;
		printf ("i=%d,addr=%p,ptr=%p \n",i,addr,ptr);
		if (ptr == MAP_FAILED){
			free(array);
			perror ("mmap");
			return EXIT_FAILURE;
		}
	}
	if (do_fork == 1){
		fork();
	}
	pid = getpid();
	printf ("PID = %d \n",pid);
	write_pattern = pid;
	for (i = 0; i < (i_till - i_from); i++){

		if (demo == 1){
			getchar ();
		}
		memset (array[i], write_pattern, set_size);
		if (debug_print == 1){
			printf ("PID = %d, touched %p \n", pid, array[i]);
		}
	}
	for (i = i_from; i < i_till; i++){
		unsigned char val = *(char*)array [i - i_from];
		if (val!=write_pattern){
			printf("PID = %d, %p read(0x%02x) != write_pattern(0x%02x)\n",
					pid, array [i - i_from], val,
					write_pattern);
		}
	}
	free(array);	
	return 0;
}
