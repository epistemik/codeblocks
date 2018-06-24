/* *****************************************************************************************

    Mark Sattolo (epistemik@gmail.com)
   -----------------------------------------------
     $File: //depot/Eclipse/CPP/Workspace/Knapsack/Src/Knapsack.cpp $
     $Revision: #8 $
     $Change: 53 $
     $DateTime: 2011/01/29 22:05:05 $

********************************************************************************************

  Best-First Search with Branch-and-Bound Pruning Algorithm for the 0-1 Knapsack Problem

  see p.235 of "Foundations of Algorithms: with C++ pseudocode", 2nd Ed. 1998,
  by Richard Neapolitan & Kumarss Naimipour, Jones & Bartlett, ISBN 0-7637-0620-5

  PROBLEM: Let n items be given, where each item has a weight and a profit.
    			 The weights and profits are positive integers.
    			 Then, let a positive integer W be given as the MAXIMUM WEIGHT.
    			 Determine an optimal selection of items that gives the MAXIMUM TOTAL PROFIT,
    			 under the constraint that the sum of item weights cannot exceed W.

  INPUTS:  1) A positive integer giving the maximum value permitted for the sum
              of the weights of all selected items.
           2) Name of a file which contains a list of items which can be chosen.
              The file contains ONE ITEM PER LINE arranged as follows:

	           <string (item NAME)>'whitespace'<int (PROFIT)>'whitespace'<int (WEIGHT)>

  OUTPUTS: 1) An integer that is the sum of the PROFITS of the optimal set.
           2) An integer that is the sum of the WEIGHTS of the optimal set.
           3) A list of the NAMES of all the items comprising the optimal set.

******************************************************************************************** */

#include <algorithm> // for sort()
#include <unistd.h>  // for sleep()
#include <cstring>
#include <fstream>
#include <sstream>
#include "Knapitems.hpp"

namespace mhs_codeblocks_knapsack
{

  /* Check the debug index string from the command line  */
  bool goodDebugInput( const char *in, char *szmain, char *szvec, int *nmain, int *nvec )
  {
    bool result = false ;
    //cout << "in = " << in << " & (in+1) = " << (in+1) << endl;

    if( strlen(in) != 2 )
      cout << "Input size NOT good... submitted debug index was '" << in << "'" << endl;
    else
    {
      strncpy( szmain, in, 1 ); // get the first digit
      strncpy( szvec, (in+1), 1 ); // get the second digit

      // test main and vector debug to see if the values are valid
      if( strpbrk(szmain,DEBUG_VALUES_STR) != NULL  &&  strpbrk(szvec,DEBUG_VALUES_STR) != NULL )
      {
        *nmain = atoi( szmain );
        *nvec = atoi( szvec );
        cout << "main debug = " << szmain << " & vector debug = " << szvec << endl;
        result = true ;
      }
      else
          cout << "Debug value NOT good... submitted debug index was '" << in << "'" << endl;
    }

    return result ;
  }


