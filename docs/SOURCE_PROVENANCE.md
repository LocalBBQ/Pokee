# Source provenance and license boundaries

PSPRecomp separates independently written framework/profile code from third-party components with their own licenses.

## Framework

The reusable PSPRecomp runtime, decoder, analyzer and code generator in the repository root are distributed under the MIT License. Game-specific addresses and implementations are not accepted in the reusable core.

The project may use public hardware documentation, observable program behavior and other implementations as technical references. Reference material is used to understand behavior and architecture; source code from incompatible copyleft projects is not imported into the MIT framework.

## Decryption

The framework contains no EBOOT/PRX decryption. The mhp3rd profile's installer (`profiles/mhp3rd/host/install`) prepares the game's executable from the player's own disc image: it accepts exactly one encrypted file, identified by its SHA-256, and checks its output against the SHA-256 of the executable the profile was generated from. It was written for this project from public descriptions of the file format and the crypto primitives; no code from other implementations was copied or adapted. Its AES implementation is tiny-AES-c (public domain), kept with its notice in `profiles/mhp3rd/third_party/tiny_aes`. Developers can still prepare the executable outside the project and supply it through `profiles/mhp3rd/game`.

## Profile code

A profile owns its generated AOT corpus, address-specific lowering, HLE behavior and native fast paths. Those files remain isolated under `profiles/<id>` so they do not become hidden dependencies of the generic framework.

## Third-party components

Third-party source, binary dependencies, shader code and notices stay beside the profile that needs them. Their original copyright and license notices must be preserved.

## Contribution rule

Do not paste or adapt source from a project whose license is incompatible with the destination file. Reimplement required behavior from specifications, observations or independently documented semantics, and record the source of third-party material when it is intentionally included under a compatible license.
