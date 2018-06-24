/*
 * ArrayHeap.hpp
 *   Created on: Jan 21, 2011
 *   Author: Mark Sattolo
 * ---------------------------------------------------------------------------
 *   $File: //depot/Eclipse/CPP/Workspace/OrigHeap/Src/ArrayHeap.hpp $
 *   $Revision: #7 $
 *   $Change: 48 $
 *   $DateTime: 2011/01/28 17:41:51 $   
 */

#ifndef MHS_CODEBLOCKS_CPP_HEAP_ARRAYHEAP_HPP
#define MHS_CODEBLOCKS_CPP_HEAP_ARRAYHEAP_HPP

using namespace std;

const int DEFAULT_ARRAY_SIZE = 256 ;

#include <iostream>
#include "Heap.hpp"

/***
  **  ArrayHeap class
  **  
  **  - Subclass of Heap<dataType>
  **  - a heap implemented as an array
  **
  **    OPERATIONS:
  **       
  **    - const dataType& top() const; 
  **        return a reference to the top element
  ** 
  **    - void print( ostream& ) const ;
  **        print the heap    
  **
  ***/
template<typename dataType>
class ArrayHeap : public Heap<dataType>
{
 private:
	
	/** 
	  *  ArrayNode class
	  *    
	  *    - Subclass of Heap<dataType>::Handle
	  *		 - it inherits the 'elem' variable
	  */
	 class ArrayNode : public Heap<dataType>::Handle
	 {
	  public:
		 // a variable to keep track of the array index of each ArrayNode
		 mutable int index ;
		 // constructor
		 ArrayNode( const dataType&, typename Heap<dataType>::compareFxn&, typename Heap<dataType>::order&, int );
		 // assignment
		 ArrayNode& operator=( const ArrayNode& a );
		 
	 };// inner class ArrayHeap<dataType>::ArrayNode

	// print a node and all its sub-nodes
	void print( ostream&, const ArrayHeap<dataType>::ArrayNode*, int=0 ) const ;
 
 protected:
	// the variables of array_heap
	int max_size ;
	ArrayNode** array ;
	
	// some useful methods
	void swap( typename Heap<dataType>::Handle&, typename Heap<dataType>::Handle& );
	int left( int ) const ;
	int right( int ) const ;
	int parent( int ) const ;
	int last() const ;
	
	// pure virtual methods of parent class 'Heap' are declared
	void siftUp( typename Heap<dataType>::Handle& );
	void siftDown( typename Heap<dataType>::Handle& );
	typename Heap<dataType>::Handle& createNew( const dataType& );
	typename Heap<dataType>::Handle& first();
	void moveLastToFirst();
	void deleteLast();
	typename Heap<dataType>::Handle& index( const typename Heap<dataType>::Handle& ) const ;
  typename Heap<dataType>::Handle& value( const dataType& ) const ;

 public:
	// constructor with a default array size
	ArrayHeap( typename Heap<dataType>::compareFxn, typename Heap<dataType>::order, int=DEFAULT_ARRAY_SIZE );

	// copy constructor
	ArrayHeap( const ArrayHeap<dataType>& );

	// assignment overload
	ArrayHeap<dataType>& operator=( const ArrayHeap<dataType>& );

	// destructor
	~ArrayHeap();

	// pure virtual method of parent class 'Heap' declared here
	const dataType& top() const ;

	// print
	void print( ostream& ) const ;

};//class ArrayHeap

#endif // MHS_CODEBLOCKS_CPP_HEAP_ARRAYHEAP_HPP
