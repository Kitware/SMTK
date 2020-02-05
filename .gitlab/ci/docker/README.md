# CI Docker images

These images build base images for use in the CI infrastructure. They are
automatically rebuilt by the CMB Superbuild nightly as well as on every
`master` update. You may either push manually built images to the `kitware/cmb`
Docker Hub repository or trigger a CMB Superbuild `master` run to make new
images available.

## Updating images

After updating a `Dockerfile` here, it needs to be rebuilt before it can be
used by the CI. You may either build and push it yourself or let the
superbuild do it for you. The former is more convenient and can be done in one
MR, but can race with the superbuild pushing its images to the same repository
(since our tags only have a granularity of a calendar day).

### Manual building

After updating the `Dockerfile` (and associated scripts), it's a standard image
build sequence:

```sh
cd $name
docker build -t kitware/cmb/ci-smtk-$name-$YYMMDD .
docker push kitware/cmb/ci-smtk-$name-$YYMMDD
```

### Superbuild method

Once the `Dockerfile` change is merged into `master` in this repository, the
superbuild will use it to rebuild the next set of images the next time it
rebuilds. This happens when any of:

  - its `master` branch is updated
  - the nightly build runs
  - a manual trigger from [its pipelines page][] (the `Run Pipeline` button)

## Using the new image(s)

Once the updated image is pushed, the `.gitlab-ci.yml` file here needs to be
updated to the new tag to use the new image.

[its pipelines page]: https://gitlab.kitware.com/cmb/cmb-superbuild/pipelines
