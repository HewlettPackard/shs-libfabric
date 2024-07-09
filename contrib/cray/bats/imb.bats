#!/usr/bin/env bats

load test_helper

XRC_FI_ENV="FI_VERBS_XRCD_FILENAME=/tmp/xrc_imb_$$ FI_OFI_RXM_USE_SRX=1 FI_VERBS_PREFER_XRC=1"

# RC
@test "IMB-P2P unirandom 2 ranks, 1 ranks per node using RC verbs" {
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 2 1) timeout 300 "$IMB_BUILD_PATH/IMB-P2P -npmin 2 -time 2 -mem 2 -msglog 2:14 unirandom"
        [ "$status" -eq 0 ]
}

# XRC
@test "IMB-P2P unirandom 2 ranks, 1 ranks per node using XRC verbs" {
        eval ${XRC_FI_ENV} \
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 2 1) timeout 300 "$IMB_BUILD_PATH/IMB-P2P -npmin 2 -time 2 -mem 2 -msglog 2:14 unirandom"
        [ "$status" -eq 0 ]
}
# RC
@test "IMB-P2P birandom 2 ranks, 1 ranks per node using RC verbs" {
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 2 1) timeout 300 "$IMB_BUILD_PATH/IMB-P2P -npmin 2 -time 2 -mem 2 -msglog 2:14 birandom"
        [ "$status" -eq 0 ]
}

# XRC
@test "IMB-P2P birandom 2 ranks, 1 ranks per node using XRC verbs" {
        eval ${XRC_FI_ENV} \
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 2 1) timeout 300 "$IMB_BUILD_PATH/IMB-P2P -npmin 2 -time 2 -mem 2 -msglog 2:14 birandom"
        [ "$status" -eq 0 ]
}
# RC
@test "IMB-P2P corandom 2 ranks, 1 ranks per node using RC verbs" {
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 2 1) timeout 300 "$IMB_BUILD_PATH/IMB-P2P -npmin 2 -time 2 -mem 2 -msglog 2:14 corandom"
        [ "$status" -eq 0 ]
}

# XRC
@test "IMB-P2P corandom 2 ranks, 1 ranks per node using XRC verbs" {
        eval ${XRC_FI_ENV} \
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 2 1) timeout 300 "$IMB_BUILD_PATH/IMB-P2P -npmin 2 -time 2 -mem 2 -msglog 2:14 corandom"
        [ "$status" -eq 0 ]
}
# RC
@test "IMB-RMA bidir_get 2 ranks, 1 ranks per node using RC verbs" {
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 2 1) timeout 300 "$IMB_BUILD_PATH/IMB-RMA -npmin 2 -time 2 -mem 2 -msglog 2:14 bidir_get"
        [ "$status" -eq 0 ]
}

# XRC
@test "IMB-RMA bidir_get 2 ranks, 1 ranks per node using XRC verbs" {
        eval ${XRC_FI_ENV} \
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 2 1) timeout 300 "$IMB_BUILD_PATH/IMB-RMA -npmin 2 -time 2 -mem 2 -msglog 2:14 bidir_get"
        [ "$status" -eq 0 ]
}
# RC
@test "IMB-RMA bidir_put 2 ranks, 1 ranks per node using RC verbs" {
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 2 1) timeout 300 "$IMB_BUILD_PATH/IMB-RMA -npmin 2 -time 2 -mem 2 -msglog 2:14 bidir_put"
        [ "$status" -eq 0 ]
}

# XRC
@test "IMB-RMA bidir_put 2 ranks, 1 ranks per node using XRC verbs" {
        eval ${XRC_FI_ENV} \
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 2 1) timeout 300 "$IMB_BUILD_PATH/IMB-RMA -npmin 2 -time 2 -mem 2 -msglog 2:14 bidir_put"
        [ "$status" -eq 0 ]
}
# RC
@test "IMB-RMA unidir_get 2 ranks, 1 ranks per node using RC verbs" {
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 2 1) timeout 300 "$IMB_BUILD_PATH/IMB-RMA -npmin 2 -time 2 -mem 2 -msglog 2:14 unidir_get"
        [ "$status" -eq 0 ]
}

