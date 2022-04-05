# lf3000-brio
 Development snapshot of LF3000 Brio SDK libraries 

Refer to parent `SConstruct` file for platform build options.

Depends on Python-based `scons` build tool framework.

```
apt-get install scons
```

Must use ARM cross-compiler toolchain for uclibc runtime,
for compatibility with original LF1000 binaries.

Must set env var `CROSS_COMPILE` via toolchain setup script
for setting compiler tools $CC, $CXX, $LD, etc.

```
export CROSS_COMPILE=arm-angstrom-linux-uclibceabi-
```

Default options preset by running `scons` without arguments.

```
scons
```

Equivalent to: 

```
scons platform=LF3000 type=embedded debug=false
```

ARM-compatible library binaries are deployed into platform-specific 
./Build directories, such as ./Build/LF3000/
