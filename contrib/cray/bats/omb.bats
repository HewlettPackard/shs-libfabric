#!/usr/bin/env bats

load test_helper

# RC
@test "osu_latency 2 ranks, 1 ranks per node using RC verbs" {
    run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
        $(batch_launcher 2 1) timeout 300 $OMB_BUILD_PATH/pt2pt/osu_latency
    [ "$status" -eq 0 ]
}

@test "osu_bw 2 ranks, 1 ranks per node using RC verbs" {
    run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
        $(batch_launcher 2 1) timeout 300 $OMB_BUILD_PATH/pt2pt/osu_bw
    [ "$status" -eq 0 ]
}

@test "osu_mbw_mr 8 ranks, 4 ranks per node using RC verbs" {
    run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
        $(batch_launcher 8 4) timeout 300 $OMB_BUILD_PATH/pt2pt/osu_mbw_mr
    [ "$status" -eq 0 ]
}

@test "osu_get_latency 2 ranks, 1 ranks per node using RC verbs" {
    run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
        $(batch_launcher 2 1) timeout 300 $OMB_BUILD_PATH/one-sided/osu_get_latency
    [ "$status" -eq 0 ]
}

@test "osu_get_bw 2 ranks, 1 ranks per node using RC verbs" {
    run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
        $(batch_launcher 2 1) timeout 300 $OMB_BUILD_PATH/one-sided/osu_get_bw
    [ "$status" -eq 0 ]
}

@test "osu_put_latency 2 ranks, 1 ranks per node using RC verbs" {
    run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
        $(batch_launcher 2 1) timeout 300 $OMB_BUILD_PATH/one-sided/osu_put_latency
    [ "$status" -eq 0 ]
}

@test "osu_put_bw 2 ranks, 1 ranks per node using RC verbs" {
    run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
        $(batch_launcher 2 1) timeout 300 $OMB_BUILD_PATH/one-sided/osu_put_bw
    [ "$status" -eq 0 ]
}

@test "osu_put_bibw 2 ranks, 1 ranks per node using RC verbs" {
    run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
        $(batch_launcher 2 1) timeout 300 $OMB_BUILD_PATH/one-sided/osu_put_bibw
    [ "$status" -eq 0 ]
}

@test "osu_allreduce 16 ranks, 4 ranks per node using RC verbs" {
    run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
        $(batch_launcher 16 4) timeout 300 $OMB_BUILD_PATH/collective/osu_allreduce
    [ "$status" -eq 0 ]
}

@test "osu_allgather 16 ranks, 4 ranks per node using RC verbs" {
    run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
        $(batch_launcher 16 4) timeout 300 $OMB_BUILD_PATH/collective/osu_allgather
    [ "$status" -eq 0 ]
}

@test "osu_allgatherv 16 ranks, 4 ranks per node using RC verbs" {
    run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
        $(batch_launcher 16 4) timeout 300 $OMB_BUILD_PATH/collective/osu_allgatherv
    [ "$status" -eq 0 ]
}

@test "osu_alltoall 16 ranks, 4 ranks per node using RC verbs" {
    run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
        $(batch_launcher 16 4) timeout 300 $OMB_BUILD_PATH/collective/osu_alltoall
    [ "$status" -eq 0 ]
}

@test "osu_alltoallv 16 ranks, 4 ranks per node using RC verbs" {
    skip "fails consistently at 128k message size"
    run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
        $(batch_launcher 16 4) timeout 300 $OMB_BUILD_PATH/collective/osu_alltoallv
    [ "$status" -eq 0 ]
}

@test "osu_barrier 16 ranks, 4 ranks per node using RC verbs" {
    run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
        $(batch_launcher 16 4) timeout 300 $OMB_BUILD_PATH/collective/osu_barrier
    [ "$status" -eq 0 ]
}

@test "osu_bcast 16 ranks, 4 ranks per node using RC verbs" {
    run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
        $(batch_launcher 16 4) timeout 300 $OMB_BUILD_PATH/collective/osu_bcast
    [ "$status" -eq 0 ]
}

@test "osu_gather 16 ranks, 4 ranks per node using RC verbs" {
    run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
        $(batch_launcher 16 4) timeout 300 $OMB_BUILD_PATH/collective/osu_gather
    [ "$status" -eq 0 ]
}

@test "osu_gatherv 16 ranks, 4 ranks per node using RC verbs" {
    skip "fails intermittently"
    run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
        $(batch_launcher 16 4) timeout 300 $OMB_BUILD_PATH/collective/osu_gatherv
    [ "$status" -eq 0 ]
}

