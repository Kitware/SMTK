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
  - Integrate changes to `master` branch
    - [ ] Create a merge request targeting `master` (do *not* add `Backport: release`)
    - [ ] Get positive review
    - [ ] `Do: merge`

  - Integrate changes to `release` branch
    - [ ] Update `.gitlab/ci/cdash-groups.json` to track the `release` CDash groups and commit it
    - [ ] `git push origin update-to-vVERSION:release vVERSION`

<!--
Once the robot supports fast-forward merges, this section replaces the above
`Integrate changes` sections:

  - Integrate changes.
    - [ ] Update `.gitlab/ci/cdash-groups.json` to track the `release` CDash groups and commit it
    - [ ] Create a merge request targeting `release`
      - [ ] Add `Backport: master:HEAD~` to end of the MR description
      - [ ] Add `Fast-forward: true` to end of the MR description
    - [ ] Get positive review
    - [ ] `Do: merge`
-->

  - Software process updates (these can all be done independently)
    - [ ] Update kwrobot with the new `release` branch rules (@ben.boeckel)
    - [ ] Run [this script][cdash-update-groups] to update the CDash groups (must be done after a nightly run to ensure all builds are in the `release` group).
    - [ ] Add (or update if `PATCH` is greater than 0) version selection entry in cmb-superbuild

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
