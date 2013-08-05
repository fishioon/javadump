#ifndef OPCODES_H_
#define OPCODES_H_
#include "classfile.h"


void InitOpcode(ClassFile *cf, FILE *fp);
void SetMethodCodes(CodeAttr *ca);
void FreeOpcode();

//return codes offset
int PrintCode();
void PrintUtfIndex(int index, ClassFile *cf, FILE *fp);

#endif


