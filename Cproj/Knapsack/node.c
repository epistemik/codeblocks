/*!***************************************************
	 Mark Sattolo (msattolo@omnisig.com)
	-----------------------------------------------
	  File: @(#)node.c
	  Version: 1.18
	  Last Update: 03/03/20 11:16:25

***************************************************!*/

#include "node.h"

/* //// MEMORY MANAGEMENT /////////////////////////////////////////////////////// */

int MemCount = 0 ,
	 MaxMemCount = 0 ;

// Obtain and initialize a new node
BOOLEAN GetNode( nodePtr *g )
{
  #if KNAPSACK_NODE_DEBUG > 0
    printf( "\nGetNode(): MemCount == %d ", MemCount );
  #endif

  *g = (nodePtr)malloc( sizeof(node) );
  if( !(*g) )
  {
	 fprintf( stderr, "GetNode(): malloc error !\n" );
	 return False ;
  }

  (*g)->name = NULL ; // have to initialize the char* of new nodes

  MemCount++ ;
  if( MemCount > MaxMemCount )
	 MaxMemCount = MemCount ;

  return True ;

}// GetNode()


// Return a node's memory
void ReturnNode( nodePtr *h )
{
  #if KNAPSACK_NODE_DEBUG > 0
    printf( "\n ReturnNode(): MemCount == %d", MemCount );
  #endif

  free( *h );
  MemCount-- ;

}// ReturnNode()


// Check dynamic memory usage
BOOLEAN MemCheck()
{
  return (BOOLEAN)( MemCount == 0 );

}// MemCheck()


/* //// NODE MANAGEMENT ////////////////////////////////////////////////////////// */

BOOLEAN setName( nodePtr x, const char *z )
{
  if( x->name ) free( x->name ); // free any existing char* memory

  if( z != NULL )
  {
	 // make space for the name
	 x->name = (char*)malloc( (strlen(z)+1) * sizeof(char) );
	 if( x->name )
	 {
		strcpy( x->name, z );
		return True ;
	 }
	 else
		  fprintf( stderr, " setName(): malloc error !\n" );
  }
  else
		fprintf( stderr, " setName(): parameter char* is NULL !\n" );

  return False ;

}// setName()


BOOLEAN appendName( nodePtr y, const char *z )
{
  char *temp ;
  int len = (y->name == NULL) ? 0 : strlen(y->name) ;

  if( z != NULL )
  {
	 // make space for the addition
	 temp = (char*)realloc( y->name, (len+strlen(z)+1) * sizeof(char) );
	 if( temp )
	 {
		y->name = temp ;
		strcat( y->name, z );
		return True ;
	 }
	 else
		  fprintf( stderr, " appendName(): realloc error !\n" );
  }
  else
		fprintf( stderr, " appendName(): parameter char* is NULL !\n" );

  return False ;

}// appendName()


void displayNode( nodePtr x )
{
  printf( "%13s: ", x->name ? x->name : "NO NAME ! " );
  printf( " items %2d ; level %2d ; bound == %8.3f ;",
				x->numitems, x->level, x->bound );
  printf( " profit == %4d ; weight == %4d ; pw == %7.3f \n",
				x->profit, x->weight, x->pw );

}// displayNode()


// copy the values from one node to another (already allocated) node
BOOLEAN copyNode( nodePtr m, const nodePtr n )
{
  #if KNAPSACK_NODE_DEBUG > 0
    printf( "\n copyNode(): " );
    displayNode( n );
  #endif

  if( !setName(m, n->name) )
	 return False ;

  m->numitems = n->numitems ;
  m->level    = n->level ;
  m->profit   = n->profit ;
  m->weight   = n->weight ;
  m->pw       = n->pw ;
  m->bound    = n->bound ;

  return True ;

}// copyNode()


/* //////// PRIORITY_QUEUE ////////////////////////////////////////////////////// */

// Priority Queue used to store and retrieve the nodes based on 'bound' value

void initQueue( PqPtr p )
{
  p->nodes = NULL ;
  p->size = 0 ;

}// initQueue()


BOOLEAN isEmptyQueue( PqPtr p )
{
  return (BOOLEAN)( p->nodes == NULL );

}// isEmptyQueue()


