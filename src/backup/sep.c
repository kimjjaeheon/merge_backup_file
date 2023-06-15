#include "merge_backup_file.h"
#include "sep.h"
#include "usrutil.h"

#define INFO_LOG_DAY_LOCATE     15
#define LOG_DAY_LOCATE          17
#define SUMM_DAY_LOCATE         1
#define ERR_LOG_DAY_LOCATE      17

#define DELIM_CHARS             "^"
#define SUMM_DELIM_CHARS        ","
#define C_PARSING               "DyCall"

/* Make 'TRAN_INFOLOG_B' file for value */
#define B_PARSING               "DyCall"
#define B_PARSING_NUM           14
#define B_PARSING_FETCH_TIME_S  11
#define B_PARSING_FETCH_LEN     8 

#define I_PARSING_NUM           17
#define DAY_BUF_SIZE            8

/* For tran_log parsing */
#define TRAN_LOG_EXCEPT_TUXEDO_ID 41

/* For tran_log parsing */
#define J_PARSING_NUM           11
#define J_PARSING_RESP_MSG      17
#define J_PARSING_XXX_CODE      3
#define J_PARSING_XXX_SVC       4
#define J_PARSING_SRC_ID        5
#define J_PARSING_XXX_LION      6
#define J_PARSING_XXX_LINE      9
#define J_PARSING_XXX_DOG       10

int parsing_C_log(char* buf, char* output);
int parsing_I_log(char* buff, char* output);
int parsing_B_log(char* buff, char* output);
int parsing_J_log(char* buff, char* output);

int sep_infolog(char* buff, char* output_buff, int log_type, char* log_day, int mode)
{
	int i = 0;
	char tmp_buff[READ_BUF_SIZE] = {0x00,};
	char *tmp_token, *tmp_next_token;

	memcpy(tmp_buff, buff, READ_BUF_SIZE);

	tmp_token = strtok_r(tmp_buff, DELIM_CHARS, &tmp_next_token);
	while (tmp_token)
	{
		tmp_token = strtok_r(NULL, DELIM_CHARS, &tmp_next_token);
		i++;
		if (i == (INFO_LOG_DAY_LOCATE - 1))
		{
			memcpy(log_day, tmp_token, DAY_BUF_SIZE);
			
		}
		if (i == INFO_LOG_DAY_LOCATE && mode == TIME)
		{
			memcpy(&(log_day[DAY_BUF_SIZE]), tmp_token, 2);
		}
	}

	switch (log_type)
	{
		case INFO_C: 
			if (parsing_C_log(buff, output_buff))
			{
				memset(output_buff, 0x00, sizeof(output_buff));
				memcpy(output_buff, buff, READ_BUF_SIZE);
				return PARSING_FAIL;
			}
			break;
		case INFO_B:
			if (parsing_B_log(buff, output_buff) == PARSING_FAIL)
			{
				memset(output_buff, 0x00, sizeof(output_buff));
				memcpy(output_buff, buff, READ_BUF_SIZE);
				return PARSING_FAIL;
			}
			break;
		case INFO_I:
			if (parsing_I_log(buff, output_buff) == PARSING_FAIL)
			{
				memset(output_buff, 0x00, sizeof(output_buff));
				memcpy(output_buff, buff, READ_BUF_SIZE);
				return PARSING_FAIL;
			}
			break;
		case INFO_J:
			/* 추가 요청으로 인한 J parsing */
			if (parsing_J_log(buff, output_buff) == PARSING_FAIL)
			{
				memset(output_buff, 0x00, sizeof(output_buff));
				memcpy(output_buff, buff, READ_BUF_SIZE);
				return PARSING_FAIL;
			}
			log_printf("[parsing_J_log] \n");
			log_printf(" buff : %s \n", buff);
			log_printf(" output_buff : %s \n", output_buff);
			break;
		case INFO_Q:
			/* Q는 parsing 하지 않는다. */
			break;
		default:
			printf("NO log type\n");
	}

	return 0;
}

