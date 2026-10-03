#!/usr/bin/env python3
"""Apply the recorded CM14.1 platform commits; --check only validates them."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import os

PATCH_ROOT = Path(__file__).resolve().parent


def git(repo, *args, **kwargs):
    return subprocess.check_output(["git", "-C", str(repo), *args], **kwargs)


def prepare(top, project):
    repo = top / project["path"]
    base = project["base_commit"]
    git(repo, "rev-parse", "--show-toplevel")
    if git(repo, "status", "--porcelain", "--untracked-files=no").strip():
        raise RuntimeError("Tracked changes present: " + project["path"])
    if subprocess.call(["git", "-C", str(repo), "merge-base", "--is-ancestor",
                        base, "HEAD"]) != 0:
        raise RuntimeError("Expected CM14.1 baseline missing: " + project["path"])
    history = git(repo, "log", "--no-merges", "--format=medium", "-p",
                  base + "..HEAD")
    ids = subprocess.check_output(["git", "patch-id", "--stable"], input=history)
    applied = {line.split()[0].decode() for line in ids.splitlines()}
    pending = []
    for patch in project["patches"]:
        path = PATCH_ROOT / patch["file"]
        if hashlib.sha256(path.read_bytes()).hexdigest() != patch["sha256"]:
            raise RuntimeError("Patch checksum mismatch: " + patch["file"])
        if patch["patch_id"] in applied:
            if pending:
                raise RuntimeError("Patch series has a gap: " + project["path"])
            print("Already applied: " + patch["file"], flush=True)
        else:
            pending.append(path)
    # Simulate the complete pending series in a separate index. This never
    # changes the real index, worktree, branch or commits during --check.
    if pending:
        with tempfile.TemporaryDirectory(prefix="mione-patches-") as tmp:
            env = dict(os.environ, GIT_INDEX_FILE=str(Path(tmp) / "index"))
            git(repo, "read-tree", "HEAD", env=env)
            for path in pending:
                git(repo, "apply", "--cached", "--whitespace=nowarn",
                    str(path), env=env)
                print("Validated: " + str(path.relative_to(PATCH_ROOT)), flush=True)
    return repo, pending


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true",
                        help="validate without changing branches or worktrees")
    parser.add_argument("--top", type=Path, default=PATCH_ROOT.parents[3],
                        help="Android source root")
    args = parser.parse_args()
    series = json.loads((PATCH_ROOT / "series.json").read_text())
    # Validate every repository before starting any git-am operation.
    plans = [prepare(args.top.resolve(), p) for p in series["projects"]]
    if not args.check:
        for repo, pending in plans:
            if pending:
                subprocess.check_call(["git", "-C", str(repo), "am",
                                       *map(str, pending)])
    print("CM14.1 platform patches " + ("validated" if args.check else "ready"))


if __name__ == "__main__":
    try:
        main()
    except (OSError, RuntimeError, subprocess.CalledProcessError) as error:
        print("Stopped: " + str(error), file=sys.stderr)
        sys.exit(1)
