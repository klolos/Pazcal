/******************************************************************************
 *  CVS version:
 *     $Id: symbol.h,v 1.1 2003/05/13 22:21:01 nickie Exp $
 ******************************************************************************
 *
 *  C header file : symbol.h
 *  Project       : PCL Compiler
 *  Version       : 1.0 alpha
 *  Written by    : Nikolaos S. Papaspyrou (nickie@softlab.ntua.gr)
 *  Date          : May 14, 2003
 *  Description   : Generic symbol table in C
 *
 *  Comments: (in Greek iso-8859-7)
 *  --------- *  ������ �������� �����������.
 *  ����� ������������ ��������� ��� ��������� �����������.
 *  ������ ����������� ������������ ��� �����������.
 *  ���������� ����������� ����������
 */


#ifndef __SYMBOL_H__
#define __SYMBOL_H__


/* ---------------------------------------------------------------------
   -------------------------- ����� bool -------------------------------
   --------------------------------------------------------------------- */

/*
 *  �� �� �������� include ��� ������������� ��� ��� ���������
 *  ��� C ��� ��������������, �������������� �� �� �� ��������:
 */
#include <stdbool.h>
#if 0
typedef enum { false, true } bool;
#endif


/* ---------------------------------------------------------------------
   ------------ ������� �������� ��� ������ �������� -------------------
   --------------------------------------------------------------------- */

#define START_POSITIVE_OFFSET 12     /* ������ ������ offset ��� �.�.   */
#define START_NEGATIVE_OFFSET 0     /* ������ �������� offset ��� �.�. */


/* ---------------------------------------------------------------------
   --------------- ������� ����� ��� ������ �������� -------------------
   --------------------------------------------------------------------- */

/* ����� ��������� ��� ��� ��������� ��� �������� */

typedef int           RepInteger;         /* ��������                  */
typedef unsigned char RepBoolean;         /* ������� �����             */
typedef char          RepChar;            /* ����������                */
typedef long double   RepReal;            /* �����������               */
typedef const char *  RepString;          /* �������������             */


/* ����� ��������� ��� ������������� ����������� */

typedef struct Type_tag * Type;

typedef enum {                           /***** �� ����� ��� ����� ****/
   TYPE_VOID,                            /* ����� ����� ������������� */
   TYPE_INTEGER,                         /* ��������                  */
   TYPE_BOOLEAN,                         /* ������� �����             */
   TYPE_CHAR,                            /* ����������                */
   TYPE_REAL,                            /* �����������               */
   TYPE_ARRAY,                           /* ������� ������� ��������  */
   TYPE_IARRAY,                          /* ������� �������� �������� */
   TYPE_POINTER                          /* �������                   */
} KIND;                           

struct Type_tag {
    KIND           kind;
    Type           refType;              /* ����� ��������            */
    RepInteger     size;                 /* �������, �� ����� ������� */
    unsigned int   refCount;             /* �������� ��������         */
};

typedef struct array_decl {              /* Node used to create the   */
    char * name;                         /* type of an array          */
    Type type;
} array_decl;

typedef struct arg_decl {                /* function argument         */
    char *name;
    Type type;
    bool byRef;
} arg_decl;

typedef struct type_node {               /* list of types used in     */
    Type type;                           /* function calls            */
    const char *place;                   /* the place of the argument */
    struct type_node *next;
    struct type_node *prev;
} type_node;

typedef union {                          /* ����                      */
    RepInteger vInteger;                 /*    �������                */
    RepBoolean vBoolean;                 /*    ������                 */
    RepChar    vChar;                    /*    ����������             */
    RepReal    vReal;                    /*    ����������             */
    RepString  vString;                  /*    ������������           */
} VALUE;                 
                         
/* ����� �������� ��� ������ �������� */

typedef enum {            
   ENTRY_VARIABLE,                       /* ����������                 */
   ENTRY_CONSTANT,                       /* ��������                   */
   ENTRY_FUNCTION,                       /* �����������                */
   ENTRY_PARAMETER,                      /* ���������� �����������     */
   ENTRY_TEMPORARY                       /* ���������� ����������      */
} EntryType;


/* ����� ���������� ���������� */

typedef enum {            
   PASS_BY_VALUE,                        /* ���' ����                  */
   PASS_BY_REFERENCE                     /* ���' �������               */
} PassMode;


typedef enum {                                /* ��������� ����������  */
    PARDEF_COMPLETE,                             /* ������ �������     */
    PARDEF_DEFINE,                               /* �� ���� �������    */
    PARDEF_CHECK                                 /* �� ���� �������    */
} PARDEF;                    

/* ����� �������� ���� ������ �������� */

