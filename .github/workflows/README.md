# GitHub Actions

## `ci.yml`

This action runs on every push, and when a PR is opened. It builds & runs tests, submits results to CDash (in the `Experimental` group), and uploads the engine artifacts. Include the string `[skip ci]` in your commit message to prevent this workflow from being triggered.

## `nightly.yml`

Similar to `ci.yml`, except this action runs on a schedule every night, and CDash results are in the `Nightly` group.

## `docs.yml`

This action builds the Doxygen documentation and deploys it to GitHub pages. This action is triggered by every push to `main`, unless the commit message contains `[skip ci]`.

## `bump_version.yml`

This action bumps the project version and creates a git tag. When the git tag is pushed, this triggers `release_from_tag.yml` which runs a build and creates a GitHub release. This action is never triggered automatically.

## Internal actions

### `build_and_test.yml`

Internal action that creates the build matrix for `ci.yml` and `nightly.yml`. Invokes build & test steps via `ctest -D`.

### `release_from_tag.yml`

This action is triggered by a tag being pushed, and creates a release from the tag.