  /* Check the command line parameters  */
  bool setup( int argc, char* argv[], int &maxWt, int &main_debug, KnapVector &kv )
  {
    int num, numVals ;

    stringstream ssDbgOff, ssDbgMax, ssMain, ssVec, ssWt ; // convert values to strings for throw
    ssDbgOff << DEBUG_OFF ;
    ssDbgMax << DEBUG_MAX ;

    string
          input = "" ,
          bin( basename(argv[0]) );

    const string
                cmd_line = " Usage: '" + bin + " <items_file> [debug_index] [max weight]'\n"
                           "        <items_file> is a text file with ONE ITEM PER LINE arranged as follows:\n"
                           "        <string (item name)> 'whitespace' <int (profit)> 'whitespace' <int (weight)>\n" ,
              debug_info = "a number from " + ssDbgOff.str() + " to " + ssDbgMax.str() + " " ,
        debug_value_info = " debug value = " + debug_info ;

    try
    {
      // must at least have 'items file name' on the command line
      if( argc < 2 )
        throw( cmd_line + string(" debug index = ##, where each # is ") + debug_info );

      // debug levels
      char szMain[2] = "" ,
            szVec[2] = "" ;

      int nMain = -1 ,
          nVec  = -1 ;

      bool good_debug_index = false ;

      if( argc >= 3 ) // have debug_index on the command line
        good_debug_index = goodDebugInput( argv[2], szMain, szVec, &nMain, &nVec );

      if( argc < 3 || !good_debug_index )// NOT on the command line OR command line entry was NOT good...
      {
        num = -1 ;
        numVals = 0 ;

        while( numVals < 2 ) // get the two debug parameters
        {
          cout << debug_value_info << endl;

          cout << ( ( numVals < 1 ) ? "Please enter the main() debug value: " : "Please enter the Vector debug value: " );

          getline( cin, input );
          // this code converts from string to number safely
          stringstream myStream( input );
          if( myStream >> num )
          {
            cout << "\nYour number: " << num << endl;
            if( num >= DEBUG_OFF  &&  num <= DEBUG_MAX )
            {
              if( numVals < 1 )
                nMain = num ;
              else
                  nVec = num ;
              numVals++ ;
            }
            else
                cout << " Please try again." ;
          }
          else
              cout << "\nYour input: " << input << "... Please try again." << endl;
        }// while( numVals < 2 )
      }// if did not have a good debug index

      main_debug = nMain ;
      kv.setDebug( nVec );
      ssMain << nMain ;
      ssVec << nVec ;
      if( main_debug < DEBUG_OFF || kv.getDebug() < DEBUG_OFF || main_debug > DEBUG_MAX || kv.getDebug() > DEBUG_MAX )
        throw( debug_value_info + string("Problem with debug index: ") + ssMain.str() + ssVec.str() );

      // maximum weight
      if( argc >= 4 ) // max Wt was on the command line
      {
        maxWt = atoi( argv[3] );
        cout << "Max weight value on command line was '" << argv[3] << "'" << endl;
      }
      if( argc < 4  ||  maxWt < 1 )// NOT on the command line OR command line entry was NOT good...
      {
        num = 0 ;

        while( true )
        {
          cout << "Max weight must be a whole number greater than zero." ;

          cout << "\nEnter the maximum weight: " ;
          getline( cin, input );

          // This code converts from string to number safely
          stringstream myStream( input );
          if( myStream >> num )
          {
            cout << "\nYour number: " << num << endl;
            if( num > 0 )
            {
              maxWt = num ;
              break ;
            }
            else
                cout << " Please try again. " ;
          }
          else
              cout << "\nYour input: " << input << "... Please try again." << endl;
        }// while(true)
      }// if did not have a good debug index

      ssWt << maxWt ;
      if( maxWt < 1 )
        throw( string("Max weight MUST be a positive integer! Not: ") + ssWt.str() );
    }
    // any problems that reach the catch clauses warrant program exit
    catch( string msg )
    {
      cout.flush();
      sleep( 1 ); // let any cout output finish
      cerr << msg << " -- TERMINATING PROGRAM!\n" << endl;
      return false ;
    }
    catch(...)
    {
      cout.flush();
      sleep( 1 ); // let any cout output finish
      cerr << "UNSPECIFIED ERROR!" << endl;
      return false ;
    }

    // No errors - display the acquired information
    cout << "\n File name is '" << argv[1] << "'\n" << " Max Weight = " << maxWt
         << "\n main() debug = " << main_debug << "\n KnapVector debug = " << kv.getDebug() << "\n" << endl;

    return true ;

  }// setup()


