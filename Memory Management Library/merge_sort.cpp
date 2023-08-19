#include "goodmalloc.h"
#include <fstream>
#define MAX_SIZE 50000
void merge(const char *lis, int scope, int l, int m, int r)
{
    startScope();
    // create temp lists
    int n1 = m - l + 1;
    int n2 = r - m;
    CreateList("a", n1);
    CreateList("b", n2);
    // copy data to temp lists L[] and R[]
    for (int j = 0; j < n2; j++)
    {
        assignVal("b", j, accessList(lis, m + 1 + j, scope));
    }
    for (int i = 0; i < n1; i++)
    {
        assignVal("a", i, accessList(lis, l + i, scope));
    }
    // merge the temp lists back into lis[l..r]
    int k = l; // initial index of merged sublist
    int j = 0; // initial index of second sublist
    int i = 0; // initial index of first sublist
    while (i < n1 && j < n2)
    {
        if (accessList("a", i) > accessList("b", j))
        {
            assignVal(lis, k, accessList("b", j), scope);
            j++;
        }
        else
        {
            assignVal(lis, k, accessList("a", i), scope);
            i++;
        }
        k++;
    }
    // copy the remaining elements of "b"[], if there are any
    while (j < n2)
    {
        assignVal(lis, k, accessList("b", j), scope);
        k++;
        j++;
    }
    // copy the remaining elements of "a"[], if there are any
    while (i < n1)
    {
        assignVal(lis, k, accessList("a", i), scope);
        k++;
        i++;
    }
    freeElem("a");
    freeElem("b");
    endScope();
}

void merge_sort(const char *lis, int scope, int l, int r)
{
    startScope();
    if (l < r)
    {
        int m = l + (r - l) / 2;
        merge_sort(lis, scope, l, m);
        merge_sort(lis, scope, m + 1, r);
        merge(lis, scope, l, m, r);
    }
    
    endScope();
}

int main()
{
    srand(time(NULL));

    CreateMem(250*1024*1024);

    startScope();
    CreateList("c", MAX_SIZE);
    for (int i = 0; i < MAX_SIZE; i++)
    {
        assignVal("c", i, (rand() % 100000)+1);
    }
  
    
    ofstream out("output.txt");
    
    out<<"Before Sorting:"<<endl;
    for (int i = 0; i < MAX_SIZE; i++)
    {
        out << accessList("c", i) << " ";
    }
    
    out << endl;
    merge_sort("c", getScope(), 0, MAX_SIZE - 1);
    out<<"After Sorting:"<<endl;
    for (int i = 0; i < MAX_SIZE; i++)
    {
        out << accessList("c", i) << " ";
    }
    out << endl;
    out.close();
    freeElem("c");
    endScope();
    freeMem();
    return 0;
}