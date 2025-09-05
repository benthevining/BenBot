# lichess-bot configuration

The BenBot executable integrates with lichess using the [lichess-bot script](https://github.com/lichess-bot-devs/lichess-bot). They provide a Docker base image; the Dockerfile in this directory builds our engine and copies the configuration file into place inside this image. Build it by running `docker build --file Dockerfile --tag benbot:latest ..` from this directory, and run it using `docker run -d --name benbot benbot`.