@test "osu_iallgather 16 ranks, 4 ranks per node using RC verbs" {
    run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
        $(batch_launcher 16 4) timeout 300 $OMB_BUILD_PATH/collective/osu_iallgather
    [ "$status" -eq 0 ]
}

@test "osu_iallgatherv 16 ranks, 4 ranks per node using RC verbs" {
    run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
        $(batch_launcher 16 4) timeout 300 $OMB_BUILD_PATH/collective/osu_iallgatherv
    [ "$status" -eq 0 ]
}

@test "osu_ialltoall 16 ranks, 4 ranks per node using RC verbs" {
    run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
        $(batch_launcher 16 4) timeout 300 $OMB_BUILD_PATH/collective/osu_ialltoall
    [ "$status" -eq 0 ]
}

@test "osu_ialltoallv 16 ranks, 4 ranks per node using RC verbs" {
    skip "fails consistently at 128k message size"
    run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
        $(batch_launcher 16 4) timeout 300 $OMB_BUILD_PATH/collective/osu_ialltoallv
    [ "$status" -eq 0 ]
}

@test "osu_ialltoallw 16 ranks, 4 ranks per node using RC verbs" {
    skip "fails consistently at 128k message size"
    run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
        $(batch_launcher 16 4) timeout 300 $OMB_BUILD_PATH/collective/osu_ialltoallw
    [ "$status" -eq 0 ]
}

@test "osu_ibarrier 16 ranks, 4 ranks per node using RC verbs" {
    run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
        $(batch_launcher 16 4) timeout 300 $OMB_BUILD_PATH/collective/osu_ibarrier
    [ "$status" -eq 0 ]
}

@test "osu_ibcast 16 ranks, 4 ranks per node using RC verbs" {
    run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
        $(batch_launcher 16 4) timeout 300 $OMB_BUILD_PATH/collective/osu_ibcast
    [ "$status" -eq 0 ]
}

@test "osu_igather 16 ranks, 4 ranks per node using RC verbs" {
    run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
        $(batch_launcher 16 4) timeout 300 $OMB_BUILD_PATH/collective/osu_igather
    [ "$status" -eq 0 ]
}

@test "osu_igatherv 16 ranks, 4 ranks per node using RC verbs" {
    skip "fails intermittently"
    run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
        $(batch_launcher 16 4) timeout 300 $OMB_BUILD_PATH/collective/osu_igatherv
    [ "$status" -eq 0 ]
}

@test "osu_iscatter 16 ranks, 4 ranks per node using RC verbs" {
    run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
        $(batch_launcher 16 4) timeout 300 $OMB_BUILD_PATH/collective/osu_iscatter
    [ "$status" -eq 0 ]
}

@test "osu_iscatterv 16 ranks, 4 ranks per node using RC verbs" {
    run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
        $(batch_launcher 16 4) timeout 300 $OMB_BUILD_PATH/collective/osu_iscatterv
    [ "$status" -eq 0 ]
}

@test "osu_reduce 16 ranks, 4 ranks per node using RC verbs" {
    run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
        $(batch_launcher 16 4) timeout 300 $OMB_BUILD_PATH/collective/osu_reduce
    [ "$status" -eq 0 ]
}

@test "osu_reduce_scatter 16 ranks, 4 ranks per node using RC verbs" {
    skip "fails consistently at 512K message size"
    run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
        $(batch_launcher 16 4) timeout 300 $OMB_BUILD_PATH/collective/osu_reduce_scatter
    [ "$status" -eq 0 ]
}

@test "osu_scatter 16 ranks, 4 ranks per node using RC verbs" {
    run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
        $(batch_launcher 16 4) timeout 300 $OMB_BUILD_PATH/collective/osu_scatter
    [ "$status" -eq 0 ]
}

@test "osu_scatterv 16 ranks, 4 ranks per node using RC verbs" {
    run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
        $(batch_launcher 16 4) timeout 300 $OMB_BUILD_PATH/collective/osu_scatterv
    [ "$status" -eq 0 ]
}

# XRC
@test "osu_latency 2 ranks, 1 ranks per node using XRC verbs" {
    FI_VERBS_XRCD_FILENAME=/tmp/xrc_omb_$$ FI_OFI_RXM_USE_SRX=1 FI_VERBS_PREFER_XRC=1 run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
        $(batch_launcher 2 1) timeout 300 $OMB_BUILD_PATH/pt2pt/osu_latency
    [ "$status" -eq 0 ]
}

