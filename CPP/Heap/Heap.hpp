/*
 * Heap.hpp
 *   Created on: Jan 21, 2011
 *   Author: Mark Sattolo
 * ---------------------------------------------------------------------------
 *   $File: //depot/Eclipse/CPP/Workspace/OrigHeap/Src/Heap.hpp $
 *   $Revision: #7 $
 *   $Change: 48 $
 *   $DateTime: 2011/01/28 17:41:51 $   
 */

#ifndef MHS_CODEBLOCKS_CPP_HEAP_HEAP_HPP
#define MHS_CODEBLOCKS_CPP_HEAP_HEAP_HPP

#include <iostream>

/***
  **  HEAP class
  **
  **    OPERATIONS:
  **       
  **    -  const dataType& top() const; 
  **         return a reference to the top element
  ** 
  **    -  void pop();
  **         remove the top element    
  **
  **    -  handle& push( const dataType& );
  **         insert a new element and return its handle
  **         this handle can be used to update its position
  **         within the heap, if its priority has changed at run-time
  **
  **    -  void priorityChange( Handle& );
  **         update the elements position in the heap because
  **         its priority has changed asynchronously
  **
  **    -  bool empty() const;
  **         returns true if and only if heap is empty
  **
  **    -  int size() const;
  **         returns the number of elements stored in the heap
  **
  ***/
template<typename dataType>
class Heap
{
  public:
  
		// a function matching this prototype should be used to establish the ordering property of the heap
		typedef bool (*compareFxn)( const dataType&, const dataType& );
		
		// a heap can be built by SMALLER_FIRST or LARGER_FIRST, depending what "higher priority" means -- less or more
		enum order { SMALLER_FIRST, LARGER_FIRST };
		
	 /**
	   *  Problem class
		 *    such an exception -- or one of its subclasses, if any -- 
		 *    should be thrown when unusual circumstances arise:
		 *    -- taking the top of an empty heap
		 *    -- popping an empty heap
		 *    -- an array implementation runs out of bounds
		 *    -- etc
		 */
	  class Problem
	  {
	   public:
			Problem()
			{ std::cerr << ">> Heap problem!" << std::endl; }
	  };
    /* inner class Heap<dataType>::Problem */
	  
	 /** 
	   *  Handle class
	   *    - an abstract class, so must be subclassed
		 *		- it is the internal building block of a Heap
		 */
	  class Handle
    {
			private:
				// ALL PRIVATE, SO SUBCLASSES SHOULD NOT CONCERN THEMSELVES WITH LOW LEVEL DETAILS
	      
				// used to generate a unique id
				static long last_id ;
				
				// a unique id, which helps to establish
				// temporal ordering if two nodes have the same priority
				long id ;
				
				// a reference to the actual ordering function - see the typedef above
				compareFxn& handleComparison ;

				// a reference to the heap's ordering - see the enum above
				order& handleOrdering ;

			protected:
				// PROTECTED SECTION -- SUBCLASSES MOST PROBABLY NEED THEM BUT THE PUBLIC SHOULD NOT!

				// actual element
				dataType elem ;

				// even the constructor is 'protected'
				// only heap and its subclasses should create handles!
				Handle( compareFxn&, order&, const dataType& );

			public:
				// INTERFACE

				// if h is a handle, *h is the element it stores
				virtual const dataType& operator*() const ;

				// returns true that "this" is higher priority than the argument
				virtual bool higherPriority( const Handle& );

				// swap "this" with the argument
				// subclasses probably have to overload this method to swap other instance variables declared there
				// if that is the case, they should call this swap as well, to have the internal id's swapped
				virtual void swap( Handle& );

    };
	  /* inner class Heap<dataType>::Handle */

  protected:
		// PROTECTED SECTION -- SUBCLASSES PROBABLY NEED ACCESS TO THE FOLLOWING INSTANCE VARIABLES

		// the function to establish ordering
		compareFxn comparison ;

		// the ordering property of this heap
		order ordering ;

    // the number of elements currently stored in the heap
    int number_of_elements ;

		// THE FOLLOWING METHODS FORM A 'PROTECTED' INTERFACE TO THE SUBCLASSES OF STANDARD HEAP OPERATIONS
		//
		// THE PUBLIC INTERFACE IS IMPLEMENTED USING THESE PRIMITIVE HEAP OPERATIONS

		// the 'sift up' operation, start at the argument
		virtual void siftUp( Handle& ) = 0 ;

		// the 'sift down' operation, start at the argument
		virtual void siftDown( Handle& ) = 0 ;

		// create a new handle at the 'right' position and return its handle
		virtual Handle& createNew( const dataType& ) = 0 ;

		// return the handle of the first or 'highest priority' element
		virtual Handle& first() = 0 ;

		// mode the last element to become the first, needed for pop
		virtual void moveLastToFirst() = 0 ;

		// delete the last element's handle
		virtual void deleteLast() = 0 ;

		// returns a reference to the handle where the value is actually stored
		virtual Handle& index( const Handle& ) const = 0 ;
		
		// returns a reference to the handle that stores a dataType with the same value as the parameter
	  virtual Handle& value( const dataType& ) const = 0 ;
	  
  public:
		// HEAP INTERFACE

		// NOTE: THE COPY CONSTRUCTOR AND THE ASSIGNMENT OPERATOR ARE NOT IMPLEMENTED 
		//       BECAUSE THERE ARE NO POINTER OR RESOURCE TYPE INSTANCE VARIABLES.  
		//       THIS MAY NOT BE THE CASE FOR SUBCLASSES OF HEAP, WHICH PROBABLY HAVE TO IMPLEMENT THEM

		// constructor with ordering function and the order
		Heap( compareFxn, order );

		// so the right version of the destructor gets called in the subclasses
		virtual ~Heap();

		// return a reference to the top element
		virtual const dataType& top() const = 0 ;

		// remove the highest priority element
		virtual void pop();

		// insert a new element and return its handle
		virtual Handle& push( const dataType& );

		// change the position of the element with handle because its priority has changed
		virtual void priorityChange( Handle& );

		// true iff heap is empty
		bool vide() const ;

		// number of elements in the heap
		int size() const ;
};

#endif // MHS_CODEBLOCKS_CPP_HEAP_HEAP_HPP
