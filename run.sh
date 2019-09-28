#!/bin/bash


mrmake(){
	mkdir -p ./build/linux
	cd ./build/linux
	cmake ../../
	make
}

mrkill(){
	PID=$(ps -ef|grep ${1}|grep -v grep)
	if [[ ! -z $PID ]]; then
		PID=$(echo "${PID}"|awk '{print $2}')
		kill -9 ${PID}
	fi
}


case ${1} in
	make)run_make ${@};;
	kill)shift 1;run_kill ${@};;
	*)echo "Usage : ${@} unknown"
esac