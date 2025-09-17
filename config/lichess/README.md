# lichess-bot configuration

The BenBot executable integrates with lichess using the [lichess-bot script](https://github.com/lichess-bot-devs/lichess-bot). They provide a Docker base image; the Dockerfile in this directory builds our engine and copies the configuration file into place inside this image. This Docker image is then run on a VPS. See the `tag_and_release.yml` workflow file.
