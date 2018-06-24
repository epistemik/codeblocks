/*
 * Instance.cpp
 *   Created on: Jan 21, 2011
 *   Author: Mark Sattolo
 * ---------------------------------------------------------------------------
 *   $File: //depot/Eclipse/CPP/Workspace/OrigHeap/Src/Instance.cpp $
 *   $Revision: #6 $
 *   $Change: 45 $
 *   $DateTime: 2011/01/27 19:35:54 $   
 */

// NEED ALL THESE #include's FOR THE PROJECT TO COMPILE!!
#include "Test.hpp"
#include "Heap.cpp"
#include "ArrayHeap.cpp"
#include "LinkHeap.cpp"

// instantiate an ArrayHeap with TestType
template class ArrayHeap<TestType> ;

// instantiate a LinkHeap with TestType
template class LinkHeap<TestType> ;
