#include "merge_backup_file.h"
#include "append_file.h"
#include "sep.h"

#define  TIME_PASS 1001
#define  UNLINK    1

#define TRAN_LOG_DAY_LEN  21 /* append 할 파일의 길이 ex) TRAN_LOG_20221108 */
#define TRAN_LOG_TIME_LEN 23 /* append 할 파일의 길이 ex) TRAN_LOG_2022110810 */

#define TRX_SUMM_DAY_LEN  21 /* append 할 파일의 길이 ex) TRAN_LOG_20221108 */
#define TRX_SUMM_TIME_LEN 23 /* append 할 파일의 길이 ex) TRAN_LOG_2022110810 */

#define TRAN_INFOLOG_DAY_LEN  27 /* append 할 파일의 길이 ex) TRAN_LOG_20221108 */
#define TRAN_INFOLOG_TIME_LEN 29 /* append 할 파일의 길이 ex) TRAN_LOG_2022110810 */

#define TRAN_ERRLOG_DAY_LEN   24  
#define TRAN_ERRLOG_TIME_LEN  26

extern char backup_files_home[128];

int create_TRAN_LOG_file(FILE_LIST* file_list, MERGE_FILE_LIST* merge_file_list, int mode);
int create_TRX_SUMM_file(FILE_LIST* file_list, MERGE_FILE_LIST* merge_file_list, int mode);
int create_TRAN_INFO_LOG_file(FILE_LIST* file_list, MERGE_FILE_LIST* merge_file_list, int mode);
int create_TRAN_ERRLOG_file(FILE_LIST* file_list, MERGE_FILE_LIST* merge_file_list, int mode);
int find_access_time(char* path);
int backup_if_list[3][BACKUP_IF_NUM];

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

/* tran_log 파일 만들기 */
int create_TRAN_LOG_file(FILE_LIST* file_list, MERGE_FILE_LIST* merge_file_list, int mode)
{
	//char file_path[FILE_NAME_LEN];	
	FILE *fp[BACKUP_IF_NUM], *input_fp, *merge_fp;
	int ret = 1;
	int if_num, i, j, tmp_idx, m_idx = 0;
	int m_file_flag = 0, same_file_flag = 0;
	char *tmp;
	char buf[READ_BUF_SIZE] = {0x00, };
	char output_buff[READ_BUF_SIZE] = {0x00, };    //tran_log 마지막 tuexdo id 제거 후 받아올 buffer
	char log_day[LOG_TIME] = {0x00, };

	for (i = 0; i < BACKUP_IF_NUM; i++)	
	{
		if (backup_if_list[LOAD_FILE_LOG][i] == 1) //init 에서 TRAN_LOG 파일 값이 1인 것만 체크
		{
			for (j = 0; j < file_list->total_file_cnt; j++)
			{
				/* tran_log 파일들만 골라낸다 */
				if (file_list->files[j].dir_num != LOAD_FILE_LOG)
				{
					continue;
				}

				/* IF 번호와 현재 i의 값이 같은 것들만 골라낸다 */
				/* 다른 IF 에 이름이 같은 파일이 있을 수 있다   */
				if (file_list->files[j].if_num != i)
				{
					continue;
				}

				/* 마지막 접근 시간이 30초 미만이면 pass 한다. */
				if (find_access_time(file_list->files[j].path) != TIME_PASS)
				{
					continue;
				}
		
				//printf("file_list->files[j].name_len : %d \n",  file_list->files[j].name_len);
				//printf("merge_file_list->files[0].name_len : %d\n", merge_file_list->files[0].name_len);
				if (file_list->files[j].name_len == TRAN_LOG_DAY_LEN || file_list->files[j].name_len == TRAN_LOG_TIME_LEN)
				{
					continue;
				}

				/* 읽으려는 파일리스트에 append 하려는 파일이 있을 때 제외 */
				if (m_idx != 0)
				{
					for (tmp_idx = 0; tmp_idx < m_idx; tmp_idx++)
					{
						//printf("file_list->files[j].path : %s \n",  file_list->files[j].path);
						//printf("merge_file_list->files[tmp_idx].path : %s\n", merge_file_list->files[tmp_idx].path);
						/* append 할 파일명과 read 해야할 파일이 같을 때 skip */
						if (!memcmp(file_list->files[j].path, merge_file_list->files[tmp_idx].path, merge_file_list->files[tmp_idx].path_len))
						{
							same_file_flag = 1;
						}
					}
					if (same_file_flag == 1)
					{
						same_file_flag = 0;
						continue;
					}
				}
	
				input_fp = fopen(file_list->files[j].path, "rb");

				if (input_fp == NULL)
				{
					log_printf("<!> file open fail (tran_log_file : %s\n", file_list->files[j].path);
					continue;
				}

				while (fgets(buf, sizeof(buf), input_fp))
				{
					sep_log(buf, output_buff, log_day, mode);		
					if (!log_day)
					{
						log_printf("<!> buff : %s ", buf);
						log_printf("    log_day를 가져오지 못해 pass 되었음 ");
						continue;
					}

					/* 첫번째 파일의 YYYYMMDD 를 가져와서 Merge 할 파일을 만든다 */
					/* 그 날짜를 기준으로동일하다면 파일에 merge할 merge_file_idx 의 값을 넣어준다 */
					if ( m_idx == 0 )
					{
						sprintf(merge_file_list->files[m_idx].path, "%s/tran_log/%02d/TRAN_LOG_%s.dat", backup_files_home, i, log_day);
						sprintf(merge_file_list->files[m_idx].name, "TRAN_LOG_%s.dat", log_day);

						log_printf("(I) append 할 파일 생성 중 ");
						log_printf("(I) append 할 파일 명 : %s ", merge_file_list->files[m_idx].path);
						merge_file_list->files[m_idx].if_num = i;

						merge_fp = fopen(merge_file_list->files[m_idx].path, "a+b");
						if (merge_fp == NULL)
						{
							log_printf("<!> append 할 파일 open fail : %s ", merge_file_list->files[m_idx].path);
							continue;
						}
						else
						{   
							fputs(output_buff, merge_fp);
							fclose(merge_fp);
							merge_file_list->files[m_idx].path_len = strlen(merge_file_list->files[m_idx].path);
							merge_file_list->files[m_idx].name_len = strlen(merge_file_list->files[m_idx].name);
							memcpy(merge_file_list->files[m_idx].log_day, log_day, LOG_TIME);
							m_idx++;
							log_printf("(I) append 할 파일 생성 완료 ");
						}
					}
					else
					{
						for (tmp_idx = 0; tmp_idx < m_idx; tmp_idx++)
						{
							if (!memcmp(merge_file_list->files[tmp_idx].log_day, log_day, LOG_TIME)) // 현재 날짜와 파일의 날짜가 같을 때 */
							{
								m_file_flag = 1;
								merge_fp = fopen(merge_file_list-> files[tmp_idx].path, "a+b");
								if (merge_fp == NULL)
								{
									log_printf("<!> append 할 파일 open fail : %s \n", merge_file_list->files[tmp_idx].path);
									continue;
								}
								else
								{
									fputs(output_buff, merge_fp);
									fclose(merge_fp);
									break;
								}
							}
						}

						if (m_file_flag == 0) // merge할 파일과 날짜가 다를때 다른 날짜로 merge할 파일을 생성시켜준다.
						{
							sprintf(merge_file_list->files[m_idx].path, "%s/tran_log/%02d/TRAN_LOG_%s.dat", backup_files_home, i, log_day);

							log_printf("(I) append 할 파일 생성 중 ");
							log_printf("(I) append 할 파일 명 : %s ", merge_file_list->files[m_idx].path);
							merge_fp = fopen(merge_file_list->files[m_idx].path, "a+b");
							if (merge_fp == NULL)
							{   
								log_printf("<!> append 할 파일 open fail : %s \n", merge_file_list->files[m_idx].path);
								continue;
							}
							else
							{
								fputs(output_buff, merge_fp);
								fclose(merge_fp);
							}
							memcpy(merge_file_list->files[m_idx].log_day, log_day, LOG_TIME);
							merge_file_list->files[m_idx].if_num = i;
							merge_file_list->files[m_idx].path_len = strlen(merge_file_list->files[m_idx].path);
							m_idx++;
							log_printf("(I) append 할 파일 생성 완료 ");
						}
						else
						{
							m_file_flag = 0;
						}
					}
					//printf("file list : %s \n",file_list->files[j].path);
					//printf("-> j cont : %d \n",j);
				}
				fclose(input_fp);
#if UNLINK
				int unlink_ret = unlink(file_list->files[j].path);
				if (unlink_ret != 0) /* 삭제 실패 */ 
				{       
					log_printf("<!> 삭제 실패 (파일 : %s)", file_list->files[j].path);
				}   
#endif

			}
		}
		else
		{
			continue;
		}
	}
	log_printf("(I) TRAN_LOG append success ");

	return ret;
}


