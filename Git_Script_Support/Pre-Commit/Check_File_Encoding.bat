#!/bin/bash

# List of files with incorrect encoding
invalid_files=()

# Check files that are staged for commit
files=$(git diff --cached --name-only --diff-filter=ACM | grep -E '\.c$|\.h$')

# Exit if there are no .c or .h files
if [ -z "$files" ]; then
    exit 0
fi

echo "Checking file encoding for EUC-KR compliance..."

for file in $files; do
    # Check the encoding of the file
    encoding=$(file -bi "$file" | grep -o 'charset=[^ ]*' | cut -d= -f2)
    
    # If the encoding is neither EUC-KR nor unknown, add it to the invalid_files list
    if [[ "$encoding" != "euc-kr" && "$encoding" != "unknown-8bit" ]]; then
        invalid_files+=("$(realpath "$file")")
    fi
done

# If there are files with incorrect encoding, print the list and reject the commit
if [ ${#invalid_files[@]} -gt 0 ]; then
    echo -e "\033[31mError: The following files are not encoded in\033[0m \033[93m--- EUC-KR ---\033[0m"
    echo -e "\033[92m	------------------------------------------------------------------------\033[0m"
    for invalid_file in "${invalid_files[@]}"; do
        echo -e "	 > \033[93m$invalid_file\033[0m"
    done
    echo -e "\033[92m	------------------------------------------------------------------------\033[0m"
    echo -e "\033[31mPlease convert these files to EUC-KR encoding before committing.\033[0m"
    exit 1
fi

echo "All files are properly encoded in EUC-KR."
exit 0
