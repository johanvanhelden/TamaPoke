#!/usr/bin/env python3
"""preToolUse guardrail for agent shell commands (not a security boundary).

Denies recurring mistakes in this private project:
- `gh` (GitHub CLI): the repository is private; the owner does not want it used.
- `git push`: the developer reviews and pushes.
- `esptool` erase commands: erasing the flash wipes the pet's save (NVS).
- `find /...`, which crawls the slow Windows mount under WSL.

Accepts both the Copilot CLI/SDK payload (toolName + toolArgs) and the VS Code
Local payload (tool_name + tool_input), and answers in both output shapes.
Fails open on anything it cannot parse.
"""

import json
import os
import re
import shlex
import sys

SHELL_TOOLS = {"bash", "run_in_terminal"}

ESPTOOLS = {"esptool", "esptool.py", "esptool.exe"}
ERASE_COMMANDS = {"erase_flash", "erase-flash", "erase_region", "erase-region"}

# Prefixes that run the next word as the actual command.
PASSTHROUGH = {"sudo", "time", "nohup", "exec", "command", "nice", "xargs", "env"}
NESTED_SHELLS = {"sh", "bash", "zsh", "dash"}
OPERATOR_CHARACTERS = set(";&|()\n")
KEYWORD_SEPARATORS = {"{", "}", "!", "then", "do", "else"}
ASSIGNMENT = re.compile(r"^[A-Za-z_][A-Za-z0-9_]*=")

GH_REASON = (
    "Blocked: `gh` is not used in this private project. Use plain `git` for local history; "
    "ask the owner when GitHub information is needed."
)
PUSH_REASON = "Blocked: never push. The developer reviews and pushes. See AGENTS.md."
ERASE_REASON = (
    "Blocked: erasing the flash wipes the pet's save (NVS). Flash with 'Erase device' off, "
    "or ask the owner first. See AGENTS.md."
)
FIND_REASON = (
    "Blocked: `find /...` searches the entire filesystem, which is slow and noisy under WSL. "
    "Scope the search (e.g. `find . -name ...`) or use the glob/grep tools."
)


def tokenize(command):
    lexer = shlex.shlex(command, posix=True, punctuation_chars=";&|()\n")
    lexer.whitespace = " \t\r"
    lexer.whitespace_split = True
    return list(lexer)


def segments(tokens):
    current = []
    for token in tokens:
        if token in KEYWORD_SEPARATORS or set(token) <= OPERATOR_CHARACTERS:
            if current:
                yield current
            current = []
        else:
            current.append(token)
    if current:
        yield current


def check_segment(words, depth):
    index = 0
    while index < len(words):
        word = words[index]
        if ASSIGNMENT.match(word):
            index += 1
            continue
        if word in PASSTHROUGH:
            index += 1
            while index < len(words) and (words[index].startswith("-") or ASSIGNMENT.match(words[index])):
                index += 1
            continue
        break
    else:
        return None

    word = words[index]
    arguments = words[index + 1:]

    if word == "find" and arguments and arguments[0] == "/":
        return FIND_REASON

    name = os.path.basename(word)
    if name == "gh":
        return GH_REASON

    if name == "git" and "push" in arguments[:4]:
        return PUSH_REASON

    if name in ESPTOOLS or (name.startswith("python") and ESPTOOLS & {os.path.basename(a) for a in arguments[:3]}):
        if ERASE_COMMANDS & set(arguments):
            return ERASE_REASON

    if os.path.basename(word) in NESTED_SHELLS and depth < 3:
        for position, argument in enumerate(arguments[:-1]):
            if argument.startswith("-") and not argument.startswith("--") and "c" in argument:
                return check_command(arguments[position + 1], depth + 1)

    return None


def check_command(command, depth=0):
    for words in segments(tokenize(command)):
        reason = check_segment(words, depth)
        if reason:
            return reason
    return None


def extract_command(payload):
    tool_name = payload.get("toolName") or payload.get("tool_name") or ""
    if tool_name not in SHELL_TOOLS:
        return None

    arguments = payload.get("toolArgs", payload.get("tool_input", {}))
    if isinstance(arguments, str):
        arguments = json.loads(arguments)
    command = arguments.get("command") if isinstance(arguments, dict) else None
    return command if isinstance(command, str) else None


def main():
    try:
        command = extract_command(json.load(sys.stdin))
        reason = check_command(command) if command else None
    except (ValueError, AttributeError, TypeError):
        return 0

    if reason:
        json.dump({
            "permissionDecision": "deny",
            "permissionDecisionReason": reason,
            "hookSpecificOutput": {
                "hookEventName": "PreToolUse",
                "permissionDecision": "deny",
                "permissionDecisionReason": reason,
            },
        }, sys.stdout)
    return 0


if __name__ == "__main__":
    sys.exit(main())