# XRC
@test "IMB-RMA unidir_get 2 ranks, 1 ranks per node using XRC verbs" {
        eval ${XRC_FI_ENV} \
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 2 1) timeout 300 "$IMB_BUILD_PATH/IMB-RMA -npmin 2 -time 2 -mem 2 -msglog 2:14 unidir_get"
        [ "$status" -eq 0 ]
}
# RC
@test "IMB-RMA unidir_put 2 ranks, 1 ranks per node using RC verbs" {
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 2 1) timeout 300 "$IMB_BUILD_PATH/IMB-RMA -npmin 2 -time 2 -mem 2 -msglog 2:14 unidir_put"
        [ "$status" -eq 0 ]
}

# XRC
@test "IMB-RMA unidir_put 2 ranks, 1 ranks per node using XRC verbs" {
        eval ${XRC_FI_ENV} \
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 2 1) timeout 300 "$IMB_BUILD_PATH/IMB-RMA -npmin 2 -time 2 -mem 2 -msglog 2:14 unidir_put"
        [ "$status" -eq 0 ]
}
# RC
@test "IMB-EXT window 16 ranks, 4 ranks per node using RC verbs" {
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-EXT -npmin 20 -time 2 -mem 2 -msglog 2:14 window"
        [ "$status" -eq 0 ]
}

# XRC
@test "IMB-EXT window 16 ranks, 4 ranks per node using XRC verbs" {
        eval ${XRC_FI_ENV} \
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-EXT -npmin 20 -time 2 -mem 2 -msglog 2:14 window"
        [ "$status" -eq 0 ]
}
# RC
@test "IMB-EXT accumulate 16 ranks, 4 ranks per node using RC verbs" {
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-EXT -npmin 20 -time 2 -mem 2 -msglog 2:14 accumulate"
        [ "$status" -eq 0 ]
}

# XRC
@test "IMB-EXT accumulate 16 ranks, 4 ranks per node using XRC verbs" {
        eval ${XRC_FI_ENV} \
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-EXT -npmin 20 -time 2 -mem 2 -msglog 2:14 accumulate"
        [ "$status" -eq 0 ]
}
# RC
@test "IMB-NBC ialltoall 16 ranks, 4 ranks per node using RC verbs" {
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-NBC -npmin 20 -iter 20 -time 2 -mem 2 -msglog 2:14 ialltoall"
        [ "$status" -eq 0 ]
}

# XRC
@test "IMB-NBC ialltoall 16 ranks, 4 ranks per node using XRC verbs" {
        eval ${XRC_FI_ENV} \
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-NBC -npmin 20 -iter 20 -time 2 -mem 2 -msglog 2:14 ialltoall"
        [ "$status" -eq 0 ]
}
# RC
@test "IMB-NBC ialltoall_pure 16 ranks, 4 ranks per node using RC verbs" {
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-NBC -npmin 20 -iter 20 -time 2 -mem 2 -msglog 2:14 ialltoall_pure"
        [ "$status" -eq 0 ]
}

# XRC
@test "IMB-NBC ialltoall_pure 16 ranks, 4 ranks per node using XRC verbs" {
        eval ${XRC_FI_ENV} \
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-NBC -npmin 20 -iter 20 -time 2 -mem 2 -msglog 2:14 ialltoall_pure"
        [ "$status" -eq 0 ]
}
# RC
@test "IMB-NBC ialltoallv 16 ranks, 4 ranks per node using RC verbs" {
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-NBC -npmin 20 -iter 20 -time 2 -mem 2 -msglog 2:14 ialltoallv"
        [ "$status" -eq 0 ]
}

# XRC
@test "IMB-NBC ialltoallv 16 ranks, 4 ranks per node using XRC verbs" {
        eval ${XRC_FI_ENV} \
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-NBC -npmin 20 -iter 20 -time 2 -mem 2 -msglog 2:14 ialltoallv"
        [ "$status" -eq 0 ]
}
# RC
@test "IMB-NBC ialltoallv_pure 16 ranks, 4 ranks per node using RC verbs" {
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-NBC -npmin 20 -iter 20 -time 2 -mem 2 -msglog 2:14 ialltoallv_pure"
        [ "$status" -eq 0 ]
}

