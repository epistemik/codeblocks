/*
 * SimUnit.h
 *   Ported from SimUnit.pas
 *     - a library of Pascal functions & procedures originally created in Nov 1999 for CSI2111
 *   Author: Mark Sattolo <epistemik@gmail.com>
 * ------------------------------------------------------------------------------------------------------
 *   $File: //depot/Eclipse/CPP/Workspace/Sim68k/src/SimUnit.h $
 *   $Revision: #18 $
 *   $Change: 114 $
 *   $DateTime: 2011/02/21 15:14:53 $
 *
 *   UPDATED Nov 12, 2018
 */

#ifndef MHS_CODEBLOCKS_CPROJ_SIM68K_SIMUNIT_H
#define MHS_CODEBLOCKS_CPROJ_SIM68K_SIMUNIT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*
 *  DATA DEFINITIONS
 *=========================================================================================================*/

#define memorySize 4097 // hex values 0x0000 to 0x1000

#define EOL '\n'
#define COMMENT_MARKER '/' 
#define HEX_MARKER '$'
#define QUIT 'q'
#define EXECUTE 'e'

// List of OpCode | OpId 
#define iADD    0  // Regular binary integer addition
#define iADDQ   1  // Quick binary integer addition
#define iSUB    2  // Regular binary integer subtraction
#define iSUBQ   3  // Quick binary integer subtraction
#define iMULS   4  // Signed binary multiplication
#define iDIVS   5  // Signed binary division
#define iNEG    6  // Signed binary negation
#define iCLR    7  // Clear (set to 0)
#define iNOT    8  // Logical NOT
#define iAND    9  // Logical AND
#define iOR    10  // Logical OR
#define iEOR   11  // Logical EXCLUSIVE-OR
#define iLSL   12  // Logical Shift Left
#define iLSR   13  // Logical Shift Right
#define iROL   14  // Rotate Left
#define iROR   15  // Rotate Right
#define iCMP   16  // Compare (to adjust CVNZH according to D-S)
#define iTST   17  // Test    (to adjust CVNZH according to D)
#define iBRA   18  // Unconditional branch to the given address
#define iBVS   19  // Branch to the given address if overflow
#define iBEQ   20  // Branch to the given address if equal
#define iBCS   21  // Branch to the given address if carry
#define iBGE   22  // Branch to the given address if greater | equal
#define iBLE   23  // Branch to the given address if less | equal
#define iMOV   24  // Regular move
#define iMOVQ  25  // Quick move
#define iEXG   26  // Exchange 2 registers
#define iMOVA  27  // Move the given address into A[i]
#define iINP   28  // Read from keyboard (input)
#define iDSP   29  // Display the name, the source & the contents
#define iDSR   30  // Display the contents of the Status bits
#define iHLT   31  // HALT

// 0x0..0xFF = 0x80..0x7F in 2's CF
/* SHOULD be a char, but doesn't function properly for data input or as array index if a char ! */
typedef  int  byte ;

// 0x0..0xFFFF = 0x8000..0x7FFF in 2's CF
/* SHOULD be a short, but doesn't function properly for data input or as array index if a short ! */
typedef  int  word ;

// long = 0x0..0xFFFFFFFF = 0x80000000..0x7FFFFFFF in 2's CF
// >> long is equivalent to C int on this platform (Linux x86_64) -- actual C long is 8 bytes

typedef  char   boolean ;
typedef  char*  string ;

enum    bit { False, True }; // True = 1, False = 0
enum  rwbit { Write, Read }; // Read/Write Bit: Read = 1, Write = 0
enum sigbit { Least, Most }; // Least = 0 = Least Significant, Most = 1 = Most Significant

enum    dataSize { byteSize, wordSize, intSize   }; // intSize was originally 'longSize'
enum numOperands { noZero, noOne, noTwo, noThree }; // Number of necessary operands = opCode bit P + 1

enum addressmode { DATA_REGISTER_DIRECT,
                   ADDRESS_REGISTER_DIRECT,
                   AM_TWO_UNUSED,
                   RELATIVE_ABSOLUTE,
                   ADDRESS_REGISTER_INDIRECT,
                   AM_FIVE_UNUSED,
                   ADDRESS_REGISTER_INDIRECT_POSTINC,
                   ADDRESS_REGISTER_INDIRECT_PREDEC };

/*
 *  VARIABLES
 *=========================================================================================================*/

int nDebugLevel = 0 ;

enum rwbit RW ;
enum dataSize DS ; 
enum numOperands numOprd ;

// status bits
enum bit C ; // Carry
enum bit V ; // Overflow
enum bit Z ; // Zero
enum bit N ; // Negative
enum bit H ; // Halt

string  bitName[] = { "FALSE", "TRUE" };
string sizeName[] = { "byte", "word", "long" }; // use 'long' as name to accord with original Pascal result printouts

string Mnemo[ iHLT + 1 ]; // Mnemonic string for opCodes

byte OpId ; // numeric id for opCodes
word PC ;   // Program Counter

/*
 *   FUNCTIONS
 *=========================================================================================================*/

// Generic error verification function, with message display.
// if Cond is False, display an error message (including the OpName)
// the Halt Status bit will also be set if there is an Error.
boolean CheckCond( boolean Cond, string Message )
{
  if( Cond == True )
    return True ;
  
  printf( "*** ERROR >> %s at PC = %d for operation %s\n", Message, (PC-2), Mnemo[OpId] );
  H = True ; // program will halt
  
  return False ;
}

