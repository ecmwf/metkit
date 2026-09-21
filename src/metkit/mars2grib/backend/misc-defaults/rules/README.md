# Misc Rules

Each header in this directory owns the absence behavior and accepted type of one
physical misc keyword. Headers named after logical aggregate properties, such as
`pv.h`, `waveDirectionGrid.h`, and `waveFrequencyGrid.h`, additionally select
between alternative physical representations.

Simple rule structs expose:

```cpp
using value_type = ...;
static constexpr std::string_view name = ...;
static constexpr MiscPolicy policy = ...;
static resolve(...);
```

All dictionary reads are delegated to `../Access.h`. Unknown mandatory
fallbacks deliberately throw `Mars2GribMiscDefaultsException` containing
`TODO: fallback not implemented`.
