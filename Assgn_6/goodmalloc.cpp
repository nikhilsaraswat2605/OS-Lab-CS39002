#include "goodmalloc.h"
stack *GLOBAL_STACK;   // global stack
s_table *SYMBOL_TABLE; // global symbol table (page table)

int big_memory_sz;
int *BIG_MEMORY = NULL;                  // Points to the start of the BIG_MEMORY
int *BOOKKEEP_MEMORY = NULL;             // Points to the memory segment used for bookkeeping data structures
const int bookkeeping_memory_size = 1e8; // maximum size of bookkeeping memory
const int max_stack_size = 1e5;          // maximum size of symbol table
int CURRENT_SCOPE = 0;                   // current scope
int cnt_del = 0;                         // number of elements deleted by freeElem
int mem_in_use = 0, foot_print = 0;      // memory in use and foot print

void startScope()
{
    CURRENT_SCOPE++; // increment the scope
}
void endScope()
{
    cout << "endScope called" << endl;
    for (int i = GLOBAL_STACK->top; ~i; i--)
    { // iterate over the stack
        if ((GLOBAL_STACK->lis[i].scope_tbf >> 1) == CURRENT_SCOPE)
        {                                                                            // if the scope of the element is the current scope
            SYMBOL_TABLE->unmark(GLOBAL_STACK->lis[i].redirect - SYMBOL_TABLE->lis); // unmark the element
            GLOBAL_STACK->lis[i].scope_tbf |= 1;                                     // mark the element to be freed
            GLOBAL_STACK->pop();                                                     // pop the element from the stack
        }
        else
        {
            CURRENT_SCOPE--; // decrement the scope
            break;
        }
    }
}

int getScope()
{
    return CURRENT_SCOPE; // return the current scope
}

void s_table::s_table_init(int mx, s_table_entry *mem_block)
{                            // initialize the symbol table
    this->lis = mem_block;   // set the lis pointer to the memory block
    this->head_idx = 0;      // set the head index to 0
    this->tail_idx = mx - 1; // set the tail index to mx - 1
    for (int i = 0; i + 1 < mx; i++)
    {                                       // iterate over the lis
        this->lis[i].addr_in_mem = -1;      // set the address in memory to -1
        this->lis[i].next = ((i + 1) << 1); // set the next pointer to i + 1
        this->lis[i].next |= 1;             // mark the next pointer as free
    }
    this->lis[mx - 1].addr_in_mem = -1; // set the address in memory to -1
    this->lis[mx - 1].next = -1;        // set the next pointer to -1
    this->current_size = 0;             // set the current size to 0
    this->mx_size = mx;                 // set the maximum size to mx
}
int s_table::insert(uint32_t addr, uint32_t unit_size, uint32_t total_size)
{ // insert an element in the symbol table
    if (this->current_size == this->mx_size)
    { // if the symbol table is full
        printf("Printed by insert function of s_table class: Symbol table is full\n");
        return -1;
    }
    int idx = head_idx;                     // set the index to the head index
    this->lis[idx].total_size = total_size; // set the total size
    this->lis[idx].unit_size = unit_size;   // set the unit size
    this->lis[idx].addr_in_mem = addr;      // set the address in memory
    this->lis[idx].next |= 1;               // mark as allocated
    head_idx = lis[idx].next >> 1;          // set the head index to the next pointer
    this->current_size++;                   // increment the current size
    return idx;
}
void s_table::remove(uint32_t idx)
{ // remove an element from the symbol table
    if (idx >= this->mx_size or this->current_size <= 0)
    { // if the index is out of bounds or the symbol table is empty
        return;
    }
    this->lis[idx].unit_size = 0;        // set the unit size to 0
    this->lis[idx].total_size = 0;       // set the total size to 0
    this->lis[idx].next = -1;            // set the next pointer to -1
    this->lis[idx].addr_in_mem = -1;     // set the address in memory to -1
    this->lis[tail_idx].next = idx << 1; // set the next pointer of the tail to idx
    this->lis[tail_idx].next |= 1;       // mark the next pointer as free
    tail_idx = idx;                      // set the tail index to idx
    this->current_size--;                // decrement the current size
}
void s_table::unmark(uint32_t idx)
{                              // unmark an element in the symbol table
    this->lis[idx].next &= -2; // unmark the next pointer
}
void stack::stack_init(int mx, stack_entry *mem_block)
{                          // initialize the stack
    this->lis = mem_block; // set the lis pointer to the memory block
    this->top = -1;        // set the top to -1
    this->max_size = mx;   // set the maximum size to mx
}
stack_entry *stack::pop()
{ // pop an element from the stack
    if (this->top != -1)
    {                                         // if the stack is not empty
        this->top--;                          // decrement the top
        auto ret = &this->lis[this->top + 1]; // set the return pointer to the top + 1
        return ret;                           // should be processed before the next push else race condition
    }
    else
    { // if the stack is empty
        printf("Printed by pop function of stack class: stack is empty\n");
        return NULL;
    }
}
void stack::push(s_table_entry *redirect_ptr, const char *name)
{                // push an element in the stack
    this->top++; // increment the top
    if (this->top >= this->max_size)
    { // if the stack is full
        printf("Printed by push function of stack class: Stack Overflow\n");
        return;
    }
    this->lis[this->top].redirect = redirect_ptr;        // set the redirect pointer
    strcpy(this->lis[this->top].name, name);             // set the name
    this->lis[this->top].scope_tbf = CURRENT_SCOPE << 1; // set the scope
}

