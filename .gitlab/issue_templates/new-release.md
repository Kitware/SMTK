<!--
This template is for tracking a release of smtk. Please replace the
following strings with the associated values:

  - `VERSION`: e.g. yy.mm.n
  - `MAJOR`: e.g. yy is the year
  - `MINOR`: e.g. mm is the month
  - `PATCH`: e.g. the release sequence number (start at 0)

Please remove this comment.
-->

# Preparatory steps

  - Update smtk guides
    - Assemble release notes into `doc/release/notes/smtk-MAJOR.MINOR.md`.
      - [ ] If `PATCH` is greater than 0, add items to the end of this file.
      - [ ] Get positive review and merge.

# Update smtk

Keep the relevant items for the kind of release this is.

If making a first release candidate from master, i.e., `PATCH` is 0.

  - [ ] Update `master` branch for **smtk**
```
git fetch origin
git checkout master
git merge --ff-only origin/master
```
  - [ ] Update `version.txt` and tag the commit
```
git checkout -b update-to-vVERSION
echo VERSION > version.txt
git commit -m 'Update version number to VERSION' version.txt
git tag -a -m 'SMTK VERSION' vVERSION HEAD
```

  - Integrate changes.
    - [ ] Update `.gitlab/ci/cdash-groups.json` to track the `release` CDash groups and commit it
    - Create a merge request targeting `release`
      - [ ] Obtain a GitLab API token for the `kwrobot.release.cmb` user (ask @ben.boeckel if you do not have one)
      - [ ] Add the `kwrobot.release.cmb` user to your fork with at least `Developer` privileges (so it can open MRs)
      - [ ] Use [the `release-mr`][release-mr] script to open the create the Merge Request (see script for usage)
    - [ ] Get positive review
    - [ ] `Do: merge`
    - [ ] Push the tag to the main repository
      - [ ] `git push origin vVERSION`

  - Software process updates (these can all be done independently)
    - [ ] Update kwrobot with the new `release` branch rules (@ben.boeckel)
    - [ ] Run [this script][cdash-update-groups] to update the CDash groups
      - This must be done after a nightly run to ensure all builds are in the `release` group
      - See the script itself for usage documentation
    - [ ] Add (or update if `PATCH` is greater than 0) version selection entry in cmb-superbuild

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
