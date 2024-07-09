#!/bin/bash

ROOTDIR=$(dirname $(readlink -f ${BASH_SOURCE[0]}))

VERBS_BUILD=y $ROOTDIR/cray_rpmbuild.sh
