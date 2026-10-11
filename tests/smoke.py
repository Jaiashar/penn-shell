"""Exercise the built shell with real processes, pipes, files and a terminal."""
from pathlib import Path
import re
import tempfile
from terminal import Terminal

ROOT = Path(__file__).resolve().parents[1]

with tempfile.TemporaryDirectory(prefix="penn-shell-ci-") as work:
    terminal = Terminal([str(ROOT / "penn-shell")], work, "penn-shell> ")
    try:
        terminal.expect_prompt()
        output = terminal.command("printf hello | wc -c")
        assert re.search(r"(?m)^\s*5\s*$", output), output
        terminal.command("echo first > message.txt")
        terminal.command("echo second >> message.txt")
        assert (Path(work) / "message.txt").read_text() == "first\nsecond\n"
        output = terminal.command("cat < message.txt")
        assert "first\nsecond\n" in output, output
        output = terminal.command("printf abcdef | tr a-z A-Z | wc -c")
        assert re.search(r"(?m)^\s*6\s*$", output), output
        terminal.command("sleep 3 &")
        output = terminal.command("jobs")
        assert "sleep 3" in output and "running" in output.lower(), output
        terminal.command("fg")
        output = terminal.command("jobs")
        assert "sleep 3" not in output, output
        terminal.finish(b"\x04")
    finally:
        terminal.close()
print("PASS: two/three-stage pipelines, overwrite/append/input redirection, jobs, fg, clean EOF")
