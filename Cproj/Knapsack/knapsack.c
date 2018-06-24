/*!***************************************************
	 Mark Sattolo (msattolo@omnisig.com)
	-----------------------------------------------
	  File: @(#)knapsack.c
	  Version: 1.23
	  Last Update: 03/03/20 10:59:55

***************************************************!*/
/*
  Best-First Search with Branch-and-Bound Pruning Algorithm for the 0-1 Knapsack Problem

  see p.235 of "Foundations of Algorithms: with C++ pseudocode", 2nd Ed. 1998,
	  by Richard Neapolitan & Kumarss Naimipour, Jones & Bartlett, ISBN 0-7637-0620-5

  Problem: Let n items be given, where each item has a weight and a profit.
		The weights and profits are positive integers.  Furthermore, let a positive
		integer W be given.  Determine a set of items with maximum total profit,
		under the constraint that the sum of their weights cannot exceed W.

  Inputs: positive integers n and W,
	  arrays of positive integers w[] and p[],
		  ( each indexed from 1 to n, and each of which is sorted in non-increasing
			  order according to the values of p[i]/w[i] ).

  Outputs: an integer maxprofit that is the sum of the profits of an optimal set.
		       ?[an array giving the indices of the items comprising the optimal set]
*/

#include "node.h"

// FUNCTION PROTOTYPES
void bound( node*, const nodeArray, const int, const int, const int );
int bestFirstSearch( const nodeArray, const int, const int, const int, int*, char** );

/* ===  MAIN  ================================================================== */

int main( int argc, char *argv[] )
{
  int i=0 ,	    // loop index
		j ,	          // check each input line
		n ,	          // total items (in input file)
		W ,	          // maximum allowed weight of items (user-supplied)
		maxItems=0 ,  // maximum allowed number of items (user-supplied or default)

		totweight=0 , // of items selected
		maxProfit ;

  char *bestitems = NULL ; // keep track of items in the max profit list
  char temp[ KNAP_MAX_NAME_LEN ] = {0};

  FILE *inputfile ;
  nodeArray pw ;

  // check command line parameters
  if( argc < 2 )
  {
	  printf( "\nUsage: %s 'input file name' [max weight] [max items]\n\n", argv[0] );
	  exit( __LINE__ );
  }

  if( argc > 3 )
	  maxItems = atoi( argv[3] );

  if( argc < 3 )
  {
	  printf( "\nPlease enter the maximum weight: " );
	  scanf( "%u", &W );
  }
  else
		  W = atoi( argv[2] );

  printf( "File name is '%s' \n", argv[1] );
  printf( "W == %u \n", W );
  #if KNAPSACK_MAIN_DEBUG > 1
    printf( "sizeof(node) == %lu \n", sizeof(node) );
  #endif

  // open the file
  inputfile = fopen( argv[1], "r" );
  if( !inputfile )
  {
	  fprintf( stderr, "Error occurred opening file '%s' !\n", argv[1] );
	  exit( __LINE__ );
  }

  // first entry in the file should be the # of items
  if( fscanf(inputfile, "%u", &n) != 1 )
  {
	  fprintf( stderr, "Error getting # of items (%d) in file '%s' !\n", n, argv[1] );
	  exit( __LINE__ );
  }
  printf( "\nThere should be %d items in file '%s' \n", n, argv[1] );

  // if maxItems wasn't specified or was an invalid value
  if( argc < 4 || maxItems < 1 || maxItems > n )
	  maxItems = n ;
  printf( "Max number of items in solution is %u \n", maxItems );

  // allocate the array to sort items by greater profit/weight ratio
  if( !initNodeArray(&pw, n) )
	  exit( __LINE__ );

  // scan in the input lines
  while( !feof(inputfile) )
  {
	  // get the data
	  j = fscanf( inputfile, "%s %u %u", temp, &(pw[i].profit), &(pw[i].weight) );

	  #if KNAPSACK_MAIN_DEBUG > 1
      printf( "\n i == %d \n", i );
      printf( "temp == %s ; strlen(temp) == %d \n", temp, (int)strlen(temp) );
	  #endif

	  if( j == 3 ) // got a complete line
	  {
		  setName( &pw[i], temp );

		  pw[i].level = 0 ;
		  pw[i].numitems = 1 ;
		  pw[i].bound = 0.0 ;

		  // calculate the p/w ratio
		  pw[i].pw = (pw[i].weight > 0) ? ((float)pw[i].profit / (float)pw[i].weight) : 0.0 ;

		  #if KNAPSACK_MAIN_DEBUG > 1
        displayNode( pw+i );
		  #endif

		  getc( inputfile ); // eat the EOL
		  i++ ; // index for pw array

	  }// if( j == 3 )

  }// while( !feof(inputfile) )

  #if KNAPSACK_MAIN_DEBUG > 0
    printf( "\nThere were %d items in file '%s' \n", i, argv[1] );
  #endif

  fclose( inputfile );

  #if KNAPSACK_MAIN_DEBUG > 1
    puts( "\nBEFORE SORTING:" );
    displayNodeArray( pw, n );
  #endif

  // sort the pw array
  qsort( pw, n, sizeof(node), compareNode );

  puts( "\nAFTER SORTING:" );
  displayNodeArray( pw, n );

  // run the algorithm and display the results
  maxProfit = bestFirstSearch( pw, n, W, maxItems, &totweight, &bestitems );
  printf( "\nFor Weight limit %d: Max Profit == %d (actual weight == %d)\n",
				 W, maxProfit, totweight );
  printf( "Best items are: %s \n", bestitems ? bestitems : "NOT AVAILABLE !" );

  // clean up allocated memory
  free( bestitems );
  deleteNodeArray( pw, n );

  printf( "\n PROGRAM ENDED.\n" );

  return 0 ;

}// main()

