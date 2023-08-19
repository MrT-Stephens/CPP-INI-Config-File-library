
<h1 align="center">
  <br>
  C++ INI Config File Library
  <br>
</h1>

<h4 align="center">A simple c++ INI configiration file library for different applications!</h4>

<p align="center">
  <a href="#infomation">Infomation</a> •
  <a href="#installation">Installation</a> •
  <a href="#basic-examples">Basic Examples</a>
</p>

<p align="center">
  <img src="https://github.com/MrT-Stephens/CPP-INI-Config-File-library/blob/main/Images/INI-File.png" alt="screenshot">
</p>

## Infomation
This is a simple and easy-to-use C++ header-only INI config file library.

Features:
* Reading and writing to INI files.
* Can be made to be case insensitive.
* Can add new keys and values.
* Can remove keys and values.
* Ability to have multiple sections with multiple keys and values.

The generated INI files will follow the following format:
```INI
[section]
key=value
key=value

[section]
key=value
key=value
```

Files are inputted on demand in one go, after which the data is kept in memory and is ready to be manipulated. Files are closed after read or write operations. The section and key order are preserved after read-and-write operations.
The library uses `std::string` when manipulating the data, but when reading the data, you can specify what type you would like the value returned in.

## Installation
This is a header-only library. To install it, just copy everything in the repository into your own project's source code folder or clone the repository using:
```bash
# Clone this repository
$ git clone https://github.com/MrT-Stephens/CPP-INI-Config-File-library
```
After which use `#include` to integrate the header into your project:
```C++
//Include the library.
#include "ConfigFile.h"
```
## Basic Examples

### Generating / inputting the INI
To generate or input a file you can use one of two constructors.

The first constructor will take no parameters and will use `\FileConfig.ini` as a default file name and either input the file if it already exists in the project's directory or will generate a new file with the default name:
```C++
//Default constructor.
mrt::ConfigFile<std::string> file;
  OR
mrt::ConfigFile<std::wstring> file;
```
The second constructor takes a file path as a parameter and will either input the file if it exists at the given path or generate a new file with the given file name.
```C++
//Second constructor.
mrt::ConfigFile<std::string> file("C:\\Users\\MrTst\\Downloads\\ConfigFile.ini");
  OR
mrt::ConfigFile<std::wstring> file(L"C:\\Users\\MrTst\\Downloads\\ConfigFile.ini");
```

### Outputting the INI
To output the INI you need to call one of two functions.

To output the file to the specified path you need to call the `file.save()` function.
```C++
//Save the INI file.
file.save();
```

To output the file to the specified stream you need to call the `file.save(&stream)` function, passing in a pointer to the output stream object.
```C++
//Save the INI file to a stream.
file.save(&std::cout);
```

### Writing Data
The `file.write()` function takes four parameters. These are a section name, a key, a key value, and a boolean.

If you would not like to update the key value if the key is already present:
```C++
//Will not update the key value.
file.write("[Windows]", "User", "MrT Stephens");
```
If you would like to update the key value if the key is already present:
```C++
//Will update the key value.
file.write("[Windows]", "User", "MrT Stephens", true);
```

### Remove Data
The `file.remove()` function takes two parameters. These are a section name and a section key.
```C++
//Remove a key.
file.remove("[Windows]", "User");
```
> **Note**
> Once the `file.remove()` function has been called the key will be removed from memory, but not the file until the `file.save()` function has been called.

### Reading Data
To read the data the are two different `file.read()` functions that can be used.

The first `file.read()` function takes two parameters and returns the key value. The parameters are a section name and a key.
```C++
//Read a string.
std::string strVariable = file.read<std::string>("[Windows]", "User");

//Read an unsigned long long.
unsigned long long ullVariable = file.read<unsigned long long>("[Windows]", "Id");
```
The second `file.read()` function takes three parameters. These are a section name, a key, and a pointer to the variable you would like the value to be stored in.
```C++
//Read a string.
std::string strVariable;
file.read("[Windows]", "User", &strVariable);

//Read a unsigned long long.
unsigned long long ullVariable;
file.read("[Windows]", "User", &ullVariable);
```
The `file.read()` function can accept `std::string OR std::wstring`, `int`, `long`, `long long`, `unsigned long long`, `double`, `long double`, and `bool`.
