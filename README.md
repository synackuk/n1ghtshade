# n1ghtshade

### A bootchain jailbreak/downgrade utility for A6.

#### Features

* Allows downgrades and custom firmwares to be loaded.
* Jailbreaks.
* Hacktivates.

#### Beta 1 Caveats

* Currently cannot jailbreak iOS 8 and up
* Mac only; Works on macOS mojave and up

#### Download

* Can be downloaded from [here](https://github.com/synackuk/n1ghtshade/releases)

#### Support

* Please report any bugs in the [github issues](https://github.com/synackuk/n1ghtshade/issues), or to me on twitter [(@synackuk)](https://twitter.com/synackuk)
* You can get help from me on Twitter also

#### Layout

* atropine - iBoot payload
* belladonna - Injector - responsible for loading checkm8, tethered boot etc.
* hyoscine - jailbreak ramdisk
* physostigmine - Jailbreak ramdisk modules (fstab patcher, hacktivation, etc).
* pilocarpine - interfaces

#### Dependencies

* Most of these can be installed with homebrew
* arm-elf-gcc (you can install with macports)
* [bin2c](https://github.com/gwilymk/bin2c) in the path
* hfsplus from [xpwn](https://github.com/xerub/xpwn) in the path
* libcrypto
* libzip
* libplist
* libusb
* libirecovery
* libimobiledevice
* theos

#### Credits

* Thanks to [axi0mX](https://github.com/axi0mX) for [checkm8](https://github.com/axi0mX/ipwndfu)
* Thanks to [Chronic Dev](https://github.com/Chronic-Dev) for [greenpois0n](https://github.com/Chronic-Dev/gprc5)
* Thanks to [xerub](https://github.com/xerub) for [ibex](https://github.com/xerub/ibex)
* Thanks to [iH8Sn0w](https://github.com/ih8sn0w), [tihmstar](https://github.com/tihmstar), [nyan_satan](https://github.com/nyan_satan) for [iBoot32Patcher](https://github.com/NyanSatan/iBoot32Patcher)
* Thanks to [libimobiledevice](https://github.com/libimobiledevice/) for [idevicerestore](https://github.com/libimobiledevice/idevicerestore)
* Thanks to [RealiMuseum](https://twitter.com/RealiMuseum) for testing
