# Source provenance and license boundaries

PSPRecomp separates independently written framework/profile code from third-party components with their own licenses.

## Framework

The reusable PSPRecomp runtime, decoder, analyzer and code generator in the repository root are distributed under the MIT License. Game-specific addresses and implementations are not accepted in the reusable core.

The project may use public hardware documentation, observable program behavior and other implementations as technical references. Reference material is used to understand behavior and architecture; source code from incompatible copyleft projects is not imported into the MIT framework.

## Decryption

The framework contains no EBOOT/PRX decryption. The mhp3rd profile's installer (`profiles/mhp3rd/host/install`) prepares the game's executable from the player's own disc image: it accepts exactly one encrypted file, identified by its SHA-256, and checks its output against the SHA-256 of the executable the profile was generated from. It was written for this project from public descriptions of the file format and the crypto primitives; no code from other implementations was copied or adapted. Its AES implementation is tiny-AES-c (public domain), kept with its notice in `profiles/mhp3rd/third_party/tiny_aes`. Developers can still prepare the executable outside the project and supply it through `profiles/mhp3rd/game`.

## Save data

`profiles/mhp3rd/host/save_data` implements the PSP save-data format — the `PARAM.SFO` layout, the encryption of the data file and the hashes that protect a save — so that saves can be exchanged with a PSP. It was written for this project from public descriptions of the PSP save data format, with no code copied or adapted from other implementations, and checked against saves made by a PSP. Its AES-128 cipher is the same tiny-AES-c the installer uses; the self-tests check it against FIPS-197 and the CMAC built on it against RFC 4493. The fixed key values it uses are published technical constants. None of it decrypts executables.

## Profile code

A profile owns its generated AOT corpus, address-specific lowering, HLE behavior and native fast paths. Those files remain isolated under `profiles/<id>` so they do not become hidden dependencies of the generic framework.

## Third-party components

Third-party source, binary dependencies, shader code and notices stay beside the profile that needs them. Their original copyright and license notices must be preserved.

| Component | Used by | License | How it is included |
| --- | --- | --- | --- |
| [FFmpeg](https://ffmpeg.org/) (`libavcodec`, `libavutil`) | mhp3rd profile: ATRAC3 music and H.264/ATRAC3plus movie decoding (`profiles/mhp3rd/host/audio/atrac_decoder.cpp`, `profiles/mhp3rd/host/movie/avc_decoder.cpp`) | LGPL-2.1-or-later | Optional external dependency, found at configure time through `pkg-config` and linked dynamically; no FFmpeg source is in the repository |

## Contribution rule

Do not paste or adapt source from a project whose license is incompatible with the destination file. Reimplement required behavior from specifications, observations or independently documented semantics, and record the source of third-party material when it is intentionally included under a compatible license.
