Haiku port of aobook
----
This is Haiku OS port of Aozora Bunko viewer, aobook.

* Original README file can be found in README file.
* License terms can be seen in COPING file. Licensed under BSD 2 (3-clause).

Build on haikuporter
----
```
haikuporter aobook
```

Build on Haiku OS
----
If you need to develop in the tree, you can build as follows.
```
cd mlib
make -f Makefile.haiku

cd ../src
make -f Makefile.haiku
```

```
src/objects.xxx/aobook
```
