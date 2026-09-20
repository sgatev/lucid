#!/bin/bash
# Publishes the page that the benchmark results are read on to the branch the
# results are kept on.
#
# github-action-benchmark writes a page of its own only when the branch has
# none, so the one put there here is what it leaves alone from then on. This
# runs before the results are recorded, so that the first run of all is the
# one that settles which page the branch has.
#
# Nothing is pushed when the published page already matches this one.

set -euo pipefail

readonly branch=gh-pages
readonly page=.github/benchmark/index.html
readonly published=dev/bench/index.html

# Checked out detached rather than onto a local gh-pages branch, which is the
# name the recording step fetches into afterwards.
worktree=$(mktemp -d)/page
trap 'git worktree remove --force "${worktree}" 2>/dev/null || true
      rm -rf "$(dirname "${worktree}")"' EXIT

git fetch origin "${branch}"
git worktree add --detach "${worktree}" "origin/${branch}"

mkdir -p "$(dirname "${worktree}/${published}")"
cp "${page}" "${worktree}/${published}"

if git -C "${worktree}" diff --quiet HEAD -- "${published}"; then
  echo "The published page is already up to date." >&2
  exit 0
fi

git -C "${worktree}" add "${published}"
git -C "${worktree}" \
  -c user.name=github-action-benchmark \
  -c user.email=github@users.noreply.github.com \
  commit --message "Update the page the benchmark results are read on"
git -C "${worktree}" push origin "HEAD:${branch}"
