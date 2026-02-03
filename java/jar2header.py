#!/usr/bin/env python3
"""
JAR to C++ Hex Converter
Converts a JAR file into a C++ byte array header file
"""

import sys
import os
from pathlib import Path

def read_jar_file(jar_path):
    """Read JAR file and return its bytes"""
    try:
        with open(jar_path, 'rb') as f:
            return f.read()
    except Exception as e:
        print(f"[ERROR] Failed to read JAR file: {e}")
        return None


def generate_header(jar_bytes, output_path):
    """Generate C++ header file with JAR bytes as array"""
    try:
        output_dir = os.path.dirname(output_path)
        if output_dir and not os.path.exists(output_dir):
            os.makedirs(output_dir)

        with open(output_path, 'w', encoding='utf-8') as f:
            f.write("#pragma once\n\n")
            f.write("#include <cstdint>\n")
            f.write("#include <cstddef>\n\n")

            f.write("namespace nexus\n{\n")
            f.write("   namespace data\n{\n")

            f.write(f"      constexpr size_t JAR_SIZE = {len(jar_bytes)};\n")
            f.write("       constexpr uint8_t JAR_BYTES[] = {\n")

            for i in range(0, len(jar_bytes), 16):
                chunk = jar_bytes[i:i+16]
                hex_values = ', '.join(f'0x{b:02X}' for b in chunk)
                f.write(f"            {hex_values}")
                
                if i + 16 < len(jar_bytes):
                    f.write(",")
                
                f.write("\n")

            f.write("        };\n")
            f.write("    }\n}")

        return True
    except Exception as e:
        print(f"[ERROR] Failed to write header file: {e}")
        return False


def format_size(size):
    """Format byte size in human-readable format"""
    for unit in ['B', 'KB', 'MB', 'GB']:
        if size < 1024.0:
            return f"{size:.2f} {unit}"
        size /= 1024.0
    return f"{size:.2f} TB"


def main():
    if len(sys.argv) < 2:
        print("Usage: python jar2header.py <input.jar> [output.h]")
        print()
        print("Arguments:")
        print("  input.jar  - Path to the JAR file to convert")
        print("  output.h   - Path to the output header file (optional)")
        sys.exit(1)

    jar_path = sys.argv[1]
    output_path = sys.argv[2] if len(sys.argv) > 2 else "jar_data.h"

    if not os.path.exists(jar_path):
        print(f"[ERROR] JAR file not found: {jar_path}")
        sys.exit(1)

    print(f"[INFO] Input JAR: {jar_path}")
    print(f"[INFO] Output header: {output_path}")
    print()

    print("[INFO] Reading JAR file...")
    jar_bytes = read_jar_file(jar_path)
    if jar_bytes is None:
        sys.exit(1)

    jar_size = len(jar_bytes)
    print(f"[SUCCESS] Read {format_size(jar_size)} ({jar_size:,} bytes)")
    print()

    print("[INFO] Generating C++ header...")
    if not generate_header(jar_bytes, output_path):
        sys.exit(1)

    print(f"[SUCCESS] Header file generated successfully!")
    print()
    print("========================================")
    print(f"Output: {output_path}")
    print(f"Array size: {jar_size:,} bytes")
    print("========================================")


if __name__ == "__main__":
    main()
