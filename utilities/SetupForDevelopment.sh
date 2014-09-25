#!/usr/bin/env bash

# Sets up the repository ready for development. Aims to provide
# reasonable defaults for developers.

# Make sure we are inside the repository.
cd "${BASH_SOURCE%/*}/.."

# Rebase master by default
git config rebase.stat true
git config branch.master.rebase true
git config push.default tracking

utilities/GitSetup/setup-user && echo &&
utilities/GitSetup/setup-hooks && echo &&
utilities/GitSetup/SetupGitAliases.sh && echo &&
utilities/GitSetup/setup-stage && echo &&
(utilities/GitSetup/setup-ssh ||
 echo 'Failed to setup SSH.  Run this again to retry.') && echo &&
utilities/GitSetup/tips ||
exit $?
