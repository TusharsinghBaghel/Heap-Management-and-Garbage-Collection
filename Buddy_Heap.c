#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<math.h>
typedef enum{FALSE, TRUE} Bool;
//////////////////////////IMPLEMENTATION OF BUDDY HEAP MANAGEMENT SYSTEM//////////////////////
#define POWER 10
typedef struct metalist metalist;
typedef struct meta_node{
    int index;
    int st_address;
    size_t mem_size;
    struct meta_node *next;
    struct meta_node *prev;
    //The values inside the memory block
    int value;
    metalist *pointers;
    //For reference counting
    int no_ref;
    //For Mark-Sweep Mechanism
    Bool Mark;
    
} meta_node;

typedef struct metalist{
    meta_node *data;
    struct metalist *nextptr;
}metalist;


meta_node *buddy_freelist[POWER+1];

meta_node *allocated_list = NULL;

void* heap;
void IncrementReference(meta_node*, int inc);
void IncrementReference(meta_node *incref, int inc){
    incref->no_ref+=inc;
    metalist *adj = incref->pointers;
    while(adj){
        IncrementReference(adj->data, inc);
        adj = adj->nextptr;
    }
}

metalist *insertPointerAtEnd(meta_node *parent, meta_node *newptr){
    metalist *list = parent->pointers;
    metalist *newnode = (metalist *)malloc(sizeof(metalist));
    newnode->data = newptr;
    newnode->nextptr=NULL;
    IncrementReference(newnode->data, parent->no_ref);
    if(list==NULL) return newnode;
    metalist *tail = list;
    while(tail->nextptr){
        //IncrementReference(tail->data, parent->no_ref);
        tail = tail->nextptr;
    }
    tail->nextptr = newnode;
    //IncrementReference(newnode->data, parent->no_ref);
    return list;

}

void Initialize_Buddylist(){
    buddy_freelist[POWER] = (meta_node*)malloc(sizeof(meta_node));
    buddy_freelist[POWER]->index=10;
    buddy_freelist[POWER]->st_address = 0;
    buddy_freelist[POWER]->mem_size = 1024;
    buddy_freelist[POWER]->next=NULL;
    buddy_freelist[POWER]->prev=NULL;
    buddy_freelist[POWER]->pointers=NULL;
    buddy_freelist[POWER]->no_ref = 1;

    for(int i=0;i<POWER;i++){
        buddy_freelist[i] = NULL;
    }

}

int getindex(int alloc_size){
    int n = ceil(log(alloc_size) / log(2)); 
    return n;
}

//returns the free block to allocate or split
meta_node *Alloctheindex(int index){
    if(index>POWER) return NULL;
    else if(buddy_freelist[index]==NULL){
        return Alloctheindex(index+1);
    }
    else{

        meta_node *new = buddy_freelist[index];
        buddy_freelist[index] = buddy_freelist[index]->next;
        if(buddy_freelist[index]!=NULL) buddy_freelist[index]->prev =NULL;
        new->next=NULL;
        new->prev=NULL;
        new->pointers=NULL;
        new->no_ref=0;

        return new;
    }
}


void AddToFreelist(meta_node *new){
    int index = new->index;
    if(buddy_freelist[index]==NULL) buddy_freelist[index]= new;
    else{
        meta_node *tail = buddy_freelist[index];
        while(tail->next!=NULL){
            tail = tail->next;
        }
        tail->next = new;
        new->prev =tail;
    }

}



meta_node* SplitTheApprox(meta_node* approx, int index){
    if(approx->index==index) return approx;
    else{
        //we will split the block once into two meta_nodes
        meta_node *new = (meta_node*)malloc(sizeof(meta_node)); //new is the later part of the splitted block
        new->next=NULL;
        new->prev=NULL;
        new->index = approx->index-1;
        new->mem_size = approx->mem_size/2;
        new->st_address = approx->st_address+(approx->mem_size)/2; //updating the approx to become the first part
        approx->index--;
        approx->mem_size = approx->mem_size/2;
        
        //store one of the splitted block in buddy_freelist(Storing the new block)
        AddToFreelist(new);
        //then recursively call the function again on one of the splitted block
        return SplitTheApprox(approx, index);

    }
}

void AddToAllocList(meta_node *new){
    new->next = allocated_list;
    if(allocated_list!=NULL) allocated_list->prev = new;
    allocated_list = new;
}

meta_node *BuddyMalloc(int size){
    int index = getindex(size);
    meta_node *approxmem = Alloctheindex(index);
    if(approxmem->index==index){
        AddToAllocList(approxmem);
        return approxmem;
    }
    else{
        meta_node *new =  SplitTheApprox(approxmem,index);
        AddToAllocList(new);
        return new;
    }
}

