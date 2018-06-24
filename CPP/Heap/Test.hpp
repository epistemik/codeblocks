/*
 * Test.hpp
 *   Created on: Jan 21, 2011
 *   Author: Mark Sattolo
 * ---------------------------------------------------------------------------
 *   $File: //depot/Eclipse/CPP/Workspace/OrigHeap/Src/Test.hpp $
 *   $Revision: #6 $
 *   $Change: 47 $
 *   $DateTime: 2011/01/27 20:03:23 $   
 */

#ifndef MHS_CODEBLOCKS_CPP_HEAP_TEST_HPP
#define MHS_CODEBLOCKS_CPP_HEAP_TEST_HPP

using namespace std ;

#include <iostream>

class TestType 
{
 private:
  // static int for the class to keep track of all instances
	static int last_id ;
	// individual id of each object
	int id ;
	// value to store
	long value ;

 public:
	// CONSTRUCTOR sets 'value'
	TestType( const long& );
	
	// overload operator* to return 'value'
	long& operator*();

 // print out a TestType
 friend ostream& operator<<( ostream&, const TestType& );
};

// free function - implemented in Test.cpp
ostream& operator<<( ostream&, const TestType& );

bool lt( const TestType&, const TestType& );

#endif // MHS_CODEBLOCKS_CPP_HEAP_TEST_HPP