@test "osu_bw 2 ranks, 1 ranks per node using XRC verbs" {
    FI_VERBS_XRCD_FILENAME=/tmp/xrc_omb_$$ FI_OFI_RXM_USE_SRX=1 FI_VERBS_PREFER_XRC=1 run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
        $(batch_launcher 2 1) timeout 300 $OMB_BUILD_PATH/pt2pt/osu_bw
    [ "$status" -eq 0 ]
}

@test "osu_mbw_mr 8 ranks, 4 ranks per node using XRC verbs" {
    FI_VERBS_XRCD_FILENAME=/tmp/xrc_omb_$$ FI_OFI_RXM_USE_SRX=1 FI_VERBS_PREFER_XRC=1 run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
        $(batch_launcher 8 4) timeout 300 $OMB_BUILD_PATH/pt2pt/osu_mbw_mr
    [ "$status" -eq 0 ]
}

@test "osu_get_latency 2 ranks, 1 ranks per node using XRC verbs" {
    FI_VERBS_XRCD_FILENAME=/tmp/xrc_omb_$$ FI_OFI_RXM_USE_SRX=1 FI_VERBS_PREFER_XRC=1 run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
        $(batch_launcher 2 1) timeout 300 $OMB_BUILD_PATH/one-sided/osu_get_latency
    [ "$status" -eq 0 ]
}

@test "osu_get_bw 2 ranks, 1 ranks per node using XRC verbs" {
    FI_VERBS_XRCD_FILENAME=/tmp/xrc_omb_$$ FI_OFI_RXM_USE_SRX=1 FI_VERBS_PREFER_XRC=1 run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
        $(batch_launcher 2 1) timeout 300 $OMB_BUILD_PATH/one-sided/osu_get_bw
    [ "$status" -eq 0 ]
}

@test "osu_put_latency 2 ranks, 1 ranks per node using XRC verbs" {
    FI_VERBS_XRCD_FILENAME=/tmp/xrc_omb_$$ FI_OFI_RXM_USE_SRX=1 FI_VERBS_PREFER_XRC=1 run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
        $(batch_launcher 2 1) timeout 300 $OMB_BUILD_PATH/one-sided/osu_put_latency
    [ "$status" -eq 0 ]
}

@test "osu_put_bw 2 ranks, 1 ranks per node using XRC verbs" {
    FI_VERBS_XRCD_FILENAME=/tmp/xrc_omb_$$ FI_OFI_RXM_USE_SRX=1 FI_VERBS_PREFER_XRC=1 run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
        $(batch_launcher 2 1) timeout 300 $OMB_BUILD_PATH/one-sided/osu_put_bw
    [ "$status" -eq 0 ]
}

@test "osu_put_bibw 2 ranks, 1 ranks per node using XRC verbs" {
    FI_VERBS_XRCD_FILENAME=/tmp/xrc_omb_$$ FI_OFI_RXM_USE_SRX=1 FI_VERBS_PREFER_XRC=1 run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
        $(batch_launcher 2 1) timeout 300 $OMB_BUILD_PATH/one-sided/osu_put_bibw
    [ "$status" -eq 0 ]
}

@test "osu_allreduce 16 ranks, 4 ranks per node using XRC verbs" {
    FI_VERBS_XRCD_FILENAME=/tmp/xrc_omb_$$ FI_OFI_RXM_USE_SRX=1 FI_VERBS_PREFER_XRC=1 run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
        $(batch_launcher 16 4) timeout 300 $OMB_BUILD_PATH/collective/osu_allreduce
    [ "$status" -eq 0 ]
}

@test "osu_allgather 16 ranks, 4 ranks per node using XRC verbs" {
    FI_VERBS_XRCD_FILENAME=/tmp/xrc_omb_$$ FI_OFI_RXM_USE_SRX=1 FI_VERBS_PREFER_XRC=1 run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
        $(batch_launcher 16 4) timeout 300 $OMB_BUILD_PATH/collective/osu_allgather
    [ "$status" -eq 0 ]
}

@test "osu_allgatherv 16 ranks, 4 ranks per node using XRC verbs" {
    FI_VERBS_XRCD_FILENAME=/tmp/xrc_omb_$$ FI_OFI_RXM_USE_SRX=1 FI_VERBS_PREFER_XRC=1 run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
        $(batch_launcher 16 4) timeout 300 $OMB_BUILD_PATH/collective/osu_allgatherv
    [ "$status" -eq 0 ]
}

@test "osu_alltoall 16 ranks, 4 ranks per node using XRC verbs" {
    FI_VERBS_XRCD_FILENAME=/tmp/xrc_omb_$$ FI_OFI_RXM_USE_SRX=1 FI_VERBS_PREFER_XRC=1 run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
        $(batch_launcher 16 4) timeout 300 $OMB_BUILD_PATH/collective/osu_alltoall
    [ "$status" -eq 0 ]
}

