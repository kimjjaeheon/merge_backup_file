#include "merge_backup_file.h"
#include "init.h"

char backup_files_home[128];
long file_cnt = 0;
int  backup_if_list[3][BACKUP_IF_NUM];
char backup_list[128][FILE_NAME_LEN];
int  backup_list_cnt;

_Bool init()
{
	if (!init_config()) return false;
	printf("\n (i) init complete \n");

	return true;
}

_Bool init_config(void)
{
	char ret_str[128], log_dir[128];
	int i, flag;

	if (get_nth_token_in_file_section(getenv("TRAN_SVR_CFG_FILE"), "DB_LOADER_BACKUP", "BACKUP_FILES_HOME", 1, ret_str) < 0)
	{
		return false;
	}
	else
	{
		if (ret_str[0] == '/')
		{
			strcpy(backup_files_home, ret_str);
		}
		else
		{
			sprintf(backup_files_home, "%s/%s", getenv("TRAN_SVR_HOME"), ret_str);
		}
	}

    if (get_nth_token_in_file_section(getenv("TRAN_SVR_CFG_FILE"), "COMMON", "PROCESS_LOG_HOME", 1, ret_str) < 0)
    {
        return false;
    }
    else
    {
        if (ret_str[0] == '/')
        {
            sprintf(log_dir,"%s/%s", ret_str, MY_PROCESS_NAME);
        }
        else
        {
            sprintf(log_dir,"%s/%s/%s", getenv("TRAN_SVR_HOME"), ret_str, MY_PROCESS_NAME);
        }
        open_log(log_dir, MY_PROCESS_NAME, true);
    }

	if (get_nth_token_in_file_section(getenv("TRAN_SVR_CFG_FILE"), "DB_LOADER_BACKUP", "FILE_MAX_COUNT", 1, ret_str) < 0)
	{
		log_printf("<!> FILE_MAX_COUNT 을 찾을수 없습니다 (svr.cfg)"); 
		return false;
	}
	else
	{
		file_cnt = strtol(ret_str, NULL, 10);

		if (file_cnt < MIN_FILE_CNT)
		{
			log_printf("<!> FILE_MAX_COUNT error! set %d (min %d)", file_cnt, MIN_FILE_CNT);
			return false;
		}
	}

	memset(backup_if_list, 0x00, sizeof(backup_if_list));

    for (i = 0; i < BACKUP_IF_NUM; i++)
    {
        /* LOG */
        if (get_nth_token_in_file_section(getenv("TRAN_SVR_CFG_FILE"), "DB_LOADER_BACKUP", "LOG", i + 1, ret_str) < 0)
        {
			log_printf("<!> svr.cfg [DB_LOADER_BACKUP] section LOG 개수 확인");
        	return false;
        }
        else
        {
			backup_if_list[LOAD_FILE_LOG][i] = strtol(ret_str, NULL, 10);
        }
        /* ERRLOG */
        if (get_nth_token_in_file_section(getenv("TRAN_SVR_CFG_FILE"), "DB_LOADER_BACKUP", "ERRLOG", i + 1, ret_str) < 0)
        {
			log_printf("<!> svr.cfg [DB_LOADER_BACKUP] section ERRLOG 개수 확인");
        	return false;
        }
        else
        {
			backup_if_list[LOAD_FILE_ERRLOG][i] = strtol(ret_str, NULL, 10);
        }
        /* INFOLOG */
        if (get_nth_token_in_file_section(getenv("TRAN_SVR_CFG_FILE"), "DB_LOADER_BACKUP", "INFOLOG", i + 1, ret_str) < 0)
        {
			log_printf("<!> svr.cfg [DB_LOADER_BACKUP] section INFOLOG 개수 확인");
        	return false;
        }
        else
        {
			backup_if_list[LOAD_FILE_INFOLOG][i] = strtol(ret_str, NULL, 10);
        }
    }


	if (get_nth_token_in_file_section(getenv("TRAN_SVR_CFG_FILE"), "DB_LOADER_BACKUP", "BACKUP_LIST_CNT", 1, ret_str) < 0)
    {
		log_printf("<!> svr.cfg BACKUP_LIST_CNT 개수 확인\n");
        return false;
    }
    else
    {
		backup_list_cnt = strtol(ret_str, NULL, 10);
    }
	
	memset(backup_list, 0x00, sizeof(backup_list));
	for (i = 0; i < backup_list_cnt; i++)
	{
		if (get_nth_token_in_file_section(getenv("TRAN_SVR_CFG_FILE"), "DB_LOADER_BACKUP", "BACKUP_LIST_ONEDAY", i+1, ret_str) < 0)
		{
			return false;
		}
		else
		{
			if (!memcmp("LOG" , ret_str, 3))
			{
				sprintf(backup_list[i], "tran_log");
			}
			if (!memcmp("ERRLOG" , ret_str, 6))
			{
				sprintf(backup_list[i], "tran_errlog");
			}
			if (!memcmp("INFOLOG" , ret_str, 7))
			{
				sprintf(backup_list[i], "tran_infolog");
			}
			if (!memcmp("TRX_SUMM" , ret_str, 8))
			{
				if (get_nth_token_in_file_section(getenv("TRAN_SVR_CFG_FILE"), "DB_LOADER_BACKUP", "TRX_SUMM", 1, ret_str) < 0)
				{
					log_printf("<!> svr.cfg [DB_LOADER_BACKUP] section TRX_SUMM 을 못가져왔습니다.\n");
					return false;
				}
				else
				{
					flag = strtol(ret_str, NULL, 10);
					if (flag == 1)
					{
						sprintf(backup_list[i], "tran_statis");
					}
					else
					{
						log_printf("<!> svr.cfg [DB_LOADER_BACKUP] section TRX_SUMM 개수 확인\n");
					}
				}
			}
		}
	}

	printf("(i) backup_files_home: %s \n", backup_files_home);
	printf("(i) file_cnt         : %ld \n", file_cnt);
	printf("(i) config-file(%s) read complete. \n", getenv("TRAN_SVR_CFG_FILE"));

	return true;
}
