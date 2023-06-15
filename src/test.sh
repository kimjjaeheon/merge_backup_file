#!/bin/bash

function Trxsumm_Test()
{
	F=( `ls -al /data/tranm/backup/tran_statis/trx_summ_jh/TRX_SUMM_1209* | awk -F' ' '{print $5}'` )
	T=( `ls -al /data/tranm/backup/tran_statis/trx_summ_jh/TRX_SUMM_2022* | awk -F' ' '{print $5}'` )
	f_len=0
	t_len=0
	i=0
	j=0
	for f in ${F[@]}
	do
		(( f_len += f ))
		(( i++ ))
	done


	for t in ${T[@]}
	do
		(( t_len += t ))
		(( j++ ))
	done


	echo "<TRX_SUMM> f_len : ${f_len}, file 개수 : ${i}"
	echo "<TRX_SUMM> t_len : $t_len, file 개수 : ${j}"

	echo "---------------------------------------------------------------------------"
}

function Log_Test()
{
	F=( `ls -al /data/tranm/backup/tran_log_jh/01/TRAN_LOG-* | awk -F' ' '{print $5}'` )
	T=( `ls -al /data/tranm/backup/tran_log_jh/01/TRAN_LOG_* | awk -F' ' '{print $5}'` )
	f_len=0
	t_len=0
	i=0
	j=0

	for f in ${F[@]}
	do
		(( f_len += f ))
		(( i++ ))
	done


	for t in ${T[@]}
	do
		(( t_len += t ))
		(( j++ ))
	done

	echo "<TRAN_LOG> file_summ_len : $f_len, file 개수 :  ${i}"
	echo "<TRAN_LOG> merge_file_len : $t_len, file 개수 : ${j}"

	echo "---------------------------------------------------------------------------"
}

function Errlog_Test()
{
#	F=( `ls -al /data/tranm/backup/tran_errlog_jh/01/TRAN_ERRLOG-* | awk -F' ' '{print $5}'` )
#	T=( `ls -al /data/tranm/backup/tran_errlog_jh/01/TRAN_ERRLOG_* | awk -F' ' '{print $5}'` )
	F=( `ls -al /data/tranmgr/datfs/backup_tmp/tran_errlog/01/TRAN_ERRLOG-* | awk -F' ' '{print $5}'` )
	T=( `ls -al /data/tranmgr/datfs/backup_tmp/tran_errlog/01/TRAN_ERRLOG_* | awk -F' ' '{print $5}'` )
	f_len=0
	t_len=0
	i=0
	j=0

	for f in ${F[@]}
	do
		(( f_len += f ))
		(( i++ ))
	done


	for t in ${T[@]}
	do
		(( t_len += t ))
		(( j++ ))
	done

	echo "<TRAN_ERRLOG> file_summ_len : $f_len, file 개수 :  ${i}"
	echo "<TRAN_ERRLOG> merge_file_len : $t_len,  file 개수 : ${j}"


	echo "---------------------------------------------------------------------------"
}

function Infolog_Test()
{
	F=( `ls -al /data/tranmgr/datfs/backup_tmp/tran_infolog/01/TRAN_INFOLOG_J_
	T=( `ls -al /data/tranmgr/datfs/backup_tmp/tran_infolog/01/TRAN_INFOLOG_J_* | awk -F' ' '{print $5}'` )
}

#Trxsumm_Test
#Log_Test
#Errlog_Test
Infolog_Test