int sep_log(char* buff, char* output_buff, char* log_day, int mode)
{
/* tran_log 디렉토리는 parsing 하지 않는다.
 * 따라서 파일을 만드는데 필요한 날짜만 체크한다.
 * 2023.01 version
 */

    int i = 0, j = 0, buff_len = 0, del_cnt = 0;
    char tmp_buff[READ_BUF_SIZE] = {0x00,};
    char *tmp_token, *tmp_next_token;

    memcpy(tmp_buff, buff, READ_BUF_SIZE);

    tmp_token = strtok_r(tmp_buff, DELIM_CHARS, &tmp_next_token);
    while (tmp_token)
    {
        tmp_token = strtok_r(NULL, DELIM_CHARS, &tmp_next_token);
        i++;
        if (i == (LOG_DAY_LOCATE - 1))
        {
            memcpy(log_day, tmp_token, DAY_BUF_SIZE);
        }
        if (i == LOG_DAY_LOCATE && mode == TIME)
        {
            memcpy(&(log_day[DAY_BUF_SIZE]), tmp_token, 2);
        }
    }

/* 추가 요청사항으로 맨 마지막 값인 tuxedo id 값을 뺀다.
 * 2023.05 version
 */

    buff_len = strlen(buff);

    for(j=0; j<buff_len ;j++)
    {
		if(buff[j] == '^')
		{
			del_cnt++;

			/* 마지막 부분인 TUXEDO_ID 제거 후 for 문 종료 */
			if(del_cnt == TRAN_LOG_EXCEPT_TUXEDO_ID)
			{
				output_buff[j] = '\n';
				break;
			}
		}

		output_buff[j] = buff[j];
    }

    return 0;
}

int sep_trx_summ(char* buff, char* log_day, int mode)
{
    int i = 0;
    char tmp_buff[READ_BUF_SIZE] = {0x00,};
    char *tmp_token, *tmp_next_token;

    memcpy(tmp_buff, buff, READ_BUF_SIZE);

    tmp_token = strtok_r(tmp_buff, SUMM_DELIM_CHARS, &tmp_next_token);
    while (tmp_token)
    {
        if (i == (SUMM_DAY_LOCATE - 1))
        {
            memcpy(log_day, tmp_token, DAY_BUF_SIZE);
			//printf("<sep> log_day:  %s", log_day);
        }
        if (i == SUMM_DAY_LOCATE && mode == TIME)
        {
            memcpy(&(log_day[DAY_BUF_SIZE]), tmp_token, 2);
			//printf("<sep> log_day:  %s", log_day);
			break;
        }
        tmp_token = strtok_r(NULL, SUMM_DELIM_CHARS, &tmp_next_token);
        i++;
    }
}

int sep_errlog(char* buff, char* log_day, int mode)
{
    int i = 0;
    char tmp_buff[READ_BUF_SIZE] = {0x00,};
    char *tmp_token, *tmp_next_token;

    memcpy(tmp_buff, buff, READ_BUF_SIZE);

    tmp_token = strtok_r(tmp_buff, DELIM_CHARS, &tmp_next_token);
    while (tmp_token)
    {
        tmp_token = strtok_r(NULL, DELIM_CHARS, &tmp_next_token);
        i++;
        if (i == (ERR_LOG_DAY_LOCATE - 1))
        {
            memcpy(log_day, tmp_token, DAY_BUF_SIZE);
        }
        if (i == ERR_LOG_DAY_LOCATE && mode == TIME)
        {
            memcpy(&(log_day[DAY_BUF_SIZE]), tmp_token, 2);
        }
    }

	return 0;
}

