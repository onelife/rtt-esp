#!/bin/bash
set -e

if [ -d "$IDF_PATH/components/freertos" ]
then
    echo -e "* RTOS already exists\n"
else
    if [ -d "$RTT_PATH/rtos" ]
    then
        ln -s $RTT_PATH/rtos $IDF_PATH/components/freertos
        echo -e "* RTOS ready\n"
    else
        echo -e "* RTOS does't exist\n"
    fi
fi

. $IDF_PATH/export.sh

exec "$@"
