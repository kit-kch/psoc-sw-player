# PSoC Extension PCB Test

This repository contains small software to test the PSoC extension PCB.

## Using the Repository

For dependencies and paths to be setup correctly, this repo should not be checked out manually.
It is checked out as part of the `psoc_soc` repository in its `ext` folder instead.

## Compiling the Software

If the repo was checked out as part of `psoc_soc`, there are two options to build the software:
* Use the rules in the top-level Makefile, e.g. `make boardtest`.
* Make sure to setup the environment, then `cd` into the source folder and use the local Makefile:
  ```bash
  source script/sw/sw_env.sh
  cd ext/psoc_boardtest_sw
  # Generate executable for bootloader
  make exe
  # Remove all generated files
  make clean
  ```
