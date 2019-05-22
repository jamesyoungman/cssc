#! /bin/sh

find . \
     \( -path ./gl -o -path ./gnulib -o -path ./unit-tests \) -prune -o \
     \( -type f \( -name '*.c' -o -name '*.cc' \) -print \) |
    while read file
    do
        # Extract the first include directive and complain if it is not
        # config.h.
        awk  '{
                 if (match($0, "^ *# *include +[\"<](.*)[\">]", f)) {
                                 if (f[1] != "config.h") {
                                    printf("%s:%s\n", FILENAME, f[1]);
                                 }
                                 exit(0);
                 }
             }' "$file"
    done
