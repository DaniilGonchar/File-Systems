#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <dirent.h>
#include <ctype.h>

const char proc_directory_key[] = "/proc/";
const char stat_key[] = "/stat";

struct stat_file
{
	int pid;
	char name[256];
	char status;
};

int main(void)
{
	DIR *dir;
	FILE *file_status;
	
	struct dirent *entry;
	struct stat_file statFile;
	
	dir = opendir(proc_directory_key);
	
	if (!dir)
	{
		perror("opendir");
		exit(1);
	};
	
	while ((entry = readdir(dir)))
	{
		if (!isdigit(entry->d_name[0])) continue;
		char tmp_proc[256];
		snprintf(tmp_proc, sizeof(tmp_proc), "%s%s%s", proc_directory_key, entry->d_name, stat_key);
		
		file_status = fopen(tmp_proc, "r");
		
		if (!file_status)
		{
			perror("fopen");
			exit(1);
		}
		
		fscanf(file_status, "%d %s %c", &statFile.pid, statFile.name, &statFile.status);
		
		fclose(file_status);
		
		printf("PID: %s \t status: %c \t name: %s\n", entry->d_name, statFile.status, statFile.name);
	};
	
	if (errno != 0)
	{
		perror("reading");
		exit(1);
	}
	
	closedir(dir);
};
