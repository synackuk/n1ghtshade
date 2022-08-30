## n1ghtshade 

A 32 bit checkm8 based downgrade utility.

* n1ghtshade permits the downgrade/jailbreak of 32 bit iOS devices
* Supports iOS 6 - 9.3.5

### Download

* n1ghtshade can be downloaded from [here](https://github.com/synackuk/n1ghtshade/releases).

### Bugs

* if you identify any bugs please contact me on Twitter or use the github issues tab.

### Apple Silicon compatibility
* n1ghtshade does not work with Apple Silicon Macs running macOS Big Sur 11.3 or newer due to the checkm8 exploit used to enter pwned DFU mode being incompatible.

### Dependencies

* arm-elf-gcc (you can install with macports)
* libcrypto
* libfragmentzip
* libzip
* libplist
* [libirecovery](https://github.com/synackuk/libirecovery)
* libimobiledevice
* libusb (if on linux)