s_table_entry *stack::get_s_table_entry(const char *name, int scope)
{                                    // get the s_table_entry pointer from the stack
    for (int i = this->top; ~i; i--) // iterate over the stack
    {
        if (!strcmp(this->lis[i].name, name) && (this->lis[i].scope_tbf >> 1) == scope) // if the name and scope matches
        {
            auto ret = this->lis[i].redirect; // set the return pointer to the redirect pointer
            return ret;
        }
        else if ((this->lis[i].scope_tbf >> 1) < scope) // if the scope is less than the current scope
        {
            break;
        }
    }

    // // binary search for the name in the stack and return the pointer to the s_table_entry
    // int l = 0, r = this->top;
    // while (l <= r)
    // {
    //     int mid = (l + r) / 2;
    //     if(scope < (this->lis[mid].scope_tbf >> 1)){
    //         r = mid - 1;
    //     }else if(scope > (this->lis[mid].scope_tbf >> 1)){
    //         l = mid + 1;
    //     }else{
    //         // now linear search for the name
    //         int i = mid;
    //         while(i >= 0 && (this->lis[i].scope_tbf >> 1) == scope){
    //             if(!strcmp(this->lis[i].name, name)){
    //                 return this->lis[i].redirect;
    //             }
    //             i--;
    //         }
    //         i = mid + 1;
    //         while(i <= this->top && (this->lis[i].scope_tbf >> 1) == scope){
    //             if(!strcmp(this->lis[i].name, name)){
    //                 return this->lis[i].redirect;
    //             }
    //             i++;
    //         }
    //     }
    // }

    return NULL;
}

void CreateMem(int size)
{
    BIG_MEMORY = new int[((size + 3) / 4)](); // nearest multiple of 4 to size
    big_memory_sz = ((size + 3) / 4);         // nearest multiple of 4 to size
    printf("Printed by CreateMem Function: Allocated %d bytes of data as requested\n", ((size + 3) / 4) * 4);

    BOOKKEEP_MEMORY = new int[bookkeeping_memory_size](); // bookkeeping memory
    BIG_MEMORY[0] = (big_memory_sz) << 1;                 // set the size of the memory
    BIG_MEMORY[big_memory_sz - 1] = (big_memory_sz) << 1; // set the size of the memory

    printf("Printed by CreateMem Function: Allocated %d bytes of data for bookkeeping\n", bookkeeping_memory_size);

    GLOBAL_STACK = (stack *)(BOOKKEEP_MEMORY);                                   // set the global stack pointer to the bookkeeping memory
    GLOBAL_STACK->stack_init(max_stack_size, (stack_entry *)(GLOBAL_STACK + 1)); // initialize the global stack

    s_table *S_TABLE_START = (s_table *)(GLOBAL_STACK->lis + GLOBAL_STACK->max_size); // set the symbol table pointer to the bookkeeping memory
    SYMBOL_TABLE = S_TABLE_START;                                                     // set the symbol table pointer to the bookkeeping memory
    SYMBOL_TABLE->s_table_init(max_stack_size, (s_table_entry *)(SYMBOL_TABLE + 1));  // initialize the symbol table

    printf("Printed by CreateMem Function: Setup Stack and Symbol Table\n");
}


