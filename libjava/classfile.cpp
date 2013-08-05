#include "classfile.h"

#include <string.h>
#include <stdlib.h>
#include "reader.h"

static Reader *m_rd = NULL;
static ClassFile *m_cf = NULL;

static int ReadAnnotation(Annotation *ai);
static int ReadVerifyTypeInfo(VerifyTypeInfo *vi)
{
    int tag = m_rd->ReadU1();
    vi->tag = tag;
    if (tag == 7)
    {
        vi->cpool_index = m_rd->ReadU2();
    }
    else if (tag == 8)
    {
        vi->offset = m_rd->ReadU2();
    }
    else if (tag >= 0 && tag < 7)
    {
        return CF_OK;
    }
    else
    {
        return CF_NOTOK;
    }
    return 0;
}
static int ReadStackMapTable(AttributeInfo *ai)
{
    ai->sma.tables_count = m_rd->ReadU2();
    if (ai->sma.tables_count)
    {
        ai->sma.tables = (StackMapTable*)malloc(ai->sma.tables_count * sizeof(StackMapTable));
        memset(ai->sma.tables, 0, ai->sma.tables_count * sizeof(StackMapTable));
        for (int i = 0; i < ai->sma.tables_count; i++)
        {
            int tt = m_rd->ReadU1();
            ai->sma.tables[i].table_type = tt;
            if (tt <= 63)
            {
                ai->sma.tables[i].offset_delta = tt;
            }
            else if (tt <= 127)
            {
                ai->sma.tables[i].offset_delta = tt - 64;
                ai->sma.tables[i].stacks_count = 1;
                ai->sma.tables[i].stacks = (VerifyTypeInfo*)malloc(
                    sizeof(VerifyTypeInfo));
                ReadVerifyTypeInfo(ai->sma.tables[i].stacks);
            }
            else if (tt <= 246)
            {
                return CF_NOTOK;
            }
            else if (tt == 247)
            {
                ai->sma.tables[i].offset_delta = m_rd->ReadU2();
                ai->sma.tables[i].stacks_count = 1;
                ai->sma.tables[i].stacks = (VerifyTypeInfo*)malloc(
                    sizeof(VerifyTypeInfo));
                ReadVerifyTypeInfo(ai->sma.tables[i].stacks);
            }
            else if (tt <= 250)
            {
                ai->sma.tables[i].offset_delta = m_rd->ReadU2();
            }
            else if (tt == 251)
            {
                ai->sma.tables[i].offset_delta = m_rd->ReadU2();
            }
            else if (tt <= 254)
            {
                ai->sma.tables[i].offset_delta = m_rd->ReadU2();
                ai->sma.tables[i].locals_count = tt - 251;
                ai->sma.tables[i].locals = (VerifyTypeInfo*)malloc(
                    ai->sma.tables[i].locals_count*sizeof(VerifyTypeInfo));
                for (int j = 0; j < ai->sma.tables[i].locals_count; j++)
                {
                    ReadVerifyTypeInfo(&ai->sma.tables[i].locals[j]);
                }
            }
            else
            {
                ai->sma.tables[i].offset_delta = m_rd->ReadU2();
                ai->sma.tables[i].locals_count = m_rd->ReadU2();
                ai->sma.tables[i].locals = (VerifyTypeInfo*)malloc(
                    ai->sma.tables[i].locals_count*sizeof(VerifyTypeInfo));
                for (int j = 0; j < ai->sma.tables[i].locals_count; j++)
                {
                    ReadVerifyTypeInfo(&ai->sma.tables[i].locals[j]);
                }
                ai->sma.tables[i].stacks_count = m_rd->ReadU2();
                ai->sma.tables[i].stacks = (VerifyTypeInfo*)malloc(
                    ai->sma.tables[i].stacks_count*sizeof(VerifyTypeInfo));
                for (int j = 0; j < ai->sma.tables[i].stacks_count; j++)
                {
                    ReadVerifyTypeInfo(&ai->sma.tables[i].stacks[j]);
                }
            }
        }
    }
    return CF_OK;
}
static int ReadStackMap(AttributeInfo *ai)
{
    ai->sma.tables_count = m_rd->ReadU2();
    if (ai->sma.tables_count)
    {
        ai->sma.tables = (StackMapTable*)malloc(ai->sma.tables_count * sizeof(StackMapTable));
        memset(ai->sma.tables, 0, ai->sma.tables_count * sizeof(StackMapTable));
        for (int i = 0; i < ai->sma.tables_count; i++)
        {
            ai->sma.tables[i].offset_delta = m_rd->ReadU2();
            ai->sma.tables[i].locals_count = m_rd->ReadU2();
            ai->sma.tables[i].locals = (VerifyTypeInfo*)malloc(
                ai->sma.tables[i].locals_count*sizeof(VerifyTypeInfo));
            for (int j = 0; j < ai->sma.tables[i].locals_count; j++)
            {
                ReadVerifyTypeInfo(&ai->sma.tables[i].locals[j]);
            }
            ai->sma.tables[i].stacks_count = m_rd->ReadU2();
            ai->sma.tables[i].stacks = (VerifyTypeInfo*)malloc(
                ai->sma.tables[i].stacks_count*sizeof(VerifyTypeInfo));
            for (int j = 0; j < ai->sma.tables[i].stacks_count; j++)
            {
                ReadVerifyTypeInfo(&ai->sma.tables[i].stacks[j]);
            }
        }
    }
    return CF_OK;
}
static int ReadElementValue(ElementValue *ev)
{
    ev->tag = m_rd->ReadU1();
    switch (ev->tag)
    {
    case 'B':
    case 'C':
    case 'D':
    case 'F':
    case 'I':
    case 'J':
    case 'S':
    case 'Z':
    case 's':
        ev->const_value_index = m_rd->ReadU2();
        break;
    case  'e':
        ev->enum_const_value.type_name_index = m_rd->ReadU2();
        ev->enum_const_value.const_name_index = m_rd->ReadU2();
        break;
    case 'c':
        ev->class_info_index = m_rd->ReadU2();
        break;
    case '@':
        ReadAnnotation(&ev->annotaion_value);
        break;
    case '[':
        ev->array_value.values_count = m_rd->ReadU2();
        ev->array_value.values = (ElementValue*)malloc(
            ev->array_value.values_count*sizeof(ElementValue));
        for (int i = 0; i < ev->array_value.values_count; i++)
        {
            ReadElementValue(&ev->array_value.values[i]);
        }
        break;
    default:
        return CF_NOTOK;
    }
    return CF_OK;
}
static int ReadAnnotation(Annotation *ai)
{
    ai->type_index = m_rd->ReadU2();
    ai->pairs_count = m_rd->ReadU2();
    ai->pairs = (ElementPair*)malloc(ai->pairs_count*sizeof(ElementPair));
    for (int i = 0; i < ai->pairs_count; i++)
    {
        ai->pairs[i].name_index = m_rd->ReadU2();
        ai->pairs[i].evalue = (ElementValue*)malloc(sizeof(ElementValue));
        ReadElementValue(ai->pairs[i].evalue);
    }
    return 0;
}
static int ReadParamenterAnnotation(RuntimeParamerterAnnotationsAttr *rpa)
{
    rpa->paramerters_count = m_rd->ReadU1();
    rpa->paramerters = (AnnotationList *)malloc(
        sizeof(AnnotationList)*rpa->paramerters_count);
    for (int i = 0; i < rpa->paramerters_count; i++)
    {
        rpa->paramerters[i].annotations_count = m_rd->ReadU2();
        rpa->paramerters[i].annotations = (Annotation*)malloc(
            rpa->paramerters[i].annotations_count*sizeof(Annotation));
        for (int j = 0; j < rpa->paramerters[i].annotations_count; j++)
        {
            ReadAnnotation(&rpa->paramerters[i].annotations[j]);
        }
    }
    return 0;
}

