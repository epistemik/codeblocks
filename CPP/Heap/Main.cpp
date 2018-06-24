/*
 * Main.cpp
 *   Created on: Jan 21, 2011
 *   Author: Mark Sattolo <epistemik@gmail.com>
 * ---------------------------------------------------------------------------
 *   $File: //depot/Eclipse/CPP/Workspace/OrigHeap/Src/Main.cpp $
 *   $Revision: #5 $
 *   $Change: 48 $
 *   $DateTime: 2011/01/28 17:41:51 $   
 */

#include <cstdlib>
#include <iostream>
#include <libgen.h> // for basename()

#include "Test.hpp"
#include "Heap.hpp"
#include "ArrayHeap.hpp"
#include "LinkHeap.hpp"

const int DEFAULT_VALUE = 9999 ;
const int DEFAULT_NUM_VALUES = 17 ;

// implement a compare_fxn for 'less than' (from the FORTRAN .lt. -> less than)
// *** MUST BE STRICTLY LESS - NOT LESS OR EQUAL !!! ***
bool lt( const TestType& c1, const TestType& c2 )
{
  return( *const_cast<TestType&>(c1) < *const_cast<TestType&>(c2) );
}

// get a random number
long getRand( int n )
{
  return static_cast<long>( rand() % n );
}

int main( int argc, char* argv[] )
{
	long val ;
	bool cmdlineinput = true ;
	
  // seed the RNG
  srand( time(0) );

  // check that there are node values entered on the command line
  if( argc < 3 )
  {
	  // advise of usage
	  cout << endl << "Usage: '" << basename( argv[0] ) << " int1 int2 ... int_n' \n" << endl ;
	  
	  cmdlineinput = false ;
	  
	  // use default parameters if command line was empty
	  argc = DEFAULT_NUM_VALUES ;
	}
  
  /* create a LINKED LIST heap, of template type 'TestType',
	   with 'lt' as the compareFxn and 'SMALLER_FIRST' as the ordering  */
  //LinkHeap<TestType> myHeap( lt, Heap<TestType>::SMALLER_FIRST );
  
  /* create an ARRAY heap, of template type 'TestType',
     with 'lt' as the compareFxn and 'SMALLER_FIRST' as the ordering  */
  ArrayHeap<TestType> myHeap( lt, Heap<TestType>::SMALLER_FIRST );
  
  // push the command line values onto the heap
  for( int i=1; i < argc; ++i )
  {
    // create a TestType with var i and put it on the heap
		val = cmdlineinput ? atol( argv[i] ) : getRand( DEFAULT_VALUE );
		cout << "Add value '" << val << "' to the heap..." << endl;
	  Heap<TestType>::Handle& h1 = myHeap.push( TestType(val) );
	  
		// print out the heap
		myHeap.print( cout );
		
	  // randomly change some node values and see if the heap adjusts properly
	  if( i % 2 == 0 )
	  {
      // get a random value for k
		  long k = getRand( DEFAULT_VALUE );
		  
			cout << "Add random increment '" << k << "' to value '" << val << "' = " << (val+k) << endl;
		  /* use the overloaded * on Handle h1 to get a reference to the element (TestType) of h1,
		     then remove the "const-ness" with a cast,
		     then use the overloaded * on TestType to get a reference to the stored value and increment it by k */
			cout << "Value of h1 = " << *const_cast<TestType&>( *h1 ) << endl;
		  *const_cast<TestType&>( *h1 ) += k ;
			cout << "NOW, Value of h1 = " << *const_cast<TestType&>( *h1 ) << endl;
			
		  /* inform the heap that one of the nodes has changed and the heap may need to adjust itself */
		  myHeap.priorityChange( h1 );
		  
			// print out the heap
		  cout << "Now, the heap is:" << endl;
			myHeap.print( cout );
	  }
  }

  cout << "-----------------------------------------------------------" << endl;

  // print each node in the heap then destroy it and recover the memory
  while( myHeap. )
  {
	  cout << myHeap.top() << ' ' ;
	  myHeap.pop();
  }
  cout << endl;

	cout << "PROGRAM ENDED.\n" << endl;
  return 0 ;
  
}//main()
