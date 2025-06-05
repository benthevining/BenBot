# ben-bot

This directory contains the `ben-bot` executable. This executable is a command line UCI engine.

## Running a Lichess bot instance

* Clone the [lichess-bot](https://github.com/lichess-bot-devs/lichess-bot) repository into a subdirectory of this project's root
* Follow their [installation instructions](https://github.com/lichess-bot-devs/lichess-bot/wiki/How-to-Install) to get the Python venv set up
* Make sure your local `.env` file sets the `LICHESS_BOT_TOKEN` variable
* Build the `ben_bot` target
* In your terminal, `cd` into `lichess-bot`, then run the following:
```shell
. venv/bin/activate
python3 lichess-bot.py
```

This will start the bot and wait for challenges via Lichess.
