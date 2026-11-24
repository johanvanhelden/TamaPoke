#!/usr/bin/env python3
"""Checks that the agent documentation is not stale (Python standard library only).

- Every `applyTo` glob in .github/instructions/*.instructions.md matches an existing file.
- Every `make <target>` in backticks or code blocks of AGENTS.md, README.md and .github/**/*.md exists.
- Every path-like backtick token in AGENTS.md exists.
- Every skill under .github/skills has a SKILL.md with name and description frontmatter.

Exits 1 with a list of problems, 0 when everything is in order.
"""

import os
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

SKIPPED_DIRECTORIES = {".git", ".sdl", "build", "node_modules"}
# Generated, installed or gitignored locations that do not exist in a fresh checkout.
UNTRACKED_PREFIXES = (
    ".gh-auth",
    "tests/build",
    ".dev/tmp",
    "_site",
    "tools/emu/.sdl",
    "tools/emu/tamapoke-emu",
    "tools/emu/proto.h",
    "tools/emu/sketch.cpp",
)

CODE_SPAN = re.compile(r"`([^`\n]+)`")
FENCED_BLOCK = re.compile(r"^```.*?^```", re.DOTALL | re.MULTILINE)
MAKE_COMMAND = re.compile(r"(?:^|[\s;&|(])make\s+([a-z][a-z0-9_-]*)")
MAKE_TARGET = re.compile(r"^([a-zA-Z0-9_.-]+)\s*:(?!=)", re.MULTILINE)
HELP_TARGETS = re.compile(r"@echo\s+\"[^\"]*?:\s*((?:[a-z][a-z0-9_-]*\s*)+)\"")
PLACEHOLDER = re.compile(r"[<>*{}$|\s]|://|YYYY|\.\.\.")


def walk_files():
    for directory, directories, files in os.walk(ROOT):
        directories[:] = [name for name in directories if name not in SKIPPED_DIRECTORIES]
        for name in files:
            yield (Path(directory) / name).relative_to(ROOT).as_posix()


def glob_to_regex(glob):
    parts = []
    index = 0
    while index < len(glob):
        if glob.startswith("**/", index):
            parts.append("(?:.*/)?")
            index += 3
        elif glob.startswith("**", index):
            parts.append(".*")
            index += 2
        elif glob[index] == "*":
            parts.append("[^/]*")
            index += 1
        elif glob[index] == "?":
            parts.append("[^/]")
            index += 1
        else:
            parts.append(re.escape(glob[index]))
            index += 1
    return re.compile("".join(parts) + r"\Z")


def frontmatter(path):
    text = path.read_text(encoding="utf-8")
    match = re.match(r"---\n(.*?)\n---\n", text, re.DOTALL)
    if not match:
        return None
    fields = {}
    for line in match.group(1).splitlines():
        key, separator, value = line.partition(":")
        if separator:
            fields[key.strip()] = value.strip().strip("'\"")
    return fields


def check_apply_to(files):
    problems = []
    for path in sorted((ROOT / ".github/instructions").glob("*.instructions.md")):
        fields = frontmatter(path)
        name = path.relative_to(ROOT).as_posix()
        if not fields or not fields.get("applyTo"):
            problems.append(f"{name}: missing `applyTo` frontmatter")
            continue
        for glob in (part.strip() for part in fields["applyTo"].split(",")):
            expression = glob_to_regex(glob)
            if not any(expression.match(file) for file in files):
                problems.append(f"{name}: applyTo `{glob}` matches no file")
    return problems


def make_targets():
    makefile = (ROOT / "Makefile").read_text(encoding="utf-8")
    targets = set(MAKE_TARGET.findall(makefile))
    return targets


def code_snippets(text):
    for block in FENCED_BLOCK.findall(text):
        yield block
    yield from CODE_SPAN.findall(FENCED_BLOCK.sub("", text))


def doc_files():
    for name in ("AGENTS.md", "README.md"):
        yield ROOT / name
    yield from sorted((ROOT / ".github").rglob("*.md"))


def check_make_targets():
    targets = make_targets()
    problems = []
    for path in doc_files():
        if not path.is_file():
            continue
        missing = set()
        for snippet in code_snippets(path.read_text(encoding="utf-8")):
            missing.update(target for target in MAKE_COMMAND.findall(snippet) if target not in targets)
        for target in sorted(missing):
            problems.append(f"{path.relative_to(ROOT).as_posix()}: `make {target}` is not a Makefile target")
    return problems


def looks_like_path(token):
    if PLACEHOLDER.search(token) or token.startswith(("~", "/", "@")):
        return False
    if "/" not in token:
        return False
    return token.endswith("/") or "." in token.rsplit("/", 1)[-1]


def check_agents_paths():
    path = ROOT / "AGENTS.md"
    problems = []
    seen = set()
    for token in CODE_SPAN.findall(path.read_text(encoding="utf-8")):
        if token in seen or not looks_like_path(token):
            continue
        seen.add(token)
        relative = token[2:] if token.startswith("./") else token
        if relative.startswith(UNTRACKED_PREFIXES):
            continue
        if not (ROOT / relative).exists():
            problems.append(f"AGENTS.md: path `{token}` does not exist")
    return problems


def check_skills():
    problems = []
    for directory in sorted(path for path in (ROOT / ".github/skills").iterdir() if path.is_dir()):
        name = directory.relative_to(ROOT).as_posix()
        skill = directory / "SKILL.md"
        if not skill.is_file():
            problems.append(f"{name}: SKILL.md is missing")
            continue
        fields = frontmatter(skill) or {}
        for key in ("name", "description"):
            if not fields.get(key):
                problems.append(f"{name}/SKILL.md: frontmatter `{key}` is missing")
        if fields.get("name") and fields["name"] != directory.name:
            problems.append(f"{name}/SKILL.md: name `{fields['name']}` differs from the directory name")
    return problems


def main():
    files = list(walk_files())
    problems = [
        *check_apply_to(files),
        *check_make_targets(),
        *check_agents_paths(),
        *check_skills(),
    ]

    if problems:
        print("AI docs check failed:", file=sys.stderr)
        for problem in problems:
            print(f"  - {problem}", file=sys.stderr)
        return 1

    print("AI docs check passed.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
