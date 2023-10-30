#!/bin/bash
git checkout -b rebase-test-branch
db=$(git remote show git@github.hpe.com:hpe/hpc-shs-libfabric-netc.git | grep 'HEAD branch' | cut -d' ' -f5)
mb=$(git merge-base origin/${db} HEAD)

# Run a shorten test suite against each commits, except head commit, in the PR.
# Assumes that the commit subject is unique between all commits in the PR.
exclude_commit_subject=$(git log -1 --pretty=%s | tr -d ' ')
git rebase ${mb} --exec "bash ./cxi_vm_commit.sh -s -e ${exclude_commit_subject}"
if [[ $? -ne 0 ]]; then
    exit 1
fi

# Run longer test suite against all commits together.
bash ./cxi_vm_commit.sh
