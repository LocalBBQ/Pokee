"""Validate archive names before writing below a caller-selected directory."""

from pathlib import Path


def archive_component(name):
    """Accept one portable filename, never a path supplied by an archive."""
    if (not name or name in (".", "..") or any(c in name for c in '/\\:')
            or any(ord(c) < 32 or ord(c) == 127 for c in name)
            or name.endswith((".", " "))):
        raise ValueError(f"unsafe archive filename: {name!r}")
    return name


def extraction_path(directory, components):
    """Reject traversal and existing symlinks that lead outside the output root."""
    root = Path(directory).resolve()
    destination = root.joinpath(*(archive_component(name) for name in components)).resolve()
    if not destination.is_relative_to(root) or destination == root:
        raise ValueError("archive destination escapes the output directory")
    return destination
