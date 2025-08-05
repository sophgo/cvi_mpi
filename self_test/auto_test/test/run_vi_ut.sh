#!/bin/sh

BASEDIR=$(dirname "$0")
source $BASEDIR/../common/pre_check.sh
source $BASEDIR/../common/define.sh

UT_BIN_NAME=vi_ut
OUT_FILE="tmp_output"
INPUT_FILE="tmp_input"
result=$TEST_PASS
check_ret=0

LINEAR_UT_TEST="1 5 6 7 10 12 13 14"
WDR_UT_TEST="1 6 7 10 12 13 14"
WDR_LINEAR_UT="6 10 12"
YUV_UT="6 10 13 14"
MIPI_SWITCH_UT="6 10 11"

INI_SRC_LINEAR_PATH=$BASEDIR/../res/sensor_cfg.ini.327_linear
INI_SRC_WDR_PATH=$BASEDIR/../res/sensor_cfg.ini.327_wdr
INI_SRC_WDR_LINEAR_PATH=$BASEDIR/../res/sensor_cfg.ini.327_wdr_4653
INI_SRC_YUV_PATH=$BASEDIR/../res/sensor_cfg.ini.pr2100
INI_SRC_TRIPLE_MIPI_SWITCH_PATH=$BASEDIR/../res/sensor_cfg.ini.327_triple_mipi_switch
INI_SRC_TWO_MIPI_SWITCH_PATH=$BASEDIR/../res/sensor_cfg.ini.327_two_mipi_switch
INI_SRC_TWO_NORMAL_MIPI_SWITCH_PATH=$BASEDIR/../res/sensor_cfg.ini.327_two_mipi_switch_normal
INI_DST_PATH=/mnt/data/sensor_cfg.ini

function verify() {
    grep_result=$(grep "] pass" $OUT_FILE)

    if [ -z "$grep_result" ]; then
        check_ret=-1
    fi
}

function clean_tmp_files() {
    rm -rf $OUT_FILE $INPUT_FILE
}

function run_ut_linear_test() {

    cp $INI_SRC_LINEAR_PATH $INI_DST_PATH

    for i in $LINEAR_UT_TEST
    do
        echo "========== test $i =========="
        cat /sys/kernel/debug/ion/cvi_carveout_heap_dump/summary
        $UT_BIN_DIR/$UT_BIN_NAME $i < $INPUT_FILE | tee $OUT_FILE
        verify
        if [ $check_ret != 0 ]; then
            result=$TEST_FAIL
            return 1
        fi

        sleep $SLEEP_SECONDS
    done
}

function run_ut_wdr_test() {

    cp $INI_SRC_WDR_PATH $INI_DST_PATH

    for i in $WDR_UT_TEST
    do
        echo "========== test $i =========="
        cat /sys/kernel/debug/ion/cvi_carveout_heap_dump/summary
        $UT_BIN_DIR/$UT_BIN_NAME $i < $INPUT_FILE | tee $OUT_FILE
        verify
        if [ $check_ret != 0 ]; then
            result=$TEST_FAIL
            return 1
        fi

        sleep $SLEEP_SECONDS
    done
}

function run_ut_wdr_linear_test() {

    cp $INI_SRC_WDR_LINEAR_PATH $INI_DST_PATH

    for i in $WDR_LINEAR_UT
    do
        cat /sys/kernel/debug/ion/cvi_carveout_heap_dump/summary
        echo "========== test $i =========="
        $UT_BIN_DIR/$UT_BIN_NAME $i < $INPUT_FILE | tee $OUT_FILE
        verify
        if [ $check_ret != 0 ]; then
            result=$TEST_FAIL
            return 1
        fi

        sleep $SLEEP_SECONDS
    done
}

function run_ut_yuv_test() {
    cp $INI_SRC_YUV_PATH $INI_DST_PATH

    for i in $YUV_UT
    do
        cat /sys/kernel/debug/ion/cvi_carveout_heap_dump/summary
        echo "========== test $i =========="
        $UT_BIN_DIR/$UT_BIN_NAME $i < $INPUT_FILE | tee $OUT_FILE
        verify
        if [ $check_ret != 0 ]; then
            result=$TEST_FAIL
            return 1
        fi

        sleep $SLEEP_SECONDS
    done

}

function run_ut_mipi_switch_test() {
    for ini_path in \
        $INI_SRC_TRIPLE_MIPI_SWITCH_PATH \
        $INI_SRC_TWO_MIPI_SWITCH_PATH \
        $INI_SRC_TWO_NORMAL_MIPI_SWITCH_PATH
    do
        cp $ini_path $INI_DST_PATH

        for i in $MIPI_SWITCH_UT
        do
            cat /sys/kernel/debug/ion/cvi_carveout_heap_dump/summary
            echo "========== test $i =========="
            $UT_BIN_DIR/$UT_BIN_NAME $i < $INPUT_FILE | tee $OUT_FILE
            verify
            if [ $check_ret != 0 ]; then
                result=$TEST_FAIL
                return 1
            fi

            sleep $SLEEP_SECONDS
        done
    done
}

env_check
sample_check $UT_BIN_NAME
touch $INPUT_FILE

for t in $(seq 1 $TEST_TIMES)
do
    run_ut_linear_test || break
    run_ut_wdr_test || break
    run_ut_wdr_linear_test || break
    run_ut_yuv_test || break
    run_ut_mipi_switch_test || break
done

clean_tmp_files

echo "========================================="
echo "middleware $UT_BIN_NAME $result"
echo "========================================="
