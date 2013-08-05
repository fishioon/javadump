#ifndef CFDUMP_H_
#define CFDUMP_H_

#include "classfile.h"

int CF_IsJavaClassFile(u1 *buf, size_t len);
int CF_ReadClassFile(u1 *buf, size_t len, size_t *offset, ClassFile *cf);
void CF_FreeClassFile(ClassFile *cf);
int CF_DumpClassFile(ClassFile *cf, FILE *fp);

#endif



