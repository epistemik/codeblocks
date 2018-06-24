/* *****************************************************************************

    Mark Sattolo (epistemik@gmail.com)
   -----------------------------------------------
     $File: //depot/Eclipse/CPP/Workspace/Knapsack/Src/Knapitems.hpp $
     $Revision: #4 $
     $Change: 49 $
     $DateTime: 2011/01/29 08:29:09 $

****************************************************************************** */

#ifndef MHS_CODEBLOCKS_CPP_KNAPSACK_KNAPITEMS_HPP
#define MHS_CODEBLOCKS_CPP_KNAPSACK_KNAPITEMS_HPP

#include <cstdlib>
#include <iostream>
#include <string>
#include <queue>
#include <vector>
#include <iomanip>

using namespace std ;

namespace mhs_codeblocks_knapsack
{
 static int ITEMS_DISPLAY_WIDTH = 16 ;
 
 enum DEBUG_VALUES { DEBUG_OFF, DEBUG_LOW, DEBUG_MID, DEBUG_HIGH, DEBUG_MAX };
 const char DEBUG_VALUES_STR[] = "01234" ;
 
 /* attributes and operations for items that can be placed in the knapsack  */
 class KnapItem
 {
  private:
    
    string  name     ; // identify each object
    string  items    ; // each object can actually contain (the names of) several items - see class KnapSST
    int     index    ; // of last item selected from the sorted items vector (equivalent to the level in the sst)
    int	    profit   ; // from user input
    int	    weight   ; // from user input
    float	  pw_ratio ; // profit to weight ratio
    float	  bound    ; // the potential max profit of each node - calculated with the state space tree

  public:
    
		// CONSTRUCTOR with index
    KnapItem( string n, string i, int x )
      : name(n), items(i), index(x), profit(0), weight(0), pw_ratio(0.0), bound(0.0)
    { }

    // CONSTRUCTOR with pw
    KnapItem( string n, string i, int p, int w )
      : name(n), items(i), index(0), profit(p), weight(w), bound(0.0)
    {
      calcPwr();
    }

    // Accessor & Mutator functions

    string  getName() const { return name ; }
    void    setName( string s )
    {
      if( ! s.empty() )
        name.assign( s );
    }

    string  getItems() const { return items ; };
    void    setItems( string s )
    {
      if( ! s.empty() )
        items.assign( s );
    }
    KnapItem  addItem( string a )
    {
      if( ! a.empty() )
        items.append( a );
      return *this ;
    }

    int   getIndex() const { return index ; }
    void  setIndex( int x ) { index = x ; }
    void  incIndex( int i ) { index += i ; }

    int   getProfit() const { return profit ; }
    void  setProfit( int p ) { profit = p ; calcPwr(); }
    void  addProfit( int a ) { profit += a ; calcPwr(); }

    int   getWeight() const { return weight ; }
    void  setWeight( int w ) { weight = w ; calcPwr(); }
    void  addWeight( int a ) { weight += a ; calcPwr(); }

    float  getPwr() const { return pw_ratio ; }
    void   setPwr( float r ) { pw_ratio = r ; }
    float  calcPwr()
    {
      pw_ratio = (weight > 0) ? (static_cast<float>(profit)/static_cast<float>(weight)) : 0.0 ;
      return pw_ratio ;
    }

    float  addPw( int p, int w )
    {
      profit += p ;
      weight += w ;
      return calcPwr();
    }

    float  getBound() const { return bound ; }
    void   setBound( float b ) { bound = b ; }

    // OPERATOR OVERLOADS to enable less<KnapItem>, ordered by bound - the predicate used in KnapSST

    bool operator< ( const KnapItem& k2 ) const
    {
      if( bound < k2.getBound() )
        return true ;
      return false ;
    }

    bool operator== ( const KnapItem& k2 ) const
    {
      return( bound == k2.getBound() );
    }

    // DEBUG

