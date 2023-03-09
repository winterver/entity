typedef struct value
{
    int type; // data type
    union {
        int8_t i8;
        int16_t i16;
        int32_t i32;
        int64_t i64;
        uint8_t u8;
        uint16_t u16;
        uint32_t u32;
        uint64_t u64;
        float f32;
        double f64;
        char *str;
        void *obj; // entity
    };
} value;

// data types
enum { 
    TYPE_VOID, TYPE_CHAR, TYPE_SHORT, TYPE_INT, TYPE_LONG,
    TYPE_UCHAR, TYPE_USHORT, TYPE_UINT, TYPE_ULONG,
    TYPE_FLOAT, TYPE_DOUBLE, TYPE_STRING, TYPE_ENTITY,
};

const char* type_name(int type)
{
    switch(type)
    {
    case TYPE_VOID:      return "void";
    case TYPE_CHAR:      return "char";
    case TYPE_SHORT:     return "short";
    case TYPE_INT:       return "int";
    case TYPE_LONG:      return "long";
    case TYPE_UCHAR:     return "uchar";
    case TYPE_USHORT:    return "ushort";
    case TYPE_UINT:      return "uint";
    case TYPE_ULONG:     return "ulong";
    case TYPE_FLOAT:     return "float";
    case TYPE_DOUBLE:    return "double";
    case TYPE_STRING:    return "string";
    case TYPE_ENTITY:    return "entity";
    default:             return "unknown type";
    }
}

int get_type(const char* s)
{
#define CMP(type, str)  \
    if (!strcmp(str, s)) return type;

    CMP(TYPE_VOID,      "void");
    CMP(TYPE_CHAR,      "char");
    CMP(TYPE_SHORT,     "short");
    CMP(TYPE_INT,       "int");
    CMP(TYPE_LONG,      "long");
    CMP(TYPE_UCHAR,     "uchar");
    CMP(TYPE_USHORT,    "ushort");
    CMP(TYPE_UINT,      "uint");
    CMP(TYPE_ULONG,     "ulong");
    CMP(TYPE_FLOAT,     "float");
    CMP(TYPE_DOUBLE,    "double");
    CMP(TYPE_STRING,    "string");
    CMP(TYPE_ENTITY,    "entity");
    return -1;

#undef CMP
}

#define MATCH_OP(ltype, _op, rtype)                                     \
    if (lhs->type == ltype                                              \
        && rhs->type == rtype                                           \
        && op == #_op[0])

#define IMPL_OP(ltype, _op, rtype, otype, ofield, lfield, rfield)       \
    MATCH_OP(ltype, _op, rtype)                                         \
    {                                                                   \
        out->type = otype;                                              \
        out->ofield = lhs->lfield _op rhs->rfield;                      \
        return;                                                         \
    }

void binary_op(value* out, const value* lhs, int op, const value* rhs)
{
    IMPL_OP(
        TYPE_INT, +, TYPE_INT,
        TYPE_INT, i32, i32, i32
    );

    IMPL_OP(
        TYPE_INT, -, TYPE_INT,
        TYPE_INT, i32, i32, i32
    );

    IMPL_OP(
        TYPE_INT, *, TYPE_INT,
        TYPE_INT, i32, i32, i32
    );

    IMPL_OP(
        TYPE_INT, /, TYPE_INT,
        TYPE_INT, i32, i32, i32
    );

    IMPL_OP(
        TYPE_INT, <, TYPE_INT,
        TYPE_INT, i32, i32, i32
    );

    IMPL_OP(
        TYPE_INT, >, TYPE_INT,
        TYPE_INT, i32, i32, i32
    );

    ERROR("(%d) unknown operator between types '%s' and '%s': %c\n",
        lineno, type_name(lhs->type), type_name(rhs->type), op);
}

void type_convert(value* val, int type)
{
    if (val->type == type)
        return;
    ERROR("(%d) can't convert from %s to %s\n", 
        lineno, type_name(val->type), type_name(type));
}