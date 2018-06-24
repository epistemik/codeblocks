/*
 * ArrayHeap.cpp
 *   Created on: Jan 21, 2011
 *   Author: Mark Sattolo
 * ---------------------------------------------------------------------------
 *   $File: //depot/Eclipse/CPP/Workspace/OrigHeap/Src/ArrayHeap.cpp $
 *   $Revision: #8 $
 *   $Change: 48 $
 *   $DateTime: 2011/01/28 17:41:51 $   
 */

#include "ArrayHeap.hpp"

/**************************************
         ArrayNode MEMBER FUNCTIONS
		 **************************************/

// CONSTRUCTOR: ArrayNode's ind variable will record each node's position in the array
template<typename dataType>
ArrayHeap<dataType>::ArrayNode::ArrayNode( const dataType& e, typename Heap<dataType>::compareFxn& f,
													                 typename Heap<dataType>::order& o, int ind )
												        : Heap<dataType>::Handle( f, o, e )
{ index = ind ; }

// ASSIGNMENT OVERLOAD: must copy the ind variable
template<typename dataType>
typename ArrayHeap<dataType>::ArrayNode&  ArrayHeap<dataType>::ArrayNode::operator=( const ArrayNode& a )
{
	index = a.index ;
	return *this ;
}

/**************************************
				 ArrayHeap MEMBER FUNCTIONS
		 **************************************/

// CONSTRUCTOR: create the array and store its size
template<typename dataType>
ArrayHeap<dataType>::ArrayHeap( typename Heap<dataType>::compareFxn f, typename Heap<dataType>::order o, int size )
                     : Heap<dataType>( f, o )
{
	array = new ArrayNode*[ size ];
	max_size = size ;
  cout << "Create an ArrayHeap.\n" << endl;
}

// COPY CONSTRUCTOR: create a new copy of the each array element
template<typename dataType>
ArrayHeap<dataType>::ArrayHeap( const ArrayHeap<dataType>& H ) : Heap<dataType>(H)
{
	array = new ArrayNode*[ H.max_size ];
	for( int i = 0 ; i < H.size() ; i++ )
	  push( **H.array[i] );
	max_size = H.max_size ;
}

// DESTRUCTOR: have to delete the array elements as they were dynamically allocated
template<typename dataType>
ArrayHeap<dataType>::~ArrayHeap()
{
  for( int i = 0 ; i < Heap<dataType>::size() ; i++ )
    delete array[i] ;
  delete [] array ;
  
  cout << "ArrayHeap DESTRUCTOR called." << endl;
}

// ASSIGNMENT OVERLOAD: destroy the old array and copy the newly-assigned one
template<typename dataType>
ArrayHeap<dataType>& ArrayHeap<dataType>::operator=( const ArrayHeap<dataType>& H )
{
	Heap<dataType>::operator=( H );

	for( int i = 0 ; i < Heap<dataType>::size() ; i++ )
	  delete array[i] ;
	delete [] array ;

	array = new ArrayNode*[ H.max_size ];
	for( int j = 0 ; j < H.size() ; j++ )
	  push( **H.array[j] ) ;

	max_size = H.max_size ;
	return *this;
}

// swap(): swap the positions of two ArrayNodes -- used in siftUp() and siftDown()
template<typename dataType>
void ArrayHeap<dataType>::swap( typename Heap<dataType>::Handle& h1, typename Heap<dataType>::Handle& h2 )
{
	h1.swap( h2 );
	ArrayNode& a1 = static_cast<ArrayNode&>( h1 );
	ArrayNode& a2 = static_cast<ArrayNode&>( h2 );

	ArrayNode temp = *array[a1.index] ;
	*array[a1.index] = *array[a2.index] ;
	*array[a2.index] = temp ;

	for( int i=0; i < Heap<dataType>::size(); i++ )
	  (*array[i]).index = i ;
}

// left(): find the array index of the left child
template<typename dataType>
int ArrayHeap<dataType>::left( int a ) const
{
	if( Heap<dataType>::vide() )
    throw typename Heap<dataType>::Problem();
	
	return (a+1)*2 - 1 ;
}

// right(): find the arry index of the right child
template<typename dataType>
int ArrayHeap<dataType>::right( int a ) const
{
	if( Heap<dataType>::vide() )
    throw typename Heap<dataType>::Problem();
	
	return (a+1)*2 ;
}

// parent(): find the array index of the parent node
template<typename dataType>
int ArrayHeap<dataType>::parent( int a ) const
{
	if( Heap<dataType>::vide() )
    throw typename Heap<dataType>::Problem();
	
	return (a+1)/2 - 1 ;
}

// siftUp: move child node up the array if it is higher priority than its parent
template<typename dataType>
void ArrayHeap<dataType>::siftUp( typename Heap<dataType>::Handle& h )
{
	ArrayNode& a = static_cast<ArrayNode&>( h );
	int son = a.index ;
	while( son != 0 )
	{
		int dad = parent( son ) ;
		if( (*array[son]).higherPriority(*array[dad]) )
		{
			swap( *array[son], *array[dad] );
			son = dad ;
		}
		else break ;
	}
}

