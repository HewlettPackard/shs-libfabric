#!/bin/bash

set -e
set -x

function move_rpms() {
	if [[ ! -d RPMS ]]
	then
		mkdir RPMS
	fi

	# Move the RPMs and SRPMS to where the "Publish" stage expects to find them
	mv `find rpmbuild/RPMS | grep rpm$` `find rpmbuild/SRPMS | grep rpm$` RPMS
	chmod a+rX -R RPMS
}

function run_rpmlint() {
	# Finish up rpmlint to check for warnings and errors.
	rpmlint RPMS/*.rpm ||:

	# Return codes from rpmlint:
	#  0: OK
	#  1: Unspecified error
	#  2: Interrupted
	# 64: One or more error messages
	# 66: Badness level exceeded

	# Let's not fail builds for (for example) using static linking.
	if [[ $? != 0 && $? != 64 ]]; then
	    echo "rpmlint failure!"
	    exit 1
	fi
}

function build_dl_provider() {
	provider="$1"
	config_opts="$2"

	rpmbuild -bb \
		--define "release ${RELEASE}" \
		--define "install_modulefile 1" \
		--define "install_default_module_version 1" \
		--define "modulefile_path /opt/cray/modulefiles" \
		--define "_prefix /opt/cray" \
		--define "_topdir $rpmbuilddir" \
		--define "_sourcedir $rpmbuilddir/SOURCES" \
		--define "_rpmdir $rpmbuilddir/RPMS" \
		--define "_specdir $rpmbuilddir/SPECS" \
		--define "_tmppath $rpmbuilddir/tmp" \
		--define "configopts ${config_opts}" \
		./prov/${provider}/libfabric-${provider}.spec
}

configure_options="LDFLAGS=-Wl,--build-id --enable-only"

rpmbuilddir=$PWD/rpmbuild

if [[ "${TARGET_OS}" == sle*  || "${TARGET_OS}" == rhel_8_6* ]]; then

    if [[ "${TARGET_ARCH}" == x86_64 ]]; then
        ROCM_CONFIG="--with-rocr=/opt/rocm --enable-rocr-dlopen"
    else
        ROCM_CONFIG=""
    fi
    CUDA_CONFIG="--with-cuda=/usr/local/cuda --enable-cuda-dlopen"
    if [[ "${OBS_TARGET_OS}" == cos* ]]; then
        GDRCOPY_CONFIG="--enable-gdrcopy-dlopen"
    else
        GDRCOPY_CONFIG=""
    fi
else
    ROCM_CONFIG=""
    CUDA_CONFIG=""
    GDRCOPY_CONFIG=""
fi

if [[ ( ${TARGET_OS} == sle15_sp4* || ${TARGET_OS} == sle15_sp5* ) \
        && ${TARGET_ARCH} == x86_64 ]]; then
    ZE_CONFIG="--with-ze=/usr --enable-ze-dlopen"
else
    ZE_CONFIG=""
fi

ENABLED_PROVIDERS="""
tcp
udp
rxm
rxd
hook_debug
hook_hmem
dmabuf_peer_mem
"""

if [[ -z "$BUILD_METADATA" ]] ; then
	if [[ -n "$LOCAL_BUILD" ]]
	then
		# If LOCAL_BUILD is defined, then use the branch information to determine how this
		# RPM should be named. This is not production path code, and purely for use by engineering
		# to generate more descriptive RPMs. 
		parent=$(git describe --tags --first-parent --exclude '*-ss*' --match 'v*' --abbrev=0)
		commits=$(git describe --tags --first-parent --exclude '*-ss*' --match 'v*' | awk -F- '{print $2}')
		parent_commit=$(git rev-list -n 1 $parent)
		branches=$(git branch --all --contains $parent_commit | \
			grep remotes/origin | \
			sed -e 's:  remotes/origin/::g' | \
			egrep '^release/slingshot-[[:digit:]]+.[[:digit:]]+$' | \
			sort --version-sort)
		num_branches=1
		current_branch=${current_branch:-$(git rev-parse --abbrev-ref HEAD)}
		for b in $branches
		do
			# if this branch is the current branch, stop counting
			if [[ "$b" == "$current_branch" ]]
			then
				break
			fi

			# for each release branch this commit is included on, increment the value
			# to preserve monotonically increasing release versions.
			let num_branches+=1
		done
		githash=$(git rev-parse --short HEAD)
		RELEASE=${num_branches}.${commits}_dev_g${githash}
	else
		RELEASE=1
	fi
else
	BRANCH=`git branch --show-current`
	git clone https://$HPE_GITHUB_TOKEN@github.hpe.com/hpe/hpc-sshot-slingshot-version.git

	cd hpc-sshot-slingshot-version

	if ! git checkout $BRANCH_NAME ; then
	echo "INFO: Branch '$BRANCH_NAME' does not exist in hpc-sshot-slingshot-version repo, using version string from master branch"
	else
	echo "INFO: Using Slingshot release version from '$BRANCH_NAME'"
	fi

	cd -

	PRODUCT_VERSION=$(cat hpc-sshot-slingshot-version/slingshot-version|| echo "0.0.0")
	echo "INFO: Slingshot release version '$PRODUCT_VERSION'"
	RELEASE="SSHOT${PRODUCT_VERSION}_${BUILD_METADATA}"
fi


if [ -z "${DL_PROV_BUILD}" ]
then
	if [ -z "${VERBS_BUILD}" ]
	then
		ENABLED_PROVIDERS+="cxi"
	else
		ENABLED_PROVIDERS+="verbs"
	fi
fi

base_providers=""
for entry in ${ENABLED_PROVIDERS}
do
    echo enabling provider: $entry
    base_providers+=" --enable-${entry} "
done

extra_configure_options="$CUDA_CONFIG $GDRCOPY_CONFIG $ROCM_CONFIG $ZE_CONFIG"
libfabric_core_configure_options="${configure_options} ${base_providers} ${extra_configure_options}"
verbs_dl_configure_options="${configure_options} --enable-verbs=dl ${extra_configure_options}"
cxi_dl_configure_options="${configure_options} --enable-cxi=dl ${extra_configure_options}"

RPMBUILD_OPTS=$(echo """
--define 'release ${RELEASE}'
--define 'install_modulefile 1'
--define 'install_default_module_version 1'
--define 'modulefile_path /opt/cray/modulefiles'
--define 'prefix /opt/cray'
--define '_topdir $rpmbuilddir'
--define '_sourcedir $rpmbuilddir/SOURCES'
--define '_rpmdir $rpmbuilddir/RPMS'
--define '_specdir $rpmbuilddir/SPECS'
--define '_tmppath $rpmbuilddir/tmp'
""" | tr '\n' ' ')

sed -i -e 's/Release: .*/Release: %{release}/g' libfabric.spec.in
./autogen.sh
./configure ${libfabric_core_configure_options}
LIBFABRIC_DISTSCRIPT_DIRTY_FILES=${LIBFABRIC_DISTSCRIPT_DIRTY_FILES:-libfabric.spec.in} RPMBUILD_OPTS="$RPMBUILD_OPTS --define 'configopts ${libfabric_core_configure_options}'" make rpm


# more base RPMs
move_rpms

if [ -n "${DL_PROV_BUILD}" ]
then
	if [[ ! -d $rpmbuilddir/SOURCES ]]
	then
		mkdir -p $rpmbuilddir/SOURCES || true
	fi

	cp libfabric-*.tar.bz2 $rpmbuilddir/SOURCES

	if [ -n "${VERBS_BUILD}" ]
	then
		build_dl_provider "verbs" "${verbs_dl_configure_options}"
	else
		build_dl_provider "cxi" "${cxi_dl_configure_options}"
	fi
	move_rpms
fi

run_rpmlint

exit 0