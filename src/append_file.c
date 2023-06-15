#include "merge_backup_file.h"
#include "append_file.h"

#include <time.h>
#include <errno.h>

int merge(char* path, char* output);
int append_file(char* file, char* output);

static char time_str[32];

static char *get_cur_time(char *format)
{
    struct tm ts;
    struct timeval  curr;
    gettimeofday(&curr, NULL);
    localtime_r((time_t*)&curr.tv_sec, &ts);
    strftime(time_str, sizeof(time_str), format, &ts);
    return time_str;
}

int merge(char* path, char* output)
{   
    append_file(path, output);
    return 0;
}

int append_file(char* file, char* output)
{
	FILE *output_fp, *input_fp = NULL; 
	char ch;
	int ret = 1;

	char   buff[READ_BUF_SIZE];
	char*  buff_pointer = NULL;
	size_t n_size;
	int error = 0;

	if (access(file, F_OK) != 0)
	{
		return NO_SUCH_FILE;
	}

	input_fp  = fopen(file, "r");	
	output_fp = fopen(output, "a+");
	
	do {

		if (input_fp == NULL)
		{
			printf("<!>   input_fp is NULL\n");
			printf("<!>   <append_file.c> input file  : %s\n ", file);
			printf("errno :%d\n", errno);
			break;
		}
		
		if (output_fp == NULL)
		{
			printf("<!>  output_fp is NULL\n");
			printf("<!>  <append_file.c> output file  : %s\n ", output);
			printf("errno :%d\n", errno);
			break;
		}

//	 file line 단위로 읽기
	while (fgets(buff, sizeof(buff), input_fp))
	{
    	log_printf("<!> append_file.c : puts");
		fputs(buff, output_fp);
	}

//     1개 문자씩 읽기
/* 
		while ( (ch = fgetc(input_fp)) != EOF )
		{
			fputc(ch, output_fp);
		}
*/

//     buff 만큼 읽기
/*
		while (0 < (n_size = fread(buff, 1, READ_BUF_SIZE, input_fp)))
		{
			fwrite(buff, 1, n_size, output_fp);
		}
*/
		ret = 0;
	} while(0);

	if(input_fp) 
	{
		fclose(input_fp);	
	}
	if(output_fp)
	{
		fclose(output_fp);	
	}
	
	return ret;
}