typedef int (*ReadAttrFunc) (char* str, int len, AttributeInfo *ai);

static int ReadAttributes(ClassFile *cf, AttributeInfo *ai);
static int read_code(char *str, int len, AttributeInfo *ai);
static int read_error(char *str, int len, AttributeInfo *ai)
{
    return CF_NOTOK;
}
static int read_code(char *str, int len, AttributeInfo *ai)
{
    if (strncmp(str, "Code", len))
    {
        return CF_NOTOK;
    }
    ai->tag = TAG_CODE;
    ai->code.max_stack = m_rd->ReadU2();
    ai->code.max_locals = m_rd->ReadU2();
    ai->code.code_len = m_rd->ReadU4();
    if (ai->code.code_len)
    {
        ai->code.code = (u1*)malloc(ai->code.code_len);
        for (u4 i = 0; i < ai->code.code_len; i++)
        {
            ai->code.code[i] = m_rd->ReadU1();
        }
    }
    ai->code.exceptions_count = m_rd->ReadU2();
    if (ai->code.exceptions_count)
    {
        ai->code.exceptions = (ExceptionTable *)malloc(
            ai->code.exceptions_count*sizeof(ExceptionTable));
        for (int i = 0; i < ai->code.exceptions_count; i++)
        {
            ai->code.exceptions[i].start_pc = m_rd->ReadU2();
            ai->code.exceptions[i].end_pc = m_rd->ReadU2();
            ai->code.exceptions[i].handle_pc = m_rd->ReadU2();
            ai->code.exceptions[i].catch_pc = m_rd->ReadU2();
        }
    }
    ai->code.attributes_count = m_rd->ReadU2();
    if (ai->code.attributes_count)
    {
        ai->code.attributes = (AttributeInfo *)malloc(
            sizeof(AttributeInfo)*ai->code.attributes_count);
        memset(ai->code.attributes, 0, sizeof(AttributeInfo)*ai->code.attributes_count);
        for (int i = 0; i < ai->code.attributes_count; i++)
        {
            if (ReadAttributes(m_cf, &ai->code.attributes[i]) == CF_NOTOK)
            {
                return CF_NOTOK;
            }
        }
    }

    return CF_OK;
}
static int read_stack(char *str, int len, AttributeInfo *ai)
{
    if (strncmp(str, "StackMap", len) == 0)
    {
        ai->tag = TAG_STACKMAP;
        return ReadStackMap(ai);
    }
    return CF_NOTOK;
}
static int read_sy_si(char *str, int len, AttributeInfo *ai)
{
    if (strncmp(str, "Synthetic", len) == 0)
    {
        ai->tag = TAG_SYNTHETIC;
        if (ai->length != 0)
        {
            printf("ClassFormatError: Invalid Synthetic attribute length\n");
            return CF_NOTOK;
        }
    }
    else if (strncmp(str, "Signature", len) == 0)
    {
        ai->tag = TAG_CONSTANTVALUE;
        if (ai->length != 2)
        {
            printf("ClassFormatError: Invalid ConstantValue attribute length\n");
            return CF_NOTOK;
        }
        ai->constant.index = m_rd->ReadU2();
    }
    return CF_OK;
}
static int read_e_s_d(char *str, int len, AttributeInfo *ai)
{
    if (strncmp(str, "Exceptions", len) == 0)
    {
        ai->tag = TAG_EXCEPTIONS;
        ai->exp.exceptions_count = m_rd->ReadU2();
        if (ai->exp.exceptions_count)
        {
            ai->exp.exceptions = (u2 *)malloc(sizeof(u2)*ai->exp.exceptions_count);
            for (int i = 0; i < ai->exp.exceptions_count; i++)
            {
                ai->exp.exceptions[i] = m_rd->ReadU2();
            }
        }
    }
    else if (strncmp(str, "SourceFile", len) == 0)
    {
        ai->tag = TAG_SOURCEFILE;
        ai->src.name_index = m_rd->ReadU2();
        m_cf->src_index = ai->src.name_index;
    }


    else if (strncmp(str, "Deprecated", len) == 0)
    {
        ai->tag = TAG_DEPRECATED;
        if (ai->length != 0)
        {
            printf("ClassFormatError: Invalid Deprecated attribute length\n");
            return CF_NOTOK;
        }
    }
    return CF_OK;
}
static int read_inner(char *str, int len, AttributeInfo *ai)
{
    if (strncmp(str, "InnerClasses", len) == 0)
    {
        ai->tag = TAG_INNERCLASSES;
        ai->inner.classes_count = m_rd->ReadU2();
        if (ai->inner.classes_count)
        {
            ai->inner.classes = (ClassesTable *)malloc(
                sizeof(ClassesTable)*ai->inner.classes_count);
            for (int i = 0; i < ai->inner.classes_count; i++)
            {
                ai->inner.classes[i].inner_class_index = m_rd->ReadU2();
                ai->inner.classes[i].outer_class_index = m_rd->ReadU2();
                ai->inner.classes[i].inner_name_index = m_rd->ReadU2();
                ai->inner.classes[i].inner_access_flags = m_rd->ReadU2();
            }
        }
        return CF_OK;
    }
    return CF_NOTOK;
}
static int read_con_stc(char *str, int len, AttributeInfo *ai)
{
    if (strncmp(str, "ConstantValue", len) == 0)
    {
        ai->tag = TAG_CONSTANTVALUE;
        if (ai->length != 2)
        {
            printf("ClassFormatError: Invalid ConstantValue attribute length\n");
            return CF_NOTOK;
        }
        ai->constant.index = m_rd->ReadU2();
        return CF_OK;
    }
    else if (strncmp(str, "StackMapTable", len) == 0)
    {
        ai->tag = TAG_STACKMAPTABLE;
        return ReadStackMapTable(ai);
    }
    return CF_NOTOK;
}
static int read_clo_line(char *str, int len, AttributeInfo *ai)
{
    if (strncmp(str, ATTR_ENCLOSINGMETHOD, len) == 0)
    {
        ai->tag = TAG_ENCLOSINGMETHOD;
        ai->ema.class_index = m_rd->ReadU2();
        ai->ema.method_index = m_rd->ReadU2();
        return CF_OK;
    }
    else if (strncmp(str, "LineNumberTable", len) == 0)
    {
        ai->tag = TAG_LINENUMBERTABLE;
        ai->line.tables_count = m_rd->ReadU2();
        if (ai->line.tables_count)
        {
            ai->line.tables = (LineNumberTable *)malloc(
                sizeof(LineNumberTable)*ai->line.tables_count);
            for (int i = 0; i < ai->line.tables_count; i++)
            {
                ai->line.tables[i].start_pc = m_rd->ReadU2();
                ai->line.tables[i].line_num = m_rd->ReadU2();
            }
        }
        return CF_OK;
    }
    return CF_NOTOK;
}
static int read_local(char *str, int len, AttributeInfo *ai)
{
    if (strncmp(str, "LocalVariableTable", len))
    {
        return CF_NOTOK;
    }
    ai->tag = TAG_LOCALVARIABLETABLE;
    ai->loc.tables_count = m_rd->ReadU2();
    if (ai->loc.tables_count)
    {
        ai->loc.tables = (LocalVarableTable *)malloc(
            sizeof(LocalVarableTable)*ai->loc.tables_count);
        for (int i = 0; i < ai->loc.tables_count; i++)
        {
            ai->loc.tables[i].start_pc = m_rd->ReadU2();
            ai->loc.tables[i].length = m_rd->ReadU2();
            ai->loc.tables[i].name_index = m_rd->ReadU2();
            ai->loc.tables[i].descriptor_index = m_rd->ReadU2();
            ai->loc.tables[i].index = m_rd->ReadU2();
        }
    }
    return CF_OK;
}
static int read_src_de(char *str, int len, AttributeInfo *ai)
{
    if (strncmp(str, ATTR_SOURCEDEBUGEXTENSION, len))
    {
        return CF_NOTOK;
    }
    ai->tag = TAG_SOURCEDEBUGEXTENSION;
    ai->sea.length = ai->length;
    ai->sea.debug_extension = (u1 *)malloc(ai->sea.length);
    m_rd->ReadArray(ai->sea.debug_extension, ai->sea.length);
    return CF_OK;
}
static int read_loc_tt(char *str, int len, AttributeInfo *ai)
{
    if (strncmp(str, "LocalVariableTypeTable", len))
    {
        return CF_NOTOK;
    }
    ai->tag = TAG_LOCALVARIABLETYPETABLE;
    ai->loc.tables_count = m_rd->ReadU2();
    if (ai->loc.tables_count)
    {
        ai->loc.tables = (LocalVarableTable *)malloc(
            sizeof(LocalVarableTable)*ai->loc.tables_count);
        for (int i = 0; i < ai->loc.tables_count; i++)
        {
            ai->loc.tables[i].start_pc = m_rd->ReadU2();
            ai->loc.tables[i].length = m_rd->ReadU2();
            ai->loc.tables[i].name_index = m_rd->ReadU2();
            ai->loc.tables[i].descriptor_index = m_rd->ReadU2();
            ai->loc.tables[i].index = m_rd->ReadU2();
        }
    }
    return CF_OK;
}
static int read_anno(char *str, int len, AttributeInfo *ai)
{
    if (strncmp(str, ATTR_RUNTIMEVISIBLEANNOTATIONS, len))
    {
        return CF_NOTOK;
    }
    ai->tag = TAG_RUNTIMEVISIBLEANNOTATIONS;
    ai->ra.annotations_count = m_rd->ReadU2();
    ai->ra.annotations = (Annotation *)malloc(
        sizeof(Annotation)*ai->ra.annotations_count);
    for (int i = 0; i < ai->ra.annotations_count; i++)
    {
        ReadAnnotation(&ai->ra.annotations[i]);
    }
    return CF_OK;
}
static int read_anno_in(char *str, int len, AttributeInfo *ai)
{
    if (strncmp(str, ATTR_RUNTIMEINVISIBLEANNOTATIONS, len))
    {
        return CF_NOTOK;
    }
    ai->tag = TAG_RUNTIMEINVISIBLEANNOTATIONS;
    ai->ra.annotations_count = m_rd->ReadU2();
    ai->ra.annotations = (Annotation *)malloc(
        sizeof(Annotation)*ai->ra.annotations_count);
    for (int i = 0; i < ai->ra.annotations_count; i++)
    {
        ReadAnnotation(&ai->ra.annotations[i]);
    }
    return CF_OK;
}
static int read_anno_pa(char *str, int len, AttributeInfo *ai)
{
    if (strncmp(str, ATTR_RUNTIMEVISIBLEPARAMETERANNOTATIONS, len))
    {
        return CF_NOTOK;
    }
    ai->tag = TAG_RUNTIMEVISIBLEPARAMETERANNOTATIONS;
    ReadParamenterAnnotation(&ai->rpa);
    return CF_OK;
}
static int read_anno_pa_in(char *str, int len, AttributeInfo *ai)
{
    if (strncmp(str, ATTR_RUNTIMEINVISIBLEPARAMETERANNOTATIONS, len))
    {
        return CF_NOTOK;
    }
    ai->tag = TAG_RUNTIMEINVISIBLEPARAMETERANNOTATIONS;
    ReadParamenterAnnotation(&ai->rpa);
    return CF_OK;
}

