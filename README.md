# fritz-curl
* WARNING: use at your own risk, no warranty!!! code might be buggy
TR-069 API-client written in c++ for the AVM Fritz!BOX

FAQ:
* Why c++ ?
Because this program is used in an embedded Asterisk Telephony system
in order to be able to switch WLAN on/off from any of the connected telephones.

* The Fritz!BOX already has a built-in feature to switch on/off WLAN adapter using a telephone, why not use that?
The Fritz!BOX does not have the ability to create nice dialplans and lacks lots of other cool telephony features.


More info about the Fritz!BOX SOAP API protocol see AVM TR-064 documentation

Depencencies:
Boost library is used for XML parsing
CURL library is used for connecting to Fritz!BOX using HTTP/SOAP
SSL library is used for calculating md5 hash


Example usage:

Fritz-cURL version 0.1
No arguments given, example usage:
wlan24 0 (switch wlan 2.4 GHz off
wlan24 1 (switch wlan 2.4 GHz on
wlan24 2 (return wlan 2.4 GHz status


* Instructions to CROSSCompile for Blackfin / uCLinux:

1) Get buildroot: git clone (latest url see: http://buildroot.uclibc.org/)
2) make menuconfig
3) The buildroot buildscript has options to automatically download the blackfin compiler toolchain
4) select bf532 as a target platform, and make sure to include the libraries:
crypto/openssl
networking/libcurl
other/boost
5) save the configuration
6) build the necessary blackfin/uCLinux libraries, using the following commands:
make openssl (1.0.2d)
make libcurl
make boost
7) add the bfin-uclinux-g++ compiler location to your $PATH
8) go to the fritz-curl sources directory and execute the 'make' command in order to build the fritz-curl tool