@test "osu_alltoallv 16 ranks, 4 ranks per node using XRC verbs" {
    skip "fails consistently at 128k message size"
    FI_VERBS_XRCD_FILENAME=/tmp/xrc_omb_$$ FI_OFI_RXM_USE_SRX=1 FI_VERBS_PREFER_XRC=1 run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
        $(batch_launcher 16 4) timeout 300 $OMB_BUILD_PATH/collective/osu_alltoallv
    [ "$status" -eq 0 ]
}

@test "osu_barrier 16 ranks, 4 ranks per node using XRC verbs" {
    FI_VERBS_XRCD_FILENAME=/tmp/xrc_omb_$$ FI_OFI_RXM_USE_SRX=1 FI_VERBS_PREFER_XRC=1 run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
        $(batch_launcher 16 4) timeout 300 $OMB_BUILD_PATH/collective/osu_barrier
    [ "$status" -eq 0 ]
}

@test "osu_bcast 16 ranks, 4 ranks per node using XRC verbs" {
    FI_VERBS_XRCD_FILENAME=/tmp/xrc_omb_$$ FI_OFI_RXM_USE_SRX=1 FI_VERBS_PREFER_XRC=1 run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
        $(batch_launcher 16 4) timeout 300 $OMB_BUILD_PATH/collective/osu_bcast
    [ "$status" -eq 0 ]
}

@test "osu_gather 16 ranks, 4 ranks per node using XRC verbs" {
    FI_VERBS_XRCD_FILENAME=/tmp/xrc_omb_$$ FI_OFI_RXM_USE_SRX=1 FI_VERBS_PREFER_XRC=1 run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
        $(batch_launcher 16 4) timeout 300 $OMB_BUILD_PATH/collective/osu_gather
    [ "$status" -eq 0 ]
}

@test "osu_gatherv 16 ranks, 4 ranks per node using XRC verbs" {
    skip "fails intermittently"
    FI_VERBS_XRCD_FILENAME=/tmp/xrc_omb_$$ FI_OFI_RXM_USE_SRX=1 FI_VERBS_PREFER_XRC=1 run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
        $(batch_launcher 16 4) timeout 300 $OMB_BUILD_PATH/collective/osu_gatherv
    [ "$status" -eq 0 ]
}

@test "osu_iallgather 16 ranks, 4 ranks per node using XRC verbs" {
    FI_VERBS_XRCD_FILENAME=/tmp/xrc_omb_$$ FI_OFI_RXM_USE_SRX=1 FI_VERBS_PREFER_XRC=1 run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
        $(batch_launcher 16 4) timeout 300 $OMB_BUILD_PATH/collective/osu_iallgather
    [ "$status" -eq 0 ]
}

@test "osu_iallgatherv 16 ranks, 4 ranks per node using XRC verbs" {
    FI_VERBS_XRCD_FILENAME=/tmp/xrc_omb_$$ FI_OFI_RXM_USE_SRX=1 FI_VERBS_PREFER_XRC=1 run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
        $(batch_launcher 16 4) timeout 300 $OMB_BUILD_PATH/collective/osu_iallgatherv
    [ "$status" -eq 0 ]
}

@test "osu_ialltoall 16 ranks, 4 ranks per node using XRC verbs" {
    FI_VERBS_XRCD_FILENAME=/tmp/xrc_omb_$$ FI_OFI_RXM_USE_SRX=1 FI_VERBS_PREFER_XRC=1 run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
        $(batch_launcher 16 4) timeout 300 $OMB_BUILD_PATH/collective/osu_ialltoall
    [ "$status" -eq 0 ]
}

@test "osu_ialltoallv 16 ranks, 4 ranks per node using XRC verbs" {
    skip "fails consistently at 128k message size"
    FI_VERBS_XRCD_FILENAME=/tmp/xrc_omb_$$ FI_OFI_RXM_USE_SRX=1 FI_VERBS_PREFER_XRC=1 run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
        $(batch_launcher 16 4) timeout 300 $OMB_BUILD_PATH/collective/osu_ialltoallv
    [ "$status" -eq 0 ]
}

@test "osu_ialltoallw 16 ranks, 4 ranks per node using XRC verbs" {
    skip "fails consistently at 128k message size"
    FI_VERBS_XRCD_FILENAME=/tmp/xrc_omb_$$ FI_OFI_RXM_USE_SRX=1 FI_VERBS_PREFER_XRC=1 run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
        $(batch_launcher 16 4) timeout 300 $OMB_BUILD_PATH/collective/osu_ialltoallw
    [ "$status" -eq 0 ]
}

