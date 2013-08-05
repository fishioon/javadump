#ifndef CLASSFILE_H_
#define CLASSFILE_H_

#include <stdio.h>

#define CF_OK       0
#define CF_NOTOK    -1

#define u1  unsigned char
#define u2  unsigned short
#define u4  unsigned int
#define u8  unsigned long long

#define JAVA_MAGIC                  0xCAFEBABE
#define MIN_JAVA_VERSION            0x2c    //JDK 1.1 = 0x2D
#define MAX_JAVA_VERSION            0x35    //J2SE 7 = 0x33
#define MIN_CLASS                   0x10

#define CONSTANT_UTF8               1
#define CONSTANT_INTEGER            3
#define CONSTANT_FLOAT              4
#define CONSTANT_LONG               5
#define CONSTANT_DOUBLE             6
#define CONSTANT_CLASS              7
#define CONSTANT_STRING             8
#define CONSTANT_FIELD              9
#define CONSTANT_METHOD             10
#define CONSTANT_INTERFACEMETHOD    11
#define CONSTANT_NAMEANDTYPE        12


#define ACC_PUBLIC                  0x0001 
#define ACC_PRIVATE                 0x0002 
#define ACC_PROTECTED               0x0004 
#define ACC_STATIC                  0x0008 
#define ACC_FINAL                   0x0010 

#define ACC_VOLATILE                0x0040 
#define ACC_TRANSIENT               0x0080 

#define ACC_SYNCHRONIZED            0x0020 
#define ACC_NATIVE                  0x0100 
#define ACC_ABSTRACT                0x0400 
#define ACC_STRICT                  0x0800 

#define ACC_SUPER                   0x0020 
#define ACC_INTERFACE               0x0200

#define MAX_ATTR_LENGTH             36

// 各属性字符长度
// 4          "Code"         
// 8          "StackMap"
// 9          "Synthetic"                           
// 9          "Signature"                           
// 10         "Exceptions"                          
// 10         "SourceFile"                          
// 10         "Deprecated"                          
// 12         "InnerClasses"                        
// 13         "ConstantValue"                       
// 13         "StackMapTable"                       
// 15         "EnclosingMethod"                     
// 15         "LineNumberTable"                     
// 18         "LocalVariableTable"                  
// 20         "SourceDebugExtension"                
// 22         "LocalVariableTypeTable"              
// 25         "RuntimeVisibleAnnotations"           
// 27         "RuntimeInvisibleAnnotations"         
// 34         "RuntimeVisibleParameterAnnotations"  
// 36         "RuntimeInvisibleParameterAnnotations"

#define ATTR_CODE                                   "Code"                      
#define ATTR_STACKMAP                               "StackMap"
#define ATTR_SYNTHETIC                              "Synthetic"                              
#define ATTR_SIGNATURE                              "Signature"                              
#define ATTR_EXCEPTIONS                             "Exceptions"                             
#define ATTR_SOURCEFILE                             "SourceFile"                             
#define ATTR_DEPRECATED                             "Deprecated"                             
#define ATTR_INNERCLASSES                           "InnerClasses"                           
#define ATTR_CONSTANTVALUE                          "ConstantValue"                          
#define ATTR_STACKMAPTABLE                          "StackMapTable"                          
#define ATTR_ENCLOSINGMETHOD                        "EnclosingMethod"                        
#define ATTR_LINENUMBERTABLE                        "LineNumberTable"                        
#define ATTR_LOCALVARIABLETABLE                     "LocalVariableTable"                     
#define ATTR_SOURCEDEBUGEXTENSION                   "SourceDebugExtension"                   
#define ATTR_LOCALVARIABLETYPETABLE                 "LocalVariableTypeTable"                 
#define ATTR_RUNTIMEVISIBLEANNOTATIONS              "RuntimeVisibleAnnotations"              
#define ATTR_RUNTIMEINVISIBLEANNOTATIONS            "RuntimeInvisibleAnnotations"            
#define ATTR_RUNTIMEVISIBLEPARAMETERANNOTATIONS     "RuntimeVisibleParameterAnnotations"     
#define ATTR_RUNTIMEINVISIBLEPARAMETERANNOTATIONS   "RuntimeInvisibleParameterAnnotations"   
//#define ATTR_ANNOTATIONDEFAULT                      "AnnotationDefault"

#define TAG_CODE                                    1
#define TAG_STACKMAP                                2
#define TAG_SYNTHETIC                               3
#define TAG_SIGNATURE                               4
#define TAG_EXCEPTIONS                              5
#define TAG_SOURCEFILE                              6
#define TAG_DEPRECATED                              7
#define TAG_INNERCLASSES                            8
#define TAG_CONSTANTVALUE                           9
#define TAG_STACKMAPTABLE                           10
#define TAG_ENCLOSINGMETHOD                         11
#define TAG_LINENUMBERTABLE                         12
#define TAG_LOCALVARIABLETABLE                      13
#define TAG_SOURCEDEBUGEXTENSION                    14
#define TAG_LOCALVARIABLETYPETABLE                  15
#define TAG_RUNTIMEVISIBLEANNOTATIONS               16
#define TAG_RUNTIMEINVISIBLEANNOTATIONS             17
#define TAG_RUNTIMEVISIBLEPARAMETERANNOTATIONS      18
#define TAG_RUNTIMEINVISIBLEPARAMETERANNOTATIONS    19
//#define TAG_ANNOTATIONDEFAULT                       19


struct ClassInfo
{
    u2 name_index;
};

struct FieldrefInfo
{
    u2 class_index;
    u2 name_type_index;
};

struct MethodrefInfo
{
    u2 class_index;
    u2 name_type_index;
};

