#include "stdio.h"
#include "opcodes.h"
#include "classfile.h"
#include "reader.h"
#include <stdio.h>
#include <string.h>

#define  T_CLASS        0x02
#define  T_BOOLEAN      0x04
#define  T_CHAR         0x05
#define  T_FLOAT        0x06
#define  T_DOUBLE       0x07
#define  T_BYTE         0x08
#define  T_SHORT        0x09
#define  T_INT          0x0a
#define  T_LONG         0x0b

#define OPCODE_COUNT        0xcd
#define OPCODE_SYMBOL_LEN   16

typedef void (*opc_func) (u1 code);

struct ByteCode
{
    u1 opcode;
    u2 len;
    u1 symbol[OPCODE_SYMBOL_LEN];
    opc_func func;
};

ClassFile *m_cf = NULL;
FILE *m_fp = NULL;
Reader *m_r = NULL;

void opc_noparam(u1 code);
void opc_print(u1 code);

void opc_priv(u1 code);

void opc_wide(u1 code);

void opc_1out(u1 code);
void opc_2out(u1 code);

void opc_tableswitch(u1 code);
void opc_lookupswitch(u1 code);

void opc_newarray(u1 code);

void opc_anewarray(u1 code);

void opc_sipush(u1 code);

void opc_bipush(u1 code);

void opc_index(u1 code);

void opc_2index(u1 code);

void opc_3index(u1 code);
void opc_4index(u1 code);

void opc_2lp(u1 code);
void opc_4lp(u1 code);
void opc_return(u1 code);


