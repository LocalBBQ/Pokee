#!/usr/bin/env bash
# Packs a research APK of Yakumo for arm64-v8a without Gradle, from a build
# configured with the NDK toolchain and -DMHP3RD_ANDROID_APP=ON. It carries
# no game data: the player's disc image is installed on the device.
#
#   build_apk.sh <build dir> <SDL3 source dir> <libSDL3.so> <output.apk> [overlay limit]
#
# ANDROID_HOME must name the SDK (build-tools and a platform are needed).
# The overlay limit packs only the first N overlay libraries, to keep a test
# APK small; without it every one is packed.
set -euo pipefail

build_dir="$(cd "$1" && pwd)"
sdl_dir="$(cd "$2" && pwd)"
sdl_lib="$3"
output="$4"
overlay_limit="${5:-}"
here="$(cd "$(dirname "$0")" && pwd)"

sdk="${ANDROID_HOME:?set ANDROID_HOME to the Android SDK}"
build_tools="$(ls -d "$sdk"/build-tools/* | sort -V | tail -1)"
platform="$(ls -d "$sdk"/platforms/android-* | sort -V | tail -1)"
android_jar="$platform/android.jar"
work="$build_dir/apk"
rm -rf "$work"
mkdir -p "$work/classes" "$work/dex" "$work/lib/arm64-v8a"

echo "compiling SDL's Java activity"
javac -nowarn --release 11 -classpath "$android_jar" -d "$work/classes" \
    $(find "$sdl_dir/android-project/app/src/main/java" -name '*.java') 2> "$work/javac.log"
"$build_tools/d8" --release --min-api 29 --lib "$android_jar" --output "$work/dex" \
    $(find "$work/classes" -name '*.class')

echo "linking resources"
"$build_tools/aapt2" compile --dir "$here/res" -o "$work/res.zip"
"$build_tools/aapt2" link -I "$android_jar" --manifest "$here/AndroidManifest.xml" \
    --min-sdk-version 29 --target-sdk-version 35 -o "$work/unsigned.apk" "$work/res.zip"

echo "adding native libraries"
lib="$work/lib/arm64-v8a"
cp "$sdl_lib" "$lib/libSDL3.so"
cp "$build_dir/bin/libmain.so" "$lib/"
cp "$build_dir"/bin/lib/libavcodec.so "$build_dir"/bin/lib/libavutil.so "$lib/"
overlays=("$build_dir"/bin/overlays/libovl*.so)
if [[ -n "$overlay_limit" ]]; then overlays=("${overlays[@]:0:$overlay_limit}"); fi
cp "${overlays[@]}" "$lib/"
strip="$(ls -d "$ANDROID_NDK"/toolchains/llvm/prebuilt/*/bin/llvm-strip | head -1)"
"$strip" --strip-unneeded "$lib"/*.so
# extractNativeLibs is on, so the libraries may be compressed: the package
# manager unpacks them at install time.
(cd "$work" && cp dex/classes.dex . && zip -q unsigned.apk classes.dex && zip -q -r -9 unsigned.apk lib)

echo "aligning and signing"
keystore="$build_dir/apk-research.keystore"
if [[ ! -f "$keystore" ]]; then
    keytool -genkeypair -keystore "$keystore" -storepass research -keypass research -alias research \
        -keyalg RSA -keysize 2048 -validity 3650 -dname "CN=Yakumo research build" > /dev/null 2>&1
fi
"$build_tools/zipalign" -f -P 16 4 "$work/unsigned.apk" "$work/aligned.apk"
"$build_tools/apksigner" sign --ks "$keystore" --ks-pass pass:research --key-pass pass:research \
    --out "$output" "$work/aligned.apk"
echo "wrote $output ($(du -h "$output" | cut -f1), ${#overlays[@]} overlay libraries)"