uint32_t accessList(const char *name, int idx, int scope)
{
    if (scope == -1)
        scope = CURRENT_SCOPE;                                         // if scope is not specified, use the current scope
    s_table_entry *var = GLOBAL_STACK->get_s_table_entry(name, scope); // get the variable from the stack
    if (var == NULL)
    { // if the variable is not found
        printf("Printed by accessList: Trying to access an undefined variable %s\n", name);
        exit(1);
    }
    int correct_unit_size = var->unit_size; // correct unit size
    if (correct_unit_size == 24)
        correct_unit_size = 32;                                       // if the unit size is 24, change it to 32
    int main_idx = var->addr_in_mem + (idx * correct_unit_size) / 32; // get the index in the main memory
    int offset = (idx * correct_unit_size) % 32;                      // get the offset
    int end_offset = (offset + correct_unit_size - 1) % 32;           // get the end offset
    uint32_t val = *((int *)(BIG_MEMORY + main_idx));                 // get the value
    val = ~((1L << (31 - end_offset)) - 1) & val;                     // remove all bits after offset  val looks like  ......usefulf000000
    val = val << offset;                                              // shift left to align          val looks liek useful000000000000
    int ret = val;
    ret = ret >> (32 - var->unit_size); // shift right to align
    return ret;
}

int AllocMainMemory(int size)
{

    int *p = BIG_MEMORY;                                                            // pointer to the start of the memory
    int newsize = (((size + 3) >> 2) << 2);                                         // nearest multiple of 4 to size
    newsize += 2 * sizeof(int);                                                     // add the size of the header and footer
    while ((p - BIG_MEMORY < big_memory_sz) && ((*p & 1) || ((*p << 1) < newsize))) // while the block is in use or the block is smaller than the required size
        p = p + (*p >> 1);                                                          // increment the pointer by the size of the block
    if (p - BIG_MEMORY >= big_memory_sz)                                            // if the block is not found
    {
        cout << p - BIG_MEMORY << " " << big_memory_sz << endl;
        return -1;
    }
    int oldsize = *p << 1;               // old size of the block
    int words = newsize >> 2;            // number of 4 byte blocks we need
    *p = (words << 1) | 1;               // set the header of the new block, first 31 bits: words, last bit: 1 (in use)
    *(p + words - 1) = (words << 1) | 1; // footer: same as above
    if (newsize < oldsize)               // If some blocks are left
    {
        *(p + words) = (oldsize - newsize) >> 1;              // header of the new block, last bit 0 as free
        *(p + (oldsize >> 2) - 1) = (oldsize - newsize) >> 1; // footer of the new block, last bit is 0 as free
    }
    return (p - BIG_MEMORY);
}
void DeallocMainMemory(int *ptr)
{
    *ptr = *ptr & -2;                                      // clear allocated flag
    *(ptr + (*(ptr) >> 1) - 1) = *ptr;                     // clear allocated flag
    int *next = ptr + (*ptr >> 1);                         // find next block
    if (next - BIG_MEMORY < big_memory_sz && !(*next & 1)) // if next block is free
    {
        *ptr += *next;                   // merge with next block
        *(ptr + (*ptr >> 1) - 1) = *ptr; // update boundary tag
    }
    if (ptr != BIG_MEMORY) // there is a block before
    {
        int *prev = ptr - (*(ptr - 1) >> 1); // find previous block
        if (!(*prev & 1))                    // if previous block is free
        {
            *prev += *ptr;                      // merge with previous block
            *(prev + (*prev >> 1) - 1) = *prev; // update boundary tag
        }
    }
}