/* ///////// FUNCTIONS /////////////////////////////////////////////////////////////// */

void bound( node *x, const nodeArray pw, const int n, const int W, const int maxItems )
{
  int nextItem ;
  int totWeight = x->weight ;
  int totItems = x->numitems ;
  float result = 0.0 ;

  #if KNAPSACK_BOUND_DEBUG > 1
    printf( "\nINSIDE bound(): n == %d ; maxItems == %d \n", n, maxItems );
    displayNodeArray( pw, n );
  #endif

  #if KNAPSACK_BOUND_DEBUG > 0
    printf( "\n bound(1): " );
    displayNode( x );
  #endif

  // calculate the new bound if weight and num items are under their limits
  if( totWeight < W  &&  totItems <= maxItems )
  {
	 result = (float)x->profit ;

	 nextItem = x->level + 1 ;
	 #if KNAPSACK_BOUND_DEBUG > 0
     printf( " bound(2): starting nextItem == %d \n", nextItem );
	 #endif

	 // grab as many items as possible
	 while( totItems < maxItems  &&  nextItem < n  &&  (totWeight + pw[nextItem].weight <= W) )
	 {
		totWeight += pw[nextItem].weight ;
		result += (float)pw[nextItem].profit ;
		#if KNAPSACK_BOUND_DEBUG > 0
      printf( " bound(3): nextItem == %d ; result == %7.3f \n", nextItem, result );
		#endif
		nextItem++ ;
		totItems++ ;
	 }

	 if( totItems <= maxItems  &&  nextItem < n )
		// grab fraction of jth item
		result += ( ((float)(W - totWeight)) * pw[nextItem].pw );

	 #if KNAPSACK_BOUND_DEBUG > 0
     printf( " bound(4): node %s has bound == %7.3f \n", x->name, result );
	 #endif
  }

  x->bound = result ;

}// bound()