typedef struct SymbolEntry_tag SymbolEntry;

struct SymbolEntry_tag {
   const char   * id;                 /* ����� ��������������          */
   EntryType      entryType;          /* ����� ��� ��������            */
   unsigned int   nestingLevel;       /* ����� �����������             */
   unsigned int   hashValue;          /* ���� ���������������          */
   SymbolEntry  * nextHash;           /* ������� ������� ���� �.�.     */
   SymbolEntry  * nextInScope;        /* ������� ������� ���� �������� */
   const char   * final_code_name;    /* ����� ���� ������ ������      */
   bool           isGlobal;           /* �� ����� Global Variable      */

   union {                            /* ������� �� ��� ���� ��������: */

      struct {                                /******* ��������� *******/
         Type          type;                  /* �����                 */
         int           offset;                /* Offset ��� �.�.       */
      } eVariable;

      struct {                                /******** ������� ********/
         Type          type;                  /* �����                 */
         VALUE         value;                 /* ����                  */
      } eConstant;

      struct {                                /******* ��������� *******/
         bool          isForward;             /* ������ forward        */
         SymbolEntry * firstArgument;         /* ����� ����������      */
         SymbolEntry * lastArgument;          /* ��������� ����������  */
         Type          resultType;            /* ����� �������������   */
         PARDEF        pardef;
         int           firstQuad;             /* ������ �������        */
         unsigned int  negOffset;             /* ������� Offset        */
      } eFunction;

      struct {                                /****** ���������� *******/
         Type          type;                  /* �����                 */
         int           offset;                /* Offset ��� �.�.       */
         PassMode      mode;                  /* ������ ����������     */
         SymbolEntry * next;                  /* ������� ����������    */
      } eParameter;

      struct {                                /** ��������� ��������� **/
         Type          type;                  /* �����                 */
         int           offset;                /* Offset ��� �.�.       */
         int           number;
         bool          isReference;           /* Reference to array    */
         bool          isConst;
         bool          isArrayElement;
         VALUE         value;                 /* ����                  */
         char *        name;
      } eTemporary;

   } u;                               /* ����� ��� union               */
};


/* ����� ������� �������� ��� ���������� ���� ���� �������� */

typedef struct Scope_tag Scope;

struct Scope_tag {
    unsigned int   nestingLevel;             /* ����� �����������      */
    unsigned int   negOffset;                /* ������ �������� offset */
    Scope        * parent;                   /* ������������ ��������  */
    SymbolEntry  * entries;                  /* ������� ��� ���������  */
};


/* ����� ���������� ���� ������ �������� */

typedef enum {
    LOOKUP_CURRENT_SCOPE,
    LOOKUP_ALL_SCOPES
} LookupType;


/* ---------------------------------------------------------------------
   ------------- ��������� ���������� ��� ������ �������� --------------
   --------------------------------------------------------------------- */

extern Scope        * currentScope;       /* �������� ��������         */
extern unsigned int   quadNext;           /* ������� �������� �������� */
extern unsigned int   tempNumber;         /* �������� ��� temporaries  */

extern const Type typeVoid;
extern const Type typeInteger;
extern const Type typeBoolean;
extern const Type typeChar;
extern const Type typeReal;
extern const Type typeString;


/* ---------------------------------------------------------------------
   ------ ��������� ��� ����������� ��������� ��� ������ �������� ------
   --------------------------------------------------------------------- */

void          initSymbolTable    (unsigned int size);
void          destroySymbolTable (void);
void          printSymbolTable   (void);
void          openScope          (void);
void          closeScope         (void);
void          blockScope         (unsigned int start_offset);

SymbolEntry * newVariable        (const char * name, Type type);
SymbolEntry * newConstant        (const char * name, Type type, ...);
SymbolEntry * newFunction        (const char * name);
SymbolEntry * newParameter       (const char * name, Type type,
                                  PassMode mode, SymbolEntry * f);
SymbolEntry * newTemporary       (Type type);
SymbolEntry * newTemporaryRef    (Type type);
SymbolEntry * newTemporary       (Type type, VALUE *value);

void          forwardFunction    (SymbolEntry * f);
void          endFunctionHeader  (SymbolEntry * f, Type type);
void          destroyEntry       (SymbolEntry * e);
SymbolEntry * lookupEntry        (const char * name, LookupType type,
                                  bool err);

Type          typeArray          (RepInteger size, Type refType);
Type          typeIArray         (Type refType);
Type          typePointer        (Type refType);
void          destroyType        (Type type);
unsigned int  sizeOfType         (Type type);
bool          equalType          (Type type1, Type type2);
void          printType          (Type type);
void          printMode          (PassMode mode);
int           num_param          (const char *id);


#endif