void Compaction(meta_node *recentfree){
    int index = recentfree->index;
    int neighborend = recentfree->st_address-1;
    int neighborfront = recentfree->st_address+recentfree->mem_size;
    int found=0;
    meta_node *iter = buddy_freelist[index];
    while(iter!=NULL && !found){
        if(iter->st_address==neighborfront || iter->st_address+iter->mem_size-1 == neighborend){
            found++;
        }
        else iter = iter->next;
    }
    if(found){
        printf("Compacting size %d to %d\n",iter->mem_size,iter->mem_size*2);
        meta_node *compacted = iter;
        if(compacted->prev ==NULL){
            //first element
            buddy_freelist[index] = compacted->next;
            if(buddy_freelist[index]!=NULL) buddy_freelist[index]->prev=NULL;
            compacted->next = NULL;
        }
        else{
            if(compacted->prev!=NULL) compacted->prev->next = compacted->next;
            if(compacted->next!=NULL) compacted->next->prev = compacted->prev;
            compacted->next=NULL;
            compacted->prev=NULL;
        }
        if(neighborfront==compacted->st_address){
            //case where compacted is the first half
            compacted->index++;
            compacted->mem_size = compacted->mem_size*2;
            Compaction(compacted);
        }
        else{
            //case where compacted is the second half
            recentfree->index++;
            recentfree->mem_size = recentfree->mem_size*2;
            Compaction(recentfree);
        }
    }
    else{
        AddToFreelist(recentfree);
    }
}
void Dereference(meta_node *, int);


void BuddyFree(meta_node *tofree){
    //Remove tofree node from allocated list
    meta_node *prev = tofree->prev;
    if(prev==NULL){
        if(allocated_list!=NULL){
            allocated_list = allocated_list->next;
            if(allocated_list!=NULL) allocated_list->prev=NULL;
        } 
        tofree->next = NULL;
    }
    else{
        prev->next = tofree->next;
        if(tofree->next!=NULL) tofree->next->prev = prev;
        tofree->next=NULL;
        tofree->prev=NULL;
    }
    //Add tofree to freelist
    Compaction(tofree);
    //Update the refcount of adjacent
    //Dereference(tofree,1);
    
}

void PrintHeap(){
    printf(":::::::::::::::::::::::::::::::::::::::::PRINTING THE HEAP:::::::::::::::::::::::::::::::::::::::::::::\n");

    printf("::::::::Allocated Blocks::::::\n\n");
    meta_node *alloc = allocated_list;
    while(alloc!=NULL){
        printf("%d: %d to %d\n",alloc->value,alloc->st_address,alloc->mem_size+alloc->st_address-1);
        alloc=alloc->next;
    }
    printf("::::::Free Blocks with their pools:::::\n\n");
    for(int i=0;i<=POWER;i++){
        printf("Pool %d>>\n",(int)pow(2,i));
        meta_node *newf = buddy_freelist[i];
        while(newf!=NULL){
            printf("           %d to %d\n",newf->st_address,newf->st_address+newf->mem_size-1);
            newf = newf->next;
        }
    }
    printf(":::::::::::::::::::::::::::::::::::::::::END OF THE HEAP:::::::::::::::::::::::::::::::::::::::::::::\n");
}

void ReferenceCount_GarbageCollector(){
    printf("!!!!!! Running the Reference Counter Garbage Collector !!!!!!\n");
    meta_node *alloc = allocated_list;
    while(alloc){
        printf("%d: %d\n",alloc->value, alloc->no_ref);
        if(alloc->no_ref<1){
            printf("%d is garbage: Clearing garbage\n",alloc->value);
            meta_node *prev = alloc->prev;
            meta_node *del = alloc;
            if(prev) prev->next = alloc->next;
            if(alloc->next) alloc->next->prev = prev;
            alloc = del->next;
            BuddyFree(del);
        }
        else alloc = alloc->next;
    }
}

void Dereference(meta_node *deref, int der){
    deref->no_ref-=der;
    metalist *adjl = deref->pointers;
    while(adjl){
        Dereference(adjl->data, der);
        adjl = adjl->nextptr;
    }
}