ByteCode bytecodes[OPCODE_COUNT] =
{
    {0x00, 1,    "nop",                opc_noparam  },
    {0x01, 1,    "aconst_null",        0  },
    {0x02, 1,    "iconst_m1",          0  },
    {0x03, 1,    "iconst_0",           0  },
    {0x04, 1,    "iconst_1",           0  },
    {0x05, 1,    "iconst_2",           0  },
    {0x06, 1,    "iconst_3",           0  },
    {0x07, 1,    "iconst_4",           0  },
    {0x08, 1,    "iconst_5",           0  },
    {0x09, 1,    "lconst_0",           0  },
    {0x0a, 1,    "lconst_1",           0  },
    {0x0b, 1,    "fconst_0",           0  },
    {0x0c, 1,    "fconst_1",           0  },
    {0x0d, 1,    "fconst_2",           0  },
    {0x0e, 1,    "dconst_0",           0  },
    {0x0f, 1,    "dconst_1",           0  },
    {0x10, 2,    "bipush",             opc_bipush  },
    {0x11, 3,    "sipush",             opc_sipush  },
    {0x12, 2,    "ldc",                opc_index  },
    {0x13, 3,    "ldc2",               opc_2index  },
    {0x14, 3,    "ldc2w",              opc_2index  },
    {0x15, 2,    "iload",              opc_1out  },
    {0x16, 2,    "lload",              opc_1out  },
    {0x17, 2,    "fload",              opc_1out  },
    {0x18, 2,    "dload",              opc_1out  },
    {0x19, 2,    "aload",              opc_1out  },
    {0x1a, 1,    "iload_0",            0  },
    {0x1b, 1,    "iload_1",            0  },
    {0x1c, 1,    "iload_2",            0  },
    {0x1d, 1,    "iload_3",            0  },
    {0x1e, 1,    "lload_0",            0  },
    {0x1f, 1,    "lload_1",            0  },
    {0x20, 1,    "lload_2",            0  },
    {0x21, 1,    "lload_3",            0  },
    {0x22, 1,    "fload_0",            0  },
    {0x23, 1,    "fload_1",            0  },
    {0x24, 1,    "fload_2",            0  },
    {0x25, 1,    "fload_3",            0  },
    {0x26, 1,    "dload_0",            0  },
    {0x27, 1,    "dload_1",            0  },
    {0x28, 1,    "dload_2",            0  },
    {0x29, 1,    "dload_3",            0  },
    {0x2a, 1,    "aload_0",            0  },
    {0x2b, 1,    "aload_1",            0  },
    {0x2c, 1,    "aload_2",            0  },
    {0x2d, 1,    "aload_3",            0  },
    {0x2e, 1,    "iaload",             0  },
    {0x2f, 1,    "laload",             0  },
    {0x30, 1,    "faload",             0  },
    {0x31, 1,    "daload",             0  },
    {0x32, 1,    "aaload",             0  },
    {0x33, 1,    "baload",             0  },
    {0x34, 1,    "caload",             0  },
    {0x35, 1,    "saload",             0  },
    {0x36, 2,    "istore",             opc_1out  },
    {0x37, 2,    "lstore",             opc_1out  },
    {0x38, 2,    "fstore",             opc_1out  },
    {0x39, 2,    "dstore",             opc_1out  },
    {0x3a, 2,    "astore",             opc_1out  },
    {0x3b, 1,    "istore_0",           0  },
    {0x3c, 1,    "istore_1",           0  },
    {0x3d, 1,    "istore_2",           0  },
    {0x3e, 1,    "istore_3",           0  },
    {0x3f, 1,    "lstore_0",           0  },
    {0x40, 1,    "lstore_1",           0  },
    {0x41, 1,    "lstore_2",           0  },
    {0x42, 1,    "lstore_3",           0  },
    {0x43, 1,    "fstore_0",           0  },
    {0x44, 1,    "fstore_1",           0  },
    {0x45, 1,    "fstore_2",           0  },
    {0x46, 1,    "fstore_3",           0  },
    {0x47, 1,    "dstore_0",           0  },
    {0x48, 1,    "dstore_1",           0  },
    {0x49, 1,    "dstore_2",           0  },
    {0x4a, 1,    "dstore_3",           0  },
    {0x4b, 1,    "astore_0",           0  },
    {0x4c, 1,    "astore_1",           0  },
    {0x4d, 1,    "astore_2",           0  },
    {0x4e, 1,    "astore_3",           0  },
    {0x4f, 1,    "iastore",            0  },
    {0x50, 1,    "lastore",            0  },
    {0x51, 1,    "fastore",            0  },
    {0x52, 1,    "dastore",            0  },
    {0x53, 1,    "aastore",            0  },
    {0x54, 1,    "bastore",            0  },
    {0x55, 1,    "castore",            0  },
    {0x56, 1,    "sastore",            0  },
    {0x57, 1,    "pop",                0  },
    {0x58, 1,    "pop2",               0  },
    {0x59, 1,    "dup",                0  },
    {0x5a, 1,    "dup_x1",             0  },
    {0x5b, 1,    "dup_x2",             0  },
    {0x5c, 1,    "dup2",               0  },
    {0x5d, 1,    "dup2_x1",            0  },
    {0x5e, 1,    "dup2_x2",            0  },
    {0x5f, 1,    "swap",               0  },
    {0x60, 1,    "iadd",               0  },
    {0x61, 1,    "ladd",               0  },
    {0x62, 1,    "fadd",               0  },
    {0x63, 1,    "dadd",               0  },
    {0x64, 1,    "isub",               0  },
    {0x65, 1,    "lsub",               0  },
    {0x66, 1,    "fsub",               0  },
    {0x67, 1,    "dsub",               0  },
    {0x68, 1,    "imul",               0  },
    {0x69, 1,    "lmul",               0  },
    {0x6a, 1,    "fmul",               0  },
    {0x6b, 1,    "dmul",               0  },
    {0x6c, 1,    "idiv",               0  },
    {0x6d, 1,    "ldiv",               0  },
    {0x6e, 1,    "fdiv",               0  },
    {0x6f, 1,    "ddiv",               0  },
    {0x70, 1,    "irem",               0  },
    {0x71, 1,    "lrem",               0  },
    {0x72, 1,    "frem",               0  },
    {0x73, 1,    "drem",               0  },
    {0x74, 1,    "ineg",               0  },
    {0x75, 1,    "lneg",               0  },
    {0x76, 1,    "fneg",               0  },
    {0x77, 1,    "dneg",               0  },
    {0x78, 1,    "ishl",               0  },
    {0x79, 1,    "lshl",               0  },
    {0x7a, 1,    "ishr",               0  },
    {0x7b, 1,    "lshr",               0  },
    {0x7c, 1,    "iushr",              0  },
    {0x7d, 1,    "lushr",              0  },
    {0x7e, 1,    "iand",               0  },
    {0x7f, 1,    "land",               0  },
    {0x80, 1,    "ior",                0  },
    {0x81, 1,    "lor",                0  },
    {0x82, 1,    "ixor",               0  },
    {0x83, 1,    "lxor",               0  },
    {0x84, 3,    "iinc",               opc_2out  },
    {0x85, 1,    "i2l",                0  },
    {0x86, 1,    "i2f",                0  },
    {0x87, 1,    "i2d",                0  },
    {0x88, 1,    "l2i",                0  },
    {0x89, 1,    "l2f",                0  },
    {0x8a, 1,    "l2d",                0  },
    {0x8b, 1,    "f2i",                0  },
    {0x8c, 1,    "f2l",                0  },
    {0x8d, 1,    "f2d",                0  },
    {0x8e, 1,    "d2i",                0  },
    {0x8f, 1,    "d2l",                0  },
    {0x90, 1,    "d2f",                0  },
    {0x91, 1,    "int2byte",           0  },
    {0x92, 1,    "int2char",           0  },
    {0x93, 1,    "int2short",          0  },
    {0x94, 1,    "lcmp",               0  },
    {0x95, 1,    "fcmpl",              0  },
    {0x96, 1,    "fcmpg",              0  },
    {0x97, 1,    "dcmpl",              0  },
    {0x98, 1,    "dcmpg",              0  },
    {0x99, 3,    "ifeq",               opc_2lp  },
    {0x9a, 3,    "ifne",               opc_2lp  },
    {0x9b, 3,    "iflt",               opc_2lp  },
    {0x9c, 3,    "ifge",               opc_2lp  },
    {0x9d, 3,    "ifgt",               opc_2lp  },
    {0x9e, 3,    "ifle",               opc_2lp  },
    {0x9f, 3,    "if_icmpeq",          opc_2lp  },
    {0xa0, 3,    "if_icmpne",          opc_2lp  },
    {0xa1, 3,    "if_icmplt",          opc_2lp  },
    {0xa2, 3,    "if_icmpge",          opc_2lp  },
    {0xa3, 3,    "if_icmpgt",          opc_2lp  },
    {0xa4, 3,    "if_icmple",          opc_2lp  },
    {0xa5, 3,    "if_acmpeq",          opc_2lp  },
    {0xa6, 3,    "if_acmpne",          opc_2lp  },
    {0xa7, 3,    "goto_",              opc_2lp  },
    {0xa8, 3,    "jsr",                opc_2lp  },
    {0xa9, 2,    "ret",                opc_1out  },
    {0xaa, 99,   "tableswitch",        opc_tableswitch  },
    {0xab, 99,   "lookupswitch",       opc_lookupswitch  },
    {0xac, 1,    "ireturn",            0  },
    {0xad, 1,    "lreturn",            opc_return  },
    {0xae, 1,    "freturn",            opc_return  },
    {0xaf, 1,    "dreturn",            opc_return  },
    {0xb0, 1,    "areturn",            opc_return  },
    {0xb1, 1,    "return",             0  },
    {0xb2, 3,    "getstatic",          opc_2index  },
    {0xb3, 3,    "putstatic",          opc_2index  },
    {0xb4, 3,    "getfield",           opc_2index  },
    {0xb5, 3,    "putfield",           opc_2index  },
    {0xb6, 3,    "invokevirtual",      opc_2index  },
    {0xb7, 3,    "invokespecial",      opc_2index  },
    {0xb8, 3,    "invokestatic",       opc_2index  },
    {0xb9, 5,    "invokeinterface",    opc_4index  },
    {0xba, 0,    "invokedynamic",      0  },
    {0xbb, 3,    "new_",               opc_2index  },
    {0xbc, 2,    "newarray",           opc_newarray  },
    {0xbd, 3,    "anewarray",          opc_anewarray  },
    {0xbe, 1,    "arraylength",        0  },
    {0xbf, 1,    "athrow",             0  },
    {0xc0, 3,    "checkcast",          opc_2index  },
    {0xc1, 3,    "instanceof_",        opc_2index  },
    {0xc2, 1,    "monitorenter",       0  },
    {0xc3, 1,    "monitorexit",        0  },
    {0xc4, 0,    "",               opc_wide  },
    {0xc5, 4,    "multianewarray",     opc_3index  },
    {0xc6, 3,    "if_acmp_null",       opc_2lp  },
    {0xc7, 3,    "if_acmp_nonnull",    opc_2lp  },
    {0xc8, 5,    "goto_w",             opc_4lp  },
    {0xc9, 5,    "jsr_w",              opc_4lp  },
    {0xca, 5,    "breakpoint",         0  }
};

