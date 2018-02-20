#!/usr/bin/env bash

echo "Setting up useful Git aliases..." &&

# General aliases that could be global
git config alias.pullall '!bash -c "git pull && git submodule update --init"' &&
git config alias.prepush 'log --graph --stat origin/master..' &&

( git config --unset alias.gitlab-push; true ) &&
# Alias to sync the current topic branch from GitLab
git config alias.gitlab-sync '!bash utilities/gitsetup/git-gitlab-sync' &&

true
