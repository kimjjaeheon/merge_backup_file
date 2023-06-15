#include "merge_backup_file.h"
#include "find_file_list.h"
#include "init.h"

#include <dirent.h> 

#define NOT_MATCH  1000

extern long file_cnt;

extern int backup_if_list[3][BACKUP_IF_NUM];
extern char backup_list[128][FILE_NAME_LEN];
extern int backup_list_cnt;

int check_backup_list(char* path);
void find_if_number(char* backup_log, int* if_num);

FILE_LIST* find_file_list(char *path) 
{
	DIR *d;
	struct dirent *dir;
	int backup_num = 0;
#ifdef _DEBUG
	const char* tran_log_tmp = "tran_log_jh";
	const char* tran_infolog_tmp = "tran_infolog_jh";
	const char* tran_errlog_tmp = "tran_errlog_jh";
	const char* tran_statis_tmp = "tran_statis/trx_summ_jh";
#else
	const char* tran_log_tmp = "tran_log";
	const char* tran_infolog_tmp = "tran_infolog";
	const char* tran_errlog_tmp = "tran_errlog";
	const char* tran_statis_tmp = "tran_statis/trx_summ";
#endif

	char f_tmp[FILE_NAME_LEN];

	char tran_log[64] = {0x00,};
	char tran_infolog[64] = {0x00,};
	char tran_statis[64] = {0x00,};
	char tran_errlog[64] = {0x00,};

	int  if_idx[BACKUP_IF_NUM] = {0,}; 

	long i = 0, idx = 0;

	DIR_INFO backup_dir[128];

	FILE_LIST* f_list = (FILE_LIST*)malloc(sizeof(FILE_LIST));

	d = opendir(path);
	if (d)
	{
		/* malloc (backup_directory)
			file_cnt -> svr.cfg 파일 */

		if( !(f_list->files = (FILE_INFO*)malloc(sizeof(FILE_INFO) * file_cnt)))
		{
			log_printf("<!> [find_file_list.c] FILE_INFO 동적할당 실패 ");
			return NULL;
		}

		memset(f_list->files, 0x00, sizeof(FILE_INFO) * file_cnt);

		while ((dir = readdir(d)) != NULL) 
		{
			DIR *sub_d;
			struct dirent *sub_dir;

			if (!memcmp(dir->d_name, ".", 1))
			{
				 continue;	
			}
			else
			{
				backup_num = check_backup_list(dir->d_name);
				if (backup_num == NOT_MATCH)
				{
					 continue;
				}

				// tran_log 디렉토리 일 때 
				if (backup_num == LOAD_FILE_LOG)
				{
					find_if_number(dir->d_name, if_idx);
					for (idx = 0; idx < BACKUP_IF_NUM; idx++)
					{
						if (if_idx[idx] != 0)
						{
							DIR *tran_log_d;
							struct dirent *tran_log_dir;

							sprintf(tran_log, "%s/%s/%02d", path, tran_log_tmp, if_idx[idx]);	
		
							tran_log_d = opendir(tran_log);
							while ((tran_log_dir = readdir(tran_log_d)) != NULL)
							{
								if (!memcmp(tran_log_dir->d_name, ".", 1))
								{
									continue;	
								}
							
								memcpy(f_list->files[i].backup_dir, backup_dir[LOAD_FILE_LOG].name, DIR_NAME_LEN);
								f_list->files[i].if_num = if_idx[idx];

								// ex) make /datfs/tranmgr/backup/tran_log/00/TRAN_LOG-2022110814845_0_01.dat
								sprintf(f_tmp, "%s/%s", tran_log, tran_log_dir->d_name);
								memcpy(f_list->files[i].name, tran_log_dir->d_name, FILE_NAME_LEN);
								f_list->files[i].name_len = strlen(f_list->files[i].name); // 이름 길이 
								memcpy(f_list->files[i].path, f_tmp, FILE_NAME_LEN);
								f_list->files[i].dir_num = LOAD_FILE_LOG;
								i++;
							}
							closedir(tran_log_d);
						}
						else
						{
							continue;
						}
					}
				}
				else if (backup_num == LOAD_FILE_INFOLOG)
				{
					find_if_number(dir->d_name, if_idx);
					for (idx = 0; idx < BACKUP_IF_NUM; idx++)
					{
						if (if_idx[idx] != 0)
						{
							DIR *tran_infolog_d;
							struct dirent *tran_infolog_dir;

							sprintf(tran_infolog, "%s/%s/%02d", path, tran_infolog_tmp, if_idx[idx]);	
		
							tran_infolog_d = opendir(tran_infolog);
							while ((tran_infolog_dir = readdir(tran_infolog_d)) != NULL)
							{
								if (!memcmp(tran_infolog_dir->d_name, ".", 1)) 
								{
									continue;	
								}
							
								memcpy(f_list->files[i].backup_dir, backup_dir[LOAD_FILE_INFOLOG].name, DIR_NAME_LEN);
								f_list->files[i].if_num = if_idx[idx];

								// ex) make /datfs/tranmgr/backup/tran_log/00/TRAN_LOG-2022110814845_0_01.dat
								sprintf(f_tmp, "%s/%s", tran_infolog, tran_infolog_dir->d_name);
								memcpy(f_list->files[i].name, tran_infolog_dir->d_name, FILE_NAME_LEN);
								memcpy(f_list->files[i].path, f_tmp, FILE_NAME_LEN);
								f_list->files[i].name_len = strlen(f_list->files[i].name); // 이름 길이
								f_list->files[i].dir_num = LOAD_FILE_INFOLOG;
								i++;
							}
							closedir(tran_infolog_d);
						}
						else
						{
							continue;
						}
					}
/*
					sprintf(tran_infolog, "%s%s", path, tran_infolog_tmp);	
	
					memcpy(backup_dir[LOAD_FILE_INFOLOG].name, tran_infolog, FILE_NAME_LEN);
					printf("backup_dir : %s\n", backup_dir[LOAD_FILE_INFOLOG].name);
			
                    sub_d = opendir(backup_dir[LOAD_FILE_INFOLOG].name);
                    while ((sub_dir = readdir(sub_d)) != NULL)
                    {   
                        if (!memcmp(sub_dir->d_name, ".", 1)) continue;
                        
                        memcpy(f_list->files[i].backup_dir, backup_dir[LOAD_FILE_INFOLOG].name, DIR_NAME_LEN);
                        
                        sprintf(f_tmp, "%s/%s", tran_infolog, sub_dir->d_name);
                        memcpy(f_list->files[i].name, sub_dir->d_name, FILE_NAME_LEN);
                        memcpy(f_list->files[i].path, f_tmp, FILE_NAME_LEN);
                        f_list->files[i].dir_num = LOAD_FILE_INFOLOG;
                        i++;

                     
                    }
					closedir(sub_d);
*/
				}
				else if (backup_num == LOAD_FILE_ERRLOG)
				{
					find_if_number(dir->d_name, if_idx);
					for (idx = 0; idx < BACKUP_IF_NUM; idx++)
					{
						if (if_idx[idx] != 0)
						{
							DIR *tran_errlog_d;
							struct dirent *tran_errlog_dir;

							sprintf(tran_errlog, "%s/%s/%02d", path, tran_errlog_tmp, if_idx[idx]);	
		
							tran_errlog_d = opendir(tran_errlog);
							while ((tran_errlog_dir = readdir(tran_errlog_d)) != NULL)
							{
								if (!memcmp(tran_errlog_dir->d_name, ".", 1))
								{
									continue;
								}
				
								memcpy(f_list->files[i].backup_dir, backup_dir[LOAD_FILE_LOG].name, DIR_NAME_LEN);
                                f_list->files[i].if_num = if_idx[idx];

                                // ex) make /datfs/tranmgr/backup/tran_errlog/00/TRAN_LOG-2022110814845_0_01.dat
                                sprintf(f_tmp, "%s/%s", tran_errlog, tran_errlog_dir->d_name);
                                memcpy(f_list->files[i].name, tran_errlog_dir->d_name, FILE_NAME_LEN);
                                memcpy(f_list->files[i].path, f_tmp, FILE_NAME_LEN);
                                f_list->files[i].name_len = strlen(f_list->files[i].name); // 이름 길이 
                                f_list->files[i].dir_num = LOAD_FILE_ERRLOG;
                                i++;
							}
							closedir(tran_errlog_d);
						}
						else
						{
							continue;
						}	
					}
				}
				else if (backup_num == LOAD_FILE_TRX_SUMM)
				{
					sprintf(tran_statis, "%s/%s", path, tran_statis_tmp);	

					memcpy(backup_dir[LOAD_FILE_TRX_SUMM].name, tran_statis, FILE_NAME_LEN);

                    sub_d = opendir(backup_dir[LOAD_FILE_TRX_SUMM].name);
                    while ((sub_dir = readdir(sub_d)) != NULL)
                    {   
                        if (!memcmp(sub_dir->d_name, ".", 1))
						{
							continue;
						}
                        
                        memcpy(f_list->files[i].backup_dir, backup_dir[LOAD_FILE_TRX_SUMM].name, DIR_NAME_LEN);
                        
                        sprintf(f_tmp, "%s/%s", tran_statis, sub_dir->d_name);
                        memcpy(f_list->files[i].name, sub_dir->d_name, FILE_NAME_LEN);
                        memcpy(f_list->files[i].path, f_tmp, FILE_NAME_LEN);
						f_list->files[i].name_len = strlen(f_list->files[i].name); // 이름 길이
                        f_list->files[i].dir_num = LOAD_FILE_TRX_SUMM;
                        i++;
                    }
					closedir(sub_d);
				}
				else
				{
					continue;
				}
			}
		}
		closedir(d);
	}
	else
	{
		printf("No such directory\n");
		return NULL;
	}

	f_list->total_file_cnt = i;

	return f_list;
}

