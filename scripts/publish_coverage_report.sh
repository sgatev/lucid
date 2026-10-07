#!/bin/bash
# Publishes the coverage report that scripts/run_coverage.sh drew to the branch
# GitHub Pages serves, beside the benchmark results:
#
#   scripts/run_coverage.sh
#   scripts/publish_coverage_report.sh
#
# The report takes the place of the one published before it whole, so that a
# page for a source that has since gone goes with it.
#
# Nothing is pushed when the published report already matches this one.

set -euo pipefail

readonly branch=gh-pages
readonly report=coverage
readonly published=dev/coverage

if [[ ! -f "${report}/index.html" ]]; then
  echo "There is no report in ${report}/ to publish." >&2
  exit 1
fi

# Checked out detached rather than onto a local gh-pages branch, as the
# benchmark page is.
worktree=$(mktemp -d)/page
trap 'git worktree remove --force "${worktree}" 2>/dev/null || true
      rm -rf "$(dirname "${worktree}")"' EXIT

git fetch origin "${branch}"
git worktree add --detach "${worktree}" "origin/${branch}"

# The benchmark job pushes to the same branch, and a push that lands between
# the fetch and this one turns it away. The report is then put on top of what
# that push left, and pushed again.
for attempt in 1 2 3 4 5; do
  rm -rf "${worktree:?}/${published}"
  mkdir -p "$(dirname "${worktree}/${published}")"
  cp -R "${report}" "${worktree}/${published}"
  # The command genhtml was run with, which names paths on the machine that
  # ran it and nothing a reader of the report needs.
  rm -f "${worktree}/${published}/cmd_line"

  git -C "${worktree}" add --all -- "${published}"
  if git -C "${worktree}" diff --cached --quiet; then
    echo "The published report is already up to date." >&2
    exit 0
  fi

  git -C "${worktree}" \
    -c user.name=github-actions[bot] \
    -c user.email=41898282+github-actions[bot]@users.noreply.github.com \
    commit --quiet --message "Update the coverage report for ${GITHUB_SHA:-$(git rev-parse HEAD)}"
  if git -C "${worktree}" push origin "HEAD:${branch}"; then
    exit 0
  fi

  echo "The push was turned away, attempt ${attempt} of 5." >&2
  git fetch origin "${branch}"
  git -C "${worktree}" reset --quiet --hard "origin/${branch}"
done

echo "The report could not be published." >&2
exit 1
