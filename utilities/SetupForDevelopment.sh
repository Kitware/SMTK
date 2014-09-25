#!/usr/bin/env bash

# Sets up the repository ready for development. Aims to provide
# reasonable defaults for developers.

# Make sure we are inside the repository.
cd "${BASH_SOURCE%/*}/.."

# Rebase master by default
git config rebase.stat true
git config branch.master.rebase true
git config push.default tracking

utilities/gitsetup/setup-user && echo &&
utilities/gitsetup/setup-hooks && echo &&
utilities/gitsetup/SetupGitAliases.sh && echo &&
utilities/gitsetup/setup-stage && echo &&
(utilities/gitsetup/setup-ssh ||
 echo 'Failed to setup SSH.  Run this again to retry.') && echo &&
utilities/gitsetup/tips ||
exit $?
