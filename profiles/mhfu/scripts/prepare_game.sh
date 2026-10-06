#!/usr/bin/env bash
# Populate profiles/mhfu/game from the user's own disc image.
#
#   prepare_game.sh <image.iso> <decrypted EBOOT.ELF>
#
# Produce EBOOT.ELF from PSP_GAME/SYSDIR/EBOOT.BIN with an external tool, or
# take the one `Pokee --install <image.iso>` writes into the per-user
# data directory, before running this script.
set -euo pipefail

if [[ $# -ne 2 ]]; then
    echo "usage: $0 <image.iso> <decrypted EBOOT.ELF>" >&2
    exit 2
fi

profile_dir="$(cd "$(dirname "$0")/.." && pwd)"
game_dir="$profile_dir/game"
iso="$1"
elf="$2"

unite_sha256="3070954e2dc88fd0b0836c0c1dc526531aca4a213b0f18806e324963efb514f4"
if command -v sha256sum > /dev/null; then
    actual_sha256="$(sha256sum "$elf" | cut -d' ' -f1)"
else
    actual_sha256="$(shasum -a 256 "$elf" | cut -d' ' -f1)"
fi
if [[ "$actual_sha256" != "$unite_sha256" ]]; then
    echo "warning: $elf sha256 $actual_sha256 is not USA Freedom Unite" >&2
fi

# The image is read in place: the host serves disc0: (including raw sce_lbn
# sector reads) straight from it, so nothing is unpacked.
mkdir -p "$game_dir/ms0"
ln -sf "$(cd "$(dirname "$iso")" && pwd)/$(basename "$iso")" "$game_dir/disc.iso"
cp "$elf" "$game_dir/EBOOT.ELF"
echo "game data prepared in $game_dir"
