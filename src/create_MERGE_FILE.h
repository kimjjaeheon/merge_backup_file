#ifndef __create_merge_file_h__
#define __create_merge_file_h__

int merge(char* path, char* output);
int create_TRAN_LOG_file(FILE_LIST* file_list, MERGE_FILE_LIST* merge_file_list, int mode);
int create_TRX_SUMM_file(FILE_LIST* file_list, MERGE_FILE_LIST* merge_file_list, int mode);
int create_TRAN_INFO_LOG_file(FILE_LIST* file_list, MERGE_FILE_LIST* merge_file_list, int mode);
int create_TRAN_ERRLOG_file(FILE_LIST* file_list, MERGE_FILE_LIST* merge_file_list, int mode);

#endif /*__create_merge_file_h__*/