struct InterfacerefInfo
{
    u2 class_index;
    u2 name_type_index;
};

struct IntegerInfo
{
    u4 bytes;
};

struct FloatInfo
{
    u4 bytes;
};

struct LongInfo
{
    u4 high_bytes;
    u4 low_bytes;
};

struct DoubleInfo
{
    u4 high_bytes;
    u4 low_bytes;
};

struct NameTypeInfo
{
    u2 name_index;
    u2 descriptor_index;
};

struct UtfInfo
{
    u2 length;
    u1 *bytes;
};


struct ConstantAttr
{
    u2 index;
};

struct ExceptionTable
{
    u2 start_pc;
    u2 end_pc;
    u2 handle_pc;
    u2 catch_pc;
};

struct ExceptionAttr
{
    u2 exceptions_count;
    u2 *exceptions;
};

struct ClassesTable
{
    u2 inner_class_index;
    u2 outer_class_index;
    u2 inner_name_index;
    u2 inner_access_flags;
};

struct InnerClassesAttr
{
    u2 classes_count;
    ClassesTable *classes;
};

struct SyntheticAttr {};

struct SourceFileAttr
{
    u2 name_index;
};

struct LineNumberTable
{
    u2 start_pc;
    u2 line_num;
};

struct LineNumberTableAttr
{
    u2 tables_count;
    LineNumberTable *tables;
};

struct LocalVarableTable
{
    u2 start_pc;
    u2 length;
    u2 name_index;
    u2 descriptor_index;
    u2 index;
};

struct LocalVarableTableAttr
{
    u2 tables_count;
    LocalVarableTable *tables;
};

struct DeprecatedAttr {};

struct AttributeInfo;

struct CodeAttr
{
    u2 max_stack;
    u2 max_locals;
    u4 code_len;
    u1 *code;
    u2 exceptions_count;
    ExceptionTable *exceptions;
    u2 attributes_count;
    AttributeInfo *attributes;
};

struct VerifyTypeInfo
{
    u1 tag;
    union
    {
        u2 cpool_index;
        u2 offset;
    };
};


struct StackMapTable
{
    u1 table_type;
    u2 offset_delta;
    u2 locals_count;
    u2 stacks_count;
    VerifyTypeInfo *locals;
    VerifyTypeInfo *stacks;
};

struct StackMapAttr
{
    u2 tables_count;
    StackMapTable *tables;
};

struct EnumConstValue
{
    u2 type_name_index;
    u2 const_name_index;
};

struct ElementValue;
struct ElementPair
{
    u2 name_index;
    ElementValue *evalue;
};
struct Annotation
{
    u2 type_index;
    u2 pairs_count;
    ElementPair *pairs;
};
struct ArrayValue
{
    u2 values_count;
    ElementValue *values;
};
struct ElementValue
{
    u1 tag;
    union
    {
        u2 const_value_index;               //'B''C''D''F''I''J''S''Z''s'
        EnumConstValue enum_const_value;    //'e'
        u2 class_info_index;                //'c'
        Annotation annotaion_value;         //'@'
        ArrayValue array_value;             //'['
    };
};

struct AnnotationList
{
    u2 annotations_count;
    Annotation *annotations;
};

struct RuntimeParamerterAnnotationsAttr
{
    u2 paramerters_count;
    AnnotationList *paramerters;
};

struct EnclosingMethodAttr
{
    u2 class_index;
    u2 method_index;
};

struct AnnotationDefaultAttr
{
    ElementValue default_value;
};

struct SourceDebugExtensionAttr
{
    u4 length;
    u1 *debug_extension;
};
struct AttributeInfo
{
    u2 name_index;
    u4 length;
    u1 tag;
    union
    {
        ConstantAttr constant;
        CodeAttr code;
        ExceptionAttr exp;
        InnerClassesAttr inner;
        SyntheticAttr syn;
        SourceFileAttr src;
        LineNumberTableAttr line;
        LocalVarableTableAttr loc;
        DeprecatedAttr dep;
        StackMapAttr sma;
        AnnotationList ra;
        RuntimeParamerterAnnotationsAttr rpa;
        EnclosingMethodAttr ema;
        SourceDebugExtensionAttr sea;
    };
};



struct CpInfo 
{
    u1 tag;

    union 
    {
        ClassInfo ci;
        FieldrefInfo field;
        MethodrefInfo mi;
        InterfacerefInfo inf;
        IntegerInfo ii;
        FloatInfo fi;
        LongInfo li;
        DoubleInfo di;
        NameTypeInfo nti;
        UtfInfo utfi;
    };
};

struct FieldInfo
{
    u2 access_flags;
    u2 name_index;
    u2 descriptor_index;
    u2 attributes_count;
    AttributeInfo *attributes;
};

struct MethodInfo
{
    u2 access_flags;
    u2 name_index;
    u2 descriptor_index;
    u2 attributes_count;
    AttributeInfo *attributes;
};

struct ClassFile 
{
    u4 magic;
    u2 minor_version;
    u2 major_version;
    u2 cpools_count;
    CpInfo *cpools; //[constant_pool_count-1];
    u2 access_flags;
    u2 this_class;
    u2 super_class;
    u2 interfaces_count;
    u2 *interfaces; 
    u2 fields_count;
    FieldInfo *fields; 
    u2 methods_count;
    MethodInfo *methods; 
    u2 attributes_count;
    AttributeInfo *attributes; 

    //add help fields
    u2 src_index;

    ClassFile()
    {
        cpools_count = 0;
        interfaces_count = 0;
        fields_count = 0;
        methods_count = 0;
        attributes_count = 0;
        this_class = 0;
        super_class = 0;

        src_index = 0;
    }

};

#endif