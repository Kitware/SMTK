#!/usr/bin/env bash

set -e
set -x
shopt -s dotglob

readonly name="gitsetup"
readonly ownership="GitSetup Upstream <kwrobot@kitware.com>"
readonly subtree="utilities/gitsetup"
readonly repo="https://gitlab.kitware.com/utils/gitsetup.git"
readonly tag="setup"
readonly exact_tree_match=false
readonly paths="
.gitattributes

git-gitlab-sync
setup-gitlab
setup-hooks
setup-lfs
setup-ssh
setup-upstream
setup-user
tips

LICENSE
NOTICE
README
"

extract_source () {
    echo "* -export-ignore" >> .gitattributes
    git_archive
}

. "${BASH_SOURCE%/*}/thirdpartyupdate/update-common.sh"
