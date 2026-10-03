#!/usr/bin/env python3
"""Run every registered native CTest executable on one Android emulator."""
import argparse
import json
from pathlib import Path
import shlex
import subprocess
import time
import xml.etree.ElementTree as ET


def run(*args, **kwargs):
    return subprocess.run(args, check=True, timeout=120, **kwargs)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("build_dir", type=Path)
    parser.add_argument("stl", type=Path)
    args = parser.parse_args()
    build = args.build_dir.resolve()
    tests = json.loads(run("ctest", "--test-dir", str(build), "--show-only=json-v1",
                           capture_output=True, text=True).stdout)["tests"]
    if not tests:
        raise RuntimeError("No CTest tests registered")
    remote = "/data/local/tmp/yakumo-unit-tests"
    diagnostics = build / "android-results"
    diagnostics.mkdir(exist_ok=True)
    suite = ET.Element("testsuite", name="Android native tests", tests=str(len(tests)))
    failed = 0
    try:
        run("adb", "shell", f"rm -rf {remote} && mkdir -p {remote}/tmp {remote}/home")
        binaries = {Path(test["command"][0]) for test in tests}
        binaries.add(build / "psp_recomp")
        for binary in sorted(binaries):
            run("adb", "push", str(binary), remote + "/")
        run("adb", "push", str(args.stl), remote + "/libc++_shared.so")
        run("adb", "shell", f"chmod 755 {remote}/*tests {remote}/psp_recomp")
        for test in tests:
            name = test["name"]
            command = [remote + "/" + Path(test["command"][0]).name, *test["command"][1:]]
            shell = (f"cd {remote} && export LD_LIBRARY_PATH={remote} TMPDIR={remote}/tmp "
                     f"HOME={remote}/home PSPRECOMP_CODEGEN_PATH={remote}/psp_recomp && "
                     "timeout 120 " + shlex.join(command))
            start = time.monotonic()
            try:
                result = subprocess.run(["adb", "shell", shell], capture_output=True,
                                        text=True, timeout=135)
                output = result.stdout + result.stderr
                code = result.returncode
            except subprocess.TimeoutExpired as error:
                output = f"ADB did not return within 135 seconds: {error}"
                code = 124
            elapsed = time.monotonic() - start
            (diagnostics / f"{name}.log").write_text(output)
            case = ET.SubElement(suite, "testcase", name=name, time=f"{elapsed:.3f}")
            ET.SubElement(case, "system-out").text = output
            print(f"{name}: {'PASS' if code == 0 else 'FAIL'} ({elapsed:.2f}s)", flush=True)
            if code:
                failed += 1
                ET.SubElement(case, "failure", message=f"Exit status {code}").text = output
                print(output, flush=True)
    finally:
        suite.set("failures", str(failed))
        ET.ElementTree(suite).write(diagnostics / "test-results.xml", encoding="utf-8", xml_declaration=True)
        run("adb", "shell", f"rm -rf {remote}")
    return 1 if failed else 0


if __name__ == "__main__":
    raise SystemExit(main())
