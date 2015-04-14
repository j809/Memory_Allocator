#include<cstdio>
#include<climits>
#include<cstdlib>

#define DEBUG

#ifdef DEBUG
	#define debug(x)	(x==NULL)?printf("(null)\n"):printf("%d\n",x->st_addr)
	#define pstr(x)		printf("%s",x)
	#define pint(x)		printf("%d",x)
#else 
	#define debug(x)
	#define pstr(x)	
	#define pint(x)
#endif

#define MEM_START	1
#define MEM_END		512
#define GRN		"\x1B[1;32m"
#define NRM		"\x1B[0m"

using namespace std;

struct node
{
	int procid,st_addr,end_addr;
	struct node* next;
};

void print_list(struct node** head)
{
	struct node* temp = (*head);
	while(temp)
	{
		printf("<< P : %d : %d-%d >> -> ",temp->procid,temp->st_addr,temp->end_addr);
		temp=temp->next;
	}
	printf("NULL\n");
}

void free_init(struct node** head,int st, int end)
{
	struct node* newn = new struct node;
	newn->procid=-1;
	newn->st_addr=st;
	newn->end_addr=end;
	newn->next=NULL;
	(*head)=newn;
}

void insert_sorted(struct node** head, struct node* n)
{
	if((*head) == NULL)
	{
		(*head) = n;
		return;
	}
	if(n->st_addr < (*head)->st_addr)
	{
		n->next = (*head);
		(*head) = n;
		return;
	}
	struct node* temp = (*head);
	while(temp->next && temp->next->st_addr < n->st_addr)
	{
		temp = temp->next;
	}
	n->next = temp->next;
	temp->next = n;
}

void move_node(struct node** from, struct node** to, struct node* n)
{
	if((*from) == n)
	{
		(*from) = (*from)->next;	
		n->next = (*to);
		(*to) = n;
		return;
	}
	struct node* prev = (*from);
	struct node* temp = (*from)->next;
	while(temp)
	{
		if(temp==n)
		{
			//n->next = (*to);
			//(*to) = n;
			prev->next = temp->next;			
			insert_sorted(to, n);
			return;
		}
		prev = temp; 
		temp = temp->next;
	}
}

void del_node(struct node **head, struct node *n)
{
	if((*head) == n)
	{
		(*head) = (*head)->next;
		delete n; 
		return;
	}
	struct node *prev = (*head);
	while(prev->next != NULL && prev->next != n)
	{
		prev = prev->next;
	}
	if(prev->next == NULL)
	{
		printf("Fatal error! Given node does not exist in free list.\n");
		return;
	}
	prev->next = prev->next->next;
	delete n;
	return;
}

bool allocate_mem_first(struct node** fr, struct node** al, int size, int procid)
{
	struct node* newn = new struct node;
	newn->procid=procid;
	
	struct node* temp=(*fr);
	bool allocated=false;
	while(temp != NULL && !allocated)
	{
		if((temp->end_addr - temp->st_addr) > size)
		{
			//printf("Partial\n");
			newn->st_addr = temp->st_addr;		//allocate space
			newn->end_addr = temp->st_addr + size;

			temp->st_addr = temp->st_addr + size;	//change space in free node

			allocated=true;	
		}
		else if((temp->end_addr - temp->st_addr) == size)
		{
			//printf("FULL\n");
			newn->st_addr = temp->st_addr;		//allocate space
			newn->end_addr = temp->st_addr + size;

			del_node(fr, temp);

			allocated=true;
		}

		temp = temp->next;
	}

	if(!allocated)
	{
		printf("Enough contiguous free memory not available!\n");
	}
	else
	{
		//newn->next = (*al);	//insert into sorted allocated list
		//(*al) = newn;

		insert_sorted(al,newn);
			
		printf("Process with process id %d allocated memory from address %d to %d.\n",newn->procid,newn->st_addr,newn->end_addr);
	}
	
	printf("Free List: \n");
	print_list(fr);
	printf("Allocated List: \n");
	print_list(al);
	
	return allocated;
}

