# SMTK continuous integration with Gitlab

SMTK and related projects use continous build and test infrastructure
to ensure code quality with each merge request and dependency change.
This is accomplished with [Gitlab's CI/CD tools](https://docs.gitlab.com/ee/ci/).
Here we describe our setup and how to update and debug CI for SMTK and related
projects, like CMB and AEVA.

The build and test environment for each CI job depends heavily on
the [CMB superbuild](https://gitlab.kitware.com/cmb/cmb-superbuild), which is
used to compile all the dependencies needed to build SMTK and related projects.
For building and testing SMTK-based plugins or applications, the superbuild is
used to build SMTK itself.

## Gitlab config

All Gitlab CI starts with `.gitlab-ci.yml` at the root of the project. It lists
each of the jobs that make up a CI pipeline. The `.gitlab` folder contains all
the supporting files and scripts that make up the pipeline jobs. There are many
[predefined variables](https://docs.gitlab.com/ee/ci/variables/predefined_variables.html)
that are used in the setup.

## Linux, Windows, macOS

Linux CI uses docker containers to run jobs. The base of each job is a docker
image, produced by nightly tasks in the superbuild. Windows and macOS use
bare-metal machines with a gitlab-runner service to accept jobs to run. The base
of these is a tarball of a superbuild install tree, again produced nightly, and
uploaded to [Google Drive][cmb-upload-gdrive]. This leads to a few confusing
differences in how things are set up.

[cmb-upload-gdrive]: https://drive.google.com/drive/folders/10i4bHyCHwHj94NaxWRJBthTr54gllPmz

## Updating CI images

If there is a change in a dependency that is needed, the image and tarballs need to be
updated. SMTK's dependencies are all external projects handled by the superbuild, so this
might be an update to ParaView, for instance.
1. Make sure any needed changes are available in ParaView
2. Update [versions.cmake](https://gitlab.kitware.com/cmb/cmb-superbuild/-/blob/afd0eb923e12b8e019db249b215bae3d32768b2e/versions.cmake#L30)
   in the superbuild to point to the git sha of the change needed. Submit an MR, get it merged.
3. Wait for the nightly builds to run.
4. Update Linux:
   1. Go to the list of CMB images at [dockerhub](https://hub.docker.com/r/kitware/cmb/tags)
   2. Search for the project you are updating, here `smtk`. (also `cmb`, `aeva`, `aevasession`, etc.)
   3. Based on the date, which represents a nightly build, choose the image that includes the needed change.
   4. Update the `image:` tag at the top of [os-linux.yml](https://gitlab.kitware.com/cmb/smtk/-/blob/master/.gitlab/os-linux.yml)
      since it is used as the base for all linux jobs.
5. Update Windows/macOS:
   1. Go to the `ci/smtk` folder on [Google Drive][cmb-upload-gdrive].
   2. For each OS, find the nightly build tarball with the change you need.
   3. Select the items and move them under the `keep/ci/smtk` directory.
   6. In `keep/ci/smtk`, edit `.gitlab/ci/download_superbuild.cmake`, in the "Determine the tarball
      to download" section
      1. From Google Drive, for each item use the three-dot menu to access `Share > Copy Link`
      2. Extract the File ID from the URL. For
         `https://drive.google.com/file/d/15uih5dSgTTbfJpt3e_UXwcDmhKXcG3cq/view?usp=drive_link` the
         File ID is `15uih5dSgTTbfJpt3e_UXwcDmhKXcG3cq`
      3. Find the job which uploaded the tarball and see its SHA512 (or download the file and
         compute it locally with `sha512sum` (Linux) or `shasum -a 512` (macOS))
      4. Update the date and reason for the referenced file in the comment.
6. Submit a merge request for SMTK. The pipelines that run will use the new images/tarballs that you have
   specified. Once any issues are resolved, you can merge.

## Resolving issues

All jobs in the [gitlab CI/CD pipeline](https://docs.gitlab.com/ee/ci/yaml/)
are specified in `.gitlab-ci.yml` in the root of the repository.
Stages run in order, and depend on the previous stage. Looking through the tags for each job shows how a job
is built up and executed.

You may need to recreate one of the jobs or manually create one of the images used
by CI to diagnose a problem. Since there are so many combinations, some examples are given next.

### Example: aevasession Linux images

Build the docker image used as the basis for Linux aeva-session tests.
The steps are inside [build-aevasession-image.sh](https://gitlab.kitware.com/cmb/cmb-superbuild/-/blob/master/.gitlab/ci/build-aevasession-image.sh),
except that includes steps for activating sccache and for using podman instead of docker
(since the script is run by CI inside a docker container). This will work on Linux/macOS/Windows
in a bash shell. Make sure you have increased the default memory and cores available to docker.
Start in a local checkout of [aeva-session](https://gitlab.kitware.com/aeva/session).

```bash
cd .gitlab/ci/docker/fedora32
date="$( date "+%Y%m%d" )"
ci_image_tag=fedora32
proj="aevasession"
image_tag_date="ci-$proj-$ci_image_tag-$date"
image_tag_latest="ci-$proj-$ci_image_tag-latest"
docker build -t "kitware/cmb:$image_tag_date" . |& tee build.log
docker tag "kitware/cmb:$image_tag_date" "kitware/cmb:$image_tag_latest"
# push to docker hub if the nightly build isn't working
docker push "kitware/cmb:$image_tag_date"
docker push "kitware/cmb:$image_tag_latest"
```

### Example: aeva package

Building the aeva package is different. The superbuild starts with centos7 instead of fedora32 (for maximum compatibility). And the superbuild is not pre-built - everything is built in one go by the superbuild. The steps followed are in [os-linux.yml](https://gitlab.kitware.com/cmb/cmb-superbuild/-/blob/master/.gitlab/os-linux.yml) `.cmake_build_linux`

Run bash in base image:
`docker run -it kitware/cmb:ci-superbuild-centos7-latest bash`
then manually entered the script steps to build the superbuild with my own branch when I was debugging a compile failure with gcc7.

Notes:

* scripts install both cmake and ninja, and alter the path to find them.
* The $LAUNCHER variable is used to provide an updated gcc instead of the default.
* Set your git email for a clone to work properly in the superbuild:
```
git config --global user.email "my.username@kitware.com"
git config --global user.name "My Name"
```
* use `yum install` to add packages to centos, like editors.

### Example: recreating a CI test

On Windows (or macOS):

* Start with the tarball referenced in download_superbuild.cmake. It must be unpacked at the same absolute path as on CI to run tests, Windows: `c:/glr/builds/cmb/cmb-ci/build/`

* Download artifacts from the successful build used by the test, unzip in the same build dir.
* Setup paths and potentially other variables in `os-windows.yml`, the log of the failed test shows these steps being executed.
* Add Qt dlls, downloaded and extracted like in download_qt.cmake
* ci checks out aeva-session at `C:/glr/builds/cmb/cmb-ci/` so the data from aeva-session should be at `C:/glr/builds/cmb/cmb-ci/data/`.
* TODO: macOS paths

On Linux:
* Start with the image referenced in `os-linux.yml`.
* CI_BUILDS_DIR is a gitlab predefined var. Depends on where gitlab-runner is set up. `/builds` on Linux.
* So the initial set-up done by gitlab-runner on Linux clones the repo to `GIT_CLONE_PATH: $CI_BUILDS_DIR/gitlab-kitware-sciviz-ci`
* For an MR branch, it might be like this, for user `aron.helser` and branch `bump_linux_ci`:
```bash
docker run -it kitware/cmb:ci-aevasession-fedora32-20200913 bash
cd /builds
git clone https://gitlab.kitware.com/aron.helser/session.git gitlab-kitware-sciviz-ci
cd gitlab-kitware-sciviz-ci
git checkout bump_linux_ci
git submodule update --init --recursive
# then the commands from the log or os-linux.yml
./.gitlab/ci/cmake.sh

# or as an alternative, mount a current checkout in the image:
docker run -v $PWD:/builds/gitlab-kitware-sciviz-ci:Z -it kitware/cmb:ci-aevasession-fedora32-20200913 bash
```
* Note: In the base image, the superbuild is installed to `/root/misc/root/smtk-deps/`
* to run GUI tests, you need to forward X11 (when running from Linux):
  * on local machine: `xhost +local:docker`
  * add: `docker run -e DISPLAY=$DISPLAY -v /tmp/.X11-unix/:/tmp/.X11-unix ...`
