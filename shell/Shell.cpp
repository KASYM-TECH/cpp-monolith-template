// Copyright 2024 KASYM-TECH

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <windows.h>

#include <cstdio>

#define MAX_ARGS 64
#define BUFFER_SIZE 4096

void execute_cd(char* path, HANDLE hOut, HANDLE hErr) {
  if (SetCurrentDirectory(path) == 0) {
    const char* errorMsg = "Error: Cannot change directory.\n";
    DWORD bytesWritten;
    WriteFile(hErr, errorMsg, strlen(errorMsg), &bytesWritten, NULL);
    return;
  }
}

void execute_ls(HANDLE hOut, HANDLE hErr) {
  WIN32_FIND_DATA findFileData;
  HANDLE hFind = FindFirstFile("*", &findFileData);

  if (hFind == INVALID_HANDLE_VALUE) {
    const char* errorMsg = "Error: Unable to list directory.\n";
    DWORD bytesWritten;
    WriteFile(hErr, errorMsg, strlen(errorMsg), &bytesWritten, NULL);
    return;
  }

  do {
    DWORD bytesWritten;
    char outputBuffer[MAX_PATH + 2];
    snprintf(outputBuffer, sizeof(outputBuffer), "%s\n",
             findFileData.cFileName);

    WriteFile(hOut, outputBuffer, strlen(outputBuffer), &bytesWritten, NULL);
  } while (FindNextFile(hFind, &findFileData) != 0);

  FindClose(hFind);
}

void execute_program(char* program, char** args, HANDLE hIn, HANDLE hOut,
                     HANDLE hErr) {
  STARTUPINFO si;
  PROCESS_INFORMATION pi;
  memset(&si, 0, sizeof(si));
  si.cb = sizeof(si);
  si.dwFlags |= STARTF_USESTDHANDLES;
  si.hStdInput = hIn;
  si.hStdOutput = hOut;
  si.hStdError = hErr;

  memset(&pi, 0, sizeof(pi));

  char cmd[1024] = "";
  snprintf(cmd, sizeof(cmd), "%s", program);
  for (int i = 1; args[i] != NULL; i++) {
    snprintf(cmd + strlen(cmd), sizeof(cmd) - strlen(cmd), " %s", args[i]);
  }

  clock_t start = clock();
  if (!CreateProcess(NULL, cmd, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
    printf("Error: Failed to start program %s\n", program);
    return;
  }
  WaitForSingleObject(pi.hProcess, INFINITE);

  clock_t end = clock();
  double time_spent = static_cast<double>(end - start) / CLOCKS_PER_SEC;
  printf("Execution time of %s: %.3f seconds\n", program, time_spent);

  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);
}

void execute_command(char* command, HANDLE hIn, HANDLE hOut, HANDLE hErr) {
  char* args[MAX_ARGS];
  char* saveptr;
  char* token = strtok_r(command, " ", &saveptr);
  int argCount = 0;
  while (token != NULL) {
    args[argCount++] = token;
    token = strtok_r(NULL, " ", &saveptr);
  }
  args[argCount] = NULL;

  if (strcmp(args[0], "cd") == 0) {
    if (argCount > 1) {
      execute_cd(args[1], hOut, hErr);
    }
  } else if (strcmp(args[0], "ls") == 0) {
    execute_ls(hOut, hErr);
  } else if (args[0][0] == '.' && args[0][1] == '/') {
    execute_program(args[0], args, hIn, hOut, hErr);
  } else {
    fprintf(stderr, "Command not recognized: %s\n", args[0]);
  }
}

HANDLE open_file(char* filename, DWORD accessMode, DWORD creationDisposition) {
  SECURITY_ATTRIBUTES sa;
  sa.nLength = sizeof(sa);
  sa.lpSecurityDescriptor = NULL;
  sa.bInheritHandle = TRUE;

  HANDLE fileHandle =
      CreateFileA(filename, accessMode, FILE_SHARE_WRITE, &sa,
                  creationDisposition, FILE_ATTRIBUTE_NORMAL, NULL);
  if (fileHandle == INVALID_HANDLE_VALUE) {
    fprintf(stderr, "Failed to open file: %s!\n", filename);
  }
  return fileHandle;
}