ReadAttrFunc readfunc[MAX_ATTR_LENGTH] = {
    read_error, read_error, read_error, read_code,
    read_error, read_error, read_error, read_stack,
    read_sy_si, read_e_s_d, read_error, read_inner,
    read_con_stc, read_error, read_clo_line, read_error,
    read_error, read_local, read_error, read_src_de,
    read_error, read_loc_tt, read_error, read_error,
    read_anno, read_error, read_anno_in, read_error,
    read_error, read_error, read_error, read_error,
    read_error, read_anno_pa, read_error, read_anno_pa_in
};





static int ReadAttributes(ClassFile *cf, AttributeInfo *ai)
{
    u2 index = m_rd->ReadU2();
    ai->name_index = index;
    ai->length = m_rd->ReadU4();
    if (index < 0 || index >= cf->cpools_count)
    {
        //printf("array index out of cpool length\n");
        return CF_NOTOK;
    }

    if (cf->cpools[index].tag == CONSTANT_UTF8)
    {
        char *tmpstr = (char *)cf->cpools[index].utfi.bytes;
        int len = cf->cpools[index].utfi.length;
        if (len > 0 && len <= MAX_ATTR_LENGTH)
        {
            return readfunc[len-1](tmpstr, len, ai);
        }
    }
    return CF_NOTOK;
}
static int ReadPool(ClassFile *cf)
{
    cf->cpools_count = m_rd->ReadU2();
    if (cf->cpools_count <= 0)
    {
        return CF_NOTOK;
    }
    cf->cpools = (CpInfo *)malloc(sizeof(CpInfo)*cf->cpools_count);
    memset(cf->cpools, 0, sizeof(CpInfo)*cf->cpools_count);

    unsigned char tag = 0;
    for (int i = 1; i < cf->cpools_count; i++)
    {
        tag = m_rd->ReadU1();
        cf->cpools[i].tag = tag;
        switch (tag)
        {
        case CONSTANT_UTF8:
            cf->cpools[i].utfi.length = m_rd->ReadU2();
            cf->cpools[i].utfi.bytes = (u1 *)malloc(cf->cpools[i].utfi.length+1);
            cf->cpools[i].utfi.bytes[cf->cpools[i].utfi.length] = 0;
            for (int j = 0; j < cf->cpools[i].utfi.length; j++)
            {
                cf->cpools[i].utfi.bytes[j] = m_rd->ReadU1();
            }

            break;
        case CONSTANT_INTEGER:
            cf->cpools[i].ii.bytes = m_rd->ReadU4();
            break;
        case CONSTANT_FLOAT:
            cf->cpools[i].fi.bytes = m_rd->ReadU4();
            break;
        case CONSTANT_LONG:
            cf->cpools[i].li.high_bytes = m_rd->ReadU4();
            cf->cpools[i].li.low_bytes = m_rd->ReadU4();
            ++i;
            break;
        case CONSTANT_DOUBLE:
            cf->cpools[i].di.high_bytes = m_rd->ReadU4();
            cf->cpools[i].di.low_bytes = m_rd->ReadU4();
            ++i;
            break;
        case CONSTANT_STRING:
        case CONSTANT_CLASS:
            cf->cpools[i].ci.name_index = m_rd->ReadU2();
            break;
        case CONSTANT_FIELD:
        case CONSTANT_METHOD:
        case CONSTANT_INTERFACEMETHOD:
        case CONSTANT_NAMEANDTYPE:
            cf->cpools[i].mi.class_index = m_rd->ReadU2();
            cf->cpools[i].mi.name_type_index = m_rd->ReadU2();
            break;
        default:
            return CF_NOTOK;
        }
    }
    return CF_OK;
}
static int ReadInterfaces(ClassFile *cf)
{
    cf->interfaces_count = m_rd->ReadU2();
    if (cf->interfaces_count > 0)
    {
        cf->interfaces = (u2 *)malloc(sizeof(u2)*cf->interfaces_count);
        for (int i = 0; i < cf->interfaces_count; i++)
        {
            cf->interfaces[i] = m_rd->ReadU2();
        }
    }
    return CF_OK;
}
static int ReadFields(ClassFile *cf)
{
    cf->fields_count = m_rd->ReadU2();
    if (cf->fields_count > 0)
    {
        cf->fields = (FieldInfo *)malloc(
            sizeof(FieldInfo)*cf->fields_count);
        memset(cf->fields, 0, sizeof(FieldInfo)*cf->fields_count);
        for (int i = 0; i < cf->fields_count; i++)
        {
            cf->fields[i].access_flags = m_rd->ReadU2();
            cf->fields[i].name_index = m_rd->ReadU2();
            cf->fields[i].descriptor_index = m_rd->ReadU2();
            cf->fields[i].attributes_count = m_rd->ReadU2();
            if (cf->fields[i].attributes_count > 0)
            {
                cf->fields[i].attributes = (AttributeInfo *)malloc(
                    sizeof(AttributeInfo)*cf->fields[i].attributes_count);
                memset(cf->fields[i].attributes, 0, sizeof(AttributeInfo)*cf->fields[i].attributes_count);
                for (int j = 0; j < cf->fields[i].attributes_count; j++)
                {
                    if (ReadAttributes(cf, &cf->fields[i].attributes[j]) == CF_NOTOK)
                    {
                        return CF_NOTOK;
                    }
                }
            }
        }
    }
    return CF_OK;
}
static int ReadMethods(ClassFile *cf)
{
    cf->methods_count = m_rd->ReadU2();
    if (cf->methods_count)
    {
        cf->methods = (MethodInfo *)malloc(sizeof(MethodInfo)*cf->methods_count);
        memset(cf->methods, 0, sizeof(MethodInfo)*cf->methods_count);
        for (int i = 0; i < cf->methods_count; i++)
        {
            cf->methods[i].access_flags = m_rd->ReadU2();
            cf->methods[i].name_index = m_rd->ReadU2();
            cf->methods[i].descriptor_index = m_rd->ReadU2();

            cf->methods[i].attributes_count = m_rd->ReadU2();
            if (cf->methods[i].attributes_count)
            {
                cf->methods[i].attributes = (AttributeInfo *)malloc(
                    sizeof(AttributeInfo)*cf->methods[i].attributes_count);
                memset(cf->methods[i].attributes, 0, sizeof(AttributeInfo)*cf->methods[i].attributes_count);
                for (int j = 0; j < cf->methods[i].attributes_count; j++)
                {
                    if (ReadAttributes(cf, &cf->methods[i].attributes[j]) == CF_NOTOK)
                    {
                        return CF_NOTOK;
                    }
                }
            }
        }
    }
    return CF_OK;
}