  /* process values for item name, profit & weight from the input file  */
  int getFileData( char *filename, KnapVector &vec, int dbg )
  {
    string // temps for file data
          strItem = "" ,
          strProf = "" ,
          strWt   = "" ;

    int
       nProf  = 0 ,
       nWt    = 0 ,
       nItems = 0 ; // total number of items in the file

    stringstream ssLine ; // line number of input file for throw
    string msg ; // error handling

    try
    {
      ifstream inputfile( filename );
      if( !inputfile )
      {
        msg = string( "\nError occurred opening file " ) + string( filename );
        throw msg ;
      }
      else
          cout << "Processing file '" << filename << "'" << endl;

      if( dbg > DEBUG_LOW ) cout << endl ;

      // get the data from the file
      while( !inputfile.eof() )
      {
        ssLine << (nItems+1) ; // for throw

        inputfile >> strItem ; // accept any string as the item name

        inputfile >> strProf ;
        // this code converts from string to number safely
        stringstream ssProf( strProf );
        if( !(ssProf >> nProf) )
        {
          msg = string( "\nBAD profit input: '" ) + strProf + string( "' at line #" ) + ssLine.str()
                + string( "... EXITING file processing." );
          throw msg ; // leave the loop and just use the good data we have so far
        }

        inputfile >> strWt ;
        // this code converts from string to number safely
        stringstream ssWt( strWt );
        if( !(ssWt >> nWt) )
        {
          msg = string( "\nBAD weight input: '" ) + strWt + string( "' at line #" ) + ssLine.str()
                + string( "... EXITING file processing." );
          throw msg ; // leave the loop and just use the good data we have so far
        }

        if( inputfile.get() != EOF ) // use get() to eat the EOL
        {
          // create a new node and load it into the vector
          vec.push_back( KnapNode("input", strItem, nProf, nWt) );

          if( dbg > DEBUG_OFF )
          {
            cout << "Current line: " ;
            vec.back().display( cout ) ;
          }

          nItems++ ;
        }

      }// while( !inputfile.eof() )

      inputfile.close();
    }
    catch( string msg )
    {
      cout.flush();
      sleep( 1 ); // let any cout output finish
      cerr << msg << "\n" << endl;
      sleep( 2 );
    }
    catch(...)
    {
      cout.flush();
      sleep( 1 ); // let any cout output finish
      cerr << "UNSPECIFIED ERROR!" << endl;
      nItems = 0 ; // cause program exit for any other error
    }

    if( dbg > DEBUG_OFF )
      cout << "\nFound " << nItems << " items in file '"<< filename << "'" << endl ;

    return nItems ;

  }// getFileData()

/*
 *   KnapVector implementation
 *
 * ======================================================================================================================= */

  const string KnapVector::ROOT = "ROOT" ,
               KnapVector::ROOT_MARKER = "#" ,
               KnapVector::WITH_CHILD = "withChild" ,
               KnapVector::WITH_CHILD_MARKER = "&" ,
               KnapVector::WITHOUT_CHILD = "withoutChild" ,
               KnapVector::WITHOUT_CHILD_MARKER = "-" ;

  /* if the given node has a bound greater than the current maximum profit, then add this node to the state space tree  */
  bool KnapVector::checkPromising( KnapSST &sst, KnapNode &kn, const int maxWt, const int maxProf, string s )
  {
    if( calculateBound(kn, maxWt) > maxProf )
    {
      // copy this node and put back in sst with an optional extra item
      sst.push( KnapNode(kn).addItem(s) );
      if( debugLevel > DEBUG_OFF )
        printDest << __FUNCTION__ << "/" << __LINE__ << "/push '" << kn.getName() << "' on sst" << endl;

      return true ;
    }

    if( debugLevel > DEBUG_OFF )
      printDest << __FUNCTION__ << "/" << __LINE__ << "/" << kn.getName()
                << " bound (" << kn.getBound() << ") <= max profit" << endl ;

    return false ;

  }// KnapVector::checkPromising()


  /* if the given node has a profit greater than the current maximum profit and is LTE the max weight,
     then update the parameters tracking the current profit, weight, and best items  */
  bool KnapVector::checkProfit( KnapNode &kn, const int maxWt, int &currWt, int &maxProf, string &bestItems )
  {
    // check the total profit
    if( kn.getWeight() <= maxWt  &&  kn.getProfit() > maxProf )
    {
      maxProf = kn.getProfit() ;
      currWt = kn.getWeight() ;

      // keep track of overall list of best items
      bestItems.assign( kn.getItems() );

      return true ;
    }

    return false ;

  }// KnapVector::checkProfit()


