typedef struct entity entity;
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
        entity *obj; // entity
    };
} value;

typedef struct member
{
    struct member* next;
    char* name;
    value val;
} member;

typedef struct entity
{
    member* mbeg;
    member* mend;
} entity;

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
        && op == _op)

#define IMPL_OP(ltype, _op, rtype, otype, ofield, lfield, _op2, rfield) \
    MATCH_OP(ltype, _op, rtype)                                         \
    {                                                                   \
        out->type = otype;                                              \
        out->ofield = lhs->lfield _op2 rhs->rfield;                     \
        return;                                                         \
    }

double fmod(double, double);

void binary_op(value* out, const value* lhs, int op, const value* rhs)
{
    // operators between 2 ints
    IMPL_OP(
        TYPE_INT, '+', TYPE_INT,
        TYPE_INT, i32, i32, +, i32
    );

    IMPL_OP(
        TYPE_INT, '-', TYPE_INT,
        TYPE_INT, i32, i32, -, i32
    );

    IMPL_OP(
        TYPE_INT, '*', TYPE_INT,
        TYPE_INT, i32, i32, *, i32
    );

    IMPL_OP(
        TYPE_INT, '/', TYPE_INT,
        TYPE_INT, i32, i32, /, i32
    );

    IMPL_OP(
        TYPE_INT, '%', TYPE_INT,
        TYPE_INT, i32, i32, %, i32
    );

    IMPL_OP(
        TYPE_INT, '<', TYPE_INT,
        TYPE_INT, i32, i32, <, i32
    );

    IMPL_OP(
        TYPE_INT, '>', TYPE_INT,
        TYPE_INT, i32, i32, >, i32
    );

    // operators between 2 floats
    IMPL_OP(
        TYPE_FLOAT, '+', TYPE_FLOAT,
        TYPE_FLOAT, f32, f32, +, f32
    );

    IMPL_OP(
        TYPE_FLOAT, '-', TYPE_FLOAT,
        TYPE_FLOAT, f32, f32, -, f32
    );

    IMPL_OP(
        TYPE_FLOAT, '*', TYPE_FLOAT,
        TYPE_FLOAT, f32, f32, *, f32
    );

    IMPL_OP(
        TYPE_FLOAT, '/', TYPE_FLOAT,
        TYPE_FLOAT, f32, f32, /, f32
    );

    MATCH_OP(TYPE_FLOAT, '%', TYPE_FLOAT)
    {
        out->type = TYPE_FLOAT;
        out->f32 = fmod(lhs->f32, rhs->f32);
        return; 
    }

    IMPL_OP(
        TYPE_FLOAT, '<', TYPE_FLOAT,
        TYPE_INT, i32, f32, <, f32
    );

    IMPL_OP(
        TYPE_FLOAT, '>', TYPE_FLOAT,
        TYPE_INT, i32, f32, >, f32
    );

    // operators between float and int
    IMPL_OP(
        TYPE_FLOAT, '+', TYPE_INT,
        TYPE_FLOAT, f32, f32, +, i32
    );

    IMPL_OP(
        TYPE_FLOAT, '-', TYPE_INT,
        TYPE_FLOAT, f32, f32, -, i32
    );

    IMPL_OP(
        TYPE_FLOAT, '*', TYPE_INT,
        TYPE_FLOAT, f32, f32, *, i32
    );

    IMPL_OP(
        TYPE_FLOAT, '/', TYPE_INT,
        TYPE_FLOAT, f32, f32, /, i32
    );

    MATCH_OP(TYPE_FLOAT, '%', TYPE_INT)
    {
        out->type = TYPE_FLOAT;
        out->f32 = fmod(lhs->f32, rhs->i32);
        return; 
    }

    IMPL_OP(
        TYPE_FLOAT, '<', TYPE_INT,
        TYPE_INT, i32, f32, <, i32
    );

    IMPL_OP(
        TYPE_FLOAT, '>', TYPE_INT,
        TYPE_INT, i32, f32, >, i32
    );

    // operators between int and float
    IMPL_OP(
        TYPE_INT, '+', TYPE_FLOAT,
        TYPE_FLOAT, f32, i32, +, f32
    );

    IMPL_OP(
        TYPE_INT, '-', TYPE_FLOAT,
        TYPE_FLOAT, f32, i32, -, f32
    );

    IMPL_OP(
        TYPE_INT, '*', TYPE_FLOAT,
        TYPE_FLOAT, f32, i32, *, f32
    );

    IMPL_OP(
        TYPE_INT, '/', TYPE_FLOAT,
        TYPE_FLOAT, f32, i32, /, f32
    );

    MATCH_OP(TYPE_INT, '%', TYPE_FLOAT)
    {
        out->type = TYPE_FLOAT;
        out->f32 = fmod(lhs->i32, rhs->f32);
        return; 
    }

    IMPL_OP(
        TYPE_INT, '<', TYPE_FLOAT,
        TYPE_INT, i32, i32, <, f32
    );

    IMPL_OP(
        TYPE_INT, '>', TYPE_FLOAT,
        TYPE_INT, i32, i32, >, f32
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

value* _find_member(member* m, char* name)
{
    if (m == NULL)
        return NULL;

    if (m->name == name)
        return &m->val;
    
    return _find_member(m->next, name);
}

value* find_member(entity* e, char* name)
{
    return _find_member(e->mbeg, name);
}

value* get_member(entity* e, char* name)
{
    value* val = find_member(e, name);
    if (val == NULL)
        ERROR("(%d) no such member: %s\n", lineno, name);
    return val;
}

void append_member(value var, char* name, value val)
{
    if (var.type != TYPE_ENTITY)
    {
        ERROR("(%d) can't append member to non-entity object\n", lineno);
    }
    entity* e = var.obj;
    if (find_member(e, name) != NULL)
    {
        ERROR("(%d) member %s already exists\n", lineno, name);
    }
    member* m = malloc(sizeof(member));
    m->next = NULL;
    m->name = name;
    m->val = val;
    if (e->mend == NULL)
    {
        e->mbeg = m;
        e->mend = m;
    }
    else
    {
        e->mend->next = m;
        e->mend = m;
    }
}

value new_entity()
{
    entity* e = malloc(sizeof(entity));
    e->mbeg = NULL;
    e->mend = NULL;

    value ret;
    ret.type = TYPE_ENTITY;
    ret.obj = e;
    return ret;
}

void free_members(member* m)
{
    if (m == NULL);
        return;
    free_members(m->next);
    free(m);
}

value* get_variable(const char* name);

value del_entity()
{
    value var = *get_variable(find_string("e"));
    entity* e = var.obj;
    free_members(e->mbeg);
    free(e);

    value ret;
    memset(&ret, 0, sizeof(value));
    ret.type = TYPE_VOID;
    return ret;
}

value print_str()
{
    printf("%s", get_variable(find_string("s"))->str);

    value ret;
    memset(&ret, 0, sizeof(value));
    ret.type = TYPE_VOID;
    return ret;
}