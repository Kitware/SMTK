#!/usr/bin/env bash

cd "${BASH_SOURCE%/*}/.." &&
utilities/gitsetup/setup-user && echo &&
utilities/gitsetup/setup-hooks && echo &&
utilities/gitsetup/SetupGitAliases.sh && echo &&
(utilities/gitsetup/setup-upstream ||
 echo 'Failed to setup origin.  Run this again to retry.') && echo &&
(utilities/gitsetup/setup-gitlab ||
 echo 'Failed to setup GitLab.  Run this again to retry.') && echo &&
utilities/gitsetup/tips

# Rebase master by default
git config rebase.stat true
git config branch.master.rebase true

# Record the version of this setup so Scripts/pre-commit can check it.
SetupForDevelopment_VERSION=2
git config hooks.SetupForDevelopment ${SetupForDevelopment_VERSION}
