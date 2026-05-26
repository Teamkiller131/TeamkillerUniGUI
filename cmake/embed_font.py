#!/usr/bin/env python3
"""Embed a font file as a C header with named variables."""
import sys, os

if len(sys.argv) < 4:
    print(f"Usage: {sys.argv[0]} <font.ttf> <output.h> <var_name>")
    sys.exit(1)

font_path = sys.argv[1]
out_path = sys.argv[2]
var_name = sys.argv[3]

size = os.path.getsize(font_path)
with open(font_path, 'rb') as f:
    data = f.read()

with open(out_path, 'w') as out:
    out.write(f"// Auto-generated font data: {os.path.basename(font_path)}\n")
    out.write(f"const unsigned char {var_name}_data[] = {{\n")
    for i in range(0, len(data), 12):
        chunk = data[i:i+12]
        out.write('  ' + ', '.join(f'0x{b:02x}' for b in chunk) + ',\n')
    out.write('};\n')
    out.write(f'const unsigned int {var_name}_size = {size};\n')
