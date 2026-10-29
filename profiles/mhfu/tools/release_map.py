#!/usr/bin/env python3
"""Write the table that moves Freedom Unite (ULUS-10391) addresses to another
release's.

    release_map.py <usa EBOOT.ELF> <other EBOOT.ELF> <usa overlays> <other overlays> <out.inc>

The host features were traced on ULUS-10391 and name its addresses. Another
release built from the same source, such as Freedom Unite Complete, has the
same functions in another order and with other addresses in them. This pairs
the two:

- Functions, cut at the targets of jal, whose words agree once call targets
  and immediates are set aside, and which occur once in each image. The
  longest run of pairs that keeps both orders is kept, so one stray match
  cannot cross the rest.
- Between two paired functions, the words themselves, aligned the same way.
- Data, from the addresses the paired code builds with lui and its low half,
  in the same order on both sides. Consecutive addresses that move by the
  same amount make one range.

The overlay directories hold overlay_<base>_<name>.bin, as databin.py
extract-overlays writes them; an overlay is paired with the one of the same
name. The output holds addresses only, never bytes of either image.
"""

import bisect
import difflib
import hashlib
import os
import re
import struct
import sys
from collections import Counter, defaultdict

# Overlays the host features read, in the order of mhfu::install::Image.
OVERLAYS = ("game_task", "lobby_task", "game_sub")

LO_OPS = {0x09, 0x20, 0x21, 0x23, 0x24, 0x25, 0x28, 0x29, 0x2B, 0x31, 0x39, 0x35, 0x3D}
MASKED = LO_OPS | {0x08, 0x0C, 0x0D, 0x0F}
RAM = (0x08800000, 0x0A000000)
HEADER_BYTES = 64
SHORT_GAP = 64


def shape(word):
    """The word without what moves between releases: jump targets and
    immediates that may hold half an address."""
    op = word >> 26
    if op in (2, 3):
        return op << 26
    if op in MASKED:
        return word & 0xFFFF0000
    return word


