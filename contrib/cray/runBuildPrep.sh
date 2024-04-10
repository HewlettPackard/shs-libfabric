#!/bin/bash

set -ex

ARTI_URL=https://${ARTIFACT_REPO_HOST}/artifactory
OS_TYPE=`cat /etc/os-release | grep "^ID=" | sed "s/\"//g" | cut -d "=" -f 2`
OS_VERSION=`cat /etc/os-release | grep "^VERSION_ID=" | sed "s/\"//g" | cut -d "=" -f 2`
OS_MAJOR_VERSION=$(echo $OS_VERSION | cut -d "." -f 1)

RHEL_GPU_SUPPORTED_VERSIONS="8.8 8.9 9.3"

# Override product since we are only using the internal product stream to avoid
# clashing with slingshot10 libfabric
PRODUCT='slingshot-host-software'

echo "$0: --> BRANCH_NAME: '${BRANCH_NAME}'"
echo "$0: --> PRODUCT: '${PRODUCT}'"
echo "$0: --> TARGET_ARCH: '${TARGET_ARCH}'"
echo "$0: --> OBS_TARGET_OS: '${OBS_TARGET_OS}'"
echo "$0: --> TARGET_OS: '${TARGET_OS}'"
echo "$0: --> OS_TYPE: '${OS_TYPE}'"
echo "$0: --> OS_VERSION: '${OS_VERSION}'"

