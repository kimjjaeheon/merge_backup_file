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

#define BACKUP_IF_NUM 26 /* LOG / ERRLOG / INFOLOG �������̽� ���� */
#define NO_IF_NUM     1000 /* IF ��ȣ�� ���� �� Error code */

#define MERGE_FILE_NUM  1024 /* merge �� ������ ���� */

/* ��� mode */
#define DAY  0
#define TIME 1

/* TIME_SEP�� �ð����� �з� */
/* DAY_SEP��  �ϴ��� �з� */
#define TRAN_LOG_DAY_SEP     17   /* TRAN_LOG */
#define TRAN_LOG_TIME_SEP    19   /* TRAN_LOG */
#define TRX_SUMM_DAY_SEP     13   /* TRX_SUMM */
#define TRX_SUMM_TIME_SEP    15   /* TRX_SUMM */
#define TRAN_INFO_DAY_SEP    21   /* TRAN_INFO_LOG */
#define TRAN_INFO_TIME_SEP   23   /* TRAN_INFO_LOG */

/* INFO LOG �з� */
#define INFO_C 1
#define INFO_B 2
#define INFO_I 3
#define INFO_J 4
#define INFO_Q 5

/* log_day�� �ð����� ��¥���� üũ */
#define LOG_TIME 11  /* YYYYDDMMHH */
#define LOG_DAY  9   /* YYYYDDMM */

#define MY_PROCESS_NAME "merge_backup_file"


typedef struct
{
	char name[FILE_NAME_LEN];      /* ���� �� */
	char path[FILE_NAME_LEN];      /* ��θ� ������ ���ϸ�*/
	char backup_dir[DIR_NAME_LEN]; /* ��� ���丮 ��ġ */
	int  if_num;                   /* TRAN_LOG, TRAN_INFOLOG, TRAN_ERRLOG �� IF �ѹ� */
	int  dir_num;                  /* ��� ���丮 ��ȣ  */
	int  merge_file_idx;           /* append �� ���� ��ȣ */
	FILE *fpbin;                   /* ���� ������ */
	int  name_len;                 /* name ���� */
} FILE_INFO;

typedef struct
{
	FILE_INFO* files;
	int        total_file_cnt; // file�� �� ����
} FILE_LIST;

typedef struct
{
	char name[DIR_NAME_LEN];
} DIR_INFO;

typedef struct
{
	char path[FILE_NAME_LEN];               /* ����  ���      */
	char name[FILE_NAME_LEN];               /* ����  �̸�      */
	int  if_num;                            /* IF ��ȣ         */
	char log_day[LOG_TIME];                  /* ��¥ YYYYMMDDHH */
	char log_type;                          /* �α� Ÿ�� : INFO_C / B / I / J / Q */
	int  path_len;                         /* path ���� */
	int  name_len;                         /* name ���� */
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

