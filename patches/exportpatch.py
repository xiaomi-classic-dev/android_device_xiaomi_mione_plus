#!/usr/bin/env python3
"""Regenerate the recorded platform patch series from its baseline commits."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess

PATCH_ROOT = Path(__file__).resolve().parent


def git(repo, *args):
    return subprocess.check_output(["git", "-C", str(repo), *args])


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--top", type=Path, default=PATCH_ROOT.parents[3])
    args = parser.parse_args()
    series = json.loads((PATCH_ROOT / "series.json").read_text())
    exports = []
    for project in series["projects"]:
        repo = args.top.resolve() / project["path"]
        base = project["base_commit"]
        if git(repo, "status", "--porcelain", "--untracked-files=no").strip():
            raise RuntimeError("Tracked changes present: " + project["path"])
        git(repo, "merge-base", "--is-ancestor", base, "HEAD")
        if git(repo, "rev-list", "--merges", base + "..HEAD").strip():
            raise RuntimeError("Nonlinear patch history: " + project["path"])
        previous = {p["source_commit"]: p["file"] for p in project["patches"]}
        old_files = set(previous.values())
        patches = []
        for number, commit in enumerate(git(repo, "rev-list", "--reverse", base + "..HEAD").decode().splitlines(), 1):
            data = git(repo, "format-patch", "-1", "--stdout", "--no-signature", "--no-numbered", commit)
            subject = git(repo, "show", "-s", "--format=%s", commit).decode().strip()
            filename = previous.get(commit)
            if filename is None:
                name = re.sub(r"[^A-Za-z0-9._-]+", "-", subject).strip("-")[:64]
                filename = project["path"].replace("/", "-") + "/%04d-%s.patch" % (number, name)
            patch_id = subprocess.check_output(["git", "patch-id", "--stable"], input=data).split()[0].decode()
            patches.append({"file": filename, "source_commit": commit,
                            "patch_id": patch_id, "sha256": hashlib.sha256(data).hexdigest()})
            exports.append((PATCH_ROOT / filename, data))
        project["source_head"] = git(repo, "rev-parse", "HEAD").decode().strip()
        project["patches"] = patches
        exports.extend((PATCH_ROOT / filename, None) for filename in old_files - {p["file"] for p in patches})
    # Prepare every project before replacing any recorded files.
    for path, data in exports:
        if data is None:
            path.unlink()
        else:
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes(data)
    (PATCH_ROOT / "series.json").write_text(json.dumps(series, indent=2) + "\n")
    print("Exported %d platform patches" % sum(len(p["patches"]) for p in series["projects"]))


if __name__ == "__main__":
    main()
