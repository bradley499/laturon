# Laturon
The Laturon programming language [interpreter](interpreter), and [integrated development environment (IDE)](ide).

## Requirements

There are two different ways to compile the Laturon interpreter, either for WebAssembly (WASM) which will also bundle the IDE into the build directory, or native compilation.
For both types of compilation the following is required:
 - `make`
### WASM compilation requirements
For WASM compilation, you'll need the Emscripten toolchain, which can be downloaded and installed by following the Emscripten installation guide at:
[https://emscripten.org/docs/getting_started/downloads.html](https://emscripten.org/docs/getting_started/downloads.html)

### Native compilation requirements
For native compilation (only currently tested on Linux x86_64) you'll need a C compiler, for this project, either of the following can be used:
 - `GCC`
 - `Clang`

## Compiling
### WASM compilation
To compile to for WASM, you must use the command (from the top level directory of this repository):
```
emmake make
```
This will compile using `emcc` (which you would have installed from the installation guide reference in the section [WASM compilation requirements](#wasm-compilation-requirements)) and will output the contents of the WASM program, and IDE, within a directory in the top level directory of this repository called `build`.
### Native compilation
To compile natively you must navigate to the [interpreter](interpreter) directory, from there you can run the command:
```
make
```
This will compile the project to a native binary, and will output the content to a directory called `build` within the directory [interpreter](interpreter).

## Execution
### IDE
To launch the IDE on your own machine, you'll need to have a local http server set up. For this example, you'll need to have `node` installed. Once you have `node` installed you'll need to install a http server, which can be done with the command:
```
npm install http-server -g
```
Once you have the package `http-server` installed, you can start running the IDE by executing the command (from the top level directory of this repository):
```
http-server build
```
If all was successful, you should have a web address that it is locally available on, which you can visit within you web browser, where you can run the Laturon IDE.

### Executable
To run the native executable, you must execute the program relative to your directory, for example if you are in the same directory as the executable, you can run the command:
```
./laturon
```
However, this will through an error as it expects a source file written in the Laturon programming language. To execute a program written in Laturon, you must provide the interpreter with the (relative or absolute) path to the file. An example of a file that is in the same directory as where the interpreter is being executed from is as follows:
```
./laturon hello_world.lt
```
To see information about how to run the Laturon interpreter, you can run the command:
```
./laturon --help
```