int create_TRX_SUMM_file(FILE_LIST* file_list, MERGE_FILE_LIST* merge_file_list, int mode)
{
	//char file_path[FILE_NAME_LEN];	
	FILE *fp[BACKUP_IF_NUM], *input_fp, *merge_fp;
	int ret = 1;
	int if_num, i, j, tmp_idx, m_idx = 0, same_file_flag = 0, m_file_flag = 0;
	char *tmp;
	char log_day[LOG_TIME] = {0x00, };
	char buf[READ_BUF_SIZE] = {0x00, };

	for (i = 0; i < file_list->total_file_cnt; i++)
	{
		/* trx_summ 파일들만 골라낸다 */
		if (file_list->files[i].dir_num != LOAD_FILE_TRX_SUMM)
		{
			continue;
		}
		
		/* 마지막 접근 시간이 30초 미만이면 pass 한다. */
		if (find_access_time(file_list->files[i].path) != TIME_PASS)
		{
			continue;
		}
		if (file_list->files[i].name_len == TRX_SUMM_DAY_LEN || file_list->files[i].name_len == TRX_SUMM_TIME_LEN)
		{
			continue;
		}

		/* 읽으려는 파일리스트에 append 하려는 파일이 있을 때 제외 */
		if (m_idx != 0)
		{
			for (tmp_idx = 0; tmp_idx < m_idx; tmp_idx++)
			{
				/* append 할 파일명과 read 해야할 파일이 같을 때 skip */
				if (!memcmp(file_list->files[i].path, merge_file_list->files[tmp_idx].path, merge_file_list->files[tmp_idx].path_len))
				{
					same_file_flag = 1;
					break;
				}
			}
			if (same_file_flag == 1)
			{
				same_file_flag = 0;
				continue;
			}
		}
	
		input_fp = fopen(file_list->files[i].path, "rb");

		if (input_fp == NULL)
		{
			log_printf("<!> file open fail (tran_log_file : %s\n", file_list->files[i].path);
			continue;
		}

		while (fgets(buf, sizeof(buf), input_fp))
		{
			sep_trx_summ(buf, log_day, mode);
			if (!log_day)
			{
				log_printf("<!> buff : %s ", buf);
				log_printf("    log_day를 가져오지 못해 pass 되었음 ");
				continue;
			}
	
			/* 첫번째 파일의 YYYYMMDD 를 가져와서 Merge 할 파일을 만든다 */
			/* 그 날짜를 기준으로동일하다면 파일에 merge할 merge_file_idx 의 값을 넣어준다 */
			if ( m_idx == 0 )
			{
				sprintf(merge_file_list->files[m_idx].path, "%s/tran_statis/trx_summ/TRX_SUMM_%s.dat", backup_files_home, log_day);
				merge_file_list->files[m_idx].if_num = NO_IF_NUM;

				log_printf("(I) append 할 파일 생성 중 ");
				log_printf("(I) append 할 파일 명 : %s ", merge_file_list->files[m_idx].path);

				merge_fp = fopen(merge_file_list->files[m_idx].path, "a+b");
				if (merge_fp == NULL)
				{
					log_printf("<!> append 할 파일 open fail : %s ", merge_file_list->files[m_idx].path);
					continue;
				}
				else
				{
					fputs(buf, merge_fp);
					fclose(merge_fp);
					merge_file_list->files[m_idx].path_len = strlen(merge_file_list->files[m_idx].path);
					m_idx++;
					log_printf("(I) append 할 파일 생성 완료 ");
					continue;
				}
			}
			else
			{
				for (tmp_idx = 0; tmp_idx < m_idx; tmp_idx++)
				{
					if (!memcmp(merge_file_list->files[tmp_idx].log_day, log_day, LOG_TIME)) // 현재 날짜와 append할 파일의 날짜가 같을 때 */
					{
						m_file_flag = 1;
						merge_fp = fopen(merge_file_list->files[tmp_idx].path, "a+b");
						if (merge_fp == NULL)
						{
							log_printf("<!> append 할 파일 open fail : %s ", merge_file_list->files[tmp_idx].path);
							continue;
						}
						else
						{
							fputs(buf, merge_fp);
							fclose(merge_fp);
							break;
						}
					}
				}

				if (m_file_flag == 0) // merge할 파일과 날짜가 다를때 다른 날짜로 merge할 파일을 생성시켜준다.
				{
					sprintf(merge_file_list->files[m_idx].path, "%s/tran_statis/trx_summ/TRX_SUMM_%s.dat", backup_files_home, log_day);

					log_printf("(I) append 할 파일 생성 중 ");	
					log_printf("(I) append 할 파일 명 : %s ", merge_file_list->files[m_idx].path);
					merge_fp = fopen(merge_file_list->files[m_idx].path, "a+b");
					if (merge_fp == NULL)
					{
						log_printf("<!> append 할 파일 open fail : %s ", merge_file_list->files[m_idx].path);
						continue;
					}
					else
					{
						fputs(buf, merge_fp);
						fclose(merge_fp);
					}
					memcpy(merge_file_list->files[m_idx].log_day, log_day, LOG_TIME);
					merge_file_list->files[m_idx].if_num = NO_IF_NUM;
					merge_file_list->files[m_idx].path_len = strlen(merge_file_list->files[m_idx].path);
					m_idx++;
					log_printf("(I) append 할 파일 생성 완료 ");
				}
				else
				{
					m_file_flag = 0;
				}
			}
		}
		fclose(input_fp);
#if UNLINK
		int unlink_ret = unlink(file_list->files[i].path);
		if (unlink_ret != 0) /* 삭제 실패 */
		{
			log_printf("<!> 삭제 실패 (파일 : %s)", file_list->files[i].path);
		}
#endif

	}
	log_printf("(I) TRX_SUMM append success ");

	return ret;
}


