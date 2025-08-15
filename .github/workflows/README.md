# GitHub Actions

## `ci.yml`

This action runs on every push, and when a PR is opened. It builds & runs tests, submits results to CDash (in the `Experimental` group), and uploads the engine artifacts. Include the string `[skip ci]` in your commit message to prevent this workflow from being triggered.

## `nightly.yml`

Similar to `ci.yml`, except this action runs on a schedule every night, and CDash results are in the `Nightly` group.

## `docs.yml`

This action builds the Doxygen documentation and deploys it to GitHub pages. This action is triggered by every push to `main`, unless the commit message contains `[skip ci]`.

## `build_and_test.yml`

Internal action that actually executes the build matrix.
