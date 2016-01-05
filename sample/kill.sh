#/bin/bash

pid=`cat logs/nginx.pid`

kill $pid
