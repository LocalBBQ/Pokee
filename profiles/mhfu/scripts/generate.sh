#!/usr/bin/env bash
# Analyze the decrypted executable and regenerate the AOT corpus.
#
#   generate.sh [build_dir] [executable] [generated_dir]
#
# Requires a framework build (psp_analyze/psp_recomp) in build_dir, default
# out/mhfu. The executable defaults to profiles/mhfu/game/EBOOT.ELF and the
# corpus to profiles/mhfu/generated, with the analysis in profiles/mhfu/analysis.
# Another release's corpus goes in a folder of its own, its analysis inside it:
#
#   generate.sh out/mhfu <data dir>/EBOOT.ELF profiles/mhfu/generated-fuc
set -euo pipefail

profile_dir="$(cd "$(dirname "$0")/.." && pwd)"
repo_dir="$(cd "$profile_dir/../.." && pwd)"
build_dir="${1:-$repo_dir/out/mhfu}"
elf="${2:-$profile_dir/game/EBOOT.ELF}"
generated_dir="${3:-$profile_dir/generated}"
analysis_dir="$profile_dir/analysis"
if [[ $# -ge 3 ]]; then analysis_dir="$generated_dir/analysis"; fi

if [[ ! -f "$elf" ]]; then
    echo "missing $elf; run scripts/prepare_game.sh first" >&2
    exit 1
fi

cmake --build "$build_dir" --target psp_analyze psp_recomp
mkdir -p "$analysis_dir" "$generated_dir"
"$build_dir/psp_analyze" "$elf" "$analysis_dir/report.json"
# Regenerate in place: psp_recomp rewrites only units whose text changed and
# removes units that no longer exist, so unchanged units keep their timestamps
# and are not recompiled.
"$build_dir/psp_recomp" "$elf" --auto "$generated_dir"
if command -v sha256sum > /dev/null; then
    elf_sha256="$(sha256sum "$elf" | cut -d' ' -f1)"
else
    elf_sha256="$(shasum -a 256 "$elf" | cut -d' ' -f1)"
fi
cat > "$generated_dir/corpus_identity.hpp" << EOF
#pragma once

// Written by profiles/mhfu/scripts/generate.sh. Launch registers this corpus
// only for an executable with this SHA-256.
namespace mhfu {

inline constexpr const char *kCorpusExecutableSha256 = "${elf_sha256}";

} // namespace mhfu
EOF
echo "corpus identity ${elf_sha256}"
