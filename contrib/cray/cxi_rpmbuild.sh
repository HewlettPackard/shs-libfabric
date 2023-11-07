#!/bin/bash

ROOTDIR=$(dirname $(readlink -f ${BASH_SOURCE[0]}))

${ROOTDIR}/cray_rpmbuild.sh