int bestFirstSearch( const nodeArray pw, const int n, const int W,
							const int maxItems, int *tw, char **best )
{
  int i = 0 ; // loop count

  node u, v ; // working nodes

  const char include[] = "&" ;
  const char exclude[] = "-" ;

  PriorityQueue PQ ;
  int maxprofit = 0 ;

  u.name = v.name = NULL ; // initialize the char*'s

  initQueue( &PQ );

  // set the names of u and v to keep track of items properly
  setName( &u, exclude );
  setName( &v, "root"  );

  v.level = -1 ; // start at -1 so the root node gets index == 0 in bound()
  v.pw = u.pw = 0.0 ;
  v.numitems = u.numitems = 0 ; // no items in starting nodes
  v.profit = v.weight = 0 ;

  // get the initial bound
  bound( &v, pw, n, W, maxItems );

  #if KNAPSACK_BFS_DEBUG > 0
    puts( "" );
    displayNode( &v );
  #endif

  insertNode( &PQ, &v ); // start the state space tree with the root node

  #if KNAPSACK_BFS_DEBUG > 0
    displayQueue( &PQ );
    printf( "START WHILE LOOP... \n\n" );
  #endif

  while( !isEmptyQueue(&PQ) )// &&  i < limit ) // limit prevents a runaway loop
  {
	 #if KNAPSACK_BFS_DEBUG > 2
     displayQueue( &PQ );
	 #endif
	 #if KNAPSACK_BFS_DEBUG > 1
     printf( "\nPQ.nodes == %p \n", PQ.nodes );
	   printf( "PQ.size == %d \n", PQ.size );
	 #endif

	 removeNode( &PQ, &v ); // remove node with best bound
	 #if KNAPSACK_BFS_DEBUG > 0
	   printf( "\nBFS( v ): " );
	   displayNode( &v );
	 #endif

	 if( v.bound > maxprofit ) // check if this node is still promising
	 {
		#if KNAPSACK_BFS_DEBUG > 0
		  printf( "v.bound == %7.3f \n", v.bound );
		#endif

	// 1. SET u TO THE CHILD THAT INCLUDES THE NEXT ITEM
		u.level = v.level + 1 ;

		// keep track of all items in this node
		setName( &u, v.name );
		appendName( &u, include );
		appendName( &u, pw[u.level].name );

		u.weight = v.weight + pw[u.level].weight ;
		u.profit = v.profit + pw[u.level].profit ;
		u.numitems = v.numitems + 1 ;

		#if KNAPSACK_BFS_DEBUG > 0
		  printf( "\nBFS( u ): " );
		  displayNode( &u );
		#endif

		// see if we have a new 'best node'
		if( u.weight <= W  &&  u.profit > maxprofit  &&  u.numitems <= maxItems )
		{
		  maxprofit = u.profit ;
		  *tw = u.weight ;
		  printf( "\n**********************************************************\n");
		  printf( "\nBFS(%d): maxprofit now == %d \n", i, maxprofit );
		  printf( "\t current best items are %s \n", u.name );
		  printf( "\t weight of items is %d ; number of items is %d\n", *tw, u.numitems );
		  printf( "\n**********************************************************\n");

		  // keep track of overall list of best items
		  *best = (char*)realloc( *best, (strlen(u.name)+1) * sizeof(char) );
		  if( *best )
			 strcpy( *best, u.name );
		  else
				fprintf( stderr, "Memory allocation error for *best !\n" );
		}

		bound( &u, pw, n, W, maxItems );
		if( u.bound > maxprofit ) // see if we should add this node to the queue
		  insertNode( &PQ, &u );

	// 2. SET u TO THE CHILD THAT DOES NOT INCLUDE THE NEXT ITEM

		// keep track of all items in this node
		setName( &u, v.name );
		appendName( &u, exclude ); // alter the name here just to monitor backtracking

		// we already incremented the level in the previous section
		u.weight = v.weight ;
		u.profit = v.profit ;
		u.numitems-- ;

		bound( &u, pw, n, W, maxItems );
		if( u.bound > maxprofit ) // if this node is still promising
		  insertNode( &PQ, &u );

	 }// if( v.bound > maxprofit )

	 i++ ;
	 #if KNAPSACK_BFS_DEBUG > 0
	   printf( "\n i == %d \n\n", i );
	 #endif

  }// while( !isEmptyQueue(&PQ) )// &&  i < limit )

  #if KNAPSACK_BFS_DEBUG == 0
    printf( "\n Final i == %d \n", i );
  #endif

  free( u.name );
  free( v.name );
  deleteQueue( &PQ );

  return maxprofit ;

}// bestFirstSearch()