const char *g_tagstr[14] =
{
    0,
    "Utf8",
    0,
    "Int",
    "Float",
    "Long",
    "Double",
    "Class",
    "String",
    "Field",
    "Method",
    "Interface",
    "Namd"
};

u1* GetJavaClassName(ClassFile *cf)
{
    if (cf->this_class == 0)
    {
        return NULL;
    }
    if (cf->cpools[cf->this_class].tag == CONSTANT_CLASS)
    {
        int tcpx = cf->cpools[cf->this_class].ci.name_index;
        return cf->cpools[tcpx].utfi.bytes;
    }
    return NULL;
}

void PrintClassName(int index, ClassFile *cf, FILE *fp)
{
    if (cf->cpools[index].tag == CONSTANT_UTF8)
    {
        char *p = strchr((char*)cf->cpools[index].utfi.bytes, '.');
        if (p)
        {
            ++p;
        }
        else
        {
            p = (char*)cf->cpools[index].utfi.bytes;
        }
        fprintf(fp, "%s", p);
    }
}

int GetStringValue(int cpx, ClassFile *cf, FILE *fp)
{
    if (cpx <0 || cpx >= m_cf->cpools_count)
    {
        return 0;
    }
    int tag = m_cf->cpools[cpx].tag;
    switch (tag)
    {
    case CONSTANT_UTF8:
        fprintf(fp, "%s", cf->cpools[cpx].utfi.bytes);
        break;
    case CONSTANT_DOUBLE:
        fprintf(fp, "%d%dd", cf->cpools[cpx].di.high_bytes,
            cf->cpools[cpx].di.low_bytes);
        break;
    case CONSTANT_FLOAT:
        fprintf(fp, "%df", cf->cpools[cpx].fi.bytes);
        break;
    case CONSTANT_LONG:
        fprintf(fp, "%d%dd", cf->cpools[cpx].li.high_bytes,
            cf->cpools[cpx].li.low_bytes);
        break;
    case CONSTANT_INTEGER:
        fprintf(fp, "%d", cf->cpools[cpx].ii.bytes);
        break;
    case CONSTANT_CLASS:
        PrintClassName(cf->cpools[cpx].ci.name_index, cf, fp);
        break;
    case CONSTANT_STRING:
        PrintClassName(cf->cpools[cpx].ci.name_index, cf, fp);
        break;
    case CONSTANT_METHOD:
    case CONSTANT_FIELD:
    case CONSTANT_INTERFACEMETHOD:
        if (cf->cpools[cpx].mi.class_index != cf->this_class)
        {
            GetStringValue(cf->cpools[cpx].mi.class_index, cf, fp);
            fprintf(fp, ".");
        }
        GetStringValue(m_cf->cpools[cpx].mi.name_type_index, cf, fp);
        break;
    case CONSTANT_NAMEANDTYPE:
        GetStringValue(cf->cpools[cpx].nti.name_index, cf, fp);
        fprintf(fp, ":");
        GetStringValue(cf->cpools[cpx].nti.descriptor_index, cf, fp);
        break;
    }
    return 0;
}

