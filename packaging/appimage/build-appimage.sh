#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'EOF'
Usage: build-appimage.sh [--appimagetool PATH] [--runtime PATH]

Builds build/appimage/output/Ymir_Helga-x86_64.AppImage.
The script never downloads or installs tools. Provide the official appimagetool
and Type-2 runtime with the options above or with APPIMAGETOOL and
APPIMAGE_RUNTIME.
EOF
}

fail() {
  printf 'error: %s\n' "$1" >&2
  exit 1
}

command_exists() {
  command -v "$1" >/dev/null 2>&1 || fail "required command not found: $1"
}

read_direct_dependencies() {
  readelf -d "$1" | awk '
    /\(NEEDED\)/ {
      dependency = $0
      sub(/^.*\[/, "", dependency)
      sub(/\].*$/, "", dependency)
      print dependency
    }
  '
}

is_excluded_library() {
  local library_name=$1
  local pattern

  while IFS= read -r pattern || [[ -n $pattern ]]; do
    [[ -z $pattern || $pattern == \#* ]] && continue
    [[ $library_name == $pattern ]] && return 0
  done <"$excluded_libraries_file"

  return 1
}

resolve_dependency() {
  local object_path=$1
  local dependency_name=$2
  local search_directory=$3

  env LD_LIBRARY_PATH="$search_directory" ldd "$object_path" | \
    awk -v dependency="$dependency_name" \
      '$1 == dependency && $2 == "=>" && $3 != "not" { print $3; exit }'
}

bundle_runtime_dependencies() {
  local executable_path=$1
  local destination_directory=$2
  local object_path dependency_name source_path destination_path
  local queue_index=0
  local -a object_queue=("$executable_path")
  local -A bundled_libraries=()

  while ((queue_index < ${#object_queue[@]})); do
    object_path=${object_queue[$queue_index]}
    ((queue_index += 1))

    while IFS= read -r dependency_name; do
      [[ -n $dependency_name ]] || continue
      is_excluded_library "$dependency_name" && continue
      [[ -z ${bundled_libraries[$dependency_name]+present} ]] || continue

      source_path=$(resolve_dependency "$object_path" "$dependency_name" \
        "$destination_directory")
      [[ -n $source_path && -f $source_path ]] || fail \
        "could not resolve ${dependency_name}, required by ${object_path}"

      destination_path="${destination_directory}/${dependency_name}"
      cp -L -- "$source_path" "$destination_path"
      bundled_libraries[$dependency_name]=1
      object_queue+=("$destination_path")
      printf 'Bundled runtime library: %s\n' "$dependency_name"
    done < <(read_direct_dependencies "$object_path")
  done
}

verify_runtime_dependencies() {
  local executable_path=$1
  local bundled_directory=$2
  local context=$3
  local object_path dependency_name resolved_path dependency_report
  local -a objects=("$executable_path")

  while IFS= read -r -d '' object_path; do
    objects+=("$object_path")
  done < <(find "$bundled_directory" -maxdepth 1 -type f -print0)

  for object_path in "${objects[@]}"; do
    dependency_report=$(env LD_LIBRARY_PATH="$bundled_directory" \
      ldd "$object_path")
    if grep -q 'not found' <<<"$dependency_report"; then
      printf '%s\n' "$dependency_report" >&2
      fail "${context} has unresolved dependencies for ${object_path}"
    fi

    while IFS= read -r dependency_name; do
      [[ -n $dependency_name ]] || continue
      is_excluded_library "$dependency_name" && continue

      resolved_path=$(resolve_dependency "$object_path" "$dependency_name" \
        "$bundled_directory")
      [[ $resolved_path == "${bundled_directory}/"* ]] || {
        printf '%s\n' "$dependency_report" >&2
        fail "${context} resolves ${dependency_name} outside the bundle"
      }
    done < <(read_direct_dependencies "$object_path")
  done
}

caller_directory=$(pwd -P)
appimagetool_path=${APPIMAGETOOL:-}
runtime_path=${APPIMAGE_RUNTIME:-}

while (($# > 0)); do
  case "$1" in
    --appimagetool)
      (($# >= 2)) || fail "--appimagetool requires a path"
      appimagetool_path=$2
      shift 2
      ;;
    --runtime)
      (($# >= 2)) || fail "--runtime requires a path"
      runtime_path=$2
      shift 2
      ;;
    -h | --help)
      usage
      exit 0
      ;;
    *)
      usage >&2
      fail "unknown argument: $1"
      ;;
  esac
done

[[ $(uname -s) == Linux ]] || fail "AppImage builds are supported only on Linux"
[[ $(uname -m) == x86_64 ]] || fail "AppImage builds currently support only x86_64"

[[ -n $appimagetool_path ]] || fail \
  "provide appimagetool 1.9.1 with APPIMAGETOOL or --appimagetool"
[[ -n $runtime_path ]] || fail \
  "provide the Type-2 runtime with APPIMAGE_RUNTIME or --runtime"

if [[ $appimagetool_path != /* ]]; then
  appimagetool_path="${caller_directory}/${appimagetool_path}"
fi
if [[ $runtime_path != /* ]]; then
  runtime_path="${caller_directory}/${runtime_path}"
fi

[[ -x $appimagetool_path ]] || fail \
  "appimagetool is missing or not executable: $appimagetool_path"
[[ -r $runtime_path ]] || fail "AppImage runtime is not readable: $runtime_path"

for required_command in cmake file ldd readelf awk sha256sum; do
  command_exists "$required_command"
done

script_directory=$(
  CDPATH= cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd
)
excluded_libraries_file="${script_directory}/excluded-libraries.txt"
[[ -r $excluded_libraries_file ]] || fail \
  "library exclusion policy is missing: $excluded_libraries_file"
repository_root=$(CDPATH= cd -- "${script_directory}/../.." && pwd)
build_root="${repository_root}/build/appimage"
cmake_build_directory="${build_root}/cmake-build"
app_dir="${build_root}/AppDir"
output_directory="${build_root}/output"
extraction_directory="${build_root}/extracted"
artifact_name=Ymir_Helga-x86_64.AppImage
artifact_path="${output_directory}/${artifact_name}"

rm -rf -- "$cmake_build_directory" "$app_dir" "$output_directory" \
  "$extraction_directory"
mkdir -p -- "$cmake_build_directory" "$app_dir" "$output_directory"

cmake -S "$repository_root" -B "$cmake_build_directory" \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_TESTING=OFF \
  -DCMAKE_INSTALL_PREFIX=/usr
cmake --build "$cmake_build_directory" --parallel
DESTDIR="$app_dir" cmake --install "$cmake_build_directory"

required_paths=(
  "usr/bin/ymir-helga"
  "usr/share/ymir-helga/assets/Menu.png"
  "usr/share/ymir-helga/assets/SuperPixel-m2L8j.ttf"
  "usr/share/ymir-helga/assets/Mapas/planicie.txt"
  "usr/share/applications/io.github.ediasv.YmirHelga.desktop"
  "usr/share/icons/hicolor/48x48/apps/io.github.ediasv.YmirHelga.png"
  "usr/share/metainfo/io.github.ediasv.YmirHelga.metainfo.xml"
)

for required_path in "${required_paths[@]}"; do
  [[ -e "${app_dir}/${required_path}" ]] || fail \
    "staged AppDir path is missing: ${required_path}"
done

game_executable="${app_dir}/usr/bin/ymir-helga"
library_directory="${app_dir}/usr/lib"
mkdir -p -- "$library_directory"

dependency_report=$(ldd "$game_executable")
if grep -q 'not found' <<<"$dependency_report"; then
  printf '%s\n' "$dependency_report" >&2
  fail "the staged executable has unresolved shared-library dependencies"
fi

bundle_runtime_dependencies "$game_executable" "$library_directory"
verify_runtime_dependencies "$game_executable" "$library_directory" \
  "the staged AppDir"

for component in graphics window system; do
  bundled_library=$(find "$library_directory" \
    -name "libsfml-${component}.so*" -print -quit)
  [[ -n $bundled_library ]] || fail \
    "the AppDir does not contain libsfml-${component}"
done

cp -- "${script_directory}/AppRun" "${app_dir}/AppRun"
chmod +x "${app_dir}/AppRun"
cp -- "${app_dir}/usr/share/applications/io.github.ediasv.YmirHelga.desktop" \
  "${app_dir}/io.github.ediasv.YmirHelga.desktop"
cp -- "${app_dir}/usr/share/icons/hicolor/48x48/apps/io.github.ediasv.YmirHelga.png" \
  "${app_dir}/io.github.ediasv.YmirHelga.png"
ln -s "io.github.ediasv.YmirHelga.png" "${app_dir}/.DirIcon"

ARCH=x86_64 APPIMAGE_EXTRACT_AND_RUN=1 "$appimagetool_path" \
  --runtime-file "$runtime_path" "$app_dir" "$artifact_path"
[[ -x $artifact_path ]] || fail "appimagetool did not create $artifact_path"

mkdir -p -- "$extraction_directory"
(
  cd "$extraction_directory"
  "$artifact_path" --appimage-extract >/dev/null
)
extracted_root="${extraction_directory}/squashfs-root"
[[ -x "${extracted_root}/usr/bin/ymir-helga" ]] || fail \
  "the packaged executable is missing after AppImage extraction"

for component in graphics window system; do
  bundled_library=$(find "${extracted_root}/usr/lib" \
    -name "libsfml-${component}.so*" -print -quit)
  [[ -n $bundled_library ]] || fail \
    "the AppImage does not contain libsfml-${component}"
done

verify_runtime_dependencies \
  "${extracted_root}/usr/bin/ymir-helga" \
  "${extracted_root}/usr/lib" \
  "the extracted AppImage"

(
  cd "$output_directory"
  sha256sum "$artifact_name" >"${artifact_name}.sha256"
)

printf 'AppImage: %s\n' "$artifact_path"
printf 'SHA-256: '
awk '{ print $1 }' "${artifact_path}.sha256"
