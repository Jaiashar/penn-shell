# Validation

Verified October 11, 2026 using the Linux x86-64 Docker image (Ubuntu 20.04, Clang 10).

- `make` passes with the original `-Wall -Werror` flags.
- Interactive terminal checks pass: a pipeline (`printf hello | wc -c`), output redirection, input redirection, starting a background process and listing it with `jobs`.
- The published source is secret-scanned before publication.

Known historical behavior: noninteractive EOF returns status 1, and noninteractive runs can print `waitpid: No child processes`. The interactive workflow above was tested separately. Exhaustive signal, race-condition and memory-safety testing was not performed for this publication.
