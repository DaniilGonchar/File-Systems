#include <stdio.h>
#include <sys/stat.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

const char proc_directory_key[] = "/proc/";

int main(void)
{
	errno = 0;
	char directory_fd[64];
	DIR *proc_dir;
	struct dirent *proc_entry;
	
	DIR *fd_dir;
	struct dirent *fd_entry;
	struct stat status_buffer;
	
	proc_dir = opendir(proc_directory_key);
	if (!proc_dir)
	{
		printf("Cant open %s", proc_directory_key);
		perror("opendir proc");
		exit(1);
	};
	
	while ((proc_entry = readdir(proc_dir)) != NULL)
	{
		if (!isdigit(proc_entry->d_name[0])) continue;
		snprintf(directory_fd, sizeof(directory_fd), "/proc/%s/fd/", proc_entry->d_name);
		
		fd_dir = opendir(directory_fd);
		if (!fd_dir)
		{
			printf("Cant open %s\n", directory_fd);
			perror("opendir proc");
			errno = 0;
			continue;
		};
		
		while ((fd_entry = readdir(fd_dir)) != NULL)
		{
			if (!isdigit(fd_entry->d_name[0])) continue;
			snprintf(directory_fd, sizeof(directory_fd), "/proc/%s/fd/%s", proc_entry->d_name, fd_entry->d_name);
			if (lstat(directory_fd, &status_buffer) == -1)
			{
				printf("Error in lstat in %s\n", directory_fd);
				perror("lstat:");
				errno = 0;
				continue;
			}
			
			if (S_ISDIR(status_buffer.st_mode)) continue;
			if (status_buffer.st_uid != getuid()) continue;
			
			unsigned int buffer_size = 1024;
			char *buffer = (char *)calloc(buffer_size, 1);
			readlink(directory_fd, buffer, buffer_size);
			
			while (buffer[buffer_size - 1] != 0)
			{
				buffer_size *= 2;
				buffer = (char *) realloc(buffer, buffer_size);
				readlink(directory_fd, buffer, buffer_size);
			}
			printf("PID: %s, opened file: %s\n", proc_entry->d_name, buffer);
			
			free(buffer);
		}
		if (errno != 0)
		{
			printf("Error reading dir %s\n", fd_dir);
			perror("Readdir");
			errno = 0;
		}
		
		closedir(fd_dir);
	}
	
	
	if (errno != 0)
	{
		printf("Error reading dir %s\n", proc_dir);
		perror("Readdir");
		errno = 0;
	}
	
	closedir(proc_dir);
}

