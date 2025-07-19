About TurboFEC-ARM
==================

TurboFEC-ARM is an adaptation of the [TurboFEC library](https://github.com/ttsou/turbofec), originally designed for x86 architectures, now ported to support ARM-based systems such as Raspberry Pi 5 and similar ARM devices. This library includes implementations of LTE forward error correction encoders and decoders, covering convolutional codes, turbo codes, and the associated rate matching units that manage block interleaving, bit selection, and pruning.

The ARM version leverages NEON instructions for optimized performance on ARM hardware.

LTE specification and sections:

3GPP TS 36.212 *"LTE Multiplexing and channel coding"*

5.1.3.1 *"Tail biting convolutional code"*

5.1.3.2 *"Turbo encoding"*

5.1.3.1 *"Rate matching for turbo coded transport channels"*

5.1.3.2 *"Rate matching for convolutionally coded transport channels and control information"*

Building with CMake
===================
1. **Install Required Packages**:
   To build the project, you will need `git`, `cmake`, and a C compiler like `gcc`.
   ```sh
   $ sudo apt update
   $ sudo apt install -y git cmake build-essential
   ```

2. **Clone the repository**:
   Get the source code:
   ```sh
   $ git clone https://github.com/amcolex/turbofec-arm.git
   $ cd turbofec-arm
   ```

3. **Run the Build Commands**:
   Now you can run the build commands using CMake:
   ```sh
   $ mkdir build
   $ cd build
   $ cmake -DCMAKE_BUILD_TYPE=Release ..
   $ make
   ```
   To install the library system-wide (optional):
   ```
   $ sudo make install
   ```

Testing
=======
1. **Run all automated tests**:
   This will execute all configured tests and output results. From the `build` directory run:
   ```sh
   $ ctest
   ```

2. **Benchmark specific code**:
   Here is how to perform a benchmark test on the 3GPP LTE turbo encoder and decoder. From the `build` directory run:
    ```sh
    $ ./tests/turbo_test -b -j 4 -i 1 -p 10000

    =================================================
    [+] Testing: 3GPP LTE turbo
    [.] Specs: (N=2, K=4), Length 6144

    [.] Performance benchmark:
    [..] Decoding 40000 bursts on 4 thread(s) with 1 iteration(s)
    [..] Testing:
    [..] Elapsed time....................... 2.855458 secs
    [..] Rate............................... 86.066754 Mbps
    ```

   This example demonstrates the potential performance capabilities of TurboFEC-ARM when running on native ARM hardware without the limitations of emulation.

3. **Run specific tests**:
   Individual tests and benchmarks can also be run to evaluate specific functionalities or performance aspects:
   ```sh
   $ ./tests/conv_test -h # Displays help for convolutional tests
   $ ./tests/turbo_test -h  # Displays help for turbo tests
   ```

Benchmark
=========
You can perform various convolutional and turbo decoding tests to assess performance on your ARM device. From the `build` directory run:

```sh
$ ./tests/turbo_test -b -j 4 -i 1 -p 10000

=================================================
[+] Testing: 3GPP LTE turbo
[.] Specs: (N=2, K=4), Length 6144

[.] Performance benchmark:
[..] Decoding 40000 bursts on 4 thread(s) with 1 iteration(s)
[..] Testing:
[..] Elapsed time....................... 2.855458 secs
[..] Rate............................... 86.066754 Mbps

```

Credits
=======
All credit for the original x86-based TurboFEC code goes to the original repository at: [TurboFEC](https://github.com/ttsou/turbofec).

Additionally, this ARM NEON adaptation utilizes the 'sse2neon' library to convert SSE intrinsics to their NEON counterparts. 'sse2neon' is a critical component in enabling the TurboFEC code to run on ARM architectures. More details and the source code can be found at [sse2neon](https://github.com/DLTcollab/sse2neon).