void PrintConstant(int cpx)
{
    if (cpx == 0)
    {
        fprintf(m_fp, "#0");
        return;
    }
    if (cpx >= m_cf->cpools_count)
    {
        fprintf(m_fp, "#%d", cpx);
        return;
    }
    u1 tag = m_cf->cpools[cpx].tag;
    switch (tag)
    {
    case CONSTANT_METHOD:
    case CONSTANT_INTERFACEMETHOD:
    case CONSTANT_FIELD:
        if (m_cf->cpools[cpx].mi.class_index == m_cf->cpools[m_cf->this_class].mi.class_index)
        {
            cpx = m_cf->cpools[cpx].mi.name_type_index;
        }
        break;
    default:
        break;
    }
    fprintf(m_fp, "%s ", g_tagstr[tag]);
    GetStringValue(cpx, m_cf, m_fp);
}

void opc_print(u1 code)
{
    fprintf(m_fp, "%s ", bytecodes[code].symbol);
}

void opc_noparam(u1 code)
{
    fprintf(m_fp, "%s", bytecodes[code].symbol);
}

void opc_priv(u1 code)
{
    u1 op2 = m_r->ReadU1();
    u1 op = op2+(bytecodes[code].opcode<<8);
    if (op >= OPCODE_COUNT)
    {
        fprintf(m_fp, "%s %d", bytecodes[code].symbol, op2);
    }
    else
    {
        fprintf(m_fp, "%s", bytecodes[op].symbol);
    }
}