    void display( ostream& strm, int items_display_width ) const
    {
      strm << name << ": '" << setw(items_display_width) << ( items.empty() ? "<EMPTY>" : items )
           << "' ; ind " << setw(3) << index
           << " ; prof " << setw(6) << profit
           << " ; wt " << setw(5) << weight ;
      strm.setf( ios_base::fixed, ios_base::floatfield );
      strm << setprecision(3) << " ; bnd " << setw(9) << bound << " ; pwr " << setw(7) << pw_ratio << endl ;
    }

    void display( ostream& strm ) const
    {
      display( strm, ITEMS_DISPLAY_WIDTH );
    }

 };/* class KnapItem */


/* * * *  PRIORITY QUEUE for KnapItem objects * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  for the state space tree used in bestFirstSearch()
 */

 // nomenclature used when each 'KnapItem' actually contains several items, as in the sst
 typedef KnapItem KnapNode ;

 // typedefs for STL type 'priority_queue'
 // - see "The C++ Standard Template Library", by Plauger et al, p.455
 typedef allocator<KnapNode> KnapNodeAllocator ;
 typedef vector<KnapNode, KnapNodeAllocator> KnapNodeContainer ;
 typedef less<KnapNode> KnapNodePredicate ;

 // class for a KnapNode priority queue to implement the state space tree
 class KnapSST: public priority_queue<KnapNode, KnapNodeContainer, KnapNodePredicate>
 {
  public:

   // for simplicity, combine top() and pop()
   KnapNode get()
   {
     KnapNode kn = top();
     pop();
     return kn ;
   }

   // for debug/display
   void display( ostream& strm, string name )
   {
     strm << endl << "top of " << name << " = " ;
     top().display( strm, top().getName().size() );
   }

 };/* class KnapSST */


/* * * * *  VECTOR for KnapItem objects  * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  store, sort and display Knapitems
 */

 // class for a vector to hold the original KnapItems sorted by profit/weight ratio
 class KnapVector: public vector<KnapItem>
 {
  public:

   /* Constructor */
   KnapVector( ostream &outd, ostream &errd ) : printDest( outd ), errDest( errd )
   {
     debugLevel = DEBUG_LOW ;
   }

   /* Member Functions */

   // see if the given node has a bound greater than the current maximum profit
   bool checkPromising( KnapSST&, KnapNode&, const int, const int, string="" );

   // see if the given node has a bound greater than the current maximum profit
   bool checkProfit( KnapNode&, const int, int&, int&, string& );

   // calculate the bound for the given item using this vector and the given maxWt
   float calculateBound( KnapItem&, const int, int=DEBUG_LOW );

   // the actual work of calculating the optimal set of items
   int bestFirstSearch( const int, int&, string& );

   // for debug/display

   int getDebug() const { return debugLevel ;}
   void setDebug( const int d ) { debugLevel = d ;}

   void display()
   {
     int i = 1 ;
     iterator it = begin();

     printDest << "\nVector Elements:" << endl;
     while( it != end() )
     {
       printDest << " #" ;
       if( i < 10 )
         printDest << " " ;
       printDest << i << " " ;
       it->display( printDest );

       it++ ;
       i++ ;
     }
     //printDest << endl ;

   }// display()

   /* static variables */

   static const string ROOT ,
                       ROOT_MARKER ,
                       WITH_CHILD ,
                       WITH_CHILD_MARKER ,
                       WITHOUT_CHILD ,
                       WITHOUT_CHILD_MARKER ;

  private:

   /* member variables */

   int debugLevel ;

   // to control display destination
   ostream &printDest, &errDest ;

 };/* class KnapVector */


 // function object to order a KnapVector by pw ratio
 struct knap_order_pwr
 {
   bool operator()( const KnapItem& a, const KnapItem& b )
   {
     return( a.getPwr() > b.getPwr() ); // descending order
   }
 };

}// namespace mhs_codeblocks_knapsack

#endif // MHS_CODEBLOCKS_CPP_KNAPSACK_KNAPITEMS_HPP
