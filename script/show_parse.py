#!/usr/bin/python3
# -*- coding: utf-8 -*-

import sys
from nltk.tree import Tree

def show_parse(line):
    def recursive(tree, depth):
        #print('"%s [%d]"' % (tree, depth))
        if isinstance(tree, Tree):
            if depth > 0:
                print()
            print('    ' * depth + '(' + tree.label() + ' ', end='')
            for ch in tree:
                recursive(ch, depth + 1)
            print(')', end=('\n' if depth == 0 else ''))
        else:
            print(str(tree), end='')

    t = Tree.fromstring(line)
    recursive(t, 0)

if __name__ == '__main__':
    for line in sys.stdin:
        show_parse(line.strip())