void CreateList(const char *name, int sz)
{
    int main_memory_idx, unit_size, total_size; // main memory index, unit size, total size

    main_memory_idx = AllocMainMemory((32 * sz) / 8); // allocate memory for the list
    unit_size = 32;                                   // unit size is 32 bits
    total_size = unit_size * sz;                      // total size is unit size * number of elements
    if (name == NULL)
    {
        printf("Printed by CreateList Function: Failed to create list\n");
        return;
    }
    for (int i = 0; i < GLOBAL_STACK->top; i++)
    { // check for name conflict

        if (GLOBAL_STACK->lis[i].scope_tbf >> 1 == CURRENT_SCOPE) // check if the scope is the same
        {
            if (strcmp(GLOBAL_STACK->lis[i].name, name) == 0) // check if the name is the same
            {
                printf("Printed by CreateList Function: Failed to create list due to name conflict\n");
                return;
            }
        }
        else
        {
            break;
        }
    }

    int idx = SYMBOL_TABLE->insert(main_memory_idx + 1, unit_size, total_size); // add plus one to account for header

    printf("Printed by CreateList Function: Created list %s of size %d, total size %d at index %d in scope %d\n", name, unit_size, total_size, main_memory_idx, CURRENT_SCOPE);
    GLOBAL_STACK->push(SYMBOL_TABLE->lis + idx, name);                                         // push the list to the stack
    mem_in_use += total_size;                                                                  // update the memory in use
    foot_print = max(foot_print, (int)(mem_in_use + GLOBAL_STACK->top * sizeof(stack_entry))); // update the memory footprint
    // cout << "Memory footprint: " << foot_print / 8 << " bytes" << endl; // print the memory footprint
}

void assignVal(const char *name, int idx, uint32_t val, int scope)
{
    if (scope == -1)
        scope = CURRENT_SCOPE;                                         // if scope is not specified, use the current scope
    s_table_entry *lis = GLOBAL_STACK->get_s_table_entry(name, scope); // get the variable from the stack
    if (lis == NULL)
    { // if the variable is not found
        printf("Printed by assignVal: Trying to access an undefined variable %s\n", name);
        exit(1);
    }
    // check if the index is valid
    if (idx >= lis->total_size / lis->unit_size)
    { // if the index is out of bounds
        printf("Printed by assignVal: Trying to access an index out of bounds\n");
        exit(1);
    }
    else if (idx < 0)
    { // if the index is negative
        printf("Printed by assignVal: Trying to access an index out of bounds\n");
        exit(1);
    }
    *((int *)(BIG_MEMORY + lis->addr_in_mem + (idx * lis->unit_size) / 32)) = val; // assign the value to the main memory
}

void freeElem(const char *name)
{
    s_table_entry *var = GLOBAL_STACK->get_s_table_entry(name, CURRENT_SCOPE); // get the variable from the stack
    if (var == NULL)
    { // if the variable is not found
        printf("Printed by freeElem function: Trying to free a variable that is not declared\n");
        exit(1);
        // return;
    }

    SYMBOL_TABLE->unmark(var - SYMBOL_TABLE->lis); // unmark the symbol table entry
    cnt_del++;                                     // increment the number of deleted variables
    if (cnt_del == 1000)                           // if the number of deleted variables is 1000
    {
        freeElem_helper(); // call the helper function
        cnt_del = 0;       // reset the number of deleted variables
    }
    mem_in_use -= var->total_size;                                                             // update the memory in use
    foot_print = max(foot_print, (int)(mem_in_use + GLOBAL_STACK->top * sizeof(stack_entry))); // update the memory footprint
    // cout << "Memory footprint: " << foot_print / 8 << " bytes" << endl; // print the memory footprint
}

void freeElem()
{
    for (int i = GLOBAL_STACK->top; ~i; i--) // traverse the stack from top to bottom
    {
        // if the scope is the current scope
        if ((GLOBAL_STACK->lis[i].scope_tbf >> 1) == CURRENT_SCOPE)
        {                                                                                                   // if the scope is the current scope
            SYMBOL_TABLE->unmark(GLOBAL_STACK->lis[i].redirect - SYMBOL_TABLE->lis);                        // unmark the symbol table entry
            GLOBAL_STACK->lis[i].scope_tbf |= 1;                                                            // mark the stack entry as to be freed
            GLOBAL_STACK->pop();                                                                            // pop the stack entry
            s_table_entry *var = GLOBAL_STACK->get_s_table_entry(GLOBAL_STACK->lis[i].name, CURRENT_SCOPE); // get the variable
            if (var == NULL)
                continue;                        // if the variable is not found, continue
            freeElem(GLOBAL_STACK->lis[i].name); // free the variable
            mem_in_use -= var->total_size;       // decrease the memory in use

            foot_print = max(foot_print, (int)(mem_in_use + GLOBAL_STACK->top * sizeof(stack_entry))); // update the memory footprint

            // cout << "Memory footprint: " << foot_print / 8 << " bytes" << endl;
        }
        else
            break;
    }
}