int check_backup_list(char* path)
{
	int i = 0;
	int path_len = strlen(path);
	int tran_log_len = strlen("tran_log");
	int tran_infolog_len = strlen("tran_infolog");
	int tran_statis_len = strlen("tran_statis");
	int tran_errlog_len = strlen("tran_errlog");

	for(i = 0; i < backup_list_cnt ; i++)
	{
		if (!memcmp(path, backup_list[i], path_len))
		{
			if (!memcmp(backup_list[i], "tran_log", tran_log_len))
			{
				return LOAD_FILE_LOG;
			}
			if (!memcmp(backup_list[i], "tran_errlog", tran_errlog_len))
			{
				return LOAD_FILE_ERRLOG;
			}
			if (!memcmp(backup_list[i], "tran_infolog", tran_infolog_len))
			{
				return LOAD_FILE_INFOLOG;
			}
			if (!memcmp(backup_list[i], "tran_statis", tran_statis_len))
			{
				return LOAD_FILE_TRX_SUMM;
			}
		}
	}
	
    return NOT_MATCH;
}
// IF 별로 찾기
void find_if_number(char* backup_log, int* if_idx) 
{
	int i, j;

	//sprintf(tran_log, "%s%s", path, tran_log_tmp);	
	if (check_backup_list(backup_log) == LOAD_FILE_LOG)
	{
		for (i = 0, j = 0; i < BACKUP_IF_NUM; i++)
		{
			if (backup_if_list[LOAD_FILE_LOG][i] == 1)
			{
				if_idx[j++] = i;	
			}
		}
	}
	if (check_backup_list(backup_log) == LOAD_FILE_INFOLOG)
	{
		for (i = 0, j = 0; i < BACKUP_IF_NUM; i++)
		{
			if (backup_if_list[LOAD_FILE_INFOLOG][i] == 1)
			{
				if_idx[j++] = i;	
			}
		}
	}
	if (check_backup_list(backup_log) == LOAD_FILE_ERRLOG)
	{
		for (i = 0, j = 0; i < BACKUP_IF_NUM; i++)
		{
			if (backup_if_list[LOAD_FILE_ERRLOG][i] == 1)
			{
				if_idx[j++] = i;	
			}
		}
	}
}

