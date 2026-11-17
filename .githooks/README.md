# Git hooks: main only

Maintainers work straight on `main`. `git config core.hooksPath .githooks` turns on:

- `reference-transaction`: refuses to create any branch other than `main`.
- `pre-push`: refuses to push anything other than `main` and tags.

Contributors: these are off unless you turn them on, so branch in your fork as usual.