int parsing_C_log(char* buff, char* output_buff)
{
	int i = 0, len = 0, j = 0;
	char tmp_buff[READ_BUF_SIZE] = {0x00,};
	char buf2[15][100];
	int  buf2_len = 0, tmp_buf_idx = 0;
	char message_buff[READ_BUF_SIZE] = {0x00,};
	int n = 0;

    memset(buf2, 0x00, sizeof(buf2));
    for (i = 0; i < 15; i++)
    {
        memset(buf2[i], 0x00, sizeof(buf2[i]));
    }

	memcpy(tmp_buff, buff, READ_BUF_SIZE);

	len = strlen(tmp_buff);

	/* 문자열 길이 만큼 parsing */
	for (i = 0; i < len; i++)
	{
		if (tmp_buff[i] == ',')
		{
			//tmp_buff[i] = '^';
			output_buff[i] = '^';
		}
		else if ((tmp_buff[i] == '[') && !(memcmp(&(tmp_buff[i+1]),C_PARSING,6))) /* [DyCall 일때 parsing */
		{
			memcpy(message_buff, &(tmp_buff[i]), READ_BUF_SIZE);
			n = sscanf(message_buff, "[%[^]]][%s at %s (%[^:]::%[^)]){%[^}]}]"
				, buf2[0]
				, buf2[1]
				, buf2[2]
				, buf2[3]
				, buf2[4]
        		, buf2[5]);
			if ( n != 6 )
			{
				log_printf("<!> [sep.c] infolog 를 parsing 하지 못하였습니다. ");
				log_printf("    -> parsing data 확인 필요 (C_infolog) : %s", tmp_buff);
				return PARSING_FAIL;
			}
			else
			{
				int c = 0;
				for ( ; c<n; ++c)
				{
					buf2_len = strlen(buf2[c]);	
					if (c == 0)
					{
						memcpy(&(output_buff[i]), buf2[c], buf2_len);
						tmp_buf_idx = buf2_len + i;	
						output_buff[tmp_buf_idx] = '^';
						tmp_buf_idx += 1;
					}
					else if (c == (n-1))  /* 마지막 인덱스 일 때 */
					{	
						memcpy(&(output_buff[tmp_buf_idx]), buf2[c], buf2_len);
						tmp_buf_idx += buf2_len;
						output_buff[tmp_buf_idx] = '\n';
						break;
					}
					else
					{
						memcpy(&(output_buff[tmp_buf_idx]), buf2[c], buf2_len);
						tmp_buf_idx += buf2_len;
						output_buff[tmp_buf_idx] = '^';
						tmp_buf_idx += 1;
					}
				}
				break;	
			}
		}
		else
		{
			output_buff[i] = tmp_buff[i];
		}
		
	}
	return 0;
}

int parsing_I_log(char* buff, char* output_buff)
{
	int n = 0, i = 0, len = 0;
	char tmp_buff[READ_BUF_SIZE] = {0x00,};
	char buf2[15][100];
	int buf2_len = 0, tmp_buf_idx = 0;
	char message_buff[READ_BUF_SIZE] = {0x00,};
	char parsing_num = 0;

    memset(buf2, 0x00, sizeof(buf2));
    for (i = 0; i < 15; i++)
    {
        memset(buf2[i], 0x00, sizeof(buf2[i]));
    }
    memcpy(tmp_buff, buff, READ_BUF_SIZE);

    len = strlen(tmp_buff);
	
	for (i = 0; i < len; i++)
	{
		
		if (tmp_buff[i] == '^')
		{
			parsing_num++;	
			output_buff[i] = '^';
			continue;
		}

		if (parsing_num == (I_PARSING_NUM - 1))
		{
#if 1	
	n = sscanf(&(tmp_buff[i]), "처리 모듈(함수)통계 [ 총 호출 %s 건 ] [ Max Depth %s ] [ BIZCALL(Method) %[^건]건 (전체 Avg/Max %[^/]/ %s ms, 개별 Max %s ms) ], [ DBIO %[^건]건 (Avg/Max %[^/]/ %s ms) %s times fetched rows %[^(]( %s ms)"
				, buf2[0]
				, buf2[1]
				, buf2[2]
				, buf2[3]
				, buf2[4]
				, buf2[5]
				, buf2[6]
				, buf2[7]
				, buf2[8]
				, buf2[9]
				, buf2[10]
				, buf2[11]
				);
#else
	n = sscanf(&(tmp_buff[i]), "처리 모듈(함수)통계 [ 총 호출 %s 건 ] [ Max Depth %s ] [ BIZCALL(Method) %[^건]건 (전체 Avg/Max %[^/]/ %s ms, 개별 Max %s ms) ], [ DBIO %[^건]건 (Avg/Max %[^/]/ %s ms) "
				, buf2[0]
				, buf2[1]
				, buf2[2]
				, buf2[3]
				, buf2[4]
				, buf2[5]
				, buf2[6]
				, buf2[7]
				, buf2[8]
				);
#endif

			if ( n != 12 )
			{           
				log_printf("<!> [sep.c] infolog 를 parsing 하지 못하였습니다. ");
				log_printf("    -> parsing data 확인 필요 (I_infolog) : %s", tmp_buff);
				return PARSING_FAIL;
			}       
			else        
			{           
				int c = 0;
				for ( ; c<n; ++c)
				{   
					buf2_len = strlen(buf2[c]);
					if (c == 0)
					{   
						memcpy(&(output_buff[i]), buf2[c], buf2_len);
				 		tmp_buf_idx = buf2_len + i; 
						output_buff[tmp_buf_idx] = '^';
						tmp_buf_idx += 1;
					}
					else if (c == 11)  /* 마지막 인덱스 일 때 */
					{
						memcpy(&(output_buff[tmp_buf_idx]), buf2[c], buf2_len);
						tmp_buf_idx += buf2_len;
						output_buff[tmp_buf_idx] = '\n';
						break;
					}
					else
					{
						 memcpy(&(output_buff[tmp_buf_idx]), buf2[c], buf2_len);
						 tmp_buf_idx += buf2_len;
						 output_buff[tmp_buf_idx] = '^';
						 tmp_buf_idx += 1;
					}
				 }
				 break;
			}

			parsing_num++;
		}
        else
        {   
            output_buff[i] = tmp_buff[i];
        }
	}
	return 0;
}