int create_TRAN_INFO_LOG_file(FILE_LIST* file_list, MERGE_FILE_LIST* merge_file_list, int mode)
{
	//char file_path[FILE_NAME_LEN];	
	FILE *fp[BACKUP_IF_NUM], *input_fp, *merge_fp;
	int ret = 1;
	int if_num, i, j, tmp_idx, m_idx = 0; //m_idx = merge(append)할 파일 인덱스
	int m_file_flag = 0, buff_len = 0, same_file_flag = 0;
	char *tmp;
	char buff[READ_BUF_SIZE];    //파일 읽어서 담는 buff
	char output_buff[READ_BUF_SIZE] = {0x00, };    //infolog parsing 후 담아올 buff
	char log_day[LOG_TIME] = {0x00,};
	
	for (i = 0; i < BACKUP_IF_NUM; i++)	
	{
		if (backup_if_list[LOAD_FILE_INFOLOG][i] == 1) //init 에서 TRAN_LOG 파일 값이 1인 것만 체크
		{
			for (j = 0; j < file_list->total_file_cnt; j++) /* file_list 에서 file들의 인덱스 = j */ 
			{
				if (file_list->files[j].dir_num != LOAD_FILE_INFOLOG) /* INFOLOG가 맞는지 확인 */
				{
					continue;
				}
				if (file_list->files[j].if_num != i) /* 해당 파일의 IF 번호가 현재 탐색하고있는지 IF 번호와 일치하는지 */
				{
					continue;
				}
				/* 마지막 접근 시간이 30초 미만이면 pass 한다. */
				if (find_access_time(file_list->files[j].path) != TIME_PASS)
				{
					continue;
				}

                //printf("file_list->files[j].name_len : %d \n",  file_list->files[j].name_len);
                //printf("merge_file_list->files[0].name_len : %d\n", merge_file_list->files[0].name_len);
				if (file_list->files[j].name_len == TRAN_INFOLOG_DAY_LEN || file_list->files[j].name_len == TRAN_INFOLOG_TIME_LEN)
                {   
                    continue;
                }
				
				if(merge_file_list->files[m_idx].path) /* 읽으려는 파일리스트에 append 하려는 파일이 있을 때 제외 */
				{
					for (tmp_idx = 0; tmp_idx < m_idx; tmp_idx++)
					{
						/* append 할 파일명과 read 해야할 파일이 같을 때 skip */
						if (!memcmp(file_list->files[j].path, merge_file_list->files[tmp_idx].path,63))
						{
							same_file_flag = 1;
							continue;
						}
					}
					if (same_file_flag == 1)
					{
						same_file_flag = 0;
						continue;
					}
				}
				/* 파일 1줄씩 buff로 읽고 앞부분이 해당되는 LOG 파일에 붙혀 넣는다 */
				input_fp = fopen(file_list->files[j].path, "rb"); 

				if (input_fp == NULL)	
				{
					log_printf("<!> file open fail (tran_infolog_file : %s\n", file_list->files[j].path);
					continue;
				}
	
				while (fgets(buff, sizeof(buff), input_fp))
				{
					memset(output_buff, 0x00, sizeof(output_buff));
					if (!memcmp(buff, "C^C^" , 4)) /* C^C^ 로 시작하는 라인은 _C_*.dat 파일에 append */
					{
                        /* 시간값을 가져온다 (log_day에담는다) */
						if (sep_infolog(buff, output_buff, INFO_C, log_day, mode) == PARSING_FAIL)
                        {
                            continue;
                        }

						if ( m_idx == 0 ) /* 아직 append 할 파일이 없을 때 만든다. */
						{
							sprintf(merge_file_list->files[m_idx].path, "%s/tran_infolog/%02d/TRAN_INFOLOG_C_%s.dat", backup_files_home, i, log_day);
							sprintf(merge_file_list->files[m_idx].name, "TRAN_INFOLOG_C_%s.dat", log_day);
							memcpy(merge_file_list->files[m_idx].log_day, log_day, LOG_TIME);
							
							log_printf("(I) append 할 파일 생성 중 "); 
							log_printf("(I) append 할 파일 명 :  %s ", merge_file_list->files[m_idx].path);
							merge_fp = fopen(merge_file_list->files[m_idx].path, "a+b");
							if (merge_fp == NULL)
							{
								log_printf("<!> append 할 파일 open fail : %s ", merge_file_list->files[tmp_idx].path);
								continue;
							}
							else
							{
								fputs(output_buff, merge_fp);
								fclose(merge_fp);
								merge_file_list->files[m_idx].log_type = 'C';
                          		merge_file_list->files[m_idx].path_len = strlen(merge_file_list->files[m_idx].path);
                           		merge_file_list->files[m_idx].name_len = strlen(merge_file_list->files[m_idx].name);
								m_idx++;
								log_printf("(I) append 할 파일 생성 완료 "); 
							}
						}
						else
						{
							for (tmp_idx = 0; tmp_idx < m_idx; tmp_idx++)
							{
								
								if (!memcmp(merge_file_list->files[tmp_idx].log_day, log_day, LOG_TIME) && (merge_file_list->files[tmp_idx].log_type == 'C')) /* 현재 날짜와 파일의 날짜가 같을 때 */
								{
									m_file_flag = 1; /* merge 할 파일이 있음 ( 0:없음, 1:있음) */
									merge_fp = fopen(merge_file_list->files[tmp_idx].path, "a+b");		
									if (merge_fp == NULL)
									{
										log_printf("<!> append 할 파일 open fail : %s \n", merge_file_list->files[tmp_idx].path);
										continue;
									}
									else
									{
										fputs(output_buff, merge_fp);
										fclose(merge_fp);
									}
								}
							}

							if (m_file_flag == 0) // merge할 파일과 날짜가 다를때 다른 날짜로 merge할 파일을 생성시켜준다.
							{
								sprintf(merge_file_list->files[m_idx].path, "%s/tran_infolog/%02d/TRAN_INFOLOG_C_%s.dat", backup_files_home, i, log_day);
								sprintf(merge_file_list->files[m_idx].name, "TRAN_INFOLOG_C_%s.dat", log_day);
								log_printf("(I) append 할 파일 생성 중 ");
								log_printf("(I) append 할 파일 명 :  %s ", merge_file_list->files[m_idx].path);
								merge_fp = fopen(merge_file_list->files[m_idx].path, "a+b");
								if (merge_fp == NULL)
								{
										log_printf("<!> merge_file open fail : ", merge_file_list->files[m_idx].path);
										continue;
								}
								else
								{
									fputs(output_buff, merge_fp);
									fclose(merge_fp);
								}
								file_list->files[j].merge_file_idx = m_idx; /* append 할 파일 번호 넣기 */
								merge_file_list->files[m_idx].log_type = 'C'; /* IF 번호 넣기 */
								merge_file_list->files[m_idx].if_num = i; /* IF 번호 넣기 */
								memcpy(merge_file_list->files[m_idx].log_day, log_day, LOG_TIME);
                              	merge_file_list->files[m_idx].path_len = strlen(merge_file_list->files[m_idx].path);
                                merge_file_list->files[m_idx].name_len = strlen(merge_file_list->files[m_idx].name);
								m_idx++;
								log_printf("(I) append 할 파일 생성 완료 ");
							}
							else
							{
								m_file_flag = 0; 
							}
						}
					}
					else if (!memcmp(buff, "I^I^" , 4)) /* I^I^ 로 시작하는 라인은 _C_*.dat 파일에 append */
					{
						/* 시간값을 가져온다 (log_day에담는다) */
						if (sep_infolog(buff, output_buff, INFO_I, log_day, mode) == PARSING_FAIL)
						{
							continue;
						}
						if ( m_idx == 0 ) /* 아직 append 할 파일이 없을 때 만든다. */
						{
							sprintf(merge_file_list->files[m_idx].path, "%s/tran_infolog/%02d/TRAN_INFOLOG_I_%s.dat", backup_files_home, i, log_day);
                            sprintf(merge_file_list->files[m_idx].name, "TRAN_INFOLOG_I_%s.dat", log_day);
							memcpy(merge_file_list->files[m_idx].log_day, log_day, LOG_TIME);
							log_printf("(I) append 할 파일 생성 중 "); 
							log_printf("(I) append 할 파일 명 :  %s ", merge_file_list->files[m_idx].path);
							merge_fp = fopen(merge_file_list->files[m_idx].path, "a+b");
							if (merge_fp == NULL)
							{
								log_printf("<!> merge_file open fail : %s ", merge_file_list->files[m_idx].path);
								continue;
							}
							else
							{
								fputs(output_buff, merge_fp);
								fclose(merge_fp);
								merge_file_list->files[m_idx].log_type = 'I';
                                merge_file_list->files[m_idx].path_len = strlen(merge_file_list->files[m_idx].path);
                                merge_file_list->files[m_idx].name_len = strlen(merge_file_list->files[m_idx].name);
								m_idx++;
								log_printf("(I) append 할 파일 생성 완료 "); 
							}
						}
						else
						{
							for (tmp_idx = 0; tmp_idx < m_idx; tmp_idx++)
							{
								if (!memcmp(merge_file_list->files[tmp_idx].log_day, log_day, LOG_TIME) && (merge_file_list->files[tmp_idx].log_type == 'I')) /* 현재 날짜와 파일의 날짜가 같을 때 */
								{
									m_file_flag = 1; /* merge 할 파일이 있음 ( 0:없음, 1:있음) */
									
									merge_fp = fopen(merge_file_list->files[tmp_idx].path, "a+b");		
									if (merge_fp == NULL)
									{
										log_printf("<!> merge_file open fail : %s \n", merge_file_list->files[m_idx].path);
										continue;
									}
									else
									{
										fputs(output_buff, merge_fp);
										fclose(merge_fp);
									}
								}
							}

							if (m_file_flag == 0) // merge할 파일과 날짜가 다를때 다른 날짜로 merge할 파일을 생성시켜준다.
							{
								sprintf(merge_file_list->files[m_idx].path, "%s/tran_infolog/%02d/TRAN_INFOLOG_I_%s.dat", backup_files_home, i, log_day);
								sprintf(merge_file_list->files[m_idx].name, "TRAN_INFOLOG_I_%s.dat", log_day);
								merge_fp = fopen(merge_file_list->files[m_idx].path, "a+b");
								if (merge_fp == NULL)
								{
										log_printf("<!> merge_file open fail : %s \n", merge_file_list->files[m_idx].path);
										continue;
								}
								else
								{
									fputs(output_buff, merge_fp);
									fclose(merge_fp);
								}
								file_list->files[j].merge_file_idx = m_idx; /* append 할 파일 번호 넣기 */
								merge_file_list->files[m_idx].log_type = 'I'; /* IF 번호 넣기 */
								merge_file_list->files[m_idx].if_num = i; /* IF 번호 넣기 */
                               	merge_file_list->files[m_idx].path_len = strlen(merge_file_list->files[m_idx].path);
                                merge_file_list->files[m_idx].name_len = strlen(merge_file_list->files[m_idx].name);
								memcpy(merge_file_list->files[m_idx].log_day, log_day, LOG_TIME);
								m_idx++;
							}
							else
							{
								m_file_flag = 0; 
							}
						}
					}
					else if (!memcmp(buff, "B^B^" , 4)) /* B^B^ 로 시작하는 라인은 _B_*.dat 파일에 append */
					{
						/* 시간값을 가져온다 (log_day에담는다) */
						if (sep_infolog(buff, output_buff, INFO_B, log_day, mode) == PARSING_FAIL) 
                        {
                            continue;
                        }
						if ( m_idx == 0 ) /* 아직 append 할 파일이 없을 때 만든다. */
						{
							sprintf(merge_file_list->files[m_idx].path, "%s/tran_infolog/%02d/TRAN_INFOLOG_B_%s.dat", backup_files_home, i, log_day);
							sprintf(merge_file_list->files[m_idx].name, "TRAN_INFOLOG_B_%s.dat", log_day);
							memcpy(merge_file_list->files[m_idx].log_day, log_day, LOG_TIME);
							log_printf("(I) append 할 파일 생성 중 ");
							log_printf("(I) append 할 파일 명 :  %s ", merge_file_list->files[m_idx].path);
							merge_fp = fopen(merge_file_list->files[m_idx].path, "a+b");
							if (merge_fp == NULL)
							{
								log_printf("<!> merge_file open fail : %s ", merge_file_list->files[m_idx].path);
								continue;
							}
							else
							{
								fputs(output_buff, merge_fp);
								fclose(merge_fp);
								merge_file_list->files[m_idx].log_type = 'B';
                                merge_file_list->files[m_idx].path_len = strlen(merge_file_list->files[m_idx].path);
                                merge_file_list->files[m_idx].name_len = strlen(merge_file_list->files[m_idx].name);
								m_idx++;
								log_printf("(I) append 할 파일 생성 완료 ");
							}
						}
						else
						{
							for (tmp_idx = 0; tmp_idx < m_idx; tmp_idx++)
							{
								if (!memcmp(merge_file_list->files[tmp_idx].log_day, log_day, LOG_TIME) && (merge_file_list->files[tmp_idx].log_type == 'B')) /* 현재 날짜와 파일의 날짜가 같을 때 */
								{
									m_file_flag = 1; /* merge 할 파일이 있음 ( 0:없음, 1:있음) */
									merge_fp = fopen(merge_file_list->files[tmp_idx].path, "a+b");
									if (merge_fp == NULL)
									{
										log_printf("<!> merge_file open fail : %s \n", merge_file_list->files[m_idx].path);
										continue;
									}
									else
									{
										fputs(output_buff, merge_fp);
										fclose(merge_fp);
									}
								}
							}

							if (m_file_flag == 0) // merge할 파일과 날짜가 다를때 다른 날짜로 merge할 파일을 생성시켜준다.
							{
								sprintf(merge_file_list->files[m_idx].path, "%s/tran_infolog/%02d/TRAN_INFOLOG_B_%s.dat", backup_files_home, i, log_day);
								sprintf(merge_file_list->files[m_idx].name, "TRAN_INFOLOG_B_%s.dat", log_day);
								merge_fp = fopen(merge_file_list->files[m_idx].path, "a+b");
								if (merge_fp == NULL)
								{
										log_printf("<!> merge_file open fail : %s \n", merge_file_list->files[m_idx].path);
										continue;
								}
								else
								{
									fputs(output_buff, merge_fp);
									fclose(merge_fp);
								}
								file_list->files[j].merge_file_idx = m_idx; /* append 할 파일 번호 넣기 */
								merge_file_list->files[m_idx].log_type = 'B'; /* IF 번호 넣기 */
								merge_file_list->files[m_idx].if_num = i; /* IF 번호 넣기 */
								merge_file_list->files[m_idx].path_len = strlen(merge_file_list->files[m_idx].path);
                                merge_file_list->files[m_idx].name_len = strlen(merge_file_list->files[m_idx].name);
								memcpy(merge_file_list->files[m_idx].log_day, log_day, LOG_TIME);
								m_idx++;
							}
							else
							{
								m_file_flag = 0;
							}
						}
					}
					else if (!memcmp(buff, "J^E^" , 4)) /* J^E^ 로 시작하는 라인은 _Q_*.dat 파일에 append */
					{
                        /* 시간값을 가져온다 (log_day에담는다) */
                        if (sep_infolog(buff, output_buff, INFO_J, log_day, mode) == PARSING_FAIL)
                        {
                            continue;
                        }

						if ( m_idx == 0 ) /* 아직 append 할 파일이 없을 때 만든다. */
						{
							sprintf(merge_file_list->files[m_idx].path, "%s/tran_infolog/%02d/TRAN_INFOLOG_J_%s.dat", backup_files_home, i, log_day);
							sprintf(merge_file_list->files[m_idx].name, "TRAN_INFOLOG_J_%s.dat", log_day);
							memcpy(merge_file_list->files[m_idx].log_day, log_day, LOG_TIME);
							log_printf("(I) append 할 파일 생성 중 ");
							log_printf("(I) append 할 파일 명 :  %s ", merge_file_list->files[m_idx].path);
							merge_fp = fopen(merge_file_list->files[m_idx].path, "a+b");
							if (merge_fp == NULL)
							{
								log_printf("<!> merge_file open fail : %s ", merge_file_list->files[m_idx].path);
								continue;
							}
							else
							{
								/* J^E^ 는 그냥 append 하기 때문에 outbuff를 넣지않고 buff 그대로 넣는다.*/
								fputs(output_buff, merge_fp);
								fclose(merge_fp);
								merge_file_list->files[m_idx].log_type = 'J';
								merge_file_list->files[m_idx].path_len = strlen(merge_file_list->files[m_idx].path);
                                merge_file_list->files[m_idx].name_len = strlen(merge_file_list->files[m_idx].name);
								m_idx++;
								log_printf("(I) append 할 파일 생성 완료 ");
							}
						}
						else
						{           
							for (tmp_idx = 0; tmp_idx < m_idx; tmp_idx++)
							{
								if (!memcmp(merge_file_list->files[tmp_idx].log_day, log_day, LOG_TIME) && (merge_file_list->files[tmp_idx].log_type == 'J')) /* 현재 날짜와 파일의 날짜가 같을 때 */
								{
									m_file_flag = 1; /* merge 할 파일이 있음 ( 0:없음, 1:있음) */   
									merge_fp = fopen(merge_file_list->files[tmp_idx].path, "a+b");
									if (merge_fp == NULL)
									{
										log_printf("<!> merge_file open fail : %s ", merge_file_list->files[m_idx].path);
										continue;
									}
									else
									{
										fputs(output_buff, merge_fp);
										fclose(merge_fp);
									}
								}
							}   
								
							if (m_file_flag == 0) // merge할 파일과 날짜가 다를때 다른 날짜로 merge할 파일을 생성시켜준다.
							{   
								sprintf(merge_file_list->files[m_idx].path, "%s/tran_infolog/%02d/TRAN_INFOLOG_J_%s.dat", backup_files_home, i, log_day);
								sprintf(merge_file_list->files[m_idx].name, "TRAN_INFOLOG_J_%s.dat", log_day);
								merge_fp = fopen(merge_file_list->files[m_idx].path, "a+b");
								if (merge_fp == NULL)
								{
										log_printf("<!> merge_file open fail : %s ", merge_file_list->files[m_idx].path);
										continue;
								}
								else
								{
									fputs(output_buff, merge_fp);
									fclose(merge_fp);
								}
								file_list->files[j].merge_file_idx = m_idx; /* append 할 파일 번호 넣기 */
								merge_file_list->files[m_idx].log_type = 'J'; /* IF 번호 넣기 */
								merge_file_list->files[m_idx].if_num = i; /* IF 번호 넣기 */
								merge_file_list->files[m_idx].path_len = strlen(merge_file_list->files[m_idx].path);
                                merge_file_list->files[m_idx].name_len = strlen(merge_file_list->files[m_idx].name);
								memcpy(merge_file_list->files[m_idx].log_day, log_day, LOG_TIME);
								m_idx++;
							}
							else
							{
								m_file_flag = 0;
							}
						}
					}
					else if (!memcmp(buff, "Q^Q^" , 4)) /* Q^Q^ 로 시작하는 라인은 _Q_*.dat 파일에 append */
					{
						sep_infolog(buff, output_buff, INFO_Q, log_day, mode); /* 시간값을 가져온다 (log_day에담는다) */
						if ( m_idx == 0 ) /* 아직 append 할 파일이 없을 때 만든다. */
						{                                       
							sprintf(merge_file_list->files[m_idx].path, "%s/tran_infolog/%02d/TRAN_INFOLOG_Q_%s.dat", backup_files_home, i, log_day);
							sprintf(merge_file_list->files[m_idx].name, "TRAN_INFOLOG_Q_%s.dat", log_day);
							memcpy(merge_file_list->files[m_idx].log_day, log_day, LOG_TIME);
							log_printf("(I) append 할 파일 생성 중 ");
							log_printf("(I) append 할 파일 명 :  %s ", merge_file_list->files[m_idx].path);
							merge_fp = fopen(merge_file_list->files[m_idx].path, "a+b");
							if (merge_fp == NULL)
							{           
								log_printf("<!> merge_file open fail : %s ", merge_file_list->files[m_idx].path);
								continue;
							}       
							else    
							{           
								/* Q^Q^ 는 그냥 append 하기 때문에 outbuff를 넣지않고 buff 그대로 넣는다.*/
								fputs(buff, merge_fp);
								fclose(merge_fp);
								merge_file_list->files[m_idx].log_type = 'Q';
								merge_file_list->files[m_idx].path_len = strlen(merge_file_list->files[m_idx].path);
                                merge_file_list->files[m_idx].name_len = strlen(merge_file_list->files[m_idx].name);
								m_idx++;
								log_printf("(I) append 할 파일 생성 완료 ");
							}   
						}       
						else
						{        
							for (tmp_idx = 0; tmp_idx < m_idx; tmp_idx++)
							{
								if (!memcmp(merge_file_list->files[tmp_idx].log_day, log_day, LOG_TIME) && (merge_file_list->files[tmp_idx].log_type == 'Q')) /* 현재 날짜와 파일의 날짜가 같을 때 */
								{      
									m_file_flag = 1; /* merge 할 파일이 있음 ( 0:없음, 1:있음) */
									merge_fp = fopen(merge_file_list->files[tmp_idx].path, "a+b");
									if (merge_fp == NULL)
									{
										log_printf("<!> merge_file open fail : %s \n", merge_file_list->files[m_idx].path);
										continue;
									}
									else
									{
										fputs(buff, merge_fp);
										fclose(merge_fp);
									}
								}
							}   
								
							if (m_file_flag == 0) // merge할 파일과 날짜가 다를때 다른 날짜로 merge할 파일을 생성시켜준다.
							{   
								sprintf(merge_file_list->files[m_idx].path, "%s/tran_infolog/%02d/TRAN_INFOLOG_Q_%s.dat", backup_files_home, i, log_day); 
								sprintf(merge_file_list->files[m_idx].name, "TRAN_INFOLOG_Q_%s.dat", log_day);
								merge_fp = fopen(merge_file_list->files[m_idx].path, "a+b");
								if (merge_fp == NULL)
								{
										log_printf("<!> merge_file open fail : %s \n", merge_file_list->files[m_idx].path);
										continue;
								}
								else
								{
									fputs(buff, merge_fp);
									fclose(merge_fp);
								}
								file_list->files[j].merge_file_idx = m_idx; /* append 할 파일 번호 넣기 */
								merge_file_list->files[m_idx].log_type = 'Q'; /* IF 번호 넣기 */
								merge_file_list->files[m_idx].if_num = i; /* IF 번호 넣기 */
								merge_file_list->files[m_idx].path_len = strlen(merge_file_list->files[m_idx].path);
                                merge_file_list->files[m_idx].name_len = strlen(merge_file_list->files[m_idx].name);
								memcpy(merge_file_list->files[m_idx].log_day, log_day, LOG_TIME);
								m_idx++;
							}
							else
							{
								m_file_flag = 0;
							}
						}
					}
					else
					{
						continue;
					}
				}	
				fclose(input_fp);
#if UNLINK
				int unlink_ret = unlink(file_list->files[j].path);
				if (unlink_ret != 0) /* 삭제 실패 */
				{
					log_printf("<!> 삭제 실패 (파일 : %s)", file_list->files[j].path);
				}
#endif
			}
		}
	}
	log_printf("(I) TRAN_INFOLOG append success ");	

	return ret;
}