bool allocate_mem_best(struct node** fr, struct node** al, int size, int procid)
{
	struct node* newn = new struct node;
	newn->procid=procid;
	
	struct node* temp=(*fr);
	bool allocated=false;
	
	int min = INT_MAX;

	while(temp != NULL)
	{
		if((temp->end_addr - temp->st_addr) < min && (temp->end_addr - temp->st_addr)>=size)
		{
			min = (temp->end_addr - temp->st_addr);
		}
		if(min == 0)
		{
			allocated = true;
			break;
		}
		temp = temp->next;
	}
	
	if(min > 0)
	{
		allocated = true;
	}
	
	if(allocated)
	{
		temp = (*fr);
		while(temp && (temp->end_addr - temp->st_addr)!=min)
		{
			temp = temp->next;
		}
		if(min > 0)
		{
			//printf("Partial\n");
			newn->st_addr = temp->st_addr;		//allocate space
			newn->end_addr = temp->st_addr + size;

			temp->st_addr = temp->st_addr + size;	//change space in free node
		}
		else if(min == 0)
		{
			//printf("FULL\n");
			newn->st_addr = temp->st_addr;		//allocate space
			newn->end_addr = temp->st_addr + size;

			del_node(fr, temp);
		}
	}

	if(!allocated)
	{
		printf("Enough contiguous free memory not available!\n");
	}
	else
	{
		insert_sorted(al,newn);		
		printf("Process with process id %d allocated memory from address %d to %d.\n",newn->procid,newn->st_addr,newn->end_addr);
	}
	
	printf("Free List: \n");
	print_list(fr);
	printf("Allocated List: \n");
	print_list(al);
	
	return allocated;
}

bool allocate_mem_worst(struct node** fr, struct node** al, int size, int procid)
{
	struct node* newn = new struct node;
	newn->procid=procid;
	
	struct node* temp=(*fr);
	bool allocated=false;
	
	int max = INT_MIN;

	while(temp != NULL)
	{
		if((temp->end_addr - temp->st_addr) > max && (temp->end_addr - temp->st_addr)>=size)
		{
			max = (temp->end_addr - temp->st_addr);
		}
		if(max == 0)
		{
			allocated = true;
			break;
		}
		temp = temp->next;
	}
	
	if(max > 0)
	{
		allocated = true;
	}
	
	if(allocated)
	{
		temp = (*fr);
		while(temp && (temp->end_addr - temp->st_addr)!=max)
		{
			temp = temp->next;
		}
		if(max > 0)
		{
			//printf("Partial\n");
			newn->st_addr = temp->st_addr;		//allocate space
			newn->end_addr = temp->st_addr + size;

			temp->st_addr = temp->st_addr + size;	//change space in free node
		}
		else if(max == 0)
		{
			//printf("FULL\n");
			newn->st_addr = temp->st_addr;		//allocate space
			newn->end_addr = temp->st_addr + size;

			del_node(fr, temp);
		}
	}

	if(!allocated)
	{
		printf("Enough contiguous free memory not available!\n");
	}
	else
	{
		insert_sorted(al,newn);		
		printf("Process with process id %d allocated memory from address %d to %d.\n",newn->procid,newn->st_addr,newn->end_addr);
	}
	
	printf("Free List: \n");
	print_list(fr);
	printf("Allocated List: \n");
	print_list(al);
	
	return allocated;
}

void merge_free_memory(struct node** fr)
{
	struct node* temp = (*fr);
	while(temp->next!=NULL)
	{
		if(temp->end_addr == temp->next->st_addr)
		{
			temp->next->st_addr = temp->st_addr;
			del_node(fr,temp);
		}
		temp = temp->next;		
	}
}