int parsing_B_log(char* buff, char* output_buff)
{
	int i = 0, len = 0, j = 0;
	char tmp_buff[READ_BUF_SIZE] = {0x00,};
	char buf2[B_PARSING_NUM][100];
	int  buf2_len = 0, tmp_buf_idx = 0;
	char message_buff[READ_BUF_SIZE] = {0x00,};
	int n = 0;
	char *pos = NULL;

	memset(buf2, 0x00, sizeof(buf2));
	for (i = 0; i < B_PARSING_NUM; i++)
	{
		memset(buf2[i], 0x00, sizeof(buf2[i]));
	}
	
	memcpy(tmp_buff, buff, READ_BUF_SIZE);

	len = strlen(tmp_buff);

	/* 문자열 길이 만큼 parsing */
	for (i = 0; i < len; i++)
	{
		if (tmp_buff[i] == ',')
		{
			output_buff[i] = '^';
		}
		else if ((tmp_buff[i] == '[') && !(memcmp(&(tmp_buff[i+1]),B_PARSING,6))) /* [DyCall 일때 parsing */
		{
			memcpy(message_buff, &(tmp_buff[i]), READ_BUF_SIZE);
			n = sscanf(message_buff, "[%[^]]][%s at %s (%[^:]::%[^)]){%[^}]}]#SQLCODE %s (line %[^)]) %s times fetched rows %[^(](%s ms) DB %[^/]/%s"
				, buf2[0]
				, buf2[1]
				, buf2[2]
				, buf2[3]
				, buf2[4]
        		, buf2[5]
        		, buf2[6]
        		, buf2[7]
        		, buf2[8]
        		, buf2[9]
        		, buf2[10]
        		, buf2[11]
        		, buf2[12]);
			if (n == 11)
			{
				/* DB_USER / DBSID 부분에 모두 값이 NULL 일때 처리*/
				buf2[11][0] = 0x00;
				buf2[12][0] = 0x00;
			}
			else if (n == 12)
			{
				/*DBSID 부분에 값이 NULL 일때 처리*/
				buf2[12][0] = 0x00;
			}
			else
			{
				if (n != B_PARSING_NUM)
				{
					/* parsing 실패 */
					log_printf("<!> [sep.c] infolog 를 parsing 하지 못하였습니다. ");
					log_printf("    -> parsing data 확인 필요 (B_infolog) : %s", tmp_buff);
					return PARSING_FAIL;	
				}
			}

			int c = 0;
			for ( ; c<B_PARSING_NUM; ++c)
			{
				buf2_len = strlen(buf2[c]);	
				if (c == 0)
				{
					memcpy(&(output_buff[i]), buf2[c], buf2_len);
					tmp_buf_idx = buf2_len + i;	
					output_buff[tmp_buf_idx] = '^';
					tmp_buf_idx += 1;
				}
				/* find "FETCH_TIME_S" column */
				else if (c == (B_PARSING_FETCH_TIME_S-1))
				{
					char fetch_time_s[B_PARSING_FETCH_LEN];
					char make_fetch_time_string[READ_BUF_SIZE]={0x00,}; 
					double tmp_fetch_time_s = 0, tmp_fetch_time_ms = 0;
					int make_fetch_time_string_len = 0;

					memcpy(fetch_time_s, buf2[c], buf2_len);

					tmp_fetch_time_s = strtod(fetch_time_s, &pos);
					tmp_fetch_time_ms = tmp_fetch_time_s * 1000;

					sprintf(make_fetch_time_string, "%.0f^%.0f^", tmp_fetch_time_s, tmp_fetch_time_ms);
			
					make_fetch_time_string_len = strlen(make_fetch_time_string);
					
					/* fetch_time_s */
					memcpy(&(output_buff[tmp_buf_idx]), make_fetch_time_string, make_fetch_time_string_len);
					tmp_buf_idx += make_fetch_time_string_len;	
				}
				/* last index */
				else if (c == (B_PARSING_NUM-1))  
				{	
					memcpy(&(output_buff[tmp_buf_idx]), buf2[c], buf2_len);
					tmp_buf_idx += buf2_len;
					output_buff[tmp_buf_idx] = '\n';
					break;
				}
				else
				{
					memcpy(&(output_buff[tmp_buf_idx]), buf2[c], buf2_len);
					tmp_buf_idx += buf2_len;
					output_buff[tmp_buf_idx] = '^';
					tmp_buf_idx += 1;
				}
			}
			break;	
		}
		else
		{
			output_buff[i] = tmp_buff[i];
		}
		
	}

	return 0;
}