int create_TRAN_ERRLOG_file(FILE_LIST* file_list, MERGE_FILE_LIST* merge_file_list, int mode)
{
    FILE *fp[BACKUP_IF_NUM], *input_fp, *merge_fp;
    int ret = 1;
    int if_num, i, j, tmp_idx, m_idx = 0;
    int m_file_flag = 0, same_file_flag = 0;
    char *tmp;
    char buf[READ_BUF_SIZE] = {0x00, };
    char log_day[LOG_TIME] = {0x00, };

	for (i = 0; i < BACKUP_IF_NUM; i++)
	{
		if (backup_if_list[LOAD_FILE_ERRLOG][i] == 1)
		{
			for (j = 0; j < file_list->total_file_cnt; j++)
			{
                /* tran_log 파일들만 골라낸다 */
                if (file_list->files[j].dir_num != LOAD_FILE_ERRLOG)
                {
                    continue;
                }

                /* IF 번호와 현재 i의 값이 같은 것들만 골라낸다 */
                /* 다른 IF 에 이름이 같은 파일이 있을 수 있다   */
                if (file_list->files[j].if_num != i)
                {
                    continue;
                }

                /* 마지막 접근 시간이 30초 미만이면 pass 한다. */
                if (find_access_time(file_list->files[j].path) != TIME_PASS)
                {
                    continue;
                }

                //printf("file_list->files[j].name_len : %d \n",  file_list->files[j].name_len);
                //printf("merge_file_list->files[0].name_len : %d\n", merge_file_list->files[0].name_len);
				if (file_list->files[j].name_len == TRAN_ERRLOG_DAY_LEN || file_list->files[j].name_len == TRAN_ERRLOG_TIME_LEN)
                {
                    continue;
                }

               /* 읽으려는 파일리스트에 append 하려는 파일이 있을 때 제외 */
                if (m_idx != 0)
                {
                    for (tmp_idx = 0; tmp_idx < m_idx; tmp_idx++)
                    {
                        /* append 할 파일명과 read 해야할 파일이 같을 때 skip */
                        if (!memcmp(file_list->files[j].path, merge_file_list->files[tmp_idx].path, merge_file_list->files[tmp_idx].path_len))
                        {
                            same_file_flag = 1;
                        }
                    }
                    if (same_file_flag == 1)
                    {
                        same_file_flag = 0;
                        continue;
                    }
                }
	
				input_fp = fopen(file_list->files[j].path, "rb");

                if (input_fp == NULL)
                {
                    log_printf("<!> file open fail (tran_log_file : %s\n", file_list->files[j].path);
                    continue;
                }

				while (fgets(buf, sizeof(buf), input_fp))
				{
					sep_errlog(buf, log_day, mode);
					if (!log_day)
					{
						log_printf("<!> buff : %s ", buf);
						log_printf("    log_day를 가져오지 못해 pass 되었음 ");
						continue;
					}		

					if (m_idx == 0)
					{
                        sprintf(merge_file_list->files[m_idx].path, "%s/tran_errlog/%02d/TRAN_ERRLOG_%s.dat", backup_files_home, i, log_day);                     
                        sprintf(merge_file_list->files[m_idx].name, "TRAN_ERRLOG_%s.dat", log_day);
                        
                        log_printf("(I) append 할 파일 생성 중 ");
                        log_printf("(I) append 할 파일 명 : %s ", merge_file_list->files[m_idx].path);
                        merge_file_list->files[m_idx].if_num = i;
                        
                        merge_fp = fopen(merge_file_list->files[m_idx].path, "a+b");
                        if (merge_fp == NULL)
                        {   
                            log_printf("<!> append 할 파일 open fail : %s ", merge_file_list->files[m_idx].path);
                            continue;
                        }
                        else
                        {   
                            fputs(buf, merge_fp);
                            fclose(merge_fp);
                            merge_file_list->files[m_idx].path_len = strlen(merge_file_list->files[m_idx].path);
                            merge_file_list->files[m_idx].name_len = strlen(merge_file_list->files[m_idx].name);
                            memcpy(merge_file_list->files[m_idx].log_day, log_day, LOG_TIME);
                            m_idx++;
                            log_printf("(I) append 할 파일 생성 완료 ");
                        }

					}
					else
					{
                        for (tmp_idx = 0; tmp_idx < m_idx; tmp_idx++)
                        {
                            if (!memcmp(merge_file_list->files[tmp_idx].log_day, log_day, LOG_TIME)) // 현재 날짜와 파일의 날짜가 같을 때 */
                            {
                                m_file_flag = 1;
                                merge_fp = fopen(merge_file_list-> files[tmp_idx].path, "a+b");
                                if (merge_fp == NULL)
                                {
                                    log_printf("<!> append 할 파일 open fail : %s \n", merge_file_list->files[tmp_idx].path);
                                    continue;
                                }
                                else
                                {
                                    fputs(buf, merge_fp);
                                    fclose(merge_fp);
                                    break;
                                }
                            }
                        }

                        if (m_file_flag == 0) // merge할 파일과 날짜가 다를때 다른 날짜로 merge할 파일을 생성시켜준다.
                        {
                            sprintf(merge_file_list->files[m_idx].path, "%s/tran_errlog/%02d/TRAN_ERRLOG_%s.dat", backup_files_home, i, log_day);

                            log_printf("(I) append 할 파일 생성 중 ");
                            log_printf("(I) append 할 파일 명 : %s ", merge_file_list->files[m_idx].path);
                            merge_fp = fopen(merge_file_list->files[m_idx].path, "a+b");
                            if (merge_fp == NULL)
                            {
                                log_printf("<!> append 할 파일 open fail : %s \n", merge_file_list->files[m_idx].path);
                                continue;
                            }
                            else
                            {
                                fputs(buf, merge_fp);
                                fclose(merge_fp);
                            }
                            memcpy(merge_file_list->files[m_idx].log_day, log_day, LOG_TIME);
                            merge_file_list->files[m_idx].if_num = i;
                            merge_file_list->files[m_idx].path_len = strlen(merge_file_list->files[m_idx].path);
                            m_idx++;
                            log_printf("(I) append 할 파일 생성 완료 ");
                        }
						else
						{
							m_file_flag = 0;
						}

					}
				}
				fclose(input_fp);
#if UNLINK
                int unlink_ret = unlink(file_list->files[j].path);
                if (unlink_ret != 0) /* 삭제 실패 */
                {
                    log_printf("<!> 삭제 실패 (파일 : %s)", file_list->files[j].path);
                }
#endif

			}
		}	
	}
	log_printf("(I) TRAN_ERRLOG append success ");

	return ret;
}

int find_access_time(char* path)
{
    struct stat file;

    /* 현재 시간 */
    struct timeval rawtime;
    time_t rawtime_tm ;
    gettimeofday(&rawtime, NULL);
    rawtime_tm = rawtime.tv_sec ;

    if (lstat(path, &file) == -1)
    {
        log_printf("<!> find_access_time fail ");
        log_printf("    file name : %s ", path);
    }

    //printf("file_name : %s\n", path);
    //printf("time is : %ld\n", rawtime_tm);
    //printf("Last change time is : %ld\n", file.st_ctime);
    //printf("time - last_change : %ld\n", rawtime_tm - file.st_ctime);

    if ((rawtime_tm - file.st_ctime) > 30)
    {
        return TIME_PASS;
    }
    else
    {
        log_printf("<!> find_access_time fail(file name : %s)", path);
        log_printf("    수정된 지 30초가 지나지 않았음 ");
        return 0;
    }
}

