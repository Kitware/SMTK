<!--
This template is for tracking a release of smtk. Please replace the
following strings with the associated values:

  - `VERSION`: e.g. yy.mm.n
  - `MAJOR`: e.g. yy is the year
  - `MINOR`: e.g. mm is the month
  - `PATCH`: e.g. the release sequence number (start at 0)
  - `BRANCHPOINT`: The commit where the release should be started

Please remove this comment.
-->

# Preparatory steps

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
    - Make a commit for each of these `release`-only changes on a single topic
      (suggested branch name: `update-to-vVERSION`):
      - Assemble release notes into `doc/release/notes/smtk-MAJOR.MINOR.md`.
        - [ ] If `PATCH` is greater than 0, add items to the end of this file.
      - [ ] Update `version.txt` and tag the commit (tag this commit below)
        - [ ] `git checkout -b update-to-vVERSION BRANCHPOINT`
        - [ ] `echo VERSION > version.txt`
        - [ ] `git commit -m 'Update version number to VERSION' version.txt`
      - [ ] Update `.gitlab/ci/cdash-groups.json` to track the `release` CDash
            groups

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
    - [ ] `Do: merge`
    - [ ] Push the tag to the main repository
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
  - [ ] Remove deprecated methods on `master`

/cc @ben.boeckel
/cc @bob.obara
/cc @dcthomp
/label ~"priority:required"
