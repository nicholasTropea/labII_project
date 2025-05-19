#ifndef UTILITIES_H
#define UTILITIES_H

#include <stdbool.h>
#include <stdio.h>

void errorAndExit(const char*, ...);
void handleWithFileError(char*, FILE*);
bool validateArguments(int, char**);

#endif