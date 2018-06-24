/*
 * Test.cpp
 *   Created on: Jan 21, 2011
 *   Author: Mark Sattolo
 * ---------------------------------------------------------------------------
 *   $File: //depot/Eclipse/CPP/Workspace/OrigHeap/Src/Test.cpp $
 *   $Revision: #4 $
 *   $Change: 48 $
 *   $DateTime: 2011/01/28 17:41:51 $   
 */

#include "Test.hpp"

int TestType::last_id = 0 ; // must define a static variable

// constructor sets 'value' and increments 'last_id'
TestType::TestType( const long& v )
{
  id = ++last_id ;
  value = v ;
  cout << "Create a new TestType with val '" << v << "'." << endl;
}

// operator*() - return a reference (alias) to the 'value'
long& TestType::operator*()
{
  return value ;
}

// print a TestType
ostream& operator<<( ostream& os, const TestType& c )
{
  return( os << c.value << '(' << c.id << ')' );
}
