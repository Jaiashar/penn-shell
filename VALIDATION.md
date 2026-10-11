# Validation

Verified October 11, 2026 using the Linux x86-64 Docker image (Ubuntu 20.04, Clang 10).

- `make` passes with the original `-Wall -Werror` flags.
- Interactive terminal checks pass: a pipeline (`printf hello | wc -c`), output redirection, input redirection, starting a background process and listing it with `jobs`.
- The published source is secret-scanned before publication.

Known historical behavior: noninteractive EOF returns status 1, and noninteractive runs can print `waitpid: No child processes`. The interactive workflow above was tested separately. Exhaustive signal, race-condition and memory-safety testing was not performed for this publication.

## Continuous integration

[GitHub Actions](https://github.com/Jaiashar/penn-shell/actions/workflows/ci.yml) rebuilds the container and executes `python3 tests/smoke.py` on pushes to `main`, pull requests, and manual runs. The job has read-only repository permission, an immutable checkout-action reference, and a ten-minute timeout. Runtime tests have no network access and require neither secrets nor external services.

The assertions cover two- and three-command pipelines, output overwrite and append, input redirection, background job listing, foreground completion, and clean interactive EOF.

A passing badge establishes these checks only. It does not imply exhaustive correctness, memory-safety verification, or a production operating system.

The initial CI preparation caught and fixed the historical append-redirection bug. The original failure and one-line flag correction are documented in [PROVENANCE.md](PROVENANCE.md).
