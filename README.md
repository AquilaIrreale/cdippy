cdippy
======

A classic Diplomacy adjudicator, in C

What's this?
------------

This is an attempt at writing an adjudicator for the classic variant of
the game of Diplomacy in C, motivated by the lack of a publicly
available implementation thereof.

The code is based on the algorithms and equations proposed by Lucas
Kruijswijk in "The Math of Adjudication", available [here][1] for
reading.

Development will focus on obtaining a working version as soon as
possible, and then refactoring or rewriting the ugly bits once a stable
form is reached.

Now [DATC][2] compliant up to and including section [6.C][3] (some tests
do not apply)

Usage
-----

### Input language

**Note:** the order grammar and the program's interface in general are
not stable and may be subject to change in the future. Notice also that
this software is not intended to be used directly by humans, but is
rather to be thought as a backend "adjudication engine" to be used by
larger diplomacy-related software. The input protocol can therefore feel
coarse and rather strict; this is intended, to keep input parsing code
as simple and straightforward (and at the same time unambiguous) as
possible. _UI sold separately ;P_

cdippy does not take any command line arguments. Once started, it will
expect two blocks of statements on its standard input: a state block
first, followed by an orders block. Both blocks contain one statement
per line, and are terminated by an empty line. State statements are in
the form of `Territory Unit_type Nation` and inform cdippy of the
current state of the board. Order statements are, well... orders; they
follow one of these formats:

```
1. Territory - Territory [Coast]
2. Territory S Territory
3. Territory S Territory - Territory
4. Territory C Territory - Territory
```

These are, in order: (1) move (a.k.a. attack), (2) support to hold, (3)
support to move, (4) convoy. `Coast` is always required when ordering a
fleet to move to a split coast territory, even when the only available
destination can be inferred by the starting position of the unit. It
must never instead be specified when ordering an army. An hold order is
assumed for every unit that has not received any.

The input language is completely case insensitive; whitespace only
serves as a separator and can often be omitted, e.g. around `-`s and
between a territory and its coast, when using the parens form (see
below).

Once the two blocks have been received, the process of adjudication will
start. When done, cdippy will write to standard output a list of
resolutions in the form of `Order_#: Resolution` terminated by a blank
line. `Resolution` can either be `SUCCEEDS` or `FAILS`, depending on the
outcome of the order designated by `Order_#`. The program is then ready
to accept another two blocks of statements and perform a new
adjudication.  cdippy will not update its internal representation of the
board according to the outcome of a set of orders, but it will keep its
state between adjudications. To clear a territory of an unit, the
special state statement `CLEAR Territory` is provided, along the more
drastic `CLEAR ALL`, which completely resets the state of the board,
removing every unit currently present, and allowing for a fresh start.

### Tokens

What follows is a list of all the available valid tokens; remember they
are all case insensitive, as the rest of the language is.

#### Unit\_type

```
A, F -- army and fleet
```

#### Nation

```
Austria, England, France,
Germany, Italy, Russia, Turkey
```

#### Territory

```
Alb, Ank, Apu, Arm, Bel, Ber,
Boh, Bre, Bud, Bul, Bur, Cly,
Con, Den, Edi, Fin, Gal, Gas,
Gre, Hol, Kie, Lon, Lvn, Lvp,
Mar, Mos, Mun, NAf, Nap, Nwy,
Par, Pic, Pie, Por, Pru, Rom,
Ruh, Rum, Ser, Sev, Sil, Smy,
Spa, StP, Swe, Swi, Syr, Tri,
Tun, Tus, Tyr, Ukr, Ven, Vie,
Wal, War, Yor, AEG, ADR, BAL,
BAR, BLA, BOT, EAS, ENG, HEL,
ION, IRI, LYO, MAO, NAO, NTH,
NWG, SKA, TYS, WES
```

#### Coast

```
NC, SC,    -- short form
(NC), (SC) -- parens form
```

Bulgaria uses NC instead of EC just like Spain and St.Petersburg do.

Building
--------

### Unix & Linux systems

Make sure you have a C compiler, an implementation of Lex and Yacc, and
the GNU Autotools (Autoconf & Automake) installed on your machine, then
issue the following:

```sh
./autogen.sh
./configure
make
```

For a debug build, use `./configure CFLAGS='-g -O0'`

### Microsoft Windows

Windows is not officially supported and there is no plan to support
Microsoft's compiler toolchain in the future, but it should be (read:
certainly is) possible to build and run this software on Windows through
platforms such as Cygwin, MinGW both on MSYS and on the WSL, or the WSL
itself.

Installing
----------

Once built, running `make install` (with root privileges as needed) will
install cdippy into the configured directory, which defaults to
`/usr/local`. Does not apply to Windows.

Testing
-------

To run the automatic tests you need to have the Dejagnu test driver
installed on your machine. `make check` will run through a selection of
tests extracted from the [DATC library][3] (since not every one of those
is applicable to this kind of barebones adjudicating-only software). A
full log of the tests run and a summary can then be found in
`cdippy.log` and `cdippy.sum`.

[1]: http://uk.diplom.org/pouch/Zine/S2009M/Kruijswijk/DipMath_Chp1.htm
[2]: http://web.inter.nl.net/users/L.B.Kruijswijk/
[3]: http://web.inter.nl.net/users/L.B.Kruijswijk/#6