// return the "number of bytes" 
byte NOB( enum dataSize Size )
{
  return( (Size == intSize) ? 4 : (Size + 1) );
}

// Determines the format of the instruction: return True if F1, False if F2
boolean FormatF1( byte opid )
{
  if( (opid == iADDQ) || (opid == iSUBQ) || ((opid >= iLSL) && (opid <= iROR)) || (opid == iMOVQ) )
    return False ;
  
  return True ;
}

// Initializes Mnemo with strings corresponding to each instruction
// - to have a more useful display of each opCode
void MnemoInit()
{
  Mnemo[iADD]   = "ADD";
  Mnemo[iADDQ]  = "ADDQ";
  Mnemo[iSUB]   = "SUB";
  Mnemo[iSUBQ]  = "SUBQ";
  Mnemo[iMULS]  = "MULS";
  Mnemo[iDIVS]  = "DIVS";
  Mnemo[iNEG]   = "NEG";
  Mnemo[iCLR]   = "CLR";
  Mnemo[iNOT]   = "NOT";
  Mnemo[iAND]   = "AND";
  Mnemo[iOR]    = "OR";
  Mnemo[iEOR]   = "EOR";
  Mnemo[iLSL]   = "LSL";
  Mnemo[iLSR]   = "LSR";
  Mnemo[iROL]   = "ROL";
  Mnemo[iROR]   = "ROR";
  Mnemo[iCMP]   = "CMP";
  Mnemo[iTST]   = "TST";
  Mnemo[iBRA]   = "BRA";
  Mnemo[iBVS]   = "BVS";
  Mnemo[iBEQ]   = "BEQ";
  Mnemo[iBCS]   = "BCS";
  Mnemo[iBGE]   = "BGE";
  Mnemo[iBLE]   = "BLE";
  Mnemo[iMOV]   = "MOVE";
  Mnemo[iMOVQ]  = "MOVEQ";
  Mnemo[iEXG]   = "EXG";
  Mnemo[iMOVA]  = "MOVEA";
  Mnemo[iINP]   = "INP";
  Mnemo[iDSP]   = "DSP";
  Mnemo[iDSR]   = "DSR";
  Mnemo[iHLT]   = "HLT";
  
}// MnemoInit()

/* **************************************************************************

 Functions for bit manipulation.

 Pascal, unlike C, does not have operators that allow easy manipulation
 of bits within a byte or word. The following procedures and functions
 are designed to help you manipulate (extract and set) the bits.
 You may use or modify these procedures/functions or create others.

*************************************************************************** */

// Returns a substring of bits between FirstBit and LastBit from V
// Ex:
//    Bit Positions:      1111 11
//                        5432 1098 7654 3210  // 0 to 15
//    V = 0x1234   i.e. %(0001 0010 0011 0100)
//    FirstBit = 3, LastBit = 9         
//    The bits from 3 to 9 are %10 0011 0
//    The function returns 0x0046  %(0000 0000 0100 0110)
word GetBits( word V, byte FirstBit, byte LastBit )
{
  return( (V >> FirstBit) & ((2<<(LastBit-FirstBit)) - 1) );
}

// Gets one word from V
// MSW: false = Least Significant Word, true = Most Significant Word
word GetWord( int V, enum sigbit MSW )
{
  word TmpW ;
  
  if( MSW == Most )
    TmpW = ( (V & 0xFFFF0000) >> 16 );
  else
    if( MSW == Least )
      TmpW = ( V & 0x0000FFFF );
    else
      {
        printf( "*** ERROR >> GetWord() received invalid MSW bit '%d' \n", MSW );
        H = True ;
        return 0 ;
      }
  
  return TmpW ;
  
}// GetWord()

// Sets the bit of V indicated by Position to Value (False or True)
void SetBit( int* V, byte Position, enum bit Value )
{
  *V = ( (*V & (0xFFFFFFFF - (1 << Position))) | (Value << Position) );
}

// Sets the bits of V between First and Last to the least significant bits of Value
void SetBits( int* V, byte First, byte Last, int Value )
{
  int pos ;
  for( pos=First; pos <= Last; pos++ )
    SetBit( V, pos, (GetBits(Value, pos-First, pos-First) == 1) );
}

// Sets one byte of V indicated by position to Value
void SetByte( int* V, int position, byte Value )
{
  switch( position )
  {
    case 0: *V = (*V & 0xFFFFFF00) | Value; break ;
    case 1: *V = (*V & 0xFFFF00FF) | (Value << 8); break ;
    case 2: *V = (*V & 0xFF00FFFF) | (Value << 16); break ;
    case 3: *V = (*V & 0x00FFFFFF) | (Value << 24); break ;
    
    default: printf( "*** ERROR >> SetByte() received invalid position '%d' \n", position );
             H = True ;
             return ;
  }
}// SetByte()

// Sets one word of V indicated by MSW to Value
void SetWord( int* V, enum sigbit MSW, word Value )
{
  if( MSW == Most )
    *V = ( (*V & 0x0000FFFF) | (Value << 16) );
  else
    if( MSW == Least )
      *V = ( (*V & 0xFFFF0000) | Value );
    else
      {
        printf( "*** ERROR >> SetWord() received invalid MSW bit '%d' \n", MSW );
        H = True ;
        return ;
      }
}// SetWord()

#endif // MHS_CODEBLOCKS_CPROJ_SIM68K_SIMUNIT_H