# XRC
@test "IMB-NBC ialltoallv_pure 16 ranks, 4 ranks per node using XRC verbs" {
        eval ${XRC_FI_ENV} \
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-NBC -npmin 20 -iter 20 -time 2 -mem 2 -msglog 2:14 ialltoallv_pure"
        [ "$status" -eq 0 ]
}
# RC
@test "IMB-NBC iallgather 16 ranks, 4 ranks per node using RC verbs" {
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-NBC -npmin 20 -iter 20 -time 2 -mem 2 -msglog 2:14 iallgather"
        [ "$status" -eq 0 ]
}

# XRC
@test "IMB-NBC iallgather 16 ranks, 4 ranks per node using XRC verbs" {
        eval ${XRC_FI_ENV} \
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-NBC -npmin 20 -iter 20 -time 2 -mem 2 -msglog 2:14 iallgather"
        [ "$status" -eq 0 ]
}
# RC
@test "IMB-NBC iallgather_pure 16 ranks, 4 ranks per node using RC verbs" {
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-NBC -npmin 20 -iter 20 -time 2 -mem 2 -msglog 2:14 iallgather_pure"
        [ "$status" -eq 0 ]
}

# XRC
@test "IMB-NBC iallgather_pure 16 ranks, 4 ranks per node using XRC verbs" {
        eval ${XRC_FI_ENV} \
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-NBC -npmin 20 -iter 20 -time 2 -mem 2 -msglog 2:14 iallgather_pure"
        [ "$status" -eq 0 ]
}
# RC
@test "IMB-NBC iallgatherv 16 ranks, 4 ranks per node using RC verbs" {
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-NBC -npmin 20 -iter 20 -time 2 -mem 2 -msglog 2:14 iallgatherv"
        [ "$status" -eq 0 ]
}

# XRC
@test "IMB-NBC iallgatherv 16 ranks, 4 ranks per node using XRC verbs" {
        eval ${XRC_FI_ENV} \
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-NBC -npmin 20 -iter 20 -time 2 -mem 2 -msglog 2:14 iallgatherv"
        [ "$status" -eq 0 ]
}
# RC
@test "IMB-NBC iallgatherv_pure 16 ranks, 4 ranks per node using RC verbs" {
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-NBC -npmin 20 -iter 20 -time 2 -mem 2 -msglog 2:14 iallgatherv_pure"
        [ "$status" -eq 0 ]
}

# XRC
@test "IMB-NBC iallgatherv_pure 16 ranks, 4 ranks per node using XRC verbs" {
        eval ${XRC_FI_ENV} \
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-NBC -npmin 20 -iter 20 -time 2 -mem 2 -msglog 2:14 iallgatherv_pure"
        [ "$status" -eq 0 ]
}
# RC
@test "IMB-NBC iallreduce 16 ranks, 4 ranks per node using RC verbs" {
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-NBC -npmin 20 -iter 20 -time 2 -mem 2 -msglog 2:14 iallreduce"
        [ "$status" -eq 0 ]
}

# XRC
@test "IMB-NBC iallreduce 16 ranks, 4 ranks per node using XRC verbs" {
        eval ${XRC_FI_ENV} \
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-NBC -npmin 20 -iter 20 -time 2 -mem 2 -msglog 2:14 iallreduce"
        [ "$status" -eq 0 ]
}
# RC
@test "IMB-NBC iallreduce_pure 16 ranks, 4 ranks per node using RC verbs" {
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-NBC -npmin 20 -iter 20 -time 2 -mem 2 -msglog 2:14 iallreduce_pure"
        [ "$status" -eq 0 ]
}

# XRC
@test "IMB-NBC iallreduce_pure 16 ranks, 4 ranks per node using XRC verbs" {
        eval ${XRC_FI_ENV} \
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-NBC -npmin 20 -iter 20 -time 2 -mem 2 -msglog 2:14 iallreduce_pure"
        [ "$status" -eq 0 ]
}
# RC
@test "IMB-NBC ibarrier 16 ranks, 4 ranks per node using RC verbs" {
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-NBC -npmin 20 -iter 20 -time 2 -mem 2 -msglog 2:14 ibarrier"
        [ "$status" -eq 0 ]
}

