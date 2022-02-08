# Nazi Zombies: Portable Quakespasm

## About
This repository contains NZ:P Team's fork of [Quakespasm](https://sourceforge.net/projects/quakespasm/), with support for building to Nintendo Switch (thanks to fgsfdsfgs) and PS VITA (thanks to Rinnegatamante). It contains many enhancements to attempt parity with our fork of [dQuakePlus](https://github.com/nzp-developers/dquakeplus).

## Building for Nintendo Switch (Advanced)
Building requires a full install of [libnx](https://switchbrew.org/wiki/Setting_up_Development_Environment). Follow the instructions by either installing manually or by using the Docker container linked.

Now that libnx is installed, you can build the Switch binaries:
```bash
make -f Makefile.nx
```

The `.NRO` will be built at `build/nx`. We also provide binaries on the [Releases](https://github.com/nzp-developers/quakespasm/releases/tag/bleeding-edge) page.

## Building for PlayStation VITA (Advanced)
Similarly as building for NX, building a `.VPK` will require the [Vita SDK](https://vitasdk.org/). We recommend `gnuton`'s Docker container, it can be found [here](https://hub.docker.com/r/gnuton/vitasdk-docker).

You can build like so:
```bash
make -f Makefile.vita
```

The `.VPK` will be built at `build/vita`. We also provide binaries on the [Releases](https://github.com/nzp-developers/quakespasm/releases/tag/bleeding-edge) page.