#!/bin/bash

timeout=8000 # in ms

urls=(\
http://ftp.sjtu.edu.cn \
http://ipads.se.sjtu.edu.cn/courses/ics/ \
http://ipads.se.sjtu.edu.cn/courses/ics/labs/proxylab/trustkernel.html \
http://ipads.se.sjtu.edu.cn/courses/ics/labs/proxylab/electsys.html \
http://ipads.se.sjtu.edu.cn/courses/ics/labs/proxylab/2048.html \
http://ipads.se.sjtu.edu.cn/courses/ics/labs/proxylab/video.html \
http://ipads.se.sjtu.edu.cn/courses/ics/labs/proxylab/163.html \
)

isos=(\
http://mirrors.163.com/ubuntu-releases/14.04/ubuntu-14.04-desktop-amd64.iso \
http://mirrors.sohu.com/FreeBSD/amd64/ISO-IMAGES/9.2/FreeBSD-9.2-RELEASE-amd64-disc1.iso \
)

cwd=`pwd`
test=$cwd/test
ipads=ipads.se.sjtu.edu.cn/courses/ics
lab=$ipads/labs/proxylab


function timed_wget {
local args=$*
local t=0
local s=100
wget $args &> /dev/null &
local pid=$!
while [ $t -lt $timeout ]; do
    sleep 0.100
    t=$((t+s))
    #echo $t / $timeout
    ps | grep $pid &> /dev/null
    if [ $? -ne 0 ]; then
        return
    fi
done
echo "timeout, killed"
kill $pid
}

function reset_and_cd {
cd $test
if [ -d $1 ]; then
    rm -rf $1
fi
mkdir $1 && cd $1
}

function set_proxy {
export http_proxy="127.0.0.1:8800"
export https_proxy="127.0.0.1:8800"
}


function pp {

local tot=0
set_proxy

reset_and_cd pp1
echo "downloading iso via proxy..."
echo "downloading: ${isos[0]}";
wget --limit-rate=20k ${isos[0]} &> /dev/null &
echo "downloading via proxy..."
echo "downloading: ${urls[6]}";
timed_wget -L -p ${urls[6]} &> /dev/null
killall wget &> /dev/null
run_grade $test 6 pp1 $lab/163.html $lab/163_files
tot=$((tot+$?))

reset_and_cd pp2
echo "downloading iso via proxy..."
echo "downloading: ${isos[0]}";
wget --limit-rate=20k ${isos[0]} &> /dev/null &
echo "downloading: ${isos[1]}";
wget --limit-rate=20k ${isos[1]} &> /dev/null &
echo "downloading via proxy..."
echo "downloading: ${urls[5]}";
timed_wget -L -p ${urls[5]} &> /dev/null
echo "downloading: ${urls[6]}";
timed_wget -L -p ${urls[6]} &>> /dev/null
killall wget &> /dev/null
run_grade $test 5 pp2 $lab/video.html $lab/video_files
tot=$((tot+$?))
run_grade $test 5 pp2 $lab/163.html $lab/163_files
tot=$((tot+$?))

# check log file
logscore=9
while read -r l;
do
    w=`echo $l | wc -w`
    if [ $w -ne 9 ]; then
        echo $l
        logscore=0
        break
    fi
done < $cwd/proxy.log;

echo Logfile Score: $logscore

echo Score for Part II: $((tot+logscore))
}

function download_all {
for url in ${urls[*]}
do
    echo "downloading: $url";
    timed_wget -L -p $url
done;
}


function try_make_std {
# build test directory
cd $test
if [ ! -d "std" ]; then
    mkdir std && cd std
    unset http_proxy
    unset https_proxy
    echo "downloading std"
    download_all
fi
}


function force_make_proxy {
# remove existing dir
reset_and_cd "proxy"
export http_proxy="127.0.0.1:8800"
export https_proxy="127.0.0.1:8800"
echo "downloading via proxy"
download_all
}


function run_grade {
local tot=0
local pass=0
local path=$1
local score=$2
local cmp=$3
shift 3
for arg in $@;
do
    echo $arg;
    std_arg=$path/std/$arg
    if [ -d $std_arg ]; then
        for std_f in `find $std_arg -type f | grep -v robots.txt`;
        do
            tot=$((tot+1))
            proxy_f=`echo $std_f | sed s/std/$cmp/g`
            diff $std_f $proxy_f
            if [ $? -eq 0 ]; then
                pass=$((pass+1))
            fi
        done;
    else
        tot=$((tot+1))
        std_f=$std_arg
        proxy_f=`echo $std_f | sed s/std/$cmp/g`
        diff $std_f $proxy_f
        if [ $? -eq 0 ]; then
            pass=$((pass+1))
        fi
    fi
done
echo $pass/$tot
echo Score: $((score*(2*pass/tot)/2))
return $((score*(2*pass/tot)/2))
}

function run_diff {
local tot=0

run_grade $test 4 proxy ftp.sjtu.edu.cn
tot=$((tot+$?))
ipads=ipads.se.sjtu.edu.cn/courses/ics
run_grade $test 4 proxy $ipads/index.html $ipads/ics.css
tot=$((tot+$?))

lab=$ipads/labs/proxylab
run_grade $test 5 proxy $lab/trustkernel.html $lab/trustkernel_files
tot=$((tot+$?))
run_grade $test 5 proxy $lab/electsys.html $lab/electsys_files
tot=$((tot+$?))
run_grade $test 5 proxy $lab/2048.html $lab/2048_files
tot=$((tot+$?))

run_grade $test 6 proxy $lab/video.html $lab/video_files
tot=$((tot+$?))
run_grade $test 6 proxy $lab/163.html $lab/163_files
tot=$((tot+$?))

echo Score for Part I: $tot
}


killall wget &> /dev/null
cd $cwd
# compile and run the proxy
killall proxy
make clean && make
rm -f proxy.log
./proxy 8800 &> proxy_run.log &

mkdir test && cd test
try_make_std
echo "==================================================="
echo "        Part I"
echo "==================================================="
force_make_proxy
run_diff

echo "==================================================="
echo "        Part II"
echo "==================================================="
pp