static void GetJavaClassName(ClassFile *cf)
{
    int index = cf->cpools[cf->this_class].ci.name_index;
    u1 *p = cf->cpools[index].utfi.bytes;
    for (int i = 0; i < cf->cpools[index].utfi.length; i++)
    {
        if (p[i] == '/')
        {
            p[i] = '.';
        }
    }
}



int CF_IsJavaClassFile(u1 *buf, size_t len)
{
    //if (len < MIN_CLASS)
    //{
    //    return CF_NOTOK;
    //}
    if (*(u4*)buf == 0xbebafeca)
    {
        int version = *(u1*)(buf+7);
        if ( version < 0x40 && version > 0x20)
        {
            return CF_OK;
        }
    }
    return CF_NOTOK;
}

int CF_ReadClassFile(u1 *buf, size_t len, size_t *offset, ClassFile *cf)
{
    m_rd = new Reader(buf, len);
    m_cf = cf;
    cf->magic = m_rd->ReadU4();
    if (cf->magic != JAVA_MAGIC)
    {
        return CF_NOTOK;
    }
    cf->minor_version = m_rd->ReadU2();
    cf->major_version = m_rd->ReadU2();
    if (cf->major_version > 0x40 || cf->major_version < 0x20)
    {
        return CF_NOTOK;
    }
    if (ReadPool(cf) == CF_NOTOK)
    {
        *offset = m_rd->GetCurrentOffset();
        return CF_NOTOK;
    }
    cf->access_flags = m_rd->ReadU2();
    cf->this_class = m_rd->ReadU2();

    cf->super_class = m_rd->ReadU2();

    if (ReadInterfaces(cf) == CF_NOTOK)
    {
        return CF_NOTOK;
    }
    if (ReadFields(cf) == CF_NOTOK)
    {
        return CF_NOTOK;
    }
    if (ReadMethods(cf) == CF_NOTOK)
    {
        return CF_NOTOK;
    }

    cf->attributes_count = m_rd->ReadU2();

    if (cf->attributes_count > 0)
    {
        cf->attributes = (AttributeInfo *)malloc(
            sizeof(AttributeInfo)*cf->attributes_count);
        memset(cf->attributes, 0, sizeof(AttributeInfo)*cf->attributes_count);
        for (int i = 0; i < cf->attributes_count; i++)
        {
            if (ReadAttributes(cf, &cf->attributes[i]) == CF_NOTOK)
            {
                return CF_NOTOK;
            }
        }
    }
    GetJavaClassName(cf);

    *offset = m_rd->GetCurrentOffset();
    return CF_OK;
}