@test "osu_ibarrier 16 ranks, 4 ranks per node using XRC verbs" {
    FI_VERBS_XRCD_FILENAME=/tmp/xrc_omb_$$ FI_OFI_RXM_USE_SRX=1 FI_VERBS_PREFER_XRC=1 run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
        $(batch_launcher 16 4) timeout 300 $OMB_BUILD_PATH/collective/osu_ibarrier
    [ "$status" -eq 0 ]
}

@test "osu_ibcast 16 ranks, 4 ranks per node using XRC verbs" {
    FI_VERBS_XRCD_FILENAME=/tmp/xrc_omb_$$ FI_OFI_RXM_USE_SRX=1 FI_VERBS_PREFER_XRC=1 run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
        $(batch_launcher 16 4) timeout 300 $OMB_BUILD_PATH/collective/osu_ibcast
    [ "$status" -eq 0 ]
}

@test "osu_igather 16 ranks, 4 ranks per node using XRC verbs" {
    FI_VERBS_XRCD_FILENAME=/tmp/xrc_omb_$$ FI_OFI_RXM_USE_SRX=1 FI_VERBS_PREFER_XRC=1 run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
        $(batch_launcher 16 4) timeout 300 $OMB_BUILD_PATH/collective/osu_igather
    [ "$status" -eq 0 ]
}

@test "osu_igatherv 16 ranks, 4 ranks per node using XRC verbs" {
    skip "fails intermittently"
    FI_VERBS_XRCD_FILENAME=/tmp/xrc_omb_$$ FI_OFI_RXM_USE_SRX=1 FI_VERBS_PREFER_XRC=1 run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
        $(batch_launcher 16 4) timeout 300 $OMB_BUILD_PATH/collective/osu_igatherv
    [ "$status" -eq 0 ]
}

@test "osu_iscatter 16 ranks, 4 ranks per node using XRC verbs" {
    FI_VERBS_XRCD_FILENAME=/tmp/xrc_omb_$$ FI_OFI_RXM_USE_SRX=1 FI_VERBS_PREFER_XRC=1 run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
        $(batch_launcher 16 4) timeout 300 $OMB_BUILD_PATH/collective/osu_iscatter
    [ "$status" -eq 0 ]
}

@test "osu_iscatterv 16 ranks, 4 ranks per node using XRC verbs" {
    FI_VERBS_XRCD_FILENAME=/tmp/xrc_omb_$$ FI_OFI_RXM_USE_SRX=1 FI_VERBS_PREFER_XRC=1 run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
        $(batch_launcher 16 4) timeout 300 $OMB_BUILD_PATH/collective/osu_iscatterv
    [ "$status" -eq 0 ]
}

@test "osu_reduce 16 ranks, 4 ranks per node using XRC verbs" {
    FI_VERBS_XRCD_FILENAME=/tmp/xrc_omb_$$ FI_OFI_RXM_USE_SRX=1 FI_VERBS_PREFER_XRC=1 run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
        $(batch_launcher 16 4) timeout 300 $OMB_BUILD_PATH/collective/osu_reduce
    [ "$status" -eq 0 ]
}

@test "osu_reduce_scatter 16 ranks, 4 ranks per node using XRC verbs" {
    skip "fails consistently at 512K message size"
    FI_VERBS_XRCD_FILENAME=/tmp/xrc_omb_$$ FI_OFI_RXM_USE_SRX=1 FI_VERBS_PREFER_XRC=1 run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
        $(batch_launcher 16 4) timeout 300 $OMB_BUILD_PATH/collective/osu_reduce_scatter
    [ "$status" -eq 0 ]
}

@test "osu_scatter 16 ranks, 4 ranks per node using XRC verbs" {
    FI_VERBS_XRCD_FILENAME=/tmp/xrc_omb_$$ FI_OFI_RXM_USE_SRX=1 FI_VERBS_PREFER_XRC=1 run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
        $(batch_launcher 16 4) timeout 300 $OMB_BUILD_PATH/collective/osu_scatter
    [ "$status" -eq 0 ]
}

@test "osu_scatterv 16 ranks, 4 ranks per node using XRC verbs" {
    FI_VERBS_XRCD_FILENAME=/tmp/xrc_omb_$$ FI_OFI_RXM_USE_SRX=1 FI_VERBS_PREFER_XRC=1 run $CONTRIB_BIN/logwrap -w ${BATS_TEST_LOGFILE} -- \
        $(batch_launcher 16 4) timeout 300 $OMB_BUILD_PATH/collective/osu_scatterv
    [ "$status" -eq 0 ]
}