# XRC
@test "IMB-NBC ibarrier 16 ranks, 4 ranks per node using XRC verbs" {
        eval ${XRC_FI_ENV} \
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-NBC -npmin 20 -iter 20 -time 2 -mem 2 -msglog 2:14 ibarrier"
        [ "$status" -eq 0 ]
}
# RC
@test "IMB-NBC ibarrier_pure 16 ranks, 4 ranks per node using RC verbs" {
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-NBC -npmin 20 -iter 20 -time 2 -mem 2 -msglog 2:14 ibarrier_pure"
        [ "$status" -eq 0 ]
}

# XRC
@test "IMB-NBC ibarrier_pure 16 ranks, 4 ranks per node using XRC verbs" {
        eval ${XRC_FI_ENV} \
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-NBC -npmin 20 -iter 20 -time 2 -mem 2 -msglog 2:14 ibarrier_pure"
        [ "$status" -eq 0 ]
}
# RC
@test "IMB-NBC ibcast 16 ranks, 4 ranks per node using RC verbs" {
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-NBC -npmin 20 -iter 20 -time 2 -mem 2 -msglog 2:14 ibcast"
        [ "$status" -eq 0 ]
}

# XRC
@test "IMB-NBC ibcast 16 ranks, 4 ranks per node using XRC verbs" {
        eval ${XRC_FI_ENV} \
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-NBC -npmin 20 -iter 20 -time 2 -mem 2 -msglog 2:14 ibcast"
        [ "$status" -eq 0 ]
}
# RC
@test "IMB-NBC ibcast_pure 16 ranks, 4 ranks per node using RC verbs" {
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-NBC -npmin 20 -iter 20 -time 2 -mem 2 -msglog 2:14 ibcast_pure"
        [ "$status" -eq 0 ]
}

# XRC
@test "IMB-NBC ibcast_pure 16 ranks, 4 ranks per node using XRC verbs" {
        eval ${XRC_FI_ENV} \
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-NBC -npmin 20 -iter 20 -time 2 -mem 2 -msglog 2:14 ibcast_pure"
        [ "$status" -eq 0 ]
}
# RC
@test "IMB-NBC igather 16 ranks, 4 ranks per node using RC verbs" {
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-NBC -npmin 20 -iter 20 -time 2 -mem 2 -msglog 2:14 igather"
        [ "$status" -eq 0 ]
}

# XRC
@test "IMB-NBC igather 16 ranks, 4 ranks per node using XRC verbs" {
        eval ${XRC_FI_ENV} \
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-NBC -npmin 20 -iter 20 -time 2 -mem 2 -msglog 2:14 igather"
        [ "$status" -eq 0 ]
}
# RC
@test "IMB-NBC igather_pure 16 ranks, 4 ranks per node using RC verbs" {
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-NBC -npmin 20 -iter 20 -time 2 -mem 2 -msglog 2:14 igather_pure"
        [ "$status" -eq 0 ]
}

# XRC
@test "IMB-NBC igather_pure 16 ranks, 4 ranks per node using XRC verbs" {
        eval ${XRC_FI_ENV} \
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-NBC -npmin 20 -iter 20 -time 2 -mem 2 -msglog 2:14 igather_pure"
        [ "$status" -eq 0 ]
}
# RC
@test "IMB-NBC igatherv 16 ranks, 4 ranks per node using RC verbs" {
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-NBC -npmin 20 -iter 20 -time 2 -mem 2 -msglog 2:14 igatherv"
        [ "$status" -eq 0 ]
}

# XRC
@test "IMB-NBC igatherv 16 ranks, 4 ranks per node using XRC verbs" {
        eval ${XRC_FI_ENV} \
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-NBC -npmin 20 -iter 20 -time 2 -mem 2 -msglog 2:14 igatherv"
        [ "$status" -eq 0 ]
}
# RC
@test "IMB-NBC igatherv_pure 16 ranks, 4 ranks per node using RC verbs" {
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-NBC -npmin 20 -iter 20 -time 2 -mem 2 -msglog 2:14 igatherv_pure"
        [ "$status" -eq 0 ]
}

