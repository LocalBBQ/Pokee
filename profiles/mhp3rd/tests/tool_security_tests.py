"""Synthetic archive and command-argument regressions; no game data required."""

import io
from pathlib import Path
import struct
import sys
import tempfile
import unittest
from unittest.mock import patch

TOOLS = Path(__file__).resolve().parents[1] / "tools"
sys.path.insert(0, str(TOOLS))
import add_overlay
import databin
import extract_iso
from extraction_paths import extraction_path


def record(name, lba=21, size=4, directory=False):
    result = bytearray(33 + len(name) + (len(name) % 2 == 0))
    result[0] = len(result)
    struct.pack_into("<I", result, 2, lba)
    struct.pack_into("<I", result, 10, size)
    result[25] = 2 if directory else 0
    result[32] = len(name)
    result[33:33 + len(name)] = name
    return bytes(result)


def image_with_entry(entry):
    result = bytearray(22 * extract_iso.SECTOR)
    start = 16 * extract_iso.SECTOR
    result[start + 1:start + 6] = b"CD001"
    root = record(b"\0", lba=20, size=extract_iso.SECTOR, directory=True)
    result[start + 156:start + 190] = root
    result[20 * extract_iso.SECTOR:20 * extract_iso.SECTOR + len(entry)] = entry
    result[21 * extract_iso.SECTOR:21 * extract_iso.SECTOR + 4] = b"test"
    return result


class ArchiveSecurityTests(unittest.TestCase):
    def test_iso_extracts_valid_name(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            iso = root / "synthetic.iso"
            iso.write_bytes(image_with_entry(record(b"SAFE.BIN;1")))
            self.assertEqual(extract_iso.main(["extract_iso", str(iso), str(root / "output")]), 0)
            self.assertEqual((root / "output" / "SAFE.BIN").read_bytes(), b"test")

    def test_iso_rejects_archive_path_components(self):
        for name in (b"../escape", b"..", b"/absolute", b"dir\\escape", b"C:escape", b"bad\x00name", b"CON", b"LPT1.BIN"):
            with self.subTest(name=name), tempfile.TemporaryDirectory() as temporary:
                iso = Path(temporary) / "synthetic.iso"
                iso.write_bytes(image_with_entry(record(name)))
                with self.assertRaises(ValueError):
                    extract_iso.main(["extract_iso", str(iso), str(Path(temporary) / "output")])

    def test_iso_rejects_truncated_record(self):
        malformed = bytearray(record(b"SAFE"))
        malformed[32] = 100
        with self.assertRaises(ValueError):
            extract_iso.walk(io.BytesIO(image_with_entry(malformed)), record(b"\0", 20, 2048, True), "", lambda *args: None)

    def test_iso_rejects_directory_cycle(self):
        entry = record(b"LOOP", 20, 2048, True)
        with self.assertRaises(ValueError):
            extract_iso.walk(io.BytesIO(image_with_entry(entry)), record(b"\0", 20, 2048, True), "", lambda *args: None)

    def test_existing_symlinks_cannot_escape_output(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            output = root / "output"
            output.mkdir()
            outside = root / "outside"
            outside.mkdir()
            try:
                (output / "LINK").symlink_to(outside, target_is_directory=True)
                (output / "FILE").symlink_to(outside / "file")
            except OSError as error:
                self.skipTest(f"symlinks unavailable: {error}")
            for components in (["LINK", "escape"], ["FILE"]):
                with self.subTest(components=components), self.assertRaises(ValueError):
                    extraction_path(output, components)
            self.assertFalse((outside / "file").exists())

    def test_databin_rejects_overlay_filename_traversal(self):
        class FakeArchive:
            def read(self, index):
                return b"test"
            def overlay(self, index):
                return {"name": "dir/../../escape.bin"}
        with tempfile.TemporaryDirectory() as temporary, self.assertRaises(ValueError):
            databin.write_entry(FakeArchive(), 0, temporary)

    def test_databin_valid_overlay_filename(self):
        class FakeArchive:
            def read(self, index):
                return b"test"
            def overlay(self, index):
                return {"name": "demo_task.bin"}
        with tempfile.TemporaryDirectory() as temporary:
            path, size = databin.write_entry(FakeArchive(), 7, temporary)
            self.assertEqual(path.name, "00007_demo_task.bin")
            self.assertEqual(path.read_bytes(), b"test")
            self.assertEqual(size, 4)

    def test_overlay_command_paths_cannot_be_interpreted_as_options(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            build = root / "-build"
            build.mkdir()
            (build / ("psp_recomp.exe" if sys.platform == "win32" else "psp_recomp")).touch()
            dump = root / "-dump.bin"
            header = bytearray(64)
            header[:4] = b"MWo3"
            struct.pack_into("<4I", header, 4, 0, 0x08800000, 0, 0)
            header[32:36] = b"demo"
            dump.write_bytes(header)
            with patch.object(add_overlay, "OVERLAY_DIR", str(root / "overlays")), patch.object(add_overlay.subprocess, "run") as run:
                add_overlay.main(["add_overlay", "--no-build", str(build), str(dump), "0x08800000"])
                commands = [call.args[0] for call in run.call_args_list]
                self.assertTrue(Path(commands[0][2]).is_absolute())
                self.assertIn(commands[1][0], ("./psp_recomp", "psp_recomp.exe"))
                self.assertEqual(run.call_args_list[1].kwargs["cwd"], str(build))
                self.assertEqual(commands[1][4], "0x08800000")


if __name__ == "__main__":
    unittest.main()