void parse_redirection(char* command, HANDLE* hOut, HANDLE* hErr) {
  char* redirect;
  char* filename;

  while ((redirect = strpbrk(command, "12>"))) {
    int type_of_redirect = 0;
    if (*redirect == '>') {
      type_of_redirect = 1;
      filename = redirect + 1;
    } else if ((*redirect == '1' || *redirect == '2') &&
               *(redirect + 1) == '>') {
      if (*redirect == '1')
        type_of_redirect = 1;
      else
        type_of_redirect = 2;
      filename = redirect + 2;
    } else {
      command = redirect + 1;
      continue;
    }

    while (*filename && isspace(*filename)) {
      filename++;
    }

    char* end = filename;
    while (*end && !isspace(*end)) {
      end++;
    }

    char saved = *end;
    *end = '\0';
    if (type_of_redirect == 1) {
      *hOut = open_file(filename, GENERIC_WRITE, CREATE_ALWAYS);
      if (*hOut == INVALID_HANDLE_VALUE) {
        *hOut = GetStdHandle(STD_OUTPUT_HANDLE);
      }
    } else if (type_of_redirect == 2) {
      *hErr = open_file(filename, GENERIC_WRITE, CREATE_ALWAYS);
      if (*hErr == INVALID_HANDLE_VALUE) {
        *hErr = GetStdHandle(STD_ERROR_HANDLE);
      }
    }
    *end = saved;

    memset(redirect, ' ', end - redirect);
  }
}

void parse_command(char* input, char** commands, int* cmdCount) {
  char* saveptr;
  char* token = strtok_r(input, "|", &saveptr);
  while (token != NULL) {
    commands[(*cmdCount)++] = token;
    token = strtok_r(NULL, "|", &saveptr);
  }
}

void execute_pipeline(char* input) {
  char* commands[64];
  int cmdCount = 0;
  parse_command(input, commands, &cmdCount);

  HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
  HANDLE hOut, hErr;
  HANDLE hPipe[2] = {INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE};

  for (int i = 0; i < cmdCount; i++) {
    hErr = GetStdHandle(STD_ERROR_HANDLE);
    hOut = GetStdHandle(STD_OUTPUT_HANDLE);

    if (i < cmdCount - 1) {
      SECURITY_ATTRIBUTES saAttr;
      saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
      saAttr.bInheritHandle = TRUE;
      saAttr.lpSecurityDescriptor = NULL;
      if (!CreatePipe(&hPipe[0], &hPipe[1], &saAttr, 0))
        printf("StdoutRd CreatePipe");
      hOut = hPipe[1];
    }

    parse_redirection(commands[i], &hOut, &hErr);

    execute_command(commands[i], hIn, hOut, hErr);

    if (hOut != GetStdHandle(STD_OUTPUT_HANDLE)) {
      CloseHandle(hOut);
    }
    if (hErr != GetStdHandle(STD_ERROR_HANDLE)) {
      CloseHandle(hErr);
    }

    if (i < cmdCount - 1) {
      hIn = hPipe[0];
    }
  }

  if (hPipe[0] != INVALID_HANDLE_VALUE) {
    CloseHandle(hPipe[0]);
  }
  if (hPipe[1] != INVALID_HANDLE_VALUE) {
    CloseHandle(hPipe[1]);
  }
}

void print_prompt() {
  char cwd[1024];
  if (GetCurrentDirectory(sizeof(cwd), cwd)) {
    printf("%s$ ", cwd);
  } else {
    printf("$ ");
  }
}

int main() {
  char input[1024];

  while (1) {
    print_prompt();
    fflush(stdout);

    if (!fgets(input, sizeof(input), stdin)) break;
    input[strcspn(input, "\n")] = 0;

    if (strcmp(input, "exit") == 0) {
      break;
    }

    execute_pipeline(input);
  }

  return 0;
}