/*
 * LinkHeap.hpp
 *   Created on: Jan 23, 2011
 *   Author: Mark Sattolo
 * ---------------------------------------------------------------------------
 *   $File: //depot/Eclipse/CPP/Workspace/OrigHeap/Src/LinkHeap.hpp $
 *   $Revision: #8 $
 *   $Change: 48 $
 *   $DateTime: 2011/01/28 17:41:51 $   
 */

#ifndef MHS_CODEBLOCKS_CPP_HEAP_LINKHEAP_HPP
#define MHS_CODEBLOCKS_CPP_HEAP_LINKHEAP_HPP

using namespace std;

#include <iostream>
#include "Heap.hpp"

/***
  ** class LinkHeap - a heap implemented as a linked list
  **
  **   the Heap interface is not modified and not extended
  **/
template<typename dataType>
class LinkHeap: public Heap<dataType> 
{
 private:
  
  /***
    ** LinkNode subclass of Heap<dataType>::Handle
    ** 
    **   it inherits elem, and the unique id mechanism
    **/
  class LinkNode: public Heap<dataType>::Handle 
  {
   public:
    LinkNode* left;
    LinkNode* right;
    LinkNode* up;
    
    // original index (this) of the node
    mutable LinkNode* pIndex ;
    
    // alias to the previous created node
    mutable LinkNode* pPrev ;
    
    // CONSTRUCTOR
    LinkNode( const dataType&, typename Heap<dataType>::compareFxn&, typename Heap<dataType>::order& );
           
    void swap( typename Heap<dataType>::Handle& );
    
  };
  /* inner class LinkHeap::LinkNode */
  
  // pointer to top element 
  LinkNode* pFirst ;
  
  // pointer to last inserted element
  LinkNode* pLast ;
  
  // find where the previous element was inserted - this would be a piece of cake for array 
  LinkNode* prev() const ;

  // find where the next element should be added - this would be a piece of cake for array 
  LinkNode* next() const ;

  // create deep copy recursively
  void copy( LinkNode* );

  // destroy recursively below LinkNode
  void destroy( LinkNode* );

  // return the child with higher priority
  LinkNode* getHigherPriorityChild( LinkNode* );

  // print everything below LinkNode
  void print( ostream&, const LinkNode*, int = 0 ) const ;

 protected:
  
  // see Heap.hpp
  // pure virtual methods implemented
  //
  void siftUp( typename Heap<dataType>::Handle& );
  void siftDown( typename Heap<dataType>::Handle& );
  typename Heap<dataType>::Handle& createNew( const dataType& );
  typename Heap<dataType>::Handle& first();
  void moveLastToFirst();
  void deleteLast();
  typename Heap<dataType>::Handle& index( const typename Heap<dataType>::Handle& ) const ;
  typename Heap<dataType>::Handle& value( const dataType& ) const ;
  
 public:

  // usual constructor and destructor business
  LinkHeap( typename Heap<dataType>::compareFxn, typename Heap<dataType>::order );
  LinkHeap( const LinkHeap<dataType>& );
  LinkHeap<dataType>& operator=( const LinkHeap<dataType>& );
  ~LinkHeap();

  // used for debugging
  void print( ostream& ) const ;

  const dataType& top() const ;
  
};// class LinkHeap

#endif // MHS_CODEBLOCKS_CPP_HEAP_LINKHEAP_HPP
