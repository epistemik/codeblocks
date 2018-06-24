/*
 * LinkHeap.cpp
 *   Created on: Jan 23, 2011
 *   Author: Mark Sattolo
 * ---------------------------------------------------------------------------
 *   $File: //depot/Eclipse/CPP/Workspace/OrigHeap/Src/LinkHeap.cpp $
 *   $Revision: #7 $
 *   $Change: 48 $
 *   $DateTime: 2011/01/28 17:41:51 $   
 */

#include "LinkHeap.hpp"

/*************************************
         LinkNode MEMBER FUNCTIONS
     *************************************/

template<typename dataType>
LinkHeap<dataType>::LinkNode::LinkNode( const dataType& e, typename Heap<dataType>::compareFxn& f,
                                        typename Heap<dataType>::order& o )
                              : Heap<dataType>::Handle( f, o, e ), left( 0 ), right( 0 ), up( 0 ), pIndex( this )
{ }// LinkNode CONSTRUCTOR

template<typename dataType>
void LinkHeap<dataType>::LinkNode::swap( typename Heap<dataType>::Handle& h )
{
  Heap<dataType>::Handle::swap( h );
  LinkNode& n = dynamic_cast<LinkNode&>( h );
  LinkNode* tmp = pIndex ;
  pIndex = n.pIndex ;
  n.pIndex = tmp ;

}// LinkNode::swap()

/*************************************
        LinkHeap MEMBER FUNCTIONS
    *************************************/

template<typename dataType>
LinkHeap<dataType>::LinkHeap( typename Heap<dataType>::compareFxn f, typename Heap<dataType>::order o )
                    : Heap<dataType>( f, o ), pFirst( 0 ), pLast( 0 )
{
  cout << "Create a LinkHeap.\n" << endl;
}// LinkHeap CONSTRUCTOR

template<typename dataType>
LinkHeap<dataType>::LinkHeap( const LinkHeap<dataType>& hp ) : Heap<dataType>( hp ), pFirst( 0 ), pLast( 0 )
{
  copy( hp.pFirst );

}// LinkHeap COPY CONSTRUCTOR

template<typename dataType>
LinkHeap<dataType>::~LinkHeap()
{
  destroy( pFirst );
  cout << "LinkHeap DESTRUCTOR called." << endl;
  
}// LinkHeap DESTRUCTOR

template<typename dataType>
LinkHeap<dataType>& LinkHeap<dataType>::operator=( const LinkHeap<dataType>& hp )
{
  Heap<dataType>::operator=( hp );
  destroy( pFirst );

  copy( hp.pFirst );
  return *this ;

}// LinkHeap ASSIGNMENT OVERLOAD

template<typename dataType>
typename LinkHeap<dataType>::LinkNode* LinkHeap<dataType>::prev() const
{
  if( pLast == pFirst )
    return pFirst ;

  LinkNode* ptr = pLast ;

  if( ptr->up->right == ptr )
    return ptr->up->left ; // 50 % of cases are trivial

  while( ptr->up  &&  ptr->up->left == ptr )
    ptr = ptr->up ; // go up while left

  if( ptr == pFirst ) // if top reached go down as far right as possible
  {
    while( ptr->right )
      ptr = ptr->right ;
    return ptr ;
  }

  ptr = ptr->up->left ; // jump to sibling

  while( ptr->right )
    ptr = ptr->right ; // go to the far right

  return ptr ;

}// prev()

template<typename dataType>
typename LinkHeap<dataType>::LinkNode* LinkHeap<dataType>::next() const
{
  if( pFirst == 0 )
    return 0 ; // empty

  if( pFirst->left == 0 || pFirst->right == 0 )
    return pFirst ; // singleton

  LinkNode* ptr = pLast ;

  if( ptr->up->right == 0 )
    return ptr->up ; // 50 % of all cases are trivial

  while( ptr->up ) // go up while right
  {
    if( ptr->up->right == ptr )
      ptr = ptr->up ;
    else
        break ;
  }

  if( ptr == pFirst ) // if top reached go as left as possible
  {
    while( ptr->left )
      ptr = ptr->left ;
    return ptr ;
  }

  ptr = ptr->up->right ; // jump to sibling

  while( ptr->left ) // go to the far left
    ptr = ptr->left ;

  return ptr ;
  
}// next()

template<typename dataType>
void LinkHeap<dataType>::copy( LinkHeap<dataType>::LinkNode* n )
{
  if( n == 0 )
    return ;
  
  push( **n );
  copy( n->left );
  copy( n->right );
  
}// copy()

template<typename dataType>
void LinkHeap<dataType>::destroy( LinkHeap<dataType>::LinkNode* n )
{
  if( n == 0 )
    return;

  destroy( n->left );
  destroy( n->right );

  delete n;
  Heap<dataType>::number_of_elements = 0 ;

}// destroy()

