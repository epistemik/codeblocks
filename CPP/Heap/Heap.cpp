/*
 * Heap.cpp
 *   Created on: Jan 21, 2011
 *   Author: Mark Sattolo
 * ---------------------------------------------------------------------------
 *   $File: //depot/Eclipse/CPP/Workspace/OrigHeap/Src/Heap.cpp $
 *   $Revision: #6 $
 *   $Change: 48 $
 *   $DateTime: 2011/01/28 17:41:51 $   
 */

#include "Heap.hpp"

/***********************************
         HANDLE MEMBER FUNCTIONS
		 ***********************************/

// used to generate the unique ids - initialize the static variable
template<typename dataType>
long Heap<dataType>::Handle::last_id = 0 ;

// CONSTRUCTOR
// set the parameter values via the initializer and increment the id variables
template<typename dataType>
Heap<dataType>::Handle::Handle( Heap<dataType>::compareFxn& f, Heap<dataType>::order& o, const dataType& e )
								        : handleComparison( f ), handleOrdering( o ), elem( e )
{ id = ++last_id ; }

// higherPriority() - returns true if 'this' is higher priority than h
template<typename dataType>
bool Heap<dataType>::Handle::higherPriority( const Heap<dataType>::Handle& h )
{
	if( handleOrdering == Heap<dataType>::SMALLER_FIRST )
  {
		if( handleComparison(h.elem, elem) )
			return false;
		if( handleComparison(elem, h.elem) )
			return true;
		return( id < h.id );
  }
	else if( handleOrdering == Heap<dataType>::LARGER_FIRST )
  {
		if( handleComparison(elem, h.elem) )
			return false;
		if( handleComparison(h.elem, elem) )
			return true;
		return( id > h.id );
  }
  else
  	  throw Problem();

}// Handle::higherPriority()

// swap() - swaps 'this' and handle
//   IMPORTANT: id is a private instance variable, hence subclasses of handle
//              that overload this method should explicitly invoke this method like Handle::swap(h)
//              and do not forget that elem is swapped here!
template<typename dataType>
void Heap<dataType>::Handle::swap( Heap<dataType>::Handle& h )
{
	long temp_i = id ;
	id = h.id ;
	h.id = temp_i ;

	dataType temp_e = elem ;
	elem = h.elem ;
	h.elem = temp_e ;
}

// operator*() - return a reference (alias) to the element (type dataType) held by Handle
template<typename dataType>
const dataType&  Heap<dataType>::Handle::operator*() const
{
  return elem ;
}

/*********************************
         HEAP MEMBER FUNCTIONS
     *********************************/

// CONSTRUCTOR
template<typename dataType>
Heap<dataType>::Heap( Heap<dataType>::compareFxn f, Heap<dataType>::order o )
{
	comparison = f ;
	number_of_elements = 0 ;
	ordering = o ;
}

// virtual DESTRUCTOR because of polymorphism
template<typename dataType>
Heap<dataType>::~Heap()
{
  std::cout << "Heap DESTRUCTOR called." << std::endl;
}

// push()
// most probably does not have to be overloaded in the subclass
// IMPORTANT: increments the number of elements so create_new() shouldn't !!!
template<typename dataType>
typename Heap<dataType>::Handle& Heap<dataType>::push( const dataType& ex )
{
	Handle& h = createNew( ex );
	++number_of_elements ;

	siftUp( h );
	
  /* because of swapping, handle h may no longer be the handle for the value just pushed onto the heap
   * -- so have to find this handle and return it.  */
  return( value(ex) );
}

// priorityChange()
// notify the heap that the element with handle h may have had its priority changed!
template<typename dataType>
void Heap<dataType>::priorityChange( Handle& h )
{
	siftUp( h );   // does nothing if priority is lower
	siftDown( h ); // does nothing if priority is higher
}

// pop()
// most probably does not have to be overloaded in the subclass
// IMPORTANT: decrements the number of elements so delete_last() shouldn't !!!
template<typename dataType>
void Heap<dataType>::pop()
{
	if( vide() )
	  throw Problem();

	moveLastToFirst();
	deleteLast();
	--number_of_elements ;

	if( !vide() )
	  siftDown( first() );
}

// empty()
template<typename dataType>
bool Heap<dataType>::vide() const
{ return number_of_elements == 0 ; }

// size() - return the number of elements
template<typename dataType>
int Heap<dataType>::size() const
{ return number_of_elements ; }
