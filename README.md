# snufflebrush
Weave your *darkest ideas* (preferrably representable as bytes) into an innocent image using this paintbrush in such way that everyone else will be left no choice but to **snuffle**.  
Or, in other words...  
A steganography tool written by me during school to randomly hide data in png images based on the used password.  

## Usage
The current version is written using C++, however there is also a previous [Python version](/python-snufflebrush) which is usable in the same way, but is much slower (and has no custom encoding features).
### Setup
1. Clone this repo using
```bash
git clone --recursive https://github.com/renbou/snufflebrush
```
(Or `git submodule update --init --recursive` if you've already classically cloned this repo without submodules)  
2. Classic cmake build steps
```bash
mkdir build
cd build
cmake ..
make
```
### Execution
Encoding data from file with path `~/secrets/sus` in image with path `~/Downloads/catmeme.png` using password `t0t4llys3cr3t` and storing result in file `~/Documents/innocent.png`:
```bash
snufflebrush -e -i ~/Downloads/catmeme.png -o ~/Documents/innocent.png -f ~/secrets/sus -p t0t4llys3cr3t
```  
  
Decoding the same secret data from the resulting image into file `~/Documents/sussy-secret`:
```bash
snufflebrush -d -i ~/Documents/innocent.png -o ~/Documents/sussy-secret -p t0t4llys3cr3t
```  
  
The C++ version also allows specifying specific bits and colors to use for storing the data, be aware though that this results in less available space.