/*	make a copy of the target node and add the copy to the given PriorityQueue
	at the proper position for its 'bound' value  */
void insertNode( PqPtr p, nodePtr s )
{
  nodePtr newnode ;

  GetNode( &newnode );
  copyNode( newnode, s );

  #if KNAPSACK_NODE_DEBUG > 0
    printf( "\n insertNode(): " );
    displayNode( newnode );
  #endif

  newnode->next = NULL ;

  // insert the new node at the start of the linked list
  if( isEmptyQueue(p)  ||  newnode->bound >= p->nodes->bound )
  {
	 newnode->next = p->nodes ;
	 p->nodes = newnode ;
  }
  else // need to find the position to insert the node
	 {
		nodePtr temp = p->nodes ;
		while( temp->next != NULL  &&  temp->next->bound > newnode->bound )
		  temp = temp->next ;

		newnode->next = temp->next ;
		temp->next = newnode ;
	 }

  p->size++ ;

}// insertNode()


void displayQueue( PqPtr p )
{
  int i=1 ;
  nodePtr temp = p->nodes ;

  printf( "\n Queue Elements: (size == %d)\n", p->size );
  while( temp != NULL )
  {
	 printf( "%d. ", i );
	 displayNode( temp );
	 temp = temp->next ;
	 i++ ;
  }
  puts( "" );

}// displayQueue()


//  remove a node from a Priority Queue and copy the node's data to node r
void removeNode( PqPtr p, nodePtr r )
{
  nodePtr head ;

  if( isEmptyQueue(p) )
  {
	 puts( "ERROR - dequeue from empty queue" );
	 exit( 1 );
  }

  // take the first member of the priority queue
  head = p->nodes ;
  #if KNAPSACK_NODE_DEBUG > 0
    printf( "\n removeNode(): " );
    displayNode( head );
  #endif

  p->nodes = p->nodes->next ;

  copyNode( r, head );
  ReturnNode( &head );

  p->size-- ;

}// removeNode()


// now, just checks that Queue is empty and verifies memory usage
void deleteQueue( PqPtr p )
{
  if( p->nodes == NULL  &&  p->size == 0 )
	 puts( "Queue is Empty" );
  else
		printf( "deleteQueue(): ERROR: p->nodes == %p and p->size == %d !!",
					p->nodes, p->size );

  if( ! MemCheck() )
	 printf( "MemCheck ERROR: MemCount == %d !!", MemCount );

  printf( "Max MemCount == %d \n", MaxMemCount );

}// deleteQueue()


/* //////// NODE ARRAY ////////////////////////////////////////////////////////// */

// NodeArray used to store and sort the nodes by profit/weight ratio (pw member)

BOOLEAN initNodeArray( nodeArray *a, const int size )
{
  int i ;

  *a = (nodeArray)malloc( size * sizeof(node) );
  if( !(*a) )
  {
	 fprintf( stderr, " Memory allocation error for nodeArray !\n" );
	 return False ;
  }

  // make sure every char* starts as NULL
  for( i=0; i < size ; i++ )
	 ((*a)+i)->name = NULL ;

  return True ;

}// initNodeArray()


void displayNodeArray( const nodeArray nodes, const int size )
{
  int i=0 ;
  #if KNAPSACK_NODE_DEBUG > 1
    puts( "\n displayNodeArray()" );
    printf( "nodeArray == %p \n", nodes );
  #endif

  if( size > 0 )
  {
	 printf( " NodeArray Elements: (size == %d)\n", size );
	 for( ; i < size; i++ )
	 {
		printf( "%3d. ", i+1 );
		displayNode( nodes+i );
	 }
  }
  else
		printf( "\n displayNodeArray(): passed array size == %d !\n", size );

}// displayNodeArray()


void deleteNodeArray( nodeArray b, const int size )
{
  int i ;

  for( i=0; i < size ; i++ )
	 free( b[i].name );

  free( b );

}// deleteNodeArray()


// for sorting the nodeArray - use with the stdlib function qsort()
int compareNode( const void *m, const void *n )
{
  const node *a = m ;
  const node *b = n ;

  // sort in descending order
  if( a->pw > b->pw )
	 return -1 ;

  if( a->pw == b->pw )
	 return 0 ;

  return 1 ;

}// compareNode()


// node.c
