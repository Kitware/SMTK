#!/usr/bin/env bash

# Sets up the repository ready for development. Aims to provide
# reasonable defaults for developers.

# Make sure we are inside the repository.
cd "${BASH_SOURCE%/*}/.."

# Rebase master by default
git config rebase.stat true
git config branch.master.rebase true
git config push.default tracking

Utilities/GitSetup/setup-user && echo &&
Utilities/GitSetup/setup-hooks && echo &&
Utilities/GitSetup/SetupGitAliases.sh && echo &&
Utilities/GitSetup/setup-stage && echo &&
(Utilities/GitSetup/setup-ssh ||
 echo 'Failed to setup SSH.  Run this again to retry.') && echo &&
Utilities/GitSetup/tips ||
exit $?
