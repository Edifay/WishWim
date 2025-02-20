# Contributing to this repository

Development of this project takes place on [GitHub].

## Creating issues for bugs and feature requests

When you find a bug or feel like suggesting a new feature, create
[an issue][issues] on the GitHub repo.  The issue templates will
guide you through it.

## Creating a pull-request

Direct contribution follows the following branching model:

- `master` contains the latest, and tested, features.  Typically, a merge to master will be accompanied by a version-bump and associated Git tag.
- `develop` contains the latest, greatest features and updates, but might not
  be fully tested yet.
- `feature/...`, `bugfix/...`, etc. might or might not work.  Consider these
  work-in-progress.

### What this means for you:

- When issuing a PR, please base it onto `develop`.
- Call your branch whatever you like -- I'm not going to enforce traditional
  `feature`, `bugfix`, etc. prefixes, because I've found them too restrictive
  in the past, but it would be nice if you prefix your branch name with
  something sensible.
- As a contributor, you might want to point your Neovim (or other editor)
  config to use the `develop` branch.

[GitHub]: https://github.com/jpt13653903/tree-sitter-vhdl
[issues]: https://github.com/jpt13653903/tree-sitter-vhdl/issues
