// Copyright 2024 KASYM-TECH

#include <stdio.h>
#include <windows.h>
#include <cstdio>

#define BUFSIZE 4096

int main(void) {
  CHAR chBuf[BUFSIZE];
  DWORD dwRead, dwWritten;
  HANDLE hStdin, hStdout;
  BOOL bSuccess;

  hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
  hStdin = GetStdHandle(STD_INPUT_HANDLE);
  if ((hStdout == INVALID_HANDLE_VALUE) || (hStdin == INVALID_HANDLE_VALUE))
    ExitProcess(1);

  printf("\n ** This is a message from the child process. ** \n");

  for (;;) {
    bSuccess = ReadFile(hStdin, chBuf, BUFSIZE, &dwRead, NULL);

    if (!bSuccess || dwRead == 0) break;

    bSuccess = WriteFile(hStdout, chBuf, dwRead, &dwWritten, NULL);

    if (!bSuccess) break;
  }
  return 0;
}
