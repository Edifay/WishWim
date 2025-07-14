All should be packed in.

# How to compile

## Dependencies :

### Ubuntu/Debian :

 - `apt install make`

### Clang version 18

 - `wget https://apt.llvm.org/llvm.sh`
 - `chmod +x llvm.sh`
 - `sudo ./llvm.sh 18`

### Install tree-sitter api.h

 - Be in the root folder
 - `make -C lib/tree-sitter/ install`

### Install rust/rustup/cargo packages

 - `rustc --version`

It's really important to be up to date ! You will be in most cases not up to date.

Ubuntu :
 - `apt install rustup`
 - `rustup update stable`

Others distro may not have rustup package. Check for your personnal distro.
You might find this useful : https://rustup.rs/

### Compile :

In the root folder :
  - `make` 


It should compile fine.


![image](https://github.com/user-attachments/assets/3f2de19a-f6e1-4cec-95f4-be5137e6bd1c)