  /* calculate the maximum additional profit of adding this item to the knapsack  */
  float KnapVector::calculateBound( KnapItem &ki, const int maxWt, int dbg )
  {
    int vi , // to advance the iterator
        totweight ; // total weight of items in the calculated bound

    float result ;

    iterator it ;

    if( dbg > DEBUG_LOW )
    {
      if( dbg > DEBUG_HIGH )
      {
        printDest << "\n\t" << __FUNCTION__ << "/" << __LINE__ << "/" << ki.getName() << ": pw vector = " ;
        display();
      }
      printDest << "\n\t" << __FUNCTION__ << "/" << __LINE__ << "/given node = " ;
      ki.display( printDest, ki.getItems().size() );
    }

    if( ki.getWeight() >= maxWt ) // this item is TOO heavy
    {
      result = 0.0 ;
      if( dbg > DEBUG_OFF )
        printDest << "\n\t" << __FUNCTION__ << "/" << __LINE__ << "/weight of '" << ki.getItems() << "' > maxWt" << endl;
    }
    else
    {
      result = static_cast<float>( ki.getProfit() );
      totweight = ki.getWeight() ;

      // get to the proper index of the vector
      for( vi=0, it=begin(); vi < ki.getIndex()+1; vi++, it++ );

      if( dbg > DEBUG_LOW )
      {
        if( dbg > DEBUG_MID )
        {
          printDest << __FUNCTION__ << "/" << __LINE__ << "/vi = " << vi << " ; current vector node = " ;
          if( it != end() )
            it->display( printDest, it->getItems().size() );
          else
              printDest << "*** END ***\n" ;
          printDest << endl;
        }
        printDest << "\n\t\t >> START BOUND() WHILE LOOP...\n" << endl;
      }

      // grab as many items as possible
      while( it != end()  &&  (totweight + it->getWeight()) <= maxWt )
      {
        if( dbg > DEBUG_LOW )
        {
          printDest << "\t\t" << __FUNCTION__ << "/" << __LINE__ << ": current item = " ;
          it->display( printDest, it->getItems().size() );
        }
        totweight += it->getWeight();
        result += static_cast<float>( it->getProfit() );

        it++ ;
        vi++ ; // just for debugging -- see how many times through this loop
        if( dbg > DEBUG_LOW )
          printDest << "\t\t" << __FUNCTION__ << "/" << __LINE__
                    << ": new bound = " << result << " / totwt = " << totweight << "\n" << endl;
      }// while

      if( dbg > DEBUG_LOW )
      {
        printDest << "\n\t\t >> END BOUND() WHILE LOOP...\n" << endl;
        if( dbg > DEBUG_MID )
          printDest << "\t" << __FUNCTION__ << "/" << __LINE__ << ": now, vi = " << vi << endl;
      }

      // grab fraction of next item to fill any remaining weight
      if( (maxWt - totweight) > 0  &&  it != end() )
        result += ( (static_cast<float>(maxWt - totweight)) * it->getPwr() );

      if( dbg > DEBUG_OFF )
        printDest << "\n\t" << __FUNCTION__ << "/" << __LINE__ << "/"
                  << ki.getName() << ": '" << ( ki.getItems().empty() ? "<EMPTY>" : ki.getItems() )
                  << "' bound = " << result << "; totwt = " << totweight << endl ;

    }// else

    ki.setBound( result );

    return result ;

  }// KnapVector::calculateBound()


