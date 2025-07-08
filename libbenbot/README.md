# libbenbot

This library defines BenBot's evaluation and search logic.

Include style is:
```cpp
#include <libbenbot/search/Search.hpp>
```

## CMake

### Targets

* `ben_bot::libbenbot`: library target

## Dependencies

* [nlohmann_json](https://json.nlohmann.me/), used for deserializing opening book from JSON. Used as private dependency only.
