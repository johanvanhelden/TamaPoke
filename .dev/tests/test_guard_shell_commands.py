import json
from pathlib import Path
import subprocess
import unittest


ROOT = Path(__file__).resolve().parents[2]
HOOK = ROOT / ".github/hooks/scripts/guard-shell-commands.py"


def decision(command):
    # The Copilot CLI/SDK sends toolArgs as a JSON-encoded string.
    payload = {"toolName": "bash", "toolArgs": json.dumps({"command": command})}
    result = subprocess.run([str(HOOK)], input=json.dumps(payload), text=True, capture_output=True)
    assert result.returncode == 0, result.stderr
    return json.loads(result.stdout)["permissionDecision"] if result.stdout.strip() else None


class GuardShellCommandsTest(unittest.TestCase):
    def test_denies_gh_push_erase_and_root_find(self):
        for command in (
            "gh repo view foo/bar",
            "cd x && gh api user",
            "git push origin main",
            "git -C . push",
            "esptool.py --port /dev/ttyACM0 erase_flash",
            "python3 -m esptool erase-flash",
            "find / -name '*.ino'",
            "bash -c 'gh pr list'",
        ):
            with self.subTest(command=command):
                self.assertEqual(decision(command), "deny")

    def test_allows_everyday_commands(self):
        for command in (
            "git status",
            "git commit -m 'feat: x'",
            "make test",
            "python3 -m http.server 8000 --directory web",
            "find . -name '*.cpp'",
            "esptool.py --port /dev/ttyACM0 write_flash 0x0 app.bin",
            "echo gh",
        ):
            with self.subTest(command=command):
                self.assertIsNone(decision(command))

    def test_fails_open_on_garbage(self):
        result = subprocess.run([str(HOOK)], input="not json", text=True, capture_output=True)
        self.assertEqual(result.returncode, 0)
        self.assertEqual(result.stdout, "")


if __name__ == "__main__":
    unittest.main()
