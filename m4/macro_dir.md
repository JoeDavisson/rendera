rendera/m4/macro_dir.md
=======

This file resides within ```rendera/m4``` (the "macro directory"); this is the
argument passed to the [```AC_CONFIG_MACRO_DIR```][autoconf-manual-input]
invocation.

Since autoconf v2.69, if this directory does not exist, then an invocation like
```autoreconf -ivf``` will forcibly create it. Earlier autoconf versions will
instead fail and report an error.

This file exists to maintain the presence of the macro directory within the
version control system.

[autoconf-manual-input]: https://www.gnu.org/software/autoconf/manual/autoconf.html#Input
