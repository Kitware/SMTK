Download SMTK with Git
==========================

This page documents how to download SMTK source code through [Git][].
See the [README](README.md) for more information.

[Git]: http://git-scm.com

Clone
-----

Optionally configure Git to [use SSH instead of HTTPS](#use-ssh-instead-of-https).

Clone SMTK using the commands:

    $ git clone --recursive https://gitlab.kitware.com/cmb/smtk.git SMTK
    $ cd SMTK

Update
------

Users that have made no local changes and simply want to update a
clone with the latest changes may run:

    $ git pull
    $ git submodule update --init

Avoid making local changes unless you have read our [developer instructions][].

[developer instructions]: develop.md


Use SSH instead of HTTPS
------------------------

Git can be configured to access ``gitlab.kitware.com`` repositories through
the ``ssh`` protocol instead of ``https`` without having to manually change
every URL found in instructions, scripts, and submodule configurations.

1.  Register for an account on [our GitLab instance][GitLab Access]  and select a user name.

2.  Add [SSH Keys][] to your GitLab account to authenticate your user via
    the ``ssh`` protocol.

3.  Configure Git to use ``ssh`` instead of ``https`` for all repositories
    on ``gitlab.kitware.com``:

        $ git config --global url."git@gitlab.kitware.com:".insteadOf https://gitlab.kitware.com/
    The ``--global`` option causes this configuration to be stored in
    ``~/.gitconfig`` instead of in any repository, so it will map URLs
    for all repositories.

4.  Return to the [Clone](#clone) step above and use the instructions as
    written.  There is no need to manually specify the ssh protocol when
    cloning.  The Git ``insteadOf`` configuration will map it for you.

[GitLab Access]: https://gitlab.kitware.com/users/sign_in
[SSH Keys]: https://gitlab.kitware.com/profile/keys
