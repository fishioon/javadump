#include "classdump.h"
#include "opcodes.h"
#include "reader.h"
#include <stdio.h>
#include <set>
#include <string>
#include <string.h>
using namespace std;


int DumpCodeAssembly(CodeAttr *ca, FILE *fp)
{
    fprintf(fp, "    Code: \n");
    SetMethodCodes(ca);
    while(PrintCode() < ca->code_len){}
    fprintf(fp, "\n");
    return 0;
}

void PrintAccess(u2 flags, FILE *fp)
{
    fprintf(fp, "  ");
    if(flags & ACC_PUBLIC)          fprintf(fp, "public ");
    if(flags & ACC_PRIVATE)         fprintf(fp, "private ");
    if(flags & ACC_PROTECTED)       fprintf(fp, "protected ");
    if(flags & ACC_STATIC)          fprintf(fp, "static ");
    if(flags & ACC_FINAL)           fprintf(fp, "final ");
    if(flags & ACC_SYNCHRONIZED)    fprintf(fp, "synchronized ");
    if(flags & ACC_NATIVE)          fprintf(fp, "native ");
    if(flags & ACC_ABSTRACT)        fprintf(fp, "abstract ");
    if(flags & ACC_STRICT)          fprintf(fp, "strictfp ");
}

void GetJavaName(int index, ClassFile *cf, FILE *fp)
{
    if (cf->cpools[index].tag == CONSTANT_UTF8)
    {
        u1 ch;
        for (int i = 0; i < cf->cpools[index].utfi.length; i++)
        {
            ch = cf->cpools[index].utfi.bytes[i];
            if (ch == '/')
            {
                ch = '.';
            }
            fprintf(fp, "%c", ch);
        }
    }
}

void PrintImport(ClassFile *cf, FILE *fp)
{
    set< string > classset;
    char buf[256] = {0};
    int class_index = cf->cpools[cf->this_class].ci.name_index;
    
    memcpy(buf, cf->cpools[class_index].utfi.bytes, cf->cpools[class_index].utfi.length);
    char *t = strrchr(buf, '.');
    if (t)
    {
        *t = '\0';
    }
    fprintf(fp, "package %s;\n", buf);
    
    
    for (int i = 0; i < cf->cpools_count; i++)
    {
        if (cf->cpools[i].tag == CONSTANT_CLASS)
        {
            if (cf->cpools[i].ci.name_index != class_index)
            {
                int len = cf->cpools[cf->cpools[i].ci.name_index].utfi.length;
                if (len <= 0 && len > 255)
                {
                    return;
                }
                unsigned char ch = cf->cpools[cf->cpools[i].ci.name_index].utfi.bytes[0];
                if (ch >= 'a' && ch <= 'z' &&
                    strchr((char*)cf->cpools[cf->cpools[i].ci.name_index].utfi.bytes, '/'))
                {
                    //memset(buf, 0, _countof(buf));
                    memcpy(buf, cf->cpools[cf->cpools[i].ci.name_index].utfi.bytes, len);
                    buf[len] = '\0';
                    char *p = buf;
                    while (*p != '\0')
                    {
                        if (*p == '/')
                        {
                            *p = '.';
                        }
                        ++p;
                    }
                    classset.insert(buf);
                }
            }
        }
    }
    set< string >::iterator it;
    for (it = classset.begin(); it != classset.end(); ++it)
    {
        fprintf(fp, "import %s;\n", (*it).c_str());
    }
}

void PrintClassHeader(ClassFile *cf, FILE *fp)
{
    PrintImport(cf, fp);
    if (cf->src_index > 0)
    {
        fprintf(fp, "Compiled from ");
        GetJavaName(cf->src_index, cf, fp);
        fprintf(fp, "\n");
    }
    int flags = cf->access_flags;
    if(flags & ACC_PUBLIC)       fprintf(fp, "public ");
    //if(flags & ACC_PRIVATE)      fprintf(fp, "private ");
    //if(flags & ACC_PROTECTED)    fprintf(fp, "protected ");

    int tcpx = 0;
    if (flags & ACC_INTERFACE)
    {
        fprintf(fp, "interface ");
    }
    else
    {
        if (flags & ACC_FINAL) fprintf(fp, "final ");
        if (flags & ACC_ABSTRACT) fprintf(fp, "abstract ");
        fprintf(fp, "class ");
        //fprintf(fp, "class %.*s", cf->cpool[tcpx].utfi.length, cf->cpool[tcpx].utfi.bytes);
        //if (cf->super_class)
        //{
        //    tcpx = cf->cpool[cf->super_class].classinfo.name_index;
        //    fprintf(fp, "extends %.*s\n", cf->cpool[tcpx].utfi.length, cf->cpool[tcpx].utfi.bytes);
        //}
    }
    //GetJavaName(cf->cpools[cf->this_class].ci.name_index, cf, fp);
    fprintf(fp, "%s", cf->cpools[cf->cpools[cf->this_class].ci.name_index].utfi.bytes);
    if (cf->interfaces_count)
    {
        if (flags & ACC_INTERFACE)
        {
            fprintf(fp, " extends ");
        }
        else
        {
            fprintf(fp, " implements ");
        }
        
        for (int i = 0; i < cf->interfaces_count; i++)
        {
            GetJavaName(cf->cpools[cf->interfaces[i]].ci.name_index, cf, fp);
        }
    }
    fprintf(fp, " {\n");
}
void PrintFields(ClassFile *cf, FILE *fp)
{
    for (int i = 0; i < cf->fields_count; i++)
    {
        int flags = cf->fields[i].access_flags;

        PrintUtfIndex(cf->fields[i].name_index, cf, fp);
        fprintf(fp, "\n");
    }
}

