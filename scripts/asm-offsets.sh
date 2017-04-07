#!/bin/bash

# Convert raw asm offsets into something that can be included as
# assembler definitions.  It converts
#   -> symbol $value source
# into
#   #define symbol value /* 0xvalue source */

cat <<EOF
/*
 * \$FILE: asm_offsets.h
 *
 * \$LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */

#ifndef __XM_ASM_OFFSETS_H__
#define __XM_ASM_OFFSETS_H__

/*
 * DO NOT MODIFY
 */
EOF

awk '
  /^->$/{printf("\n")}
  /^-> /{
    sym = $2;
    val = $3;
    sub(/^\$/, "", val);
    $1 = "";
    $2 = "";
    $3 = "";
    printf("#define _%s_OFFSET 0x%x\t/* 0x%x\t%s */\n", toupper(sym), val, val, $0)
  }
'

echo '#endif'
