# Scriptable User 4 Scripting Document

The engine has a set of builtin commands, while also supports importing commands
from the outside.

Comments are lines starts with `//`. Does not support inline comments.

### Internal commands
- `exit [exit_code]` exit the script.
- `restart` restart the script. (**Dangerous**, see [Cautions](#cautions) for
more details)
- `print [content]` output content to console
- `println [content]` output content to console, and start a new line.
- [`var <command> [...]`](#var) control variables.
- [`jump / goto <tag>`]() jump to a specific tag.
- [`loadplugin/require/include [name]`](#loadplugin--require--include) load plugin from `./scplugins` folder

### builtin Windows commands (windowsapi.dll)
- [`findwindow <class> <title>`]()
- [`showwindowtitle`]()
- [`inject`]()

# Formatting

The script engine has an buildin variable system, which is proceeded **before**
being sent into the command.

> **Warn**: It is possible to execute commands in the variables, and you should 
> be careful with that, because it can be used for injection.

Variables are used like `${name}` or `${name:formatting_options}`.

If name is number-only, it will refer to the parameter that passed into the
script while launching.

If name starts with `>`, it will be treated as an executable command, and take
the output as the variable.


## Formatting Options
Basic grammer are like `f[{:08x},uint64],a[center,20]`. Subcommands are listed
below.

One specific command should only appear **ONCE**.

### - `f` format : `f[format_specifier,type]`
command `f` is similar to `std::format`, but simpler.

Conflicts with `v`.

- `format_specifier` Is similar to `std::format`, except for it only supports
the format body with one and only one `{}`.
- `type` Declare the type the value will be converted to.
  - Supported values: `int8`, `int16`, `int32`, `int64`, `uint8`, `uint16`
  `uint32`, `uint64`, `double`, `char`, `uchar`
  - Speacially supported values: `uintptr_t`

### - `a` alignment : `a[alignment,width]`
command `a` is used to control the horizontal alignment. command `f` is already
capable for doing this, but this one is more efficent and simple.

- `alignment`
  - Supported values: `center`, `left`, `right`
- `width` Control the width for the calculation.

### - `v` value : `v[value_name]`
command `v` gets something else about the variable.

Conflicts with `f`.

- `value_names` The name of the value you want to get.
  - Supported values: [`length`](#value_names_length), [`size`](#value_names_size), [`type`](#value_names_type)

#### value_names_length
The length of the variable, treated as string.
#### value_names_size
The size of the variable, treated as bytearray (stored in hex).
#### value_names_type
The type of the variable.


### The Subcommands has sequence, like below.
```
--front---------back->
f a
v
```

## Commands

### `var`
`var` command is used to control variables.

Remember that all variables are all strings, but meta types convertion are
supported. See more details in [Developing](#developing-modules).

A valid variable name should contains at lease one non-number character, and is
only composed of A-Z, a-z, 0-9 and "_".

#### Grammer
```
var set var1 "Hello world"
var remove var1
var swap var1 var2
```
- `var set`
  - This command creates new variable or set a new value to the variable.
- `var remove`
  - This command removes the variable from the memory, like it is discarded.
- `var swap`
  - This command swaps the two variables.

#### Reserved Variables
- `ERRORLEVEL` Stores the **return code** of the last command.
- `SCUSER_DIR` Stores the executable **directory** of ScriptableUser4.
- `SCUSER_EXE` Stores the executable **path** of ScriptableUser4.
- `SCRIPT_DIR` Stores the running script's **directory**.
- `SCRIPT_FILE` Stores the running script's **path**.

### `loadplugin` / `require` / `include`
This command load a specific plugin by the name of the plugin file.

Plugin files are files with `.dll` or `.scup` extension name.

# Developing Modules
1. In your `CMakeLists.txt`, add `target_link_libraries(your_target scapi)`.
2. Include `scPluginApi.hpp` in your header file.
3. Register your functions as `int function_name(__fn_sig)`.
4. Input parameters is `sl`, and variable handler is `var`.
5. In your cpp file, write the entry as follow:
```cpp
__plugin_load_signature(register_command, var) {
    register_command({"myfunc_name", "myfunc_alias"}, myfunc);
    var["MYMODULE_LOADED"] = true;

    return true; // true on success, otherwise script fails directly.
}
```
6. Place the compiled dll file in `scplugins` folder next to the main executable. You can create it if it doesn't exist.
7. In the script, use `loadplugin myplugin` to load `myplugin.dll`, and you can then use the function you imported.

# Cautions

### - unsafe `restart` command
`restart` only set the cursor to the first line and the execution goes on, no
variables will be cleaned, no other code will be run.

In a word, this could lead to **Undefined Behavior (UB)**.
