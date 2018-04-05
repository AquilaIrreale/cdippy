#!/usr/bin/env python3

import sys, re, json

class Order:
    def __init__(self, kind, **kwargs):
        self.kind = kind

        for k, v in kwargs.items():
            setattr(self, k, v)

    def state_str(self):
            return '{} {} {}'.format(self.terr, self.unit, self.nation)

    def order_str(self):
        if self.kind == 'MOVE':
            return '{}-{}'.format(self.terr, self.targ)

        elif self.kind == 'SUPPORT':
            return '{} S {}-{}'.format(self.terr, self.orig, self.targ)

        elif self.kind == 'SUP_HOLD':
            return '{} S {}'.format(self.terr, self.orig)

        elif self.kind == 'CONVOY':
            return '{} C {}-{}'.format(self.terr, self.orig, self.targ)

        else:
            return None


ws_re = re.compile(r"\W+")

def squash_space(s):
    return ws_re.sub(' ', s).strip()


def ask_user(s):
    return input('{} -> '.format(s)).strip().upper()

try:
    with open("redatc_cache.json", "r") as f:
        simplify_cache = json.load(f)

except (FileNotFoundError, json.JSONDecodeError):
    simplify_cache = {}

def simplify(s):
    s = squash_space(s)

    if s not in simplify_cache:
        simplify_cache[s] = ask_user(s)

    return simplify_cache[s]


def convoy(nation, match):
    return Order('CONVOY',
                 nation=nation,
                 unit=match.group(1),
                 terr=simplify(match.group(2)),
                 orig=simplify(match.group(3)),
                 targ=simplify(match.group(4)))


def support(nation, match):
    return Order('SUPPORT',
                 nation=nation,
                 unit=match.group(1),
                 terr=simplify(match.group(2)),
                 orig=simplify(match.group(3)),
                 targ=simplify(match.group(4)))


def support_hold(nation, match):
    return Order('SUP_HOLD',
                 nation=nation,
                 unit=match.group(1),
                 terr=simplify(match.group(2)),
                 orig=simplify(match.group(3)))


def move(nation, match):
    return Order('MOVE',
                 nation=nation,
                 unit=match.group(1),
                 terr=simplify(match.group(2)),
                 targ=simplify(match.group(3)))


def hold(nation, match):
    return Order('HOLD',
                 nation=nation,
                 unit=match.group(1),
                 terr=simplify(match.group(2)))


orders = []
nation = None
nation_re = re.compile(r'\W*(AUSTRIA|ENGLAND|FRANCE|GERMANY|ITALY|RUSSIA|TURKEY)\W*:\W*')
actions = [
    (convoy,       re.compile(r'([AF])\W+([A-Z()\W]+)\W+CONVOYS\W+[AF]\W+([A-Z()\W]+)\W*-\W*([A-Z()\W]+)')),
    (support,      re.compile(r'([AF])\W+([A-Z()\W]+)\W+SUPPORTS\W+[AF]\W+([A-Z()\W]+)\W*-\W*([A-Z()\W]+)')),
    (support_hold, re.compile(r'([AF])\W+([A-Z()\W]+)\W+SUPPORTS\W+[AF]\W+([A-Z()\W]+)')),
    (move,         re.compile(r'([AF])\W+([A-Z()\W]+)\W*-\W*([A-Z()\W]+)')),
    (hold,         re.compile(r'([AF])\W+([A-Z()\W]+)\W+HOLD'))
]

lines = []

while True:
    try:
        l = input().strip().upper()

    except EOFError:
        print("Unexpected end of input")
        exit(1)

    if not l:
        continue

    if l == '%%':
        break

    lines.append(l)

for l in lines:
    match = nation_re.match(l)
    if match:
        nation = match.group(1)
        continue

    for action, pattern in actions:
        match = pattern.match(l)
        if match:
            orders.append(action(nation, match))
            break
    else:
        print("Error: unrecognized: " + l)
        exit(1)

print('%%')

for o in orders:
    print(o.state_str())

print()

for o in orders:
    s = o.order_str()
    if s:
        print(s)

print()

with open("redatc_cache.json", "w") as f:
    json.dump(simplify_cache, f, sort_keys=True, indent=4)
    f.write('\n')
