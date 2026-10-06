#pragma once

// SHA-256 of the executable profiles/mhfu/scripts/generate.sh last translated.
// Empty until that script writes profiles/mhfu/generated/corpus_identity.hpp,
// which the build includes in place of this file. Launch registers the linked
// corpus only when the prepared executable has this hash.
namespace mhfu {

inline constexpr const char *kCorpusExecutableSha256 = "";

} // namespace mhfu
