# GitHub Actions

## `ci.yml`

This action runs on every push, and when a PR is opened. It builds & runs tests, submits results to CDash (in the `Experimental` group), and uploads the engine artifacts. Include the string `[skip ci]` in your commit message to prevent this workflow from being triggered.

## `deploy_lichess.yml`

This action updates the Docker image used to run the lichess bot script, then deploys it to our VPS. This action is triggered by every push to `main`.

## `docs.yml`

This action builds the Doxygen documentation and deploys it to GitHub pages. This action is triggered by every push to `main`.

## `nightly.yml`

Similar to `ci.yml`, except this action runs on a schedule every night, and CDash results are in the `Nightly` group.

## `sprt.yml`

Runs an [SPRT test](https://www.chessprogramming.org/Sequential_Probability_Ratio_Test) of the given branch against the latest release using the [fastchess tool](https://github.com/Disservin/fastchess). This action runs on every push, and when a PR is opened. Include the string `[skip ci]` in your commit message to prevent this workflow from being triggered.

## `stop_lichess.yml`

This action can be run manually to stop the lichess-bot script running on the VPS. This action is never triggered automatically.

## `tag_and_release.yml`

This action bumps the project version, creates a git tag, runs a build, and creates a GitHub release. This action is never triggered automatically.

## Internal actions

### `build_and_test.yml`

Internal action that creates the build matrix for `ci.yml` and `nightly.yml`. Invokes build & test steps via `ctest -D`.