template<typename dataType>
typename LinkHeap<dataType>::LinkNode* LinkHeap<dataType>::getHigherPriorityChild( LinkHeap<dataType>::LinkNode* n )
{
  // if (n == 0) return 0;
  if( n->left == 0 )
    return n->right ;

  if( n->right == 0 )
    return n->left ;

  if( n->left->higherPriority( *(n->right) ) )
    return n->left ;

  return n->right ;

}// getHigherPriorityChild()

template<typename dataType>
void LinkHeap<dataType>::siftUp( typename Heap<dataType>::Handle& h )
{
  LinkNode* ptr = &static_cast<LinkNode&>( h );

  while( ptr->up != 0 )
  {
    if( ptr->higherPriority( *(ptr->up) ) ) // element out of order
    {
      ptr->swap( *(ptr->up) );
      ptr = ptr->up ;
    }
    else
        return ;
  }
}// siftUp()

template<typename dataType>
void LinkHeap<dataType>::siftDown( typename Heap<dataType>::Handle& h )
{
  LinkNode* ptr = &static_cast<LinkNode&>( h ), *child ;

  while( (child = getHigherPriorityChild(ptr)) != 0 )
  {
    if( child->higherPriority(*ptr) ) // compare element to larger child
    {
      child->swap( *ptr );
      ptr = child ;
    }
    else
        return ;
  }
}// siftDown()

template<typename dataType>
typename Heap<dataType>::Handle& LinkHeap<dataType>::createNew( const dataType& e )
{
  // keep track of all the nodes
  static LinkNode* prevNode = 0 ;
  
  LinkNode* ptr = next();

  LinkNode* n = new LinkNode( e, Heap<dataType>::comparison, Heap<dataType>::ordering );
  n->left = n->right = 0 ;
  n->up = ptr ;

  // each node stores an alias to the previous node created -- needed in value() to traverse the list
  n->pPrev = prevNode ;
  prevNode = n ;
  
  if( ptr == 0 )
    pLast = pFirst = n ;
  else
  {
    if( ptr->left == 0 )
      pLast = ptr->left = n ;
    else if( ptr->right == 0 )
        pLast = ptr->right = n ;
  }
  
  return *n ;

}// createNew()

template<typename dataType>
typename Heap<dataType>::Handle& LinkHeap<dataType>::first()
{
  if( Heap<dataType>::vide() )
    throw typename Heap<dataType>::Problem();
  return *pFirst ;

}// first()

template<typename dataType>
void LinkHeap<dataType>::moveLastToFirst()
{
  pFirst->swap( *pLast );

}// moveLastToFirst()

template<typename dataType>
void LinkHeap<dataType>::deleteLast()
{
  LinkNode* ptr = pLast ;
  pLast = prev();

  if( ptr->up )
  {
    if( ptr->up->right == ptr )
      ptr->up->right = 0 ;
    else
        ptr->up->left = 0 ;
  }
  else
      pLast = pFirst = 0 ;

  delete ptr ;

}// deleteLast()

template<typename dataType>
typename Heap<dataType>::Handle& LinkHeap<dataType>::index( const typename Heap<dataType>::Handle& h ) const
{
  const LinkNode& n = dynamic_cast<const LinkNode&>( h );
  return *n.pIndex ;

}// index()

template<typename dataType>
typename Heap<dataType>::Handle& LinkHeap<dataType>::value( const dataType& t ) const
{
  if( Heap<dataType>::vide() )
    throw typename Heap<dataType>::Problem();
  
  LinkNode* ptr = pLast ;
  while( ptr )
  {
    if( *const_cast<dataType&>(**ptr) == *const_cast<dataType&>(t) )
      return *ptr ;

    //cout << "pFirst= " << pFirst << " ; pLast = " << pLast << " ; ptr = " << ptr << " ; pPrev = " << ptr->pPrev << endl;

    if( ptr == pFirst )
      break ;
    
    ptr = ptr->pPrev ;
  }
  
  // did NOT find the value
  throw typename Heap<dataType>::Problem();
  
}// value()

template<typename dataType>
const dataType& LinkHeap<dataType>::top() const
{
  if( Heap<dataType>::vide() )
    throw typename Heap<dataType>::Problem();
  return **pFirst ;

}// top()

template<typename dataType>
void LinkHeap<dataType>::print( ostream& os, const LinkHeap<dataType>::LinkNode* n, int k ) const
{
  for( int i = 0; i < k; ++i )
  {
    cout << "  " ;
  }

  if( n == 0 )
  {
    os << '.' << endl;
    return;
  }

  cout << **n << "  ( this=" << (void*)n << ", pPrev=" << n->pPrev << ", pIndex=" << n->pIndex << ", l=" << n->left << ", r=" << n->right << ", u=" << n->up << " )" << endl;

  print( os, n->left, k + 1 );
  print( os, n->right, k + 1 );

}// print()

template<typename dataType>
void LinkHeap<dataType>::print( ostream& os ) const
{
  os << "pFirst = " << pFirst << " ; pLast = " << pLast << endl;
  print( os, pFirst );

}// print()
