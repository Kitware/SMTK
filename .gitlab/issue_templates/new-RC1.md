<!--
This template is for tracking a release of SMTK. Please replace the
following strings with the associated values:

  - `VERSION`
  - `MAJOR`
  - `MINOR`

Please remove this comment.
-->

# Preparatory steps

  - Update SMTK guides
    - Assemble release notes into `doc/release/notes/SMTK-VERSION`.
      - [ ] Get positive review and merge.

# Update SMTK

Keep the relevant items for the kind of release this is.

If making a first release candidate from master, i.e., `vMAJOR.MINOR.0-RC1`:

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
    - [ ] `git push origin update-to-vVERSION:release vVERSION`
    - [ ] Update kwrobot with the new `release` branch rules (@ben.boeckel)

# Post-release

  - [ ] Write and publish blog post with release notes.
  - [ ] Post an announcement in the Announcements category on
        [discourse.smtk.org](https://discourse.kitware.com/c/smtk/).

/cc @ben.boeckel
/cc @bob.obara
/cc @tjcorona
/cc @dcthomp
/label ~"priority:required"
