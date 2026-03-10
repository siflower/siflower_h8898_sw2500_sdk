#!/bin/bash
# Define the paths that need to be signed or checked
declare -a PACKAGE_DIRS=(
    "bin/packages/riscv64_c908/base"
    "bin/packages/riscv64_c908/luci"
    "bin/packages/riscv64_c908/packages"
    "bin/packages/riscv64_c908/routing"
    "bin/packages/riscv64_c908/telephony"
    "bin/packages/riscv64_c908/xluci2"
    "bin/targets/siflower/sf21h8898/packages"
)
USIGN_PATH="build_dir/hostpkg/usign-2020-05-23-f1f65026/usign"
PRIVATE_KEY="keyfile"
PUBLIC_KEY="keyfile.pub"
# Function to update signatures for all specified package directories
update_sig() {
    echo "Updating signatures..."
    for dir in "${PACKAGE_DIRS[@]}"; do
        if [ -f "$dir/Packages" ]; then
            echo "Signing $dir/Packages..."
            "$USIGN_PATH" -S -m "$dir/Packages" -s "$PRIVATE_KEY" -x "$dir/Packages.sig"
            if [ $? -eq 0 ]; then
                echo "Successfully signed $dir/Packages"
            else
                echo "Failed to sign $dir/Packages"
            fi
        else
            echo "No Packages file found in $dir"
        fi
    done
}
# Function to check signatures for all specified package directories
check_sig() {
    echo "Checking signatures..."
    for dir in "${PACKAGE_DIRS[@]}"; do
        if [ -f "$dir/Packages" ] && [ -f "$dir/Packages.sig" ]; then
            echo "Verifying signature of $dir/Packages..."
            "$USIGN_PATH" -V -m "$dir/Packages" -x "$dir/Packages.sig" -p "$PUBLIC_KEY"
            if [ $? -eq 0 ]; then
                echo "Signature verification passed for $dir/Packages"
            else
                echo "Signature verification failed for $dir/Packages"
            fi
        else
            echo "No Packages or Packages.sig file found in $dir"
        fi
    done
}
# Check if a command was provided and execute the corresponding function
case "$1" in
    update)
        update_sig
        ;;
    check)
        check_sig
        ;;
    *)
        echo "Usage: $0 {update|check}"
        exit 1
        ;;
esac