bool deallocate(struct node** al, struct node** fr, int procid)
{
	bool deallocated = false;

	if((*al)->procid == procid)
	{
		struct node* n = (*al);
		(*al)->procid = -1;
		move_node(al,fr,n);
		deallocated = true;
	}
	else
	{			
		struct node* prev = (*al);
		struct node* temp = (*al)->next;	

		while(temp)
		{
			if(temp->procid == procid)
			{
				//prev->next = temp->next;
				temp->procid = -1;			
				move_node(al,fr,temp);
				deallocated = true;
				prev = temp = NULL;
				delete prev;
				delete temp;
				break;
			}
			prev = temp;
			temp = temp->next;
		}
	}
	if(deallocated)
	{
		printf("PID %d memory deallocated.\n",procid);
		merge_free_memory(fr);
		printf("Free List: \n");
		print_list(fr);
		printf("Allocated List: \n");
		print_list(al);
		return true;
	}
	else
	{
		printf("PID not found!\n");
		return false;
	}
}

void print_memory(struct node** al)
{
	struct node* temp = (*al);
	int i;
	int end = (temp==NULL)?MEM_END:temp->st_addr;
	printf("\t");
	for(i=1;i<end;++i)
	{
		printf("%s#",NRM);
		if(!(i%64))
			printf("\n\t");
	}
	while(temp)
	{
		for(;i<temp->st_addr;++i)
		{
			printf("%s#",NRM);
			if(!(i%64))
				printf("\n\t");			
		}
		for(;i<=temp->end_addr;++i)
		{
			printf("%s*",GRN);
			if(!(i%64))
				printf("\n\t");						
		}
		temp = temp->next;
	}
	for(;i<=MEM_END;++i)
	{
		printf("%s#",NRM);
		if(!(i%64))
			printf("\n\t");					
	}
}

int main()
{
	struct node* free=NULL;
	struct node* allocated=NULL;

	free_init(&free,MEM_START,MEM_END);
	
	int ch,pid,size;
	do 
	{
		system("clear");
	
		printf("Memory allocated system\n"
		"=============================\n\n"
		"1. Run new process (Use first fit)\n"
		"2. Run new process (Use best fit)\n"
		"3. Run new process (Use worst fit)\n"
		"4. Terminate process\n"
		"5. Display memory content\n"
		"6. Exit\n"		
		"\nPlease select an option : ");
		scanf("%d",&ch);
		
		switch(ch)
		{
			case 1:	system("clear");
				printf("Uses first-fit algorithm for memory allocation\n\n"
				"Enter PID for process: ");
				scanf("%d",&pid);
				printf("Enter memory size required by process: ");
				scanf("%d",&size);
				allocate_mem_first(&free, &allocated, size, pid);
				printf("Press <Enter> to continue...\n");
				getchar();
				getchar();
				break;
			case 2:	system("clear");
				printf("Uses best-fit algorithm for memory allocation\n\n"
				"Enter PID for process: ");
				scanf("%d",&pid);
				printf("Enter memory size required by process: ");
				scanf("%d",&size);
				allocate_mem_best(&free, &allocated, size, pid);
				printf("Press <Enter> to continue...\n");
				getchar();			
				getchar();						
				break;					
			case 3:	system("clear");
				printf("Uses worst-fit algorithm for memory allocation\n\n"
				"Enter PID for process: ");
				scanf("%d",&pid);
				printf("Enter memory size required by process: ");
				scanf("%d",&size);
				allocate_mem_worst(&free, &allocated, size, pid);
				printf("Press <Enter> to continue...\n");
				getchar();			
				getchar();						
				break;
			case 4: system("clear");
				printf("Ready to terminate process\n"
				"Enter PID for process: ");
				scanf("%d",&pid);
				deallocate(&allocated, &free, pid);
				printf("Press <Enter> to continue...\n");
				getchar();			
				getchar();
				break;
			case 5: system("clear");
				printf("MEMORY CONTENT\n"
				"==================\n\n");
				
				print_memory(&allocated);
				
				printf("\n\nFree List: \n");
				print_list(&free);
				printf("Allocated List: \n");
				print_list(&allocated);				
			
				printf("\nPress <Enter> to continue...\n");
				getchar();			
				getchar();
				break;			
			case 6: return 0;
			default: printf("Invalid choice!! Press <Enter> to continue...\n");
				getchar();		
				getchar();
		}
	} while(ch!=6);
	return 0;
}
