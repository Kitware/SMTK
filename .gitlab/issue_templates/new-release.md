<!--
This template is for tracking a release of smtk. Please replace the
following strings with the associated values:

  - `VERSION`: e.g. 3.0.0
  - `MAJOR`: e.g. 3
  - `MINOR`: e.g. 0

Please remove this comment.
-->

# Preparatory steps

  - Update smtk guides
    - Assemble release notes into `doc/release/notes/smtk-VERSION`.
      - [ ] Get positive review and merge.

# Update smtk

If making a release from the `release` branch, e.g., `vMAJOR.MINOR.0-RC2 or above`:

  - [ ] Update `release` branch for **smtk**
```
git fetch origin
git checkout release
git merge --ff-only origin/release
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

 - Update documentation page
    - [ ] See `https://github.com/Kitware/paraview-docs/blob/gh-pages/versions.json`

# Upload documentation

  - [ ] Verify documentation is uploaded and is correct

# Post-release

  - [ ] Write and publish blog post with release notes.
  - [ ] Post an announcement in the Announcements category on
        [discourse.smtk.org](https://discourse.kitware.com/c/smtk/).

  - [ ] Update release notes
    (https://www.paraview.org/Wiki/ParaView_Release_Notes)
  - [ ] Move unclosed issues to next release milestone in GitLab

/cc @ben.boeckel
/cc @bob.obara
/cc @tjcorona
/cc @dcthomp
/label ~"priority:required"