# XRC
@test "IMB-NBC igatherv_pure 16 ranks, 4 ranks per node using XRC verbs" {
        eval ${XRC_FI_ENV} \
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-NBC -npmin 20 -iter 20 -time 2 -mem 2 -msglog 2:14 igatherv_pure"
        [ "$status" -eq 0 ]
}
# RC
@test "IMB-NBC ireduce 16 ranks, 4 ranks per node using RC verbs" {
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-NBC -npmin 20 -iter 20 -time 2 -mem 2 -msglog 2:14 ireduce"
        [ "$status" -eq 0 ]
}

# XRC
@test "IMB-NBC ireduce 16 ranks, 4 ranks per node using XRC verbs" {
        eval ${XRC_FI_ENV} \
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-NBC -npmin 20 -iter 20 -time 2 -mem 2 -msglog 2:14 ireduce"
        [ "$status" -eq 0 ]
}
# RC
@test "IMB-NBC ireduce_pure 16 ranks, 4 ranks per node using RC verbs" {
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-NBC -npmin 20 -iter 20 -time 2 -mem 2 -msglog 2:14 ireduce_pure"
        [ "$status" -eq 0 ]
}

# XRC
@test "IMB-NBC ireduce_pure 16 ranks, 4 ranks per node using XRC verbs" {
        eval ${XRC_FI_ENV} \
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-NBC -npmin 20 -iter 20 -time 2 -mem 2 -msglog 2:14 ireduce_pure"
        [ "$status" -eq 0 ]
}
# RC
@test "IMB-NBC ireduce_scatter 16 ranks, 4 ranks per node using RC verbs" {
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-NBC -npmin 20 -iter 20 -time 2 -mem 2 -msglog 2:14 ireduce_scatter"
        [ "$status" -eq 0 ]
}

# XRC
@test "IMB-NBC ireduce_scatter 16 ranks, 4 ranks per node using XRC verbs" {
        eval ${XRC_FI_ENV} \
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-NBC -npmin 20 -iter 20 -time 2 -mem 2 -msglog 2:14 ireduce_scatter"
        [ "$status" -eq 0 ]
}
# RC
@test "IMB-NBC iscatter 16 ranks, 4 ranks per node using RC verbs" {
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-NBC -npmin 20 -iter 20 -time 2 -mem 2 -msglog 2:14 iscatter"
        [ "$status" -eq 0 ]
}

# XRC
@test "IMB-NBC iscatter 16 ranks, 4 ranks per node using XRC verbs" {
        eval ${XRC_FI_ENV} \
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-NBC -npmin 20 -iter 20 -time 2 -mem 2 -msglog 2:14 iscatter"
        [ "$status" -eq 0 ]
}
# RC
@test "IMB-NBC iscatter_pure 16 ranks, 4 ranks per node using RC verbs" {
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-NBC -npmin 20 -iter 20 -time 2 -mem 2 -msglog 2:14 iscatter_pure"
        [ "$status" -eq 0 ]
}

# XRC
@test "IMB-NBC iscatter_pure 16 ranks, 4 ranks per node using XRC verbs" {
        eval ${XRC_FI_ENV} \
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-NBC -npmin 20 -iter 20 -time 2 -mem 2 -msglog 2:14 iscatter_pure"
        [ "$status" -eq 0 ]
}
# RC
@test "IMB-NBC iscatterv 16 ranks, 4 ranks per node using RC verbs" {
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-NBC -npmin 20 -iter 20 -time 2 -mem 2 -msglog 2:14 iscatterv"
        [ "$status" -eq 0 ]
}

# XRC
@test "IMB-NBC iscatterv 16 ranks, 4 ranks per node using XRC verbs" {
        eval ${XRC_FI_ENV} \
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-NBC -npmin 20 -iter 20 -time 2 -mem 2 -msglog 2:14 iscatterv"
        [ "$status" -eq 0 ]
}
# RC
@test "IMB-NBC iscatterv_pure 16 ranks, 4 ranks per node using RC verbs" {
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-NBC -npmin 20 -iter 20 -time 2 -mem 2 -msglog 2:14 iscatterv_pure"
        [ "$status" -eq 0 ]
}

