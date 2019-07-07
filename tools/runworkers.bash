#!/bin/bash
# 
# @file:    runworkers.bash
#

handle_children() {
    local tmp=""
    for ppid in ${pids}
    do
        local pid="$((`jobs -lp | grep ${ppid}`))"
        #echo "${ppid} -> ${pid}"
        if [ "${pid}" -ne 0 ]
        then
            tmp="${tmp} ${ppid}"
        else
            wait ${ppid}
            local result=$?
            if [ $result -ne 0 ]
            then
                echo -e "\tthe worker that has the pid ${ppid} was terminated with the code: ${result}"
                echo -n "${tred}${toend}[FAIL]"
                echo -n "${treset}"
                echo
                exit
            fi
        fi
    done
    pids=${tmp}
}

tred=$(tput setf 4)
tgreen=$(tput setf 2)
treset=$(tput sgr0)
toend=$(tput hpa $(tput cols))$(tput cub 6)

counter=0
counter_max=10
counter_total=100
dataset="ouroboros"
tbl_count=10
rec_count=10000

pids=""
workername=""

if [ -n "$1" ]
then
    workername=$1
else
    echo "error: worker name is missing"
    exit
fi

if [ -n "$2" ]
then
    counter_total=$2
fi

if [ -n "$3" ]
then
    counter_max=$3
fi

echo -e "remove old files of the dataset '${dataset}' ..."
rm -f ./${dataset}.dat ./${dataset}.bak ./${dataset}.jrn ./dataset.log /dev/shm/ouroborosworker_transaction
if [ $? -ne 0 ]
then
    echo -n "${tred}${toend}[FAIL]"
    echo -n "${treset}"
    echo
    exit
fi

echo -n "${tgreen}${toend}[OK]"
echo -n "${treset}"
echo

echo -e "run workers:"
itr=1
while [ $itr -le $counter_total ]
do
    attr="-n ${dataset} -t${tbl_count} -r${rec_count} -p"
    key=$((RANDOM % ($tbl_count)))
    attr="${attr} -k${key}"
    itr_count=$(((RANDOM % 100) + 1))
    attr="${attr} -i${itr_count}"
    is_writer=$((RANDOM % 2))
    is_full=$((RANDOM % 2))
    is_except=$((RANDOM % 100))
    is_transact=$((RANDOM % 4))
    if [ $is_writer -ne 0 ]
    then
        attr="${attr} -w"
    else
        if [ $is_full -ne 0 ]
        then
            attr="${attr} -f"
        fi
    fi
    if [ $is_except -ge 70 ]
    then
        attr="${attr} -e"
    fi
    if [ $is_transact -eq 0 ]
    then
        attr="${attr} -s"
    fi
    echo -n -e "run the worker ${itr} with the attributes '${attr}' ..."

    worker="${workername} ${attr}"
    $worker > /dev/null &
    cpid="${!}"
    sleep 0.1
    kill -s INT "${cpid}"
    pids="${pids} ${cpid}"

    echo -n "${tgreen}${toend}[OK]"
    echo -n "${treset}"
    echo
    handle_children
    counter=$((`jobs | wc -l`))
    while [ $counter -ge $counter_max ]
    do
        #echo -e "wait ..."
        sleep 1
        counter=$((`jobs | wc -l`))
        handle_children
    done
    itr=$(($itr+1))
done
echo -n "${treset}"
echo

counter=$((`jobs | wc -l`))
echo -e "wait for the completion of ${counter} workers ..."
while [ -n "${pids}" ]
do
    handle_children
    jobs > /dev/null
    sleep 1
    count=$((`jobs | wc -l`))
    if [ $counter -ne $count ]
    then
        counter=$count
        echo -e "wait for the completion of ${counter} workers ..."
    fi
done
echo -n "${tgreen}${toend}[OK]"
echo -n "${treset}"
echo
