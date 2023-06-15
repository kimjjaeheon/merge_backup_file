#ifndef __sep_h__
#define __sep_h__

int sep_infolog(char* buff, char* output_buff, int log_type, char* log_day, int mode);
int sep_log(char* buff, char* output_buff, char* log_day, int mode);
int sep_trx_summ(char* buff, char* log_day, int mode);
int sep_errlog(char* buff, char* log_day, int mode);

#endif /*__sep_h__*/