if [[ "${BRANCH_NAME}" == release/* ]]; then
    ARTI_LOCATION='rpm-stable-local'
    ARTI_BRANCH=${BRANCH_NAME}
elif [[ "${CHANGE_TARGET}" == release/* ]]; then
    # CHANGE_TARGET is only set for PR builds and points to the PR target branch
    ARTI_LOCATION='rpm-stable-local'
    ARTI_BRANCH=${CHANGE_TARGET}
else
    ARTI_LOCATION='rpm-master-local'
    ARTI_BRANCH=dev/master
fi

CNE_BRANCH=""

case "${OBS_TARGET_OS}" in
    sle15_sp4_*)    COS_BRANCH='release/cos-2.5' ;;
    cos_2_5_*)      COS_BRANCH='release/cos-2.5' ;;
    csm_1_4_*)      COS_BRANCH='release/cos-2.5' ;;
    cos_3_0_*)      COS_BRANCH='release/cos-3.0' ;;
    csm_1_5_*)      COS_BRANCH='release/cos-3.0' ;;
    sle15_sp5_*)    COS_BRANCH='release/cos-3.0' ;;
    *)              COS_BRANCH='dev/master' ;;
esac

echo "$0: --> ARTI_LOCATION: '${ARTI_LOCATION}'"
echo "$0: --> ARTI_BRANCH: '${ARTI_BRANCH}'"
echo "$0: --> COS_BRANCH: '${COS_BRANCH}'"
echo "$0: --> VERBS_BUILD: '${VERBS_BUILD:-false}'"

# Override per OS
with_rocm=0
with_cuda=0
with_ze=0

RPMS="kdreg2-devel "
if [ -n "${VERBS_BUILD}" ]
then
    RPMS+="rdma-core rdma-core-devel "
else
    RPMS+="cray-libcxi-devel "
fi

if [[ ${TARGET_OS} == "centos_8" ]]; then
    TARGET_OS="centos_8_ncn"
fi

ROCR_RPMS="hsa-rocr-devel"


if [[ ( ${TARGET_OS} == sle15_sp4* || ${TARGET_OS} == sle15_sp5* ) \
        && ${TARGET_ARCH} == x86_64 ]]; then
    with_ze=1
    ZE_RPMS="level-zero-devel"
else
    ZE_RPMS=""
fi

if [[ ${TARGET_OS} =~ ^centos || ${TARGET_OS} =~ ^rhel ]]; then
    RPMS+=" libcurl-devel json-c-devel cray-libcxi-static "
else
    RPMS+=" libcurl-devel libjson-c-devel cray-libcxi-devel-static "
fi

function rpm_install_wrapper() {
  rpm=$1

  if command -v yum > /dev/null; then
    # yum requires encoded urls
    rpm="$(echo ${rpm} | sed -e 's/ /%20/g')"

    yum install -y "$rpm"
  else
    zypper --no-gpg-checks --non-interactive install "$rpm"
  fi
}

function install_gdrcopy_cne() {
  release=$1
  os=$2

  GDR_VERSION=2.4.1-1.0_3.1__ge7e1f57.shasta

  BASE_URL=${ARTI_URL}/cne-rpm-stable-local/release/${release}/${os}
  rpm_install_wrapper "${BASE_URL}/$TARGET_ARCH/gdrcopy-${GDR_VERSION}.${TARGET_ARCH}.rpm"
  rpm_install_wrapper "${BASE_URL}/$TARGET_ARCH/gdrcopy-devel-${GDR_VERSION}.${TARGET_ARCH}.rpm"
}

function install_gdrcopy_cos() {
  release=$1
  os=$2

  case $release in
    "cos-2.4")
      GDR_VERSION=2.3-2.4_8.9__g3f1d0f8.shasta
      ;;
    "cos-2.5")
      GDR_VERSION=2.3-2.5_9.5__ga81d33c.shasta
      ;;
    *)
      echo "unrecognized target OS: $TARGET_OS"
      exit 1
  esac

  BASE_URL=${ARTI_URL}/cos-rpm-stable-local/release/$release/$os
  rpm_install_wrapper "${BASE_URL}/$TARGET_ARCH/gdrcopy-${GDR_VERSION}.${TARGET_ARCH}.rpm"
  rpm_install_wrapper "${BASE_URL}/noarch/gdrcopy-devel-${GDR_VERSION}.noarch.rpm"
}

function install_gdrcopy_nvidia() {
  GDR_VERSION=2.4-1

  case $OBS_TARGET_OS in
    sle15*)
      GDR_VERSION=${GDR_VERSION}.sles-15.5
      URL_DISTRO=sles15
      ;;
    rhel_8*)
      GDR_VERSION=${GDR_VERSION}.el8
      URL_DISTRO=rhel8
      ;;
    rhel_9*)
      GDR_VERSION=${GDR_VERSION}.el9
      URL_DISTRO=rhel9
      ;;
    *)
      echo "unrecognized target OS: ${OBS_TARGET_OS}"
      exit 1
  esac

  case $TARGET_ARCH in
    x86_64)
      URL_ARCH=x64
      ;;
    aarch64)
      URL_ARCH=${TARGET_ARCH}
      ;;
    *)
      echo "unrecognized target ARCH: ${TARGET_ARCH}"
      exit 1
  esac

  BASE_URL="${ARTI_URL}/nvidia.com-gdrcopy-remote/CUDA 12.2/${URL_DISTRO}/${URL_ARCH}"
  rpm_install_wrapper "${BASE_URL}/gdrcopy-${GDR_VERSION}.${TARGET_ARCH}.rpm"
  rpm_install_wrapper "${BASE_URL}/gdrcopy-devel-${GDR_VERSION}.noarch.rpm"
}

function install_gdrcopy() {
  case $OBS_TARGET_OS in
    cos_2_4*)
      install_gdrcopy_cos "cos-2.4" "sle15_sp4_cn"
      ;;
    cos_2_5*)
      install_gdrcopy_cos "cos-2.5" "sle15_sp4_cn"
      ;;
    cos_3*)
      install_gdrcopy_cne "cne-1.0" "sle15_sp5_cn"
      ;;
    csm_1_3*)
      install_gdrcopy_cos "cos-2.5" "sle15_sp4_cn"
      ;;
    csm_1_4*)
      install_gdrcopy_cos "cos-2.5" "sle15_sp4_cn"
      ;;
    csm_1_5*)
      install_gdrcopy_cne "cne-1.0" "sle15_sp5_cn"
      ;;
    sle15_sp5*)
      if [[ "$TARGET_ARCH" == "aarch64" ]]; then
        install_gdrcopy_cne "cne-1.0" "sle15_sp5_cn"
      else
        install_gdrcopy_nvidia
      fi
      ;;
    *)
      install_gdrcopy_nvidia
      ;;
  esac
}

install_gdrcopy

if command -v yum > /dev/null; then
    yum-config-manager --add-repo=${ARTI_URL}/${PRODUCT}-${ARTI_LOCATION}/${ARTI_BRANCH}/${TARGET_OS}/
    yum-config-manager --setopt=gpgcheck=0 --save

    if [ $OS_TYPE = "rhel"  ] && \
            [[ $RHEL_GPU_SUPPORTED_VERSIONS = *$OS_VERSION* ]]; then

	if [[ ${TARGET_ARCH} == x86_64 ]]; then
            with_rocm=1
	fi

        with_cuda=1

        case $OS_VERSION in
        8.8)
            ROCM_VERSION="5.7"
            NVIDIA_VERSION="23.9"
            ;;
        8.9)
            ROCM_VERSION="6.0"
            NVIDIA_VERSION="23.11"
            ;;
        9.3)
            ROCM_VERSION="6.0"
            NVIDIA_VERSION="23.11"
            ;;
        *)
            echo "GPU software versions not defined for OS version \"${OS_VERSION}\""
            exit 1
        esac

	rocm_rpms=""

	if [[ ${TARGET_ARCH} == x86_64 ]]; then
            if [[ $OS_VERSION =~ ^8\.[0-9]+ ]]; then
		echo "Using radeon-rocm-remote/rhel8"
		yum-config-manager --add-repo=${ARTI_URL}/radeon-rocm-remote/rhel8/${ROCM_VERSION}/main
            elif [[ $OS_VERSION =~ ^9\.[0-9]+ ]]; then
		echo "Using radeon-rocm-remote/rhel9"
		yum-config-manager --add-repo=${ARTI_URL}/radeon-rocm-remote/rhel9/${ROCM_VERSION}/main
            else
		echo "Variable: $OS_VERSION does not start with 8 or 9"
		exit 1
            fi

	    rocm_rpms="rocm-dev hip-devel"
	fi

        yum-config-manager --add-repo=${ARTI_URL}/mirror-nvhpc/rhel/${TARGET_ARCH}

        RPMS+="  ${rocm_rpms} nvhpc-${NVIDIA_VERSION} "
    fi

    yum install -y $RPMS
    
elif command -v zypper > /dev/null; then
    with_cuda=1

    if [[ ${TARGET_ARCH} == x86_64 ]]; then
        with_rocm=1
    fi

    case "${OBS_TARGET_OS}" in
        sle15_sp4_*)    CUDA_RPMS="nvhpc-2023"
                    ;;
        cos_2_5_*)      CUDA_RPMS="nvhpc-2023"
                    ;;
        csm_1_4_*)      CUDA_RPMS="nvhpc-2023"
                    ;;
        csm_1_5_*)      CUDA_RPMS="nvhpc"
                    ;;
        cos_3_0_*)      CUDA_RPMS="nvhpc"
                    ;;
        sle15_sp5_*)    CUDA_RPMS="nvhpc"
                    ;;
        *)              CUDA_RPMS="nvhpc"
                    ;;
    esac


    zypper --verbose --non-interactive addrepo --no-gpgcheck --check \
        --priority 20 --name=${PRODUCT}-${ARTI_LOCATION} \
         ${ARTI_URL}/${PRODUCT}-${ARTI_LOCATION}/${ARTI_BRANCH}/${OBS_TARGET_OS}/ \
         ${PRODUCT}-${ARTI_LOCATION}

    if [ $with_cuda -eq 1 ]; then
        zypper --verbose --non-interactive addrepo --no-gpgcheck --check \
            --priority 20 --name=cuda \
            ${ARTI_URL}/cos-internal-third-party-generic-local/nvidia_hpc_sdk/${TARGET_OS}/${TARGET_ARCH}/${COS_BRANCH}/ \
            cuda

        RPMS+=" ${CUDA_RPMS} "
    fi

    if [ $with_rocm -eq 1 ]; then
        zypper --verbose --non-interactive addrepo --no-gpgcheck --check \
            --priority 20 --name=rocm \
            ${ARTI_URL}/cos-internal-third-party-generic-local/rocm/latest/${TARGET_OS}/${TARGET_ARCH}/${COS_BRANCH}/ \
            rocm

        RPMS+=" ${ROCR_RPMS} "
    fi

    if [[ $with_ze -eq 1 ]]; then
        zypper --verbose --non-interactive  addrepo --no-gpgcheck --check \
            --priority 20 --name=ze \
            ${ARTI_URL}/cos-internal-third-party-generic-local/intel_gpu/latest/${TARGET_OS}/${TARGET_ARCH}/${COS_BRANCH}/ \
            ze

        RPMS+=" ${ZE_RPMS} "
    fi

    zypper refresh

    zypper --non-interactive --no-gpg-checks install $RPMS

else
    "Unsupported package manager or package manager not found -- installing nothing"
fi

set -x

if [[ $with_cuda -eq 1 ]]; then

    # Specify the directory where you want to search for folders
    search_dir="/opt/nvidia/hpc_sdk/Linux_${TARGET_ARCH}"

    # Define a pattern to match folders in the "x.y" format
    pattern='^[0-9]+\.[0-9]+$'

    # Initialize variables to keep track of the latest folder and its version
    latest_version=""
    latest_folder=""

    # Iterate through the directories in the search directory
    for dir in "$search_dir"/*; do
        if [[ -d "$dir" && $(basename "$dir") =~ $pattern ]]; then
            version="$(basename "$dir")"
            if [[ -z "$latest_version" || "$version" > "$latest_version" ]]; then
                latest_version="$version"
                latest_folder="$dir"
            fi
        fi
    done

    # Check if any matching folders were found
    if [ -n "$latest_folder" ]; then
        nvhpc_sdk_version="$latest_version"
        echo "Using $nvhpc_sdk_version at $latest_folder"
        nvhpc_cuda_path=$latest_folder/cuda
        echo "Using $nvhpc_sdk_version at $nvhpc_cuda_path"
        
        # Convenient symlink which allows the libfabric build process to not
        # have to call out a specific versioned CUDA directory.
        ln -s $nvhpc_cuda_path /usr/local/cuda

        # The CUDA device driver RPM provides a usable libcuda.so which is
        # required by the libfabric autoconf checks. Since artifactory does not
        # provide this RPM, the cuda-driver-devel-11-0 RPM is installed and
        # provides a stub libcuda.so. But, this stub libcuda.so is installed
        # into a non-lib path. A symlink is created to fix this.
        ln -s /usr/local/cuda/lib64/stubs/libcuda.so \
              /usr/local/cuda/lib64/libcuda.so

    else
        echo "No matching CUDA folders found."
        exit 1
    fi
fi

if [[ $with_rocm -eq 1 ]]; then
    update-alternatives --display rocm

    # Find the ROCm version directory in /opt/
    rocm_version_dir=$(ls -d /opt/rocm-* 2>/dev/null)

    # Check if a ROCm version directory was found
    if [ -n "$rocm_version_dir" ]; then
        # Extract the version from the directory path
        rocm_version=$(basename "$rocm_version_dir")
        
        # Check if the version follows the expected format
        if [[ $rocm_version =~ ^rocm-[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
            echo "ROCm version: $rocm_version"
            ln -s /opt/"$rocm_version" /opt/rocm
        else
            echo "Unexpected directory structure found: $rocm_version"
            exit 1
        fi
    else
        echo "The installation of ROCm is not found in the /opt/ directory."
        exit 1
    fi
fi

echo "ROCm Version: ${rocm_version}" > /var/tmp/gpu-versions
echo "Nvidia Version: ${nvhpc_sdk_version}" >> /var/tmp/gpu-versions
echo "GPU Versions File:"
echo "$(</var/tmp/gpu-versions)"