// siftDown(): continually move a node down if it is lower priority than its highest priority child
template<typename dataType>
void ArrayHeap<dataType>::siftDown( typename Heap<dataType>::Handle& h )
{
	ArrayNode& a = dynamic_cast<ArrayNode&>( h );
	int upper = a.index ;
	while( true )
  {
		if( left(upper) > last() )
		  // no children
		  break ;
		int hpchild ;
		if( right(upper) > last() )
		  // if no right then highest priority is left child
		  hpchild = left( upper );
		else
			// find highest priority child
			hpchild = ( *array[left(upper)] ).higherPriority( *array[right(upper)] ) ? left( upper ) : right( upper );

		// swap node and child if child is higher priority
		if( ( *array[hpchild] ).higherPriority( *array[upper] ) )
		{
			swap( *array[hpchild], *array[upper] );
			upper = hpchild ;
		}
		else break;
	}
}

// createNew(): used in Heap::push()
template<typename dataType>
typename Heap<dataType>::Handle&  ArrayHeap<dataType>::createNew( const dataType& e )
{
	if( Heap<dataType>::size() >= max_size )
	  throw typename Heap<dataType>::Problem();
  
	array[Heap<dataType>::size()] = new ArrayNode( e, Heap<dataType>::comparison, Heap<dataType>::ordering, Heap<dataType>::size() );
	return *array[ Heap<dataType>::size() ];
}

// first(): reference the first element
template<typename dataType>
typename Heap<dataType>::Handle&  ArrayHeap<dataType>::first()
{
	if( Heap<dataType>::vide() )
	  throw typename Heap<dataType>::Problem();
	
	return *array[0] ;
}

// last(): index of the current last element
template<typename dataType>
int ArrayHeap<dataType>::last() const
{
	if( Heap<dataType>::vide() || Heap<dataType>::size() > max_size )
	  throw typename Heap<dataType>::Problem();
	
	return Heap<dataType>::size() - 1 ;
}

// moveLastToFirst(): used in Heap::pop()
template<typename dataType>
void ArrayHeap<dataType>::moveLastToFirst()
{ swap( *array[0], *array[last()] ); }

// deleteLast(): ONLY used in Heap::pop()
template<typename dataType>
void ArrayHeap<dataType>::deleteLast()
{
	if( Heap<dataType>::vide() )
    throw typename Heap<dataType>::Problem();
	
	delete array[ last() ];
}

// index(): used in Heap::priorityChange()
template<typename dataType>
typename Heap<dataType>::Handle&  ArrayHeap<dataType>::index( const typename Heap<dataType>::Handle& h ) const
{
	const ArrayNode& a = dynamic_cast<const ArrayNode&>( h );
	return *array[ a.index ];
}

// value(): used in Heap::push()
template<typename dataType>
typename Heap<dataType>::Handle&  ArrayHeap<dataType>::value( const dataType& t ) const
{
  // return the node that has the same value as the parameter dataType
  for( int i=0; i < Heap<dataType>::size(); i++ )
    if( (*const_cast<dataType&>(**array[i])) == (*const_cast<dataType&>(t)) )
      return *array[i] ;
  
  // NO matching value
  throw typename Heap<dataType>::Problem();
}

// top(): get the value at the top of the array
template<typename dataType>
const dataType& ArrayHeap<dataType>::top() const
{
	if( Heap<dataType>::vide() )
    throw typename Heap<dataType>::Problem();
	
	return **array[0] ;
}

// print(): recursively print a node and all its sub-nodes
template<typename dataType>
void ArrayHeap<dataType>::print( ostream& os, const ArrayHeap<dataType>::ArrayNode* n, int k ) const
{
	for( int i=0; i < k; ++i )
	  os << "  " ;

	os << **n << "  ( this=" << (void*)n << ", index=" << n->index << ", l=" << array[left( (*n).index )]
		 << ", r=" << array[right( (*n).index )] << ", p=" << array[parent( (*n).index )] << " )" <<  endl;

	if( left( (*n).index ) <= last() )
	  print( os, array[left( (*n).index )], k+1 );
	else
	{
		for( int i=0; i < k+1; ++i )
		  os << "  " ;
		os << "."  << endl;
	}

	if( right( (*n).index ) <= last() )
	  print( os, array[right( (*n).index )], k+1 );
	else
	{
		for( int i=0; i < k+1; ++i )
		  os << "  " ;
		os << "." << endl;
	}
}

// print(): print the first and last node addresses then print the array
template<typename dataType>
void ArrayHeap<dataType>::print( ostream& os ) const
{
	os << "First = " << array[0] << " ; Last = " << array[last()] << endl;
	print( os, array[0] );
}
