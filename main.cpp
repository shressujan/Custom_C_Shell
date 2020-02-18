/*
 * Copyright (c) 2018, Yutaka Tsutano
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include <iostream>
#include <string.h>
#include <vector>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fstream>

#include "command.hpp"
#include "parser.hpp"

using namespace std;

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

int status;
int counter = 0;
int pipefd[100][2];

void executeCommand(shell_command command_struct) {

  pid_t pid;
  counter++;

  vector<string> args = command_struct.args;
  string command = command_struct.cmd;
  if(command == "ls") {
    args.insert(args.begin(), "-1");
  }
  args.insert(args.begin(), command);

  vector<char*> arglist {};
  for (auto& arg : args)
    arglist.push_back(&arg.front());

  arglist.push_back(NULL);

  //piping
  //&& command_struct.cin_mode != istream_mode::pipe
  if(command_struct.cout_mode == ostream_mode::pipe) {

    if (pipe(pipefd[counter]) < 0) {
      printf("\nPipe could not be initialized");
      return;
    }
  }

  //Forking for new child process
  pid = fork();

  if (pid == -1) {
    cout << "\n Failed forking child.." << endl;
    exit(EXIT_FAILURE);
  }
  else if (pid == 0) {

    int file_desc;

    // redirection for reading from a file
    if(command_struct.cin_file.size() > 0 ) {
      string filename = command_struct.cin_file;
      file_desc = open(filename.c_str(), O_RDONLY);
      dup2(file_desc, 0);
      close(file_desc);
    }

    // redireciton for writing or appending to file
    if(command_struct.cout_file.size() > 0) {
      string filename = command_struct.cout_file;
      //Append to the file
      if(command_struct.cout_mode == ostream_mode::append) {
        file_desc = open(filename.c_str(), O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
      }
      //creates a new file everytime
      else {
        file_desc = open(filename.c_str(), O_WRONLY | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
      }
      dup2(file_desc, 1);
      close(file_desc);
    }

    /* if the current command is related to previous command */
    /* Reading from the pipe created earlier*/
    if(command_struct.cin_mode == istream_mode::pipe) {
      close(pipefd[counter-1][1]); /* Close unused write end */
      dup2(pipefd[counter-1][0], STDIN_FILENO);
      close(pipefd[counter-1][0]);
    }

    /*Child writes the current command to the newly created pipe*/
    if (command_struct.cout_mode == ostream_mode::pipe) {
      close(pipefd[counter][0]); /* Close unused read end */
      dup2(pipefd[counter][1], STDOUT_FILENO);
      // close(pipefd[counter][1]);
    }

    if (execvp(command.c_str(), arglist.data()) < 0) {
      cout << "\n Could not execute command.." << endl;
      exit(EXIT_FAILURE);
    }

    /* Setting file_desc back to zero*/
    file_desc = 0;
  }
  else {
    if(command_struct.cin_mode == istream_mode::pipe) {
      close(pipefd[counter-1][0]);
      close(pipefd[counter-1][1]);
    }
    /* Don't wait for the child to exit incase of writing to a pipe, else will result in a hang*/
    if (command_struct.cout_mode == ostream_mode::pipe){
      return;
    }
    for (int i = 0; i < counter; i++) {
      wait(&status);
    }


    counter = 0;

    return;
  }
  exit(EXIT_SUCCESS);
}

int main(int argc, char ** argv)
{
    string input_line;
    int flag = 0;
    if(argc > 1) {
      for (size_t i = 0; i < sizeof(argv); i++) {
        if(strcmp(argv[i],"-t")) {
          flag = 1;
          break;
        }
      }
    }

    for (int should_run = 0; should_run < 25; should_run++) {
        status = EXIT_SUCCESS;
        // Print the prompt.
        if(flag == 0) {
          cout << "osh> " << flush;
        }

        // Read a single line.
        if (!getline(cin, input_line) || input_line == "exit") {
            break;
        }

        try {
            // Parse the input line.
            vector<shell_command> shell_commands
                    = parse_command_string(input_line);
            // Print the list of commands.
            // cout << "-------------------------\n";
            // for (const auto& cmd : shell_commands) {
            //     cout << cmd;
            //     cout << "-------------------------\n";
            // }

            for (size_t i = 0; i < shell_commands.size(); i++) {
              if(i > 0) {
                if(shell_commands.at(i - 1).next_mode == next_command_mode::on_success) {
                  if(status == EXIT_SUCCESS) {
                    executeCommand(shell_commands.at(i));
                  }
                }
                else if (shell_commands.at(i - 1).next_mode == next_command_mode::on_fail){
                  if(status != EXIT_SUCCESS) {
                    executeCommand(shell_commands.at(i));
                  }
                }
                else if (shell_commands.at(i - 1).next_mode == next_command_mode::always){
                  executeCommand(shell_commands.at(i));
                }
              }
              else {
                executeCommand(shell_commands.at(i));
              }
            }
        }
        catch (const runtime_error& e) {
            cout << e.what() << "\n";
        }
    }
}