void FreeElementValue(ElementValue *ev)
{
    if (ev->tag == '@')
    {
        if (ev->annotaion_value.pairs_count)
        {
            for (int i = 0; i < ev->annotaion_value.pairs_count; i++)
            {
                FreeElementValue(ev->annotaion_value.pairs[i].evalue);
                free(ev->annotaion_value.pairs[i].evalue);
            }
            free(ev->annotaion_value.pairs);
        }
    }
    else if (ev->tag == '[')
    {
        if (ev->array_value.values_count)
        {
            for (int i = 0; i < ev->array_value.values_count; i++)
            {
                FreeElementValue(&ev->array_value.values[i]);
            }
            free(ev->array_value.values);
        }
    }
}

void FreeAnnotation(Annotation *aa)
{
    if (aa->pairs_count)
    {
        for (int i = 0; i < aa->pairs_count; i++)
        {
            FreeElementValue(aa->pairs[i].evalue);
            free(aa->pairs[i].evalue);
        }
        free(aa->pairs);
    }
}

void FreeAttribute(AttributeInfo *ai)
{
    switch (ai->tag)
    {
    case TAG_CODE:
        if (ai->code.attributes_count)
        {
            for (int i = 0; i < ai->code.attributes_count; i++)
            {
                FreeAttribute(&ai->code.attributes[i]);
            }
            free(ai->code.attributes);
        }
        if (ai->code.code_len)
        {
            free(ai->code.code);
        }
        if (ai->code.exceptions_count)
        {
            free(ai->code.exceptions);
        }
        break;
    case TAG_EXCEPTIONS:
        if (ai->exp.exceptions_count)
        {
            free(ai->exp.exceptions);
        }
        break;
    case TAG_INNERCLASSES:
        if (ai->inner.classes_count)
        {
            free(ai->inner.classes);
        }
        break;
    case TAG_STACKMAPTABLE:
        if (ai->sma.tables_count)
        {
            for (int i = 0; i < ai->sma.tables_count; i++)
            {
                if (ai->sma.tables[i].locals_count)
                {
                    free(ai->sma.tables[i].locals);
                }
                if (ai->sma.tables[i].stacks_count)
                {
                    free(ai->sma.tables[i].stacks);
                }
            }
            free(ai->sma.tables);
        }
        break;
    case TAG_LINENUMBERTABLE:
        if (ai->line.tables_count)
        {
            free(ai->line.tables);
        }
        break;
    case TAG_LOCALVARIABLETABLE:
    case TAG_LOCALVARIABLETYPETABLE:
        if (ai->loc.tables_count)
        {
            free(ai->loc.tables);
            ai->loc.tables_count = 0;
        }
        break;

    case TAG_SOURCEDEBUGEXTENSION:
        if (ai->sea.length)
        {
            free(ai->sea.debug_extension);
        }
        break;
    case TAG_RUNTIMEVISIBLEANNOTATIONS:
    case TAG_RUNTIMEINVISIBLEANNOTATIONS:
        if (ai->ra.annotations_count)
        {
            for (int i = 0; i < ai->ra.annotations_count; i++)
            {
                FreeAnnotation(&ai->ra.annotations[i]);
            }
            free(ai->ra.annotations);
        }
        break;
    case TAG_RUNTIMEVISIBLEPARAMETERANNOTATIONS:
    case TAG_RUNTIMEINVISIBLEPARAMETERANNOTATIONS:
        if (ai->rpa.paramerters_count)
        {
            for (int i = 0; i < ai->rpa.paramerters_count; i++)
            {
                if (ai->rpa.paramerters[i].annotations_count)
                {
                    for (int j = 0; j < ai->rpa.paramerters[i].annotations_count; j++)
                    {
                        FreeAnnotation(&ai->rpa.paramerters[i].annotations[j]);
                    }
                    free(ai->rpa.paramerters[i].annotations);
                }
            }
            free(ai->rpa.paramerters);
        }
    default:
        break;
    }
}