  /* the actual work of calculating the optimal set of items  */
  int KnapVector::bestFirstSearch( const int maxWt, int &currWt, string &itemList )
  {
    int    i =    1 , // keep track of # of times through the while loop
       limit = 1024 ; // for debugging

    int maxProfit = 0 ;
    float topBound ;

    // set loop protection
    printDest << "\nLOOP_LIMIT = " << limit << endl;

    // start at -1 so the root node will get the proper initial bound
    KnapNode checkNode( ROOT, ROOT_MARKER, -1 );

    // a priority_queue of KnapItems ordered by highest bound
    KnapSST sst ; // state space tree

    int j ;
    iterator it ;

    // get the initial bound
    calculateBound( checkNode, maxWt );

    if( debugLevel > DEBUG_OFF )
    {
      printDest << "\n" << __FUNCTION__ << "/" << __LINE__ << "/" ;
      checkNode.display( printDest, checkNode.getItems().size() );
    }

    // put the root node on the priority queue
    sst.push( checkNode );

    if( debugLevel > DEBUG_OFF )
    {
      printDest << "\n" << __FUNCTION__ << "/" << __LINE__ ;
      sst.display( printDest, "sst" );
      printDest << "\n>>> START BFS() WHILE LOOP..." << endl;
    }

    /* LOOP */
    while( !sst.empty()  &&  i <= limit ) // limit prevents a runaway loop
    {
      if( debugLevel > DEBUG_OFF )
      {
        printDest << "\n" << __FUNCTION__ << "/" << __LINE__ << "/i = " << i << endl;
        if( debugLevel > DEBUG_LOW )
        {
          printDest << "\n" << __FUNCTION__ << "/" << __LINE__ ;
          sst.display( printDest, "sst" );
        }
      }

      checkNode = sst.get(); // remove node with best bound
      topBound = checkNode.getBound();
      checkNode.setName( "top" );

      if( debugLevel > DEBUG_OFF )
      {
          printDest << "\n" << __FUNCTION__ << "/" << __LINE__ << "/" ;
          checkNode.display( printDest, checkNode.getItems().size() );
      }

      if( topBound > static_cast<float>(maxProfit) ) // check if node is promising
      {
        if( debugLevel > DEBUG_MID )
          printDest << "\n" << __FUNCTION__ << "/" << __LINE__ << "/top node bound = " << topBound << endl;

        // increment the index
        checkNode.incIndex( 1 );

        /* check the child node that DOES NOT INCLUDE the next item */

        // set the name
        checkNode.setName( WITHOUT_CHILD );

        // see if this node is promising
        checkPromising( sst, checkNode, maxWt, maxProfit, WITHOUT_CHILD_MARKER );

        /* check the child node that INCLUDES the next item */

        // move iterator to the proper index
        for( j=0, it=begin(); it != end()  &&  j < checkNode.getIndex(); j++, it++ );

        // add the child info
        checkNode.setName( WITH_CHILD );
        checkNode.addPw( it->getProfit(), it->getWeight() );
        checkNode.addItem( WITH_CHILD_MARKER + it->getItems() );

        if( debugLevel > DEBUG_OFF )
        {
          printDest << "\n" << __FUNCTION__ << "/" << __LINE__ << "/" ;
          checkNode.display( printDest, checkNode.getItems().size() );
        }

        // see if we have a better total profit with this node
        if( checkProfit(checkNode, maxWt, currWt, maxProfit, itemList) )
        {
          printDest << "\n" << __FUNCTION__ << "/" << __LINE__ << "/ i = " << i
                    << "\n*** maxprofit now = " << maxProfit
                    << "\n*** current best items are " << itemList
                    << "\n*** current weight of items is " << currWt << endl;
        }

        // see if this node is promising
        checkPromising( sst, checkNode, maxWt, maxProfit );

      }// if top bound > max profit
      else
      {
        if( debugLevel > DEBUG_OFF )
          printDest << __FUNCTION__ << "/" << __LINE__ << "/top bound (" << topBound << ") <= max profit" << endl;

        printDest << "\n" << __FUNCTION__ << "/" << __LINE__ << "\n\n*** NO MORE PROMISING NODES! ***" << endl;
        break ;
      }

      i++ ;

    }// while( !sst.empty()  &&  i <= limit )

    printDest << "\n" << __FUNCTION__ << "/" << __LINE__ << endl << endl << ">>> END BFS() WHILE LOOP..." << endl;

    return maxProfit ;

  }// KnapVector::bestFirstSearch()

}/* namespace mhs_codeblocks_knapsack  */

/*
 *     M A I N
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

using namespace mhs_codeblocks_knapsack ;

/* assemble and sort the given items then launch KnapVector::bestFirstSearch() to find the optimal set  */
int main( int argc, char *argv[] )
{
  int debug = DEBUG_OFF ;

  int numItems = 0 , // total items
         maxWt     , // maximum allowed weight of items (user-supplied)
        currWt = 0 ; // current weight of selected items

  string bestitems ; // keep track of the optimal set

  // the p/w vector to store the given items
  KnapVector pwVec( cout, cerr );

  // check command line parameters
  if( ! setup(argc, argv, maxWt, debug, pwVec) )
    exit( __LINE__ );

  sleep( 5 ); // pause to review setup printout

  // get the file data
  numItems = getFileData( argv[1], pwVec, debug );
  if( numItems <= 1 )
  {
    cout << "\nTOO FEW ITEMS... PROGRAM ENDED." << endl;
    exit( __LINE__ );
  }

  // sort the vector - descending order
  sort( pwVec.begin(), pwVec.end(), knap_order_pwr() );

  // display the sorted vector
  cout << "\nThe sorted p/w vector:" ;
  pwVec.display();

  /* run the algorithm */
  int maxProfit = pwVec.bestFirstSearch( maxWt, currWt, bestitems );

  // and display the results
  cout << "\nFor Weight limit " << maxWt << ": Max Profit = " << maxProfit
       << " (weight of items = " << currWt << ")\n"
       << "Best items are: " << (bestitems.empty() ? "<NONE>" : bestitems) << endl;

  cout << "\n PROGRAM ENDED." << endl;

  return 0 ;

}// main()
