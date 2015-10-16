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
 *  --------- *  Εθνικό Μετσόβιο Πολυτεχνείο.
 *  Σχολή Ηλεκτρολόγων Μηχανικών και Μηχανικών Υπολογιστών.
 *  Τομέας Τεχνολογίας Πληροφορικής και Υπολογιστών.
 *  Εργαστήριο Τεχνολογίας Λογισμικού
 */


#ifndef __SYMBOL_H__
#define __SYMBOL_H__


/* ---------------------------------------------------------------------
   -------------------------- Τύπος bool -------------------------------
   --------------------------------------------------------------------- */

/*
 *  Αν το παραπάνω include δεν υποστηρίζεται από την υλοποίηση
 *  της C που χρησιμοποιείτε, αντικαταστήστε το με το ακόλουθο:
 */
#include <stdbool.h>
#if 0
typedef enum { false, true } bool;
#endif


/* ---------------------------------------------------------------------
   ------------ Ορισμός σταθερών του πίνακα συμβόλων -------------------
   --------------------------------------------------------------------- */

#define START_POSITIVE_OFFSET 12     /* Αρχικό θετικό offset στο Ε.Δ.   */
#define START_NEGATIVE_OFFSET 0     /* Αρχικό αρνητικό offset στο Ε.Δ. */


/* ---------------------------------------------------------------------
   --------------- Ορισμός τύπων του πίνακα συμβόλων -------------------
   --------------------------------------------------------------------- */

/* Τύποι δεδομένων για την υλοποίηση των σταθερών */

typedef int           RepInteger;         /* Ακέραιες                  */
typedef unsigned char RepBoolean;         /* Λογικές τιμές             */
typedef char          RepChar;            /* Χαρακτήρες                */
typedef long double   RepReal;            /* Πραγματικές               */
typedef const char *  RepString;          /* Συμβολοσειρές             */


/* Τύποι δεδομένων και αποτελέσματος συναρτήσεων */

typedef struct Type_tag * Type;

typedef enum {                           /***** Το είδος του τύπου ****/
   TYPE_VOID,                            /* Κενός τύπος αποτελέσματος */
   TYPE_INTEGER,                         /* Ακέραιοι                  */
   TYPE_BOOLEAN,                         /* Λογικές τιμές             */
   TYPE_CHAR,                            /* Χαρακτήρες                */
   TYPE_REAL,                            /* Πραγματικοί               */
   TYPE_ARRAY,                           /* Πίνακες γνωστού μεγέθους  */
   TYPE_IARRAY,                          /* Πίνακες άγνωστου μεγέθους */
   TYPE_POINTER                          /* Δείκτες                   */
} KIND;                           

struct Type_tag {
    KIND           kind;
    Type           refType;              /* Τύπος αναφοράς            */
    RepInteger     size;                 /* Μέγεθος, αν είναι πίνακας */
    unsigned int   refCount;             /* Μετρητής αναφορών         */
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

typedef union {                          /* Τιμή                      */
    RepInteger vInteger;                 /*    ακέραια                */
    RepBoolean vBoolean;                 /*    λογική                 */
    RepChar    vChar;                    /*    χαρακτήρας             */
    RepReal    vReal;                    /*    πραγματική             */
    RepString  vString;                  /*    συμβολοσειρά           */
} VALUE;                 
                         
/* Τύποι εγγραφών του πίνακα συμβόλων */

typedef enum {            
   ENTRY_VARIABLE,                       /* Μεταβλητές                 */
   ENTRY_CONSTANT,                       /* Σταθερές                   */
   ENTRY_FUNCTION,                       /* Συναρτήσεις                */
   ENTRY_PARAMETER,                      /* Παράμετροι συναρτήσεων     */
   ENTRY_TEMPORARY                       /* Προσωρινές μεταβλητές      */
} EntryType;


/* Τύποι περάσματος παραμετρων */

typedef enum {            
   PASS_BY_VALUE,                        /* Κατ' αξία                  */
   PASS_BY_REFERENCE                     /* Κατ' αναφορά               */
} PassMode;


typedef enum {                                /* Κατάσταση παραμέτρων  */
    PARDEF_COMPLETE,                             /* Πλήρης ορισμός     */
    PARDEF_DEFINE,                               /* Εν μέσω ορισμού    */
    PARDEF_CHECK                                 /* Εν μέσω ελέγχου    */
} PARDEF;                    

/* Τύπος εγγραφής στον πίνακα συμβόλων */

typedef struct SymbolEntry_tag SymbolEntry;

struct SymbolEntry_tag {
   const char   * id;                 /* Ονομα αναγνωριστικού          */
   EntryType      entryType;          /* Τύπος της εγγραφής            */
   unsigned int   nestingLevel;       /* Βάθος φωλιάσματος             */
   unsigned int   hashValue;          /* Τιμή κατακερματισμού          */
   SymbolEntry  * nextHash;           /* Επόμενη εγγραφή στον Π.Κ.     */
   SymbolEntry  * nextInScope;        /* Επόμενη εγγραφή στην εμβέλεια */
   const char   * final_code_name;    /* Όνομα στον τελικό κώδικα      */
   bool           isGlobal;           /* Αν είναι Global Variable      */