# XRC
@test "IMB-NBC iscatterv_pure 16 ranks, 4 ranks per node using XRC verbs" {
        eval ${XRC_FI_ENV} \
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-NBC -npmin 20 -iter 20 -time 2 -mem 2 -msglog 2:14 iscatterv_pure"
        [ "$status" -eq 0 ]
}
# RC
@test "IMB-MPI1 reduce 16 ranks, 4 ranks per node using RC verbs" {
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-MPI1 -npmin 20 -iter 20 -time 2 -mem 2 -msglog 2:14 reduce"
        [ "$status" -eq 0 ]
}

# XRC
@test "IMB-MPI1 reduce 16 ranks, 4 ranks per node using XRC verbs" {
        eval ${XRC_FI_ENV} \
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-MPI1 -npmin 20 -iter 20 -time 2 -mem 2 -msglog 2:14 reduce"
        [ "$status" -eq 0 ]
}
# RC
@test "IMB-MPI1 reduce_scatter 16 ranks, 4 ranks per node using RC verbs" {
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-MPI1 -npmin 20 -iter 20 -time 2 -mem 2 -msglog 2:14 reduce_scatter"
        [ "$status" -eq 0 ]
}

# XRC
@test "IMB-MPI1 reduce_scatter 16 ranks, 4 ranks per node using XRC verbs" {
        eval ${XRC_FI_ENV} \
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-MPI1 -npmin 20 -iter 20 -time 2 -mem 2 -msglog 2:14 reduce_scatter"
        [ "$status" -eq 0 ]
}
# RC
@test "IMB-MPI1 reduce_scatter_block 16 ranks, 4 ranks per node using RC verbs" {
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-MPI1 -npmin 20 -iter 20 -time 2 -mem 2 -msglog 2:14 reduce_scatter_block"
        [ "$status" -eq 0 ]
}

# XRC
@test "IMB-MPI1 reduce_scatter_block 16 ranks, 4 ranks per node using XRC verbs" {
        eval ${XRC_FI_ENV} \
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-MPI1 -npmin 20 -iter 20 -time 2 -mem 2 -msglog 2:14 reduce_scatter_block"
        [ "$status" -eq 0 ]
}
# RC
@test "IMB-MPI1 allreduce 16 ranks, 4 ranks per node using RC verbs" {
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-MPI1 -npmin 20 -iter 20 -time 2 -mem 2 -msglog 2:14 allreduce"
        [ "$status" -eq 0 ]
}

# XRC
@test "IMB-MPI1 allreduce 16 ranks, 4 ranks per node using XRC verbs" {
        eval ${XRC_FI_ENV} \
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-MPI1 -npmin 20 -iter 20 -time 2 -mem 2 -msglog 2:14 allreduce"
        [ "$status" -eq 0 ]
}
# RC
@test "IMB-MPI1 allgather 16 ranks, 4 ranks per node using RC verbs" {
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-MPI1 -npmin 20 -iter 20 -time 2 -mem 2 -msglog 2:14 allgather"
        [ "$status" -eq 0 ]
}

# XRC
@test "IMB-MPI1 allgather 16 ranks, 4 ranks per node using XRC verbs" {
        eval ${XRC_FI_ENV} \
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-MPI1 -npmin 20 -iter 20 -time 2 -mem 2 -msglog 2:14 allgather"
        [ "$status" -eq 0 ]
}
# RC
@test "IMB-MPI1 allgatherv 16 ranks, 4 ranks per node using RC verbs" {
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-MPI1 -npmin 20 -iter 20 -time 2 -mem 2 -msglog 2:14 allgatherv"
        [ "$status" -eq 0 ]
}

# XRC
@test "IMB-MPI1 allgatherv 16 ranks, 4 ranks per node using XRC verbs" {
        eval ${XRC_FI_ENV} \
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-MPI1 -npmin 20 -iter 20 -time 2 -mem 2 -msglog 2:14 allgatherv"
        [ "$status" -eq 0 ]
}
# RC
@test "IMB-MPI1 scatter 16 ranks, 4 ranks per node using RC verbs" {
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-MPI1 -npmin 20 -iter 20 -time 2 -mem 2 -msglog 2:14 scatter"
        [ "$status" -eq 0 ]
}

