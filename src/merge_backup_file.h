#ifndef __merge_backup_file_h__
#define __merge_backup_file_h__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>

/* library */
#include "read_cfg.h"
#include "log_printf.h"
#include "usrutil.h"

/* include */
#include "common.h"
#include "tran_def_len.h"

#define MIN_FILE_CNT   399999 /* min file count */
#define FILE_NAME_LEN    128   /* file name len */
#define DIR_NAME_LEN     128   /* dir  name len */

#define NO_SUCH_FILE   2 /* error code */
#define FILE_OPEN_FAIL 3 /* error code */
#define PARSING_FAIL   1002

#define READ_BUF_SIZE 4096

#define BACKUP_IF_NUM 26 /* LOG / ERRLOG / INFOLOG 인터페이스 개수 */
#define NO_IF_NUM     1000 /* IF 번호가 없을 시 Error code */

#define MERGE_FILE_NUM  1024 /* merge 할 파일의 개수 */

/* 백업 mode */
#define DAY  0
#define TIME 1

/* TIME_SEP은 시간단위 분류 */
/* DAY_SEP은  일단위 분류 */
#define TRAN_LOG_DAY_SEP     17   /* TRAN_LOG */
#define TRAN_LOG_TIME_SEP    19   /* TRAN_LOG */
#define TRX_SUMM_DAY_SEP     13   /* TRX_SUMM */
#define TRX_SUMM_TIME_SEP    15   /* TRX_SUMM */
#define TRAN_INFO_DAY_SEP    21   /* TRAN_INFO_LOG */
#define TRAN_INFO_TIME_SEP   23   /* TRAN_INFO_LOG */

/* INFO LOG 분류 */
#define INFO_C 1
#define INFO_B 2
#define INFO_I 3
#define INFO_J 4
#define INFO_Q 5

/* log_day에 시간인지 날짜인지 체크 */
#define LOG_TIME 11  /* YYYYDDMMHH */
#define LOG_DAY  9   /* YYYYDDMM */

#define MY_PROCESS_NAME "merge_backup_file"


typedef struct
{
	char name[FILE_NAME_LEN];      /* 파일 명 */
	char path[FILE_NAME_LEN];      /* 경로를 포함한 파일명*/
	char backup_dir[DIR_NAME_LEN]; /* 백업 디렉토리 위치 */
	int  if_num;                   /* TRAN_LOG, TRAN_INFOLOG, TRAN_ERRLOG 용 IF 넘버 */
	int  dir_num;                  /* 백업 디렉토리 번호  */
	int  merge_file_idx;           /* append 할 파일 번호 */
	FILE *fpbin;                   /* 파일 포인터 */
	int  name_len;                 /* name 길이 */
} FILE_INFO;

typedef struct
{
	FILE_INFO* files;
	int        total_file_cnt; // file의 총 개수
} FILE_LIST;

typedef struct
{
	char name[DIR_NAME_LEN];
} DIR_INFO;

typedef struct
{
	char path[FILE_NAME_LEN];               /* 파일  경로      */
	char name[FILE_NAME_LEN];               /* 파일  이름      */
	int  if_num;                            /* IF 번호         */
	char log_day[LOG_TIME];                  /* 날짜 YYYYMMDDHH */
	char log_type;                          /* 로그 타입 : INFO_C / B / I / J / Q */
	int  path_len;                         /* path 길이 */
	int  name_len;                         /* name 길이 */
} MERGE_FILE;

typedef struct
{
	MERGE_FILE *files;	
} MERGE_FILE_LIST;

typedef struct
{
	char name[DIR_NAME_LEN];
} BACKUP_FILE;

#endif /* __merge_backup_file_h__ */