   union {                            /* Ανάλογα με τον τύπο εγγραφής: */

      struct {                                /******* Μεταβλητή *******/
         Type          type;                  /* Τύπος                 */
         int           offset;                /* Offset στο Ε.Δ.       */
      } eVariable;

      struct {                                /******** Σταθερά ********/
         Type          type;                  /* Τύπος                 */
         VALUE         value;                 /* Τιμή                  */
      } eConstant;

      struct {                                /******* Συνάρτηση *******/
         bool          isForward;             /* Δήλωση forward        */
         SymbolEntry * firstArgument;         /* Λίστα παραμέτρων      */
         SymbolEntry * lastArgument;          /* Τελευταία παράμετρος  */
         Type          resultType;            /* Τύπος αποτελέσματος   */
         PARDEF        pardef;
         int           firstQuad;             /* Αρχική τετράδα        */
         unsigned int  negOffset;             /* Μέγιστο Offset        */
      } eFunction;

      struct {                                /****** Παράμετρος *******/
         Type          type;                  /* Τύπος                 */
         int           offset;                /* Offset στο Ε.Δ.       */
         PassMode      mode;                  /* Τρόπος περάσματος     */
         SymbolEntry * next;                  /* Επόμενη παράμετρος    */
      } eParameter;

      struct {                                /** Προσωρινή μεταβλητή **/
         Type          type;                  /* Τύπος                 */
         int           offset;                /* Offset στο Ε.Δ.       */
         int           number;
         bool          isReference;           /* Reference to array    */
         bool          isConst;
         bool          isArrayElement;
         VALUE         value;                 /* Τιμή                  */
         char *        name;
      } eTemporary;

   } u;                               /* Τέλος του union               */
};


/* Τύπος συνόλου εγγραφών που βρίσκονται στην ίδια εμβέλεια */

typedef struct Scope_tag Scope;

struct Scope_tag {
    unsigned int   nestingLevel;             /* Βάθος φωλιάσματος      */
    unsigned int   negOffset;                /* Τρέχον αρνητικό offset */
    Scope        * parent;                   /* Περιβάλλουσα εμβέλεια  */
    SymbolEntry  * entries;                  /* Σύμβολα της εμβέλειας  */
};


/* Τύπος αναζήτησης στον πίνακα συμβόλων */

typedef enum {
    LOOKUP_CURRENT_SCOPE,
    LOOKUP_ALL_SCOPES
} LookupType;


/* ---------------------------------------------------------------------
   ------------- Καθολικές μεταβλητές του πίνακα συμβόλων --------------
   --------------------------------------------------------------------- */

extern Scope        * currentScope;       /* Τρέχουσα εμβέλεια         */
extern unsigned int   quadNext;           /* Αριθμός επόμενης τετράδας */
extern unsigned int   tempNumber;         /* Αρίθμηση των temporaries  */

extern const Type typeVoid;
extern const Type typeInteger;
extern const Type typeBoolean;
extern const Type typeChar;
extern const Type typeReal;
extern const Type typeString;


/* ---------------------------------------------------------------------
   ------ Πρωτότυπα των συναρτήσεων χειρισμού του πίνακα συμβολών ------
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