int parsing_J_log(char* buff, char* output_buff)
{
	int i = 0, j =0, n = 0;
	char tmp_buff[READ_BUF_SIZE] = {0x00,};
	char* result[READ_BUF_SIZE];
	char message_buff[READ_BUF_SIZE] = {0x00,};		
	int  len = 0, result_len = 0, tmp_buf_idx = 0, message_buff_len = 0, tmp_buff_len = 0;
	int del_cnt = 0;
		
	memcpy(tmp_buff, buff, READ_BUF_SIZE);

	len = strlen(tmp_buff);
	
	for(i = 0; i < len; i++)
	{
		if(tmp_buff[i] == '^')
		{
			/* 구분자 개수 체크 */
			del_cnt++;
			output_buff[i] = tmp_buff[i];
			continue;
		}

		/* Find RESP_MSG */
		if(del_cnt == (J_PARSING_RESP_MSG-1))
		{
			tmp_buff_len = strlen(&tmp_buff[i]);
			memcpy(message_buff, &(tmp_buff[i]), tmp_buff_len);
			log_printf("split 전  message_buf : %s \n", message_buff);

			n = str_split(message_buff, "[]:;", result, tmp_buff_len);

			if((n == J_PARSING_NUM) || (n == J_PARSING_NUM+1))
			{
				for ( ; j<n; j++)
				{
					result_len = strlen(result[j]);
					log_printf("result[%d] : %s , result_len : %d \n", j, result[j], result_len);
					
					if (j == 0)
					{
						memcpy(&(output_buff[i]), result[j], result_len);
						tmp_buf_idx = result_len + i; 
						output_buff[tmp_buf_idx] = '^';
						tmp_buf_idx += 1;
					}
					else if((j == J_PARSING_XXX_CODE) || (j == J_PARSING_XXX_SVC) || (j == J_PARSING_SRC_ID) || (j == J_PARSING_XXX_LION) || (j == J_PARSING_XXX_LINE))
					{
						memcpy(&(output_buff[tmp_buf_idx]), result[j], result_len);
						tmp_buf_idx += result_len; 
						output_buff[tmp_buf_idx] = '^';
						tmp_buf_idx += 1;
					}
					/* last index */
					else if(j == J_PARSING_XXX_DOG)
					{
						log_printf("result[%d] : %s , result_len : %d \n", j, result[j], result_len);
						memcpy(&(output_buff[tmp_buf_idx]), result[j], result_len);
						tmp_buf_idx += result_len; 
                    	output_buff[tmp_buf_idx] = '\n';
						break;
					}
					else
					{
						continue;
					}
				}
			}
			else
			{
#if 0
				/* str_split() 함수에서 message_buff가 짤리기때문에 원본으로 그대로 넣을 경우 */
				memcpy(&(output_buff[i]), &(tmp_buff[i]), tmp_buff_len);

				/* 문자열을 버릴 경우 */
#else
				/* parsing 실패 */
				log_printf("<!> [sep.c] infolog 를 parsing 하지 못하였습니다. ");
				log_printf("    -> parsing data 확인 필요 (J_infolog) : %s", tmp_buff);
				return PARSING_FAIL;	
#endif
			}

			break;
		}
		else
		{
			output_buff[i] = tmp_buff[i];
		}
	}

	return 0;
}