# XRC
@test "IMB-MPI1 scatter 16 ranks, 4 ranks per node using XRC verbs" {
        eval ${XRC_FI_ENV} \
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-MPI1 -npmin 20 -iter 20 -time 2 -mem 2 -msglog 2:14 scatter"
        [ "$status" -eq 0 ]
}
# RC
@test "IMB-MPI1 scatterv 16 ranks, 4 ranks per node using RC verbs" {
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-MPI1 -npmin 20 -iter 20 -time 2 -mem 2 -msglog 2:14 scatterv"
        [ "$status" -eq 0 ]
}

# XRC
@test "IMB-MPI1 scatterv 16 ranks, 4 ranks per node using XRC verbs" {
        eval ${XRC_FI_ENV} \
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-MPI1 -npmin 20 -iter 20 -time 2 -mem 2 -msglog 2:14 scatterv"
        [ "$status" -eq 0 ]
}
# RC
@test "IMB-MPI1 gather 16 ranks, 4 ranks per node using RC verbs" {
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-MPI1 -npmin 20 -iter 20 -time 2 -mem 2 -msglog 2:14 gather"
        [ "$status" -eq 0 ]
}

# XRC
@test "IMB-MPI1 gather 16 ranks, 4 ranks per node using XRC verbs" {
        eval ${XRC_FI_ENV} \
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-MPI1 -npmin 20 -iter 20 -time 2 -mem 2 -msglog 2:14 gather"
        [ "$status" -eq 0 ]
}
# RC
@test "IMB-MPI1 gatherv 16 ranks, 4 ranks per node using RC verbs" {
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-MPI1 -npmin 20 -iter 20 -time 2 -mem 2 -msglog 2:14 gatherv"
        [ "$status" -eq 0 ]
}

# XRC
@test "IMB-MPI1 gatherv 16 ranks, 4 ranks per node using XRC verbs" {
        eval ${XRC_FI_ENV} \
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-MPI1 -npmin 20 -iter 20 -time 2 -mem 2 -msglog 2:14 gatherv"
        [ "$status" -eq 0 ]
}
# RC
@test "IMB-MPI1 alltoall 16 ranks, 4 ranks per node using RC verbs" {
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-MPI1 -npmin 20 -iter 20 -time 2 -mem 2 -msglog 2:14 alltoall"
        [ "$status" -eq 0 ]
}

# XRC
@test "IMB-MPI1 alltoall 16 ranks, 4 ranks per node using XRC verbs" {
        eval ${XRC_FI_ENV} \
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-MPI1 -npmin 20 -iter 20 -time 2 -mem 2 -msglog 2:14 alltoall"
        [ "$status" -eq 0 ]
}
# RC
@test "IMB-MPI1 bcast 16 ranks, 4 ranks per node using RC verbs" {
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-MPI1 -npmin 20 -iter 20 -time 2 -mem 2 -msglog 2:14 bcast"
        [ "$status" -eq 0 ]
}

# XRC
@test "IMB-MPI1 bcast 16 ranks, 4 ranks per node using XRC verbs" {
        eval ${XRC_FI_ENV} \
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-MPI1 -npmin 20 -iter 20 -time 2 -mem 2 -msglog 2:14 bcast"
        [ "$status" -eq 0 ]
}
# RC
@test "IMB-MPI1 barrier 16 ranks, 4 ranks per node using RC verbs" {
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-MPI1 -npmin 20 -iter 20 -time 2 -mem 2 -msglog 2:14 barrier"
        [ "$status" -eq 0 ]
}

# XRC
@test "IMB-MPI1 barrier 16 ranks, 4 ranks per node using XRC verbs" {
        eval ${XRC_FI_ENV} \
        run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
                $(batch_launcher 16 4) timeout 300 "$IMB_BUILD_PATH/IMB-MPI1 -npmin 20 -iter 20 -time 2 -mem 2 -msglog 2:14 barrier"
        [ "$status" -eq 0 ]
}
