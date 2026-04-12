#!/bin/sh
LIBDIR="$1"
INCDIR="$2"
SPECSFILE="$3"

cat > "$SPECSFILE" << SPECSEOF
%rename cpp_options old_cpp_options

*cpp_options:
-nostdlib -ffreestanding -fno-builtin -I $INCDIR %(old_cpp_options)

*cc1:
-marm -mfloat-abi=hard -fno-stack-protector -fno-unwind-tables -Os

*link_libgcc:
-L $LIBDIR

*link:
-nostdlib -static -T $LIBDIR/svclibc.ld %(link)
SPECSEOF
