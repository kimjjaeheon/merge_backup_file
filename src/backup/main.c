#include "merge_backup_file.h"
#include "find_file_list.h"
#include "init.h"
#include "main.h"
#include "append_file.h"
#include "create_MERGE_FILE.h"

/* init_config setting */
extern char backup_files_home[128];
extern long file_cnt;

int main(int argc, char* argv[])
{
	int i = 0, j = 0;

	MERGE_FILE_LIST *tran_log_merge_file, *tran_info_merge_file, *trx_summ_merge_file;
	MERGE_FILE log_files[MERGE_FILE_NUM] = {0x00,}, info_files[MERGE_FILE_NUM] = {0x00,}, trx_files[MERGE_FILE_NUM] = {0x00,};

	tran_log_merge_file  = (MERGE_FILE_LIST*)malloc(sizeof(MERGE_FILE_LIST));
	tran_info_merge_file = (MERGE_FILE_LIST*)malloc(sizeof(MERGE_FILE_LIST));
	trx_summ_merge_file  = (MERGE_FILE_LIST*)malloc(sizeof(MERGE_FILE_LIST)); 

	tran_log_merge_file->files = log_files;
	tran_info_merge_file->files = info_files;
	trx_summ_merge_file->files = trx_files;

	if (init() == false)
	{
		printf("<!> find_file_list init fail.. \n");
		exit(1);
	}

	/* find_file_list 에서 malloc */
	FILE_LIST* f_list = NULL;

	/* 백업 디렉토리에 있는 파일 정보 수집 (f_list에 받아옴) */
    f_list = find_file_list(backup_files_home);

	/* TRAN_LOG 파일 생성 및 append */
	if (!create_TRAN_LOG_file(f_list, tran_log_merge_file, TIME))
	{
		printf("<!> Fail to create TRAN_LOG file \n");
		return -1;
	}

	/* TRX_SUMM 파일 생성 및 append */
	if (!create_TRX_SUMM_file(f_list, trx_summ_merge_file, TIME))
	{
		printf("<!> Fail to create TRX_SUMM file \n");
		return -1;
	}

	/* TRAN_INFOLOG 파일 생성 및 append */
	if (!create_TRAN_INFO_LOG_file(f_list, tran_info_merge_file, TIME))
	{
		printf("<!> Fail to create TRAN_ERROLOG file \n");
		return -1;
	}

	/* TRAN_ERRLOG 파일 생성 및 append */
	if (!create_TRAN_ERRLOG_file(f_list, tran_info_merge_file, TIME))
	{
		printf("<!> Fail to create TRAN_ERROLOG file \n");
		return -1;
	}
#if 0
	for(i=0, j=0; i<f_list->total_file_cnt; i++)
	{	
		if (f_list->files[i].dir_num == LOAD_FILE_INFOLOG) // TRAN_INFOLOG
		{
			continue;
		}
		else if (f_list->files[i].dir_num == LOAD_FILE_LOG) // TRAN_LOG
		{	
			/* merge 되어질 파일 pass */
			if (!memcmp(f_list->files[i].path, tran_log_merge_file->files[f_list->files[i].merge_file_idx].name, tran_log_merge_file_len ))
			{
				continue;
			}
			merge(f_list->files[i].path, tran_log_merge_file->files[f_list->files[i].merge_file_idx].name);
		}
		else if (f_list->files[i].dir_num == LOAD_FILE_TRX_SUMM) // TRAN_STATIS
		{
			
			if (!memcmp(f_list->files[i].path, trx_summ_merge_file->files[f_list->files[i].merge_file_idx].name, trx_summ_merge_file_len ))
			{
				continue;
			}

			merge(f_list->files[i].path, trx_summ_merge_file->files[f_list->files[i].merge_file_idx].name);
		
			continue;
		}
		else
		{
			log_printf("<!>  append 할 대상이 아닙니다. ");
			log_printf("   ->수집대상: tran_loginfo, tran_log, trx_summ, tran_errlog  ");
			continue;
		}
	}
#endif 
	free(f_list);
	free(tran_log_merge_file);
	free(tran_info_merge_file);
	free(trx_summ_merge_file);
	printf("free success \n");

	printf("total files : %d \n", f_list->total_file_cnt);
		
	close_log();

    return 0;
}
