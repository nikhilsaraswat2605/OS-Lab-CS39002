#ifndef GOODMALLOC_H
#define GOODMALLOC_H
#include <iostream>
#include <stdio.h>
#include <bitset>
#include <csignal>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
using namespace std;

struct s_table_entry
{
    uint32_t next;        //  31 bits idx to the next stable_entry, last bit saying if this block is to not be freed or not
    uint32_t addr_in_mem; // index in memory
    uint32_t total_size;  // total number of bits used in memory
    uint32_t unit_size;   // size of a unit in bits, eg bool=1, int = 32, char = 8, medium_int = 24
    int is_free()
    {
        return this->next & 1;
    }
};
struct s_table
{
    int current_size;
    int mx_size;
    int head_idx;
    int tail_idx;
    s_table_entry *lis;

    void s_table_init(int, s_table_entry *);                            // constructs
    int insert(uint32_t addr, uint32_t unit_size, uint32_t total_size); // inserts at the tail of the list
    void remove(uint32_t idx);                                          // removes entry at idx
    void unmark(uint32_t idx);                                          // unmakrs entry at idx
    void print_s_table();                                               // prints the symbol table
};
struct stack_entry
{
    s_table_entry *redirect; // pointer to the stable_entry
    char name[20];
    int scope_tbf;           // first 31 bits scope number, last bit tells us if the entry has to be freed
};
// Linked list of stable_entries to make the symbol table
// create Stack out of stack_entry
struct stack
{
    int top;                             // index of top in lis
    int max_size;                        // max size of the stack
    stack_entry *lis;                    // lisay implementation of stack
    void stack_init(int, stack_entry *); // constructor
    void push(s_table_entry *, const char *);          // pushes an entry onto the stack
    stack_entry *pop();                  // pops an entry from the stack
    s_table_entry *top_ret();            // returns the top of the stack
    s_table_entry * get_s_table_entry(const char *, int); // returns the stable_entry with the given name
};
void CreateMem(int);                                        // A function to create a memory segment using malloc
void CreateList(const char *, int);                     // Returns the symbol table entry. Using this function you can create an lisay of the above types. These variables reside in the memory created by createMem.
void assignVal(const char *, int, uint32_t, int=-1);              // Pass the symbol table entry. Assign values to lisay or lisay elements. Have a light typechecking, your boolean variable cannot hold an int etc
uint32_t accessList(const char *, int = 0, int=-1);                                                       // returns the value of the variable
void freeElem(const char *);                                // Mark the element to be freed by the garbage collector
void freeElem();                                // Mark the element to be freed by the garbage collector
void freeMem();                                                // Free the memory segment created by createMem // Extra
void freeElem_inner(s_table_entry *var);                       // called by gc_run_inner to remove the element from the tables and the memory
extern stack *GLOBAL_STACK;
extern s_table *SYMBOL_TABLE;
extern int big_memory_sz;
extern int *BIG_MEMORY;                                                                             // Pointer to the start of the BIG_MEMORY, int for enforcing word allignment
extern int *BOOKKEEP_MEMORY;                                                                        // Pointer to the memory segment used for bookkeeping data structures
extern const int bookkeeping_memory_size;                                                           // max size of bookkeeping memory
extern const int max_stack_size;                                                                    // also max size of symbol table
extern int CURRENT_SCOPE;                                                                           // current scope                                                                       // prints the big memory
void startScope();
void endScope();
void freeElem_helper();
void complete_compact();
int partial_compact();
int getScope();

int AllocMainMemory(int size);
void DeallocMainMemory(int *ptr);

#endif