void opc_wide(u1 code)
{
    u1 op2 = m_r->ReadU1();
    u1 op = op2+(bytecodes[code].opcode<<8);
    if (op >= OPCODE_COUNT)
    {
        fprintf(m_fp, " %d: bytecode %d", op, code);
    }
    else
    {
        fprintf(m_fp, "%s_w\t%d", bytecodes[op].symbol, m_r->ReadU2());
        if (op2 == 0x84)
        {
            fprintf(m_fp, ", %d", m_r->ReadU2());
        }
    }
}

void opc_1out(u1 code)
{
    fprintf(m_fp, "\t%d", m_r->ReadU1());
}
void opc_2out(u1 code)
{
    fprintf(m_fp, "%hu, %hd", m_r->ReadU1(), m_r->ReadU1());
}

int align(int n)
{
    return (n+3) & ~3;
}

void opc_tableswitch(u1 code)
{
    int pc = m_r->GetCurrentOffset()-1;
    int tb = align(pc);
    int default_skip = m_r->GetOffsetU4(tb);
    int low = m_r->GetOffsetU4(tb+4);
    int high = m_r->GetOffsetU4(tb+8);
    int tcount = high - low;

    fprintf(m_fp, "{ //%d to %d", low, high);
    for (int i = 0; i <= tcount; i++)
    {
        fprintf(m_fp, "\n\t\t%d: %d;", i+low, pc+m_r->GetOffsetU4(tb+12+4*i));
    }
    fprintf(m_fp, "\n\t\tdefault: %d }", default_skip+pc);

    int offset = tb-pc-1+16+tcount*4;
    if (offset < 0)
    {
        offset = m_r->GetBufLength();
    }
    m_r->AddOffset(offset);
}

void opc_lookupswitch(u1 code)
{
    int pc = m_r->GetCurrentOffset()-1;
    int tb = align(pc);
    int default_skip = m_r->GetOffsetU4(tb);
    int npairs = m_r->GetOffsetU4(tb+4);
    fprintf(m_fp, "{ //%d", npairs);
    for (int i = 1; i <= npairs; i++)
    {
        fprintf(m_fp, "\n\t\t%d: %d;",
            m_r->GetOffsetU4(tb+i*8),
            pc+m_r->GetOffsetU4(tb+4+i*8));
    }
    fprintf(m_fp, "\n\tdefault: %d }", default_skip+pc);
    int offset = tb-pc-1+(npairs+1)*8;
    if (offset < 0)
    {
        offset = m_r->GetBufLength();
    }
    m_r->AddOffset(offset);
}