class Image:
    def __init__(self, name, base, data, code_end=None):
        self.name = name
        self.base = base
        self.words = list(struct.unpack_from("<%dI" % (len(data) // 4), data, 0))
        self.end = base + 4 * len(self.words)
        self.code_end = code_end or self.end

    def span(self, start, end):
        return self.words[(start - self.base) // 4:(end - self.base) // 4]

    def functions(self):
        starts = {self.base}
        for word in self.span(self.base, self.code_end):
            if word >> 26 == 3:
                target = (word & 0x03FFFFFF) << 2 | 0x08000000
                if self.base <= target < self.code_end:
                    starts.add(target)
        starts = sorted(starts)
        out = []
        for i, start in enumerate(starts):
            end = starts[i + 1] if i + 1 < len(starts) else self.code_end
            body = [shape(w) for w in self.span(start, end)]
            out.append((start, end, hashlib.sha1(struct.pack("<%dI" % len(body), *body)).digest()))
        return out


def elf_image(path):
    with open(path, "rb") as handle:
        data = handle.read()
    (shoff,) = struct.unpack_from("<I", data, 0x20)
    entsize, count, names_index = struct.unpack_from("<HHH", data, 0x2E)
    headers = [struct.unpack_from("<10I", data, shoff + i * entsize) for i in range(count)]
    names = headers[names_index]
    sections = {}
    for header in headers:
        at = names[4] + header[0]
        sections[data[at:data.index(b"\0", at)].decode()] = header
    text = sections[".text"]
    loaded = [h for h in headers if h[1] == 1 and h[3] >= text[3]]
    end = max(h[3] + h[5] for h in loaded)
    blob = bytearray(end - text[3])
    for h in loaded:
        blob[h[3] - text[3]:h[3] - text[3] + h[5]] = data[h[4]:h[4] + h[5]]
    return Image("executable", text[3], bytes(blob), text[3] + text[5])


def overlay_images(directory):
    out = {}
    for name in os.listdir(directory):
        match = re.match(r"overlay_([0-9A-Fa-f]{8})_(.+)\.bin$", name)
        if match:
            with open(os.path.join(directory, name), "rb") as handle:
                out[match.group(2)] = Image(match.group(2), int(match.group(1), 16), handle.read())
    return out


def longest_chain(pairs):
    """The longest run of (usa, other) pairs increasing on both sides."""
    pairs.sort()
    tails, at, previous = [], [], [None] * len(pairs)
    for i, (_, _, other) in enumerate(pairs):
        j = bisect.bisect_left(tails, other)
        if j == len(tails):
            tails.append(other)
            at.append(i)
        else:
            tails[j], at[j] = other, i
        previous[i] = at[j - 1] if j else None
    chain, i = [], at[-1] if at else None
    while i is not None:
        chain.append(pairs[i])
        i = previous[i]
    return chain[::-1]


def address_refs(words):
    """The addresses lui and its low half build, in order; None for a call,
    so the two sides stay in step only while their calls do."""
    out, high = [], {}
    for word in words:
        op, rs, rt, imm = word >> 26, (word >> 21) & 31, (word >> 16) & 31, word & 0xFFFF
        if op == 0x0F:
            high[rt] = imm << 16
        elif op in LO_OPS and rs in high:
            out.append((high[rs] + (imm - 0x10000 if imm & 0x8000 else imm)) & 0xFFFFFFFF)
            if op == 0x09 and rt == rs:
                del high[rs]
        elif op == 0x0D and rs in high:
            out.append(high[rs] | imm)
        elif op in (2, 3):
            out.append(None)
    return out


def pair_images(usa, other):
    """Code ranges (usa start, usa end, other start) and paired data addresses."""
    ours, theirs = usa.functions(), other.functions()
    our_count, their_count = Counter(k for *_, k in ours), Counter(k for *_, k in theirs)
    unique = {k: start for start, _, k in theirs if their_count[k] == 1}
    chain = longest_chain([(s, e, unique[k]) for s, e, k in ours if our_count[k] == 1 and k in unique])

    code = list(chain)
    # The gaps between paired functions, and before the first and after the
    # last of them.
    anchors = [(usa.base, usa.base, other.base)] + chain + [(usa.code_end, usa.code_end, other.code_end)]
    for (start, end, at), (next_start, _, next_at) in zip(anchors, anchors[1:]):
        gap_start, gap_end = end, next_start
        their_start, their_end = at + (end - start), next_at
        if gap_end <= gap_start or their_end <= their_start:
            continue
        a = [shape(w) for w in usa.span(gap_start, gap_end)]
        b = [shape(w) for w in other.span(their_start, their_end)]
        for block in difflib.SequenceMatcher(None, a, b, autojunk=False).get_matching_blocks():
            if block.size >= 4:
                code.append((gap_start + 4 * block.a, gap_start + 4 * (block.a + block.size), their_start + 4 * block.b))

    votes = defaultdict(Counter)
    for start, end, at in code:
        ours_refs = address_refs(usa.span(start, end))
        their_refs = address_refs(other.span(at, at + (end - start)))
        if len(ours_refs) != len(their_refs):
            continue
        for x, y in zip(ours_refs, their_refs):
            if x is not None and y is not None and RAM[0] <= x < RAM[1] and RAM[0] <= y < RAM[1]:
                votes[x][y] += 1
    data = {}
    for x, counted in votes.items():
        y, n = counted.most_common(1)[0]
        if 2 * n > sum(counted.values()):
            data[x] = y
    return sorted(code), data


def merge(ranges):
    """Joins ranges that touch, or lie a short gap apart, and move by the same amount."""
    out = []
    for start, end, at in sorted(ranges):
        if out and start < out[-1][1]:
            if end <= out[-1][1]:
                continue
            at += out[-1][1] - start
            start = out[-1][1]
        # A short gap between two ranges that move by the same amount moves
        # with them: a function of fewer words than a matched block.
        touching = out and start - out[-1][1] <= SHORT_GAP
        if touching and out[-1][2] - out[-1][0] == at - start:
            out[-1] = (out[-1][0], end, out[-1][2])
        else:
            out.append((start, end, at))
    return out


def data_ranges(data, code):
    """Runs of paired data addresses outside the paired code that move by the
    same amount, each from its first address to one past its last."""
    starts = [start for start, _, _ in code]
    out = []
    for x in sorted(data):
        i = bisect.bisect_right(starts, x) - 1
        if i >= 0 and x < code[i][1]:
            continue
        y = data[x]
        if out and out[-1][2] - out[-1][0] == y - x:
            out[-1] = (out[-1][0], x + 1, out[-1][2])
        else:
            out.append((x, x + 1, y))
    return out


def main(argv):
    if len(argv) != 6:
        sys.stderr.write(__doc__)
        return 2
    usa, other = elf_image(argv[1]), elf_image(argv[2])
    usa_overlays, other_overlays = overlay_images(argv[3]), overlay_images(argv[4])
    usa_slots = sorted({image.base for image in usa_overlays.values()})
    other_slots = sorted({image.base for image in other_overlays.values()})
    if len(usa_slots) != len(other_slots):
        sys.stderr.write("the releases have %d and %d overlay slots\n" % (len(usa_slots), len(other_slots)))
        return 1
    images = [("executable", usa, other)]
    for name in OVERLAYS:
        if name not in usa_overlays or name not in other_overlays:
            sys.stderr.write("missing overlay %s\n" % name)
            return 1
        images.append((name, usa_overlays[name], other_overlays[name]))

    lines = []
    for name, ours, theirs in images:
        code, data = pair_images(ours, theirs)
        if name != "executable":
            # The header, so the slot itself moves to the other release's.
            code.append((ours.base, ours.base + HEADER_BYTES, theirs.base))
        else:
            # Each slot's header, for code that reads a slot whatever is in it.
            code.extend(zip(usa_slots, [b + HEADER_BYTES for b in usa_slots], other_slots))
        code = merge(code)
        data = data_ranges(data, code)
        covered = sum(e - s for s, e, _ in code)
        sys.stderr.write("%s: %d code ranges cover %.1f%% of its code, %d data ranges\n" % (
            name, len(code), 100.0 * covered / (ours.code_end - ours.base), len(data)))
        # One list per image without overlaps: the lookup finds the range
        # that starts last at or below an address.
        for start, end, at in merge(code + data):
            lines.append("{0x%08Xu, 0x%08Xu, 0x%08Xu, Image::%s}," % (start, end, at, name))
    with open(argv[5], "w", encoding="utf-8") as handle:
        handle.write("// Written by profiles/mhfu/tools/release_map.py; addresses only.\n")
        handle.write("// {ULUS-10391 start, end, the other release's start, image}\n")
        handle.write("\n".join(lines) + "\n")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