void CF_FreeClassFile(ClassFile *cf)
{
    if (m_rd)
    {
        delete m_rd;
        m_rd = NULL;
    }
    if (cf->interfaces_count)
    {
        free(cf->interfaces);
    }
    if (cf->attributes_count)
    {
        for (int i = 0; i < cf->attributes_count; i++)
        {
            FreeAttribute(&cf->attributes[i]);
        }
        free(cf->attributes);
    }
    if (cf->cpools_count)
    {
        for (int i = 0; i < cf->cpools_count; i++)
        {
            if (cf->cpools[i].tag == CONSTANT_UTF8)
            {
                if (cf->cpools[i].utfi.length)
                {
                    free(cf->cpools[i].utfi.bytes);
                }
            }
        }
        free(cf->cpools);
    }

    if (cf->fields_count)
    {
        for (int i = 0; i < cf->fields_count; i++)
        {
            if (cf->fields[i].attributes_count)
            {
                for (int j = 0; j < cf->fields[i].attributes_count; j++)
                {
                    FreeAttribute(&cf->fields[i].attributes[j]);
                }
                free(cf->fields[i].attributes);
            }
        }
        free(cf->fields);
    }

    if (cf->methods_count)
    {
        for (int i = 0; i < cf->methods_count; i++)
        {
            if (cf->methods[i].attributes_count)
            {
                for (int j = 0; j < cf->methods[i].attributes_count; j++)
                {
                    FreeAttribute(&cf->methods[i].attributes[j]);
                }
                free(cf->methods[i].attributes);
            }
        }
        free(cf->methods);
    }
}