void freeElem_inner(s_table_entry *var)
{
    for (int i = 0; i <= GLOBAL_STACK->top; i++)
    {                                             // traverse the stack from top to bottom
        if (GLOBAL_STACK->lis[i].redirect != var) // if the stack entry is not the variable
        {
            continue;
        }
        else
        {
            GLOBAL_STACK->lis[i].scope_tbf |= 1; // mark the stack entry as to be freed
            break;
        }
    }
    if (var->addr_in_mem and var->addr_in_mem - 1 < big_memory_sz)
    {                                                           // if the variable is in memory
        DeallocMainMemory(BIG_MEMORY + (var->addr_in_mem - 1)); // addr is one after the header so -1
        SYMBOL_TABLE->remove(var - SYMBOL_TABLE->lis);          // remove the entry from the symbol table
    }
}

int partial_compact()
{
    int compact_count = 0;
    // write compact once code here
    // traverse the list
    // at the first hole
    // copy the elements
    // remember the old address
    // find the entry in the symbol with the old address and update it
    int *p = BIG_MEMORY;
    int *next = p + (*p >> 1);
    while (BIG_MEMORY + big_memory_sz != next)
    { // while the next is not the end of the memory
        if ((*p & 1) == 0 && (*next & 1) == 1)
        {                               // if the current block is free and the next block is allocated
            int sz2 = *next >> 1;       // size of the next block
            int sz1 = *p >> 1;          // size of the current block
            memmove(p, next, sz2 << 2); // move the next block to the current block

            for (int j = 0; j < SYMBOL_TABLE->mx_size; j++)
            { // update the address in the symbol table
                if (SYMBOL_TABLE->lis[j].addr_in_mem + 1 != (next - BIG_MEMORY))
                { // if the address is not the same as the old address
                    continue;
                }
                else
                {
                    SYMBOL_TABLE->lis[j].addr_in_mem = (p - BIG_MEMORY); // update the address
                    break;
                }
            }

            p = p + sz2;                                            // update the pointers
            *p = sz1 << 1;                                          // update the header
            *(p + sz1 - 1) = sz1 << 1;                              // update the footer
            next = p + sz1;                                         // update the next pointer
            if (next < BIG_MEMORY + big_memory_sz and !(*next & 1)) // coalesce if the next block is free
            {                                                       // coalesce
                sz1 = sz1 + (*next >> 1);                           // update the size
                *p = sz1 << 1;                                      // update the header
                *(p + sz1 - 1) = sz1 << 1;                          // update the footer
                next = p + sz1;                                     // update the next pointer
            }
            compact_count++; // increase the compact count
            break;
        }
        else
        {
            p = next;             // update the pointers
            next = p + (*p >> 1); // update the next pointer
        }
    }

    return compact_count;
}

void complete_compact()
{
    int compact_count = 1; // initialize the compact count
    while (partial_compact())
    {                    // while the compact once function returns true
        compact_count++; // increase the compact count
    }
}

void freeElem_helper()
{
    for (int i = 0; i <= GLOBAL_STACK->top; i++) // remove all other entries which are to be freed from the stack
    {
        if (GLOBAL_STACK->lis[i].scope_tbf & 1)
        {                                                                                          // if the stack entry is to be freed
            if (~SYMBOL_TABLE->lis[GLOBAL_STACK->lis[i].redirect - SYMBOL_TABLE->lis].addr_in_mem) // if the variable is in memory
            {
                SYMBOL_TABLE->unmark(GLOBAL_STACK->lis[i].redirect - SYMBOL_TABLE->lis); // unmark the symbol table entry
            }
            GLOBAL_STACK->top--;                                 // decrease the top of the stack
            for (int j = i; j < GLOBAL_STACK->top; j++)          // shift the stack entries
                GLOBAL_STACK->lis[j] = GLOBAL_STACK->lis[j + 1]; // shift the stack entries
        }
    }
    for (int i = 0; i < SYMBOL_TABLE->mx_size; i++)
    {                                         // free all the variables which are to be freed
        if (!(SYMBOL_TABLE->lis[i].next & 1)) // if the symbol table entry is to be freed
        {
            freeElem_inner(&SYMBOL_TABLE->lis[i]); // free the variable
        }
    }

    complete_compact(); // compact the memory
}

void freeMem()
{
    delete[] (BIG_MEMORY); // free the big memory

    printf("Printed by freeMem function: Big memory freed\n");
    delete[] (BOOKKEEP_MEMORY); // free the book keeping memory
    printf("Printed by freeMem function: Book keeping memory freed\n");
}
