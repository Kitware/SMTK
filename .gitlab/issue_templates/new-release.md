<!--
This template is for tracking a release of smtk. Please replace the
following strings with the associated values:

  - `VERSION`: e.g. yy.mm.n
  - `MAJOR`: e.g. yy is the year
  - `MINOR`: e.g. mm is the month
  - `PATCH`: e.g. the release sequence number (start at 0)
  - `BRANCHPOINT`: The commit where the release should be started - it is a point on master where the release process branch is started from.  The release process branch will have multiple commits including the assembling of release notes and changing of the version.

Please remove this comment.
-->

# Preparatory steps

  - Make sure you have run the `utility/SetupForDevelopment.sh` script.
  - Update smtk guides

# Update smtk


  - Update the local copy of the base branch.
    - If `PATCH` is 0, update `master`
    - Otherwise, update `release`
    - [ ] `git fetch origin`
    - [ ] `git checkout $branch`
    - [ ] `git merge --ff-only origin/$branch`
      - If this fails, there are local commits that need to be removed
  - Ensure that changes intended for the release are in it.
    - The [`backport-mrs.py`][backport-mrs] script can be used to find and
      ensure that merge requests assigned to the associated milestone are
      available on the `release` branch. See its documentation for usage.
  - Integrate changes.
    - Create branch for release commits
        - [ ] `git checkout -b update-to-vVERSION BRANCHPOINT`

    - Make a commit for each of these `release`-only changes:
      - [ ] Assemble release notes into `doc/release/smtk-MAJOR.MINOR.rst`.
        - [ ] Top of release notes should have `.. _release-notes-MAJOR.MINOR:` and a reference to the previous release's notes (See `doc/release/smtk-21.07.rst` or later for an example.)
        - [ ] Add reference to new release notes to `doc/release/index.rst`. (This publishes release notes on read-the-docs.)
        - [ ] Update the ReadMe file to refer to the new release notes
        - [ ] If `PATCH` is greater than 0, add items to the end of this file.
        - [ ] `git rm` all of the individual release note files *except* `00-example.rst`.
        - [ ] Do a git add for  ReadMe,  index.rst, and smtk-MAJOR.MINOR.rst
        - [ ] `git commit -m 'Compile release notes for VERSION'`
      - [ ] Update `version.txt` and tag the commit (tag this commit below)
        - [ ] `echo VERSION > version.txt`
        - [ ] Rebuild smtk to check for deprecation warnings
        - [ ] `git commit -m 'Update version number to VERSION' version.txt` (commit message must be verbatum)
      - [ ] Update `.gitlab/ci/cdash-groups.json` to track the `release` CDash
            groups.
        - [ ] Change "master" => "release" everywhere in `.gitlab/ci/cdash-groups.json`,
              and change "latest-master" => "latest-release".
        - [ ] `git commit -m 'Update cdash-groups.json to track the release group' .gitlab/ci/cdash-groups.json`

    - Create a merge request targeting `release`
      - [ ] Obtain a GitLab API token for the `kwrobot.release.cmb` user (ask
            @ben.boeckel if you do not have one)
      - [ ] Add the `kwrobot.release.cmb` user to your fork with at least
            `Developer` privileges (so it can open MRs)
      - [ ] Use [the `release-mr`][release-mr] script to open the create the
            Merge Request (see script for usage)
        - Pull the script for each release; it may have been updated since it
          was last used
        - The script outputs the information it will be using to create the
          merge request. Please verify that it is all correct before creating
          the merge request. See usage at the top of the script to provide
          information that is either missing or incorrect (e.g., if its data
          extraction heuristics fail).
    - [ ] Get positive review
    - [ ] Get a repo maintainer (currently Ben) or owner (currently Bob) to`Do: merge`
    - [ ] Get a repo maintainer or ownder to push the tag to the main repository
      - [ ] `git tag -a -m 'SMTK VERSION' vVERSION commit-that-updated-version.txt`
      - [ ] `git push origin vVERSION`

  - Software process updates (these can all be done independently)
    - [ ] Update kwrobot with the new `release` branch rules (@ben.boeckel)
    - [ ] Run [this script][cdash-update-groups] to update the CDash groups
      - This must be done after a nightly run to ensure all builds are in the
        `release` group
      - See the script itself for usage documentation
    - [ ] Add (or update if `PATCH` is greater than 0) version selection entry
          in cmb-superbuild

[backport-mrs]: https://gitlab.kitware.com/utils/release-utils/-/blob/master/backport-mrs.py
[release-mr]: https://gitlab.kitware.com/utils/release-utils/-/blob/master/release-mr.py
[cdash-update-groups]: https://gitlab.kitware.com/utils/cdash-utils/-/blob/master/cdash-update-groups.py

# Post-release

  - [ ] Write and publish blog post with release notes.
  - [ ] Post an announcement in the Announcements category on
        [discourse.smtk.org](https://discourse.kitware.com/c/smtk/).
  - [ ] Create a new commit on `master` that does the following:
    - [ ] Remove the deprecated methods on `master`.  Note that it should be discussed before hand which deprecated methods should be removed in this release.
    - [ ] Set the version.txt on `master` to be yy.mm.100

/cc @ben.boeckel
/cc @bob.obara
/cc @dcthomp
/label ~"priority:required"
