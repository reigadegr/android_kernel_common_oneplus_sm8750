#!/usr/bin/env python3
import re, sys

hex_re = re.compile(r'\b0x[0-9a-fA-F]+\b')

def repl(m):
    return str(int(m.group(0), 16))

def main():
    file = sys.argv[1]
    with open(file, 'r', encoding='utf-8') as f:
        data = f.read()

    new_data = hex_re.sub(repl, data)

    with open(file, 'w', encoding='utf-8') as f:
        f.write(new_data)

if __name__ == '__main__':
    main()
