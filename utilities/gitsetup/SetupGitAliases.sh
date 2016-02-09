#!/usr/bin/env bash

echo "Setting up useful Git aliases..." &&

# General aliases that could be global
git config alias.pullall '!bash -c "git pull && git submodule update --init"' &&
git config alias.prepush 'log --graph --stat origin/master..' &&

# Alias to push the current topic branch to GitLab
git config alias.gitlab-push '!bash utilities/gitsetup/git-gitlab-push' &&

true
