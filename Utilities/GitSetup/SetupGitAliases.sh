#!/usr/bin/env bash

echo "Setting up git aliases..."

GIT=git

GITCONFIG="${GIT} config"

# General aliases that could be global
${GITCONFIG} alias.prepush 'log --graph --stat origin/master..'

# Staging aliases
stage_cmd='ssh git@www.kitware.com stage SMTK'
git_branch="\$(git symbolic-ref HEAD | sed -e 's|^refs/heads/||')"
${GITCONFIG} alias.stage-cmd "!${stage_cmd}"
${GITCONFIG} alias.stage-push "!sh -c \"git fetch stage --prune && git push stage HEAD\""
${GITCONFIG} alias.stage-branch "!sh -c \"${stage_cmd} print\""
${GITCONFIG} alias.stage-merge "!sh -c \"${stage_cmd} merge ${git_branch}\""
${GITCONFIG} alias.stage-merge-delete "!sh -c \"!__BRANCH=${git_branch} && ${stage_cmd} merge \$__BRANCH && git checkout master && git pull && git branch -d \$__BRANCH\""
${GITCONFIG} alias.merge-master-to-branch "!sh -c \"git fetch origin && git merge origin/master\""
${GITCONFIG} alias.stage-checkout "!sh -c \"git fetch stage --prune && git checkout -b \$0 stage/\$0 \""
