#!/bin/bash

# Định nghĩa các tùy chọn AStyle
ASTYLE_OPTIONS="--style=kr --indent=spaces=4 --pad-oper --add-braces --suffix=none"

# Lấy danh sách các file mã nguồn C/C++ được thêm vào commit
files=$(git diff --cached --name-only --diff-filter=ACM | grep -E '\.(c|cpp|h|hpp)$')

# Kiểm tra nếu có file nào cần định dạng
if [ -n "$files" ]; then
    echo "Running AStyle on staged files..."

    # Chạy AStyle cho từng file
    for file in $files; do
        # Định dạng file
        astyle $ASTYLE_OPTIONS "$file"

        # Thêm lại file đã định dạng vào commit
        git add "$file"
    done

    echo "AStyle formatting applied."
fi

exit 0
