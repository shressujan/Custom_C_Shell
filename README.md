
## 1 Usage

Read [main.cpp](main.cpp) (usage) and [command.hpp](command.hpp) (data
structure).

### 1.1 Compiling & Running on CSE

You can run the following four commands to compile and run on cse.unl.edu:

```text
cd sshrestha11_pa2
make
./osh
```

### 1.2 Cleaning the folder(removes the current binary file and .out files)
```text
cd sshrestha11_pa2
make clean
```

### 1.3 Parsing

`parse_command_string()` returns a vector of parsed commands (defined in
[command.hpp](command.hpp)) from a line of text input from the terminal.

```cpp
// 1. Get a line from the terminal.
std::string input_line = "echo hello world";

// 2. Parse.
std::vector<shell_command> shell_commands = parse_command_string(input_line);
```

This is basically all you need to do for parsing. You just need to execute the
commands listed in `shell_commands` vector to implement the assignment.

### 1.3 Viewing the Parsed Commands


The result of parsing `echo hello world`:

```text
-------------------------
cmd: echo
arg: hello
arg: world
cin_file:
cin_mode: term
cout_file:
cout_mode: terminal
next_mode: always
-------------------------
```

## 2 Executing the parsed commands

### 2.1 Using `execvp()` with C++ Types
```text
cmd = the command to be executed
args = the arguments needed to successfully execute the command
```

```cpp
execvp(cmd, args)
```
## 3 Test

### 3.1 Executing commands inside the osh shell

```cpp
osh> echo hello world
hello world
osh> ls
main.cpp
command.cpp
command.hpp
osh> eco
could not execute command
```

### 3.2 Executing commands from outside the shell through redirection

```cpp
./sshrestha11_pa2> ./osh -t < 6.singlePipe.txt > answer6.txt
./sshrestha11_pa2> diff answer6.txt ea6.txt

./sshrestha11_pa2> ./osh -t < 7.moreLogical.txt >& answer7.txt
./sshrestha11_pa2> diff answer7.txt ea7.txt
```

## 4 Problems

### 4.1 Creating multiple pipes

```text
For this assignment, our shell needs to handle multiple pipes. It is essential that we create pipe for each pipe passed in via command. But creating and tracking the status of pipes is very tricky and challenging. The best way is to store all the created pipes into an array.
```
### 4.2 Bad file descriptor

 ```text
 When using multiple pipes, the shell gave us the bad file descriptor error. This meant that our shell was not closing the pipes in a proper way. The fix for this was to close each end of the pipe immediately after being used.
 ```

## 5 Learned in Assignment 2

### 5.1

```text
Learned the concept of pipes
Learned all about the STD_IN and STD_OUT
Learned about pipe redirection
Learned about dup2
Learned a more about programming in cpp, which was hectic but fun!!!
```
