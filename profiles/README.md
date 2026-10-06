# Profiles

Profiles contain everything PSPRecomp needs for a particular title without putting game-specific addresses or behaviour into the reusable framework. A profile supplies the executable identity and memory map, the native host and HLE modules, build scripts, tests and player documentation.

A profile is selected at configure time:

```bash
cmake -S . -B out/<profile> -DPSPRECOMP_PROFILE=<profile>
```

The repository currently includes [`mhfu`](mhfu/README.md), which builds Pokee for **Monster Hunter Freedom Unite** (USA, `ULUS-10391`). The installer accepts that release only.

See [`docs/PROFILE_GUIDE.md`](../docs/PROFILE_GUIDE.md) before adding another title.
