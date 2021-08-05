# Dirent

Dirent is a C/C++ programming interface that allows programmers to retrieve
information about files and directories under Linux/UNIX.  This project
provides Linux compatible Dirent interface for Microsoft Windows.


# How to Enable UTF-8 Support

By default, Dirent functions expect that directory names are represented in
the currently selected windows codepage.  Moverover, Dirent functions return
file names in the currently selected codepage.  If you wish to use UTF-8 file
names instead, then set the program's locale to ".utf8" or similar.  For
example, you C main program might look like

```
#include <locale.h>

int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "LC_CTYPE=.utf8");

    /*...*/
}
```

For more information on UTF-8 support, please see setlocale in Visual Studio
[C runtime library reference](https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/setlocale-wsetlocale?view=msvc-160#utf-8-support).


# Installation

Download the latest Dirent installation package from
[GitHub](https://github.com/tronkko/dirent/releases) and
unpack the installation file with 7-zip, for example.  The installation
package contains ``include/dirent.h`` file as well as a few example and test
programs.


## Installing Dirent for All Programs

To make dirent.h available for all C/C++ programs, simply copy the
``include/dirent.h`` file to the system include directory.  System include
directory contains header files such as ``assert.h`` and ``windows.h``.  In
Visual Studio 2008, for example, the system include may be found at
``C:\Program Files\Microsoft Visual Studio 9.0\VC\include``.

Everything you need is included in the single ``dirent.h`` file, and you can
start using Dirent immediately -- there is no need to add files to your
Visual Studio project.


## Embedding Dirent into Your Own Project

If you wish to distribute ``dirent.h`` alongside with your own source code,
then copy ``include/dirent.h`` file to a new sub-directory within your project
and add that directory to include path on Windows while omitting the directory
under Linux/UNIX.  This allows your project to be compiled against native
``dirent.h`` on Linux/UNIX while substituting the functionality on Microsoft
Windows.


## Examples

The installation package contains six example programs:

Program  | Purpose
-------- | -----------------------------------------------------------------
ls       | List files in a directory, e.g. ls "c:\Program Files"
find     | Find files in subdirectories, e.g. find "c:\Program Files\CMake"
updatedb | Build database of files in a drive, e.g. updatedb c:\
locate   | Locate a file from database, e.g. locate notepad
scandir  | Demonstrate scandir() function
cat      | Print a text file to screen

Please install [CMake](https://cmake.org/) to build example and test programs.
Then, open command prompt and create a temporary directory ``c:\temp\dirent``
for the build files as

```
c:\
mkdir temp
mkdir temp\dirent
cd temp\dirent
```

Generate build files as

```
cmake d:\dirent
```

where ``d:\dirent`` is the root directory of the Dirent package (containing
this README.md file).  If wish to omit example programs from the
build, then append the option ``-DDIRENT_BUILD_TESTS=OFF`` to the CMake
command line.

Once CMake is finished, open Visual Studio, load the generated ``dirent.sln``
file from the build directory and build the whole solution.  Once the build
completes, run the example programs ls, find, updatedb and locate from the
command prompt as

```
cd Debug
ls .
find .
updatedb c:\
locate cmd.exe
```

Visual Studio project also contains a solution named ``check`` which can be
used to verify that Dirent works as expected.  Just build the solution from
Visual Studio to run the test programs.


# Contributing

We love to receive contributions from you.  See the
[CONTRIBUTING](CONTRIBUTING.md) file for details.


# Copying

Dirent may be freely distributed under the MIT license.  See the
[LICENSE](LICENSE) file for details.


# Alternatives to Dirent

I ported Dirent to Microsoft Windows in 1998 when only a few alternatives
were available.  However, the situation has changed since then and nowadays
both [Cygwin](http://www.cygwin.com) and [MingW](http://www.mingw.org)
allow you to compile a great number of UNIX programs in Microsoft Windows.
They both provide a full dirent API as well as many other UNIX APIs.  MingW
can even be used for commercial applications!