void opc_newarray(u1 code)
{
    u1 index = m_r->ReadU1();
    switch (index)
    {
    case T_BOOLEAN:fprintf(m_fp, " boolean"); break;
    case T_BYTE:   fprintf(m_fp, " byte");    break;
    case T_CHAR:   fprintf(m_fp, " char");    break;
    case T_SHORT:  fprintf(m_fp, " short");   break;
    case T_INT:    fprintf(m_fp, " int");     break;
    case T_LONG:   fprintf(m_fp, " long");    break;
    case T_FLOAT:  fprintf(m_fp, " float");   break;
    case T_DOUBLE: fprintf(m_fp, " double");  break;
    case T_CLASS:  fprintf(m_fp, " class");   break;
    default:
        fprintf(m_fp, " BOGUS TYPE: %x", index);
        break;
    }
}

void opc_anewarray(u1 code)
{
    int index = m_r->ReadU2();
    fprintf(m_fp, "\t#%d; //", index);
    PrintConstant(index);
}

void opc_sipush(u1 code)
{
    fprintf(m_fp, "%hd", m_r->ReadU2());
}

void opc_bipush(u1 code)
{
    fprintf(m_fp, "\t%d", m_r->ReadU1());
}

void opc_index(u1 code)
{
    u1 index = m_r->ReadU1();
    fprintf(m_fp, "\t#%d\t//", index);
    PrintConstant(index);
}


void opc_2index(u1 code)
{
    u2 index = m_r->ReadU2();
    fprintf(m_fp, "\t#%d; //", index);
    PrintConstant(index);
}

void opc_3index(u1 code)
{
    u2 index = m_r->ReadU2();
    u1 index2 = m_r->ReadU1();
    fprintf(m_fp, "\t#%d, %d; //", index, index2);
    PrintConstant(index);
}

void opc_4index(u1 code)
{
    u2 index = m_r->ReadU2();
    u1 index2 = m_r->ReadU1();
    fprintf(m_fp, "\t#%d, %d; //", index, index2);
    PrintConstant(index);
    m_r->AddOffset(1);
}

void opc_2lp(u1 code)
{
    int pc = m_r->GetCurrentOffset()-1;
    fprintf(m_fp, "\t%hu", m_r->ReadU2()+pc);
}
void opc_4lp(u1 code)
{
    int pc = m_r->GetCurrentOffset()-1;
    fprintf(m_fp, "\t%hu", m_r->ReadU4()+pc);
}

void opc_return(u1 code)
{
    m_r->SetOffset(m_r->GetBufLength());
}



void InitOpcode(ClassFile *cf, FILE *fp)
{
    m_cf = cf;
    m_fp = fp;
    m_r = new Reader();
}

void SetMethodCodes(CodeAttr *ca)
{
    m_r->Reset(ca->code, ca->code_len);
}

void FreeOpcode()
{
    m_cf = NULL;
    m_fp = NULL;
    if (m_r)
    {
        delete m_r;
        m_r = NULL;
    }
}

void PrintUtfIndex(int index, ClassFile *cf, FILE *fp)
{
    if (cf->cpools[index].tag == CONSTANT_UTF8)
    {
        fprintf(fp, "%s", cf->cpools[index].utfi.bytes);
    }
}




int PrintCode()
{
    fprintf(m_fp, "       %d: ", m_r->GetCurrentOffset());
    u1 opcode = m_r->ReadU1();
    if (opcode >= 0 && opcode <= OPCODE_COUNT)
    {
        fprintf(m_fp, "%s", bytecodes[opcode].symbol);
        if (bytecodes[opcode].func)
        {
            bytecodes[opcode].func(opcode);
        }
        fprintf(m_fp, "\n");
    }
    return m_r->GetCurrentOffset();
}
