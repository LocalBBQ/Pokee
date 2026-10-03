# Contributing to Yakumo

Bug reports, documentation improvements, compatibility testing and code contributions are welcome. Follow the [Code of Conduct](CODE_OF_CONDUCT.md) when participating in repository discussions.

## Reporting problems

Check existing issues before opening a new one. Use the Bug report form for problems, Feature request for proposed improvements, or Test report for smoke-test results. Include the following when reporting a problem:

- The release version or exact commit you tested.
- Your platform and relevant hardware.
- Steps to reproduce the problem, expected behavior and actual behavior.
- Relevant logs or screenshots, with personal information removed.

Use minimal synthetic examples where possible. Keep game files and saves local; do not attach them to issues or pull requests. Report suspected security vulnerabilities privately as described in [SECURITY.md](SECURITY.md).

## Contribution rules

- Write code, comments, documentation, commit messages and pull requests in English. The exceptions are `README.ru.md` and `README.es.md`; update these translations in the same pull request as `README.md` when changing its content.
- Never commit game data: disc images, `EBOOT` or `DATA.BIN` contents, generated game code, overlay corpora or saves. Do not include personal paths, user names, machine names, device addresses or credentials.
- Do not share pirated copies or distribute Yakumo builds bundled with copyrighted game content through project spaces. See the [game-content policy](CODE_OF_CONDUCT.md#piracy-and-game-content).
- Write implementations yourself. Do not copy or translate code line by line from projects with licenses incompatible with this repository's MIT license. Public documentation, constants, offsets and format facts can inform your work. Record the origin of intentionally included third-party code as described in [SOURCE_PROVENANCE.md](docs/SOURCE_PROVENANCE.md).

## Getting started

Start with [ARCHITECTURE.md](docs/ARCHITECTURE.md) to understand the runtime and game-profile boundaries.

Follow [BUILDING.md](docs/BUILDING.md) for your platform. Full builds are expensive: reuse local generated code, compatible prebuilt overlays and game data as the guide describes. Runtime-header changes can require rebuilding all overlays.

Build with `cmake --build`, rather than invoking Ninja directly. Run only one build per build directory and keep parallelism low (`-j2`). Do not delete `.ninja_deps` or `.ninja_log`.

If you have a device-specific setup, keep its paths, addresses and launch instructions in the ignored `LOCAL_TESTING.md` at the repository root. Read it before using that environment; it is optional and is not part of a fresh clone.

## Testing changes

Choose checks that exercise the behavior you changed. [TESTING.md](docs/TESTING.md) describes automated checks and manual smoke tests; [COMPATIBILITY.md](docs/COMPATIBILITY.md) records verified platform results.

For the unit-test targets:

```sh
cmake --build out/mhp3rd --target psprecomp_test_binaries -j2
ctest --test-dir out/mhp3rd
```

Bound game runs with a timeout and inspect logs and captures. For regressions, compare a known-good build and your change with the same scripted input and game state. Measure performance changes before and after rather than relying on impressions. See [TESTING.md](docs/TESTING.md) for testing workflows and the profile README's [Diagnostics](profiles/mhp3rd/README.md#diagnostics) section for tracing options.

## Submitting a pull request

1. Work on a branch or fork and open a pull request against `main`. Never push directly to `main`.
2. Open a draft pull request early and push working commits so the work is available for review.
3. Keep the change focused and the commit series clean and reviewable. Pull requests are merged by rebase.
4. Explain what changed and why, reference related issues, and list validation results: platform, tested commit, checks performed and anything not verified.
5. Update documentation affected by the change, including settings, build instructions and verified compatibility results where applicable.
6. Leave the pull request in draft while it is still being prepared. Request review when it is ready; merging requires an explicit maintainer decision.

Fixes go to `main` first and are backported to release branches when applicable. Follow [RELEASING.md](docs/RELEASING.md#branches) for the release workflow.