void PrintMethodParameters(u1 *buf, int len, FILE *fp)
{
    if (!buf || len <= 0)
    {
        return;
    }
    u1 ch;
    int i = 0;
    while (buf[i] != ')' && i < len)
    {
        ch = buf[i++];
        if (ch == '(')
        {
            fprintf(fp, "(");
            ch = buf[i++];
            if (ch == ')')
            {
                fprintf(fp, ")");
                return;
            }
        }
        else
        {
            fprintf(fp, ", ");
        }
        switch(ch)
        {
        case 'B':
            fprintf(fp, "byte");
            break;

        case 'C':
            fprintf(fp, "char");
            break;

        case 'D':
            fprintf(fp, "double");
            break;

        case 'F':
            fprintf(fp, "float");
            break;

        case 'I':
            fprintf(fp, "int");
            break;

        case 'J':
            fprintf(fp, "long");
            break;

        case 'S':
            fprintf(fp, "short");
            break;

        case 'Z':
            fprintf(fp, "bool");
            break;

        case 'L':
            
            while(buf[i] != ';' && buf[i] != '\0')
            {
                if (buf[i] == '/')
                {
                    fprintf(fp, ".");
                }
                else
                {
                    fprintf(fp, "%c", buf[i]);
                }
                i++;
            }
            ++i;
            break;

        case '[':
            fprintf(fp, "[]");
            break;
        default:
            break;
        }
    }
    fprintf(fp, ")");
}
void ParseMethodReturn(u1 *buf, int len, FILE *fp)
{
    if (!buf || len <= 0)
    {
        return;
    }
    int i = 0;
    char retType[512] = {0};
    while(buf[i++] != '(');
    while(buf[i] != '\0' && i < len) 
    {
        switch(buf[i++]) 
        {
        case 'V':
            sprintf(retType, "void");
            break;

        case 'B':
            sprintf(retType, "byte");
            break;

        case 'C':
            sprintf(retType, "char");
            break;

        case 'D':
            sprintf(retType, "double");
            break;

        case 'F':
            sprintf(retType, "float");
            break;

        case 'I':
            sprintf(retType, "int");
            break;

        case 'J':
            sprintf(retType, "long");
            break;

        case 'S':
            sprintf(retType, "short");
            break;

        case 'Z':
            sprintf(retType, "bool");
            break;

        case 'L':
            while(buf[i] != ';')
            {
                if (buf[i] == '/')
                {
                    retType[strlen(retType)] = '.';
                }
                else
                {
                    retType[strlen(retType)] = buf[i];
                }
                i++;
            }
            ++i;
            break;

        case '[':
            strcat(retType, "[]");
            break;
        default:
            break;
        }
    }
    fprintf(fp, "%s", retType);
}

void ParseMethodSignature(MethodInfo *mi, ClassFile *cf, FILE *fp)
{
    u1 *buf = cf->cpools[mi->descriptor_index].utfi.bytes;
    int len = cf->cpools[mi->descriptor_index].utfi.length;
    ParseMethodReturn(buf, len, fp);
    fprintf(fp, " %s", cf->cpools[mi->name_index].utfi.bytes);
    PrintMethodParameters(buf, len, fp);
}

void PrintMethods(ClassFile *cf, FILE *fp)
{
    for (int i = 0; i < cf->methods_count; i++)
    {
        PrintAccess(cf->methods[i].access_flags, fp);
        char *tmpstr = (char *)cf->cpools[cf->methods[i].name_index].utfi.bytes;
        int len = cf->cpools[cf->methods[i].name_index].utfi.length;
        if (strncmp(tmpstr, "<init>", len) == 0)
        {
            GetJavaName(cf->cpools[cf->this_class].ci.name_index, cf, fp);
            PrintMethodParameters(cf->cpools[cf->methods[i].descriptor_index].utfi.bytes,
                cf->cpools[cf->methods[i].descriptor_index].utfi.length, fp);
        }
        else if (strncmp(tmpstr, "<clinit>", len) == 0)
        {
            fprintf(fp, "{}");
        }
        else
        {
            ParseMethodSignature(&cf->methods[i], cf, fp);
        }
        fprintf(fp, "\n");
        for (int j = 0; j < cf->methods[i].attributes_count; j++)
        {
            if (cf->methods[i].attributes[j].tag == TAG_CODE)
            {
                DumpCodeAssembly(&cf->methods[i].attributes[j].code, fp);
            }
        }
    }
}

int CF_DumpClassFile(ClassFile *cf, FILE *fp)
{
    InitOpcode(cf, fp);
    
    PrintClassHeader(cf, fp);
    //PrintFields(cf,fp);
    PrintMethods(cf, fp);
    fprintf(fp, "}\n");
    FreeOpcode();
    return 0;
}