void Morris_Traversal(meta_node *root){
    //meta_node *child=root;
    metalist *rptrs = root->pointers;
    metalist *newnode = (metalist *)malloc(sizeof(metalist));
    newnode->data=root;
    newnode->nextptr = NULL;
    metalist *child = newnode;
    meta_node *parent = NULL;
    while(child && child->data){
        if(!child->data->Mark){
            child->data->Mark=TRUE;
            printf("%d marked\n",child->data->value);
            //Adding parent to children's children
            parent = child->data;
            
            if(!child->data->pointers){
                child = child->nextptr;
                continue; 
            }
            child = child->data->pointers;
            metalist *newnode = (metalist *)malloc(sizeof(metalist));
            newnode->data=parent;
            newnode->nextptr = NULL;
            metalist *childptr = child;
            if(childptr && !childptr->data->Mark){
                while(childptr->nextptr){
                    childptr = childptr->nextptr;
                }
                childptr->nextptr=newnode;

            }
            else{
                child = newnode;
                metalist *ptr = newnode;
                while(ptr->nextptr){
                    ptr = ptr->nextptr;
                }
                parent = ptr->data;
            } 
            

        }
        else child = child->nextptr;
        
    }
}

void MarkSweep_GarbageCollector(meta_node *root){
    printf("!!!!!! Running Mark-Sweep Garbage Collector !!!!!!\n");
    meta_node *alloc = allocated_list;
    //Unmarking every allocated list node as false initially
    while(alloc){
        alloc->Mark=FALSE;
        alloc = alloc->next;
    }
    //Marking all the referenced data nodes using adjacancy matrix   
    Morris_Traversal(root);
    //Sweeping all the unmarked datanodes
    alloc = allocated_list;
    while(alloc){
        if(!alloc->Mark){
            meta_node *del = alloc;
            alloc = alloc->next;
            printf("Clearing Garbage: %d\n",del->value);
            BuddyFree(del);
        } 
        else alloc = alloc->next;
    }
}

void PrintAdjacencyMatrix(){
    meta_node *alloc = allocated_list;
    int matrix[10][10];
    printf("ADJACENCY LIST:\n");
    int i=0,j=0;
    for(int k=0;k<10;k++){
        for(int f=0;f<10;f++){
            matrix[k][f]=0;
        }
    }
    while(alloc){
        
        printf("%d: ",alloc->value);
        metalist *adjlist = alloc->pointers;
        while(adjlist){

            printf("%d, ",adjlist->data->value);
            matrix[alloc->value-1][adjlist->data->value-1]=1;
            adjlist = adjlist->nextptr;

        }
        printf("\n");
        alloc = alloc->next;
    }
    printf("ADJACENCY MATRIX:\n");
    for(int i=0;i<10;i++){
        for(int j=0;j<10;j++){
            printf("%d ",matrix[i][j]);
        }
        printf("\n");

    }
}
int main(){
    Initialize_Buddylist();
    int i=1;

    meta_node *malloc1 = BuddyMalloc(32);
    //That is root 2
    malloc1->no_ref=1;
    meta_node *malloc2 = BuddyMalloc(233);
    meta_node *malloc3 = BuddyMalloc(67);
    //meta_node *malloc4 = BuddyMalloc(14);
    meta_node *malloc5 = BuddyMalloc(12);
    //That is root 1
    //malloc5->no_ref=1;
    meta_node *malloc7 = BuddyMalloc(56);
    meta_node *malloc8 = BuddyMalloc(5);
    meta_node *malloc9 = BuddyMalloc(1);
    meta_node *malloc10 = BuddyMalloc(10);
    malloc1->value=1;
    malloc2->value=2;
    malloc3->value=3;
    malloc5->value=5;
    malloc7->value=7;
    malloc8->value=8;
    malloc9->value=9;
    malloc10->value=10;
    malloc1->pointers = insertPointerAtEnd(malloc1, malloc2);
    malloc1->pointers = insertPointerAtEnd(malloc1, malloc9);
    malloc1->pointers = insertPointerAtEnd(malloc1, malloc10);
    malloc3->pointers = insertPointerAtEnd(malloc3, malloc8);
    malloc3->pointers = insertPointerAtEnd(malloc3, malloc10);
    malloc5->pointers = insertPointerAtEnd(malloc5, malloc1);
    malloc7->pointers = insertPointerAtEnd(malloc7, malloc1);
    malloc7->pointers = insertPointerAtEnd(malloc7, malloc8);
    
    malloc8->pointers = insertPointerAtEnd(malloc8, malloc9);
    //ReferenceCount_GarbageCollector();
    PrintAdjacencyMatrix(); 
    ReferenceCount_GarbageCollector();
    //MarkSweep_GarbageCollector(malloc5);   
    //Dereference(malloc2,1);
    //Dereference(malloc1,1);
    //Dereference(malloc5,1);
    //Dereference(malloc7,1);
    PrintAdjacencyMatrix();
    //MarkSweep_GarbageCollector();
    //ReferenceCount_GarbageCollector();
    //PrintAdjacencyMatrix();    
    PrintHeap();
    printf("Good End\n");
}

