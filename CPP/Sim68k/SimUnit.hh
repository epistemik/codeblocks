/*
 * SimUnit.hh
 *   Ported from SimUnit.pas
 *     - a library of Pascal functions & procedures originally created in Nov 1999 for CSI2111
 *   Author: Mark Sattolo <epistemik@gmail.com>
 * ------------------------------------------------------------------------------------------------------
 *   $File: //depot/Eclipse/CPP/Workspace/Sim68k/src/SimUnit.h $
 *   $Revision: #18 $
 *   $Change: 114 $
 *   $DateTime: 2011/02/21 15:14:53 $   
 */

#ifndef MHS_CODEBLOCKS_CPP_SIM68K_SIMUNIT_HH
#define MHS_CODEBLOCKS_CPP_SIM68K_SIMUNIT_HH

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <iomanip>

using namespace std ;

namespace mhs_cpp_sim68k
{
  /*
   *  DATA DEFINITIONS
   *=========================================================================================================*/

  const char EOL = '\n' ;
  const char COMMENT_MARKER = '/';
  const char HEX_MARKER = '$';
  const char QUIT = 'q';
  const char EXECUTE = 'e';

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

  #define READ   true
  #define WRITE  false
  #define LEAST  false
  #define MOST   true

  #define trD  0  // Tmp Register D
  #define trR  1  // Tmp Register R
  #define trS  2  // Tmp Register S

  typedef  bool  bit ;
  string  bitValue[] = { "FALSE", "TRUE" };

  enum twobits  { byte0, byte1, byte2, byte3 };

  // 0x0..0xFF = 0x80..0x7F in 2's CF
  /* NEEDS to be an unsigned char to work properly */
  typedef  unsigned char  byte ;

  // 0x0..0xFFFF = 0x8000..0x7FFF in 2's CF
  /* NEEDS to be an unsigned short to work properly */
  typedef  unsigned short  word ;

  // long = 0x0..0xFFFFFFFF = 0x80000000..0x7FFFFFFF in 2's CF
  // >> 68k long is equivalent to C int on this platform (Linux x86_64) -- actual C long is 8 bytes

  enum dataSize    { byteSize, wordSize, longSize }; // 68k long = 4 bytes
  string sizeName[] = { "byte", "word", "long" }; // use 'long' as name to accord with original Pascal result printouts

  enum addressmode { DATA_REGISTER_DIRECT,
                     ADDRESS_REGISTER_DIRECT,
                     AM_TWO_UNUSED,
                     RELATIVE_ABSOLUTE,
                     ADDRESS_REGISTER_INDIRECT,
                     AM_FIVE_UNUSED,
                     ADDRESS_REGISTER_INDIRECT_POSTINC,
                     ADDRESS_REGISTER_INDIRECT_PREDEC
                   };

  const word memorySize = 4097 ; // hex values 0x0000 to 0x1000

  long nDebugLevel = 1 ;

  /***************************************************************************

   Functions for bit manipulation.
  
   Pascal, unlike C does not have operators that allow easy manipulation
   of bits within a byte or word. The following procedures and functions
   are designed to help you manipulate (extract and set) the bits.
   You may use or modify these procedures/functions or create others.
  
  ****************************************************************************/
  
  // Returns a substring of bits between FirstBit and LastBit from V
  // Ex:                    1111 11
  //    Bit Positions:     5432 1098 7654 3210
  //    V = 0x1234     (or %0001 0010 0011 0100)
  //    FirstBit = 3, LastBit = 9
  //    The bits from 3 to 9 are %10 0011 0
  //    The function returns 0x0046  (%0000 0000 0100 0110)
  word getBits( const word V, const byte FirstBit, const byte LastBit )
  {
    return( (V >> FirstBit) & ((2<<(LastBit-FirstBit)) - 1) );
  }
  
  // Gets one word from V
  // MSW: false = Least Significant Word, true = Most Significant Word
  word getWord( const int V, bit MSW )
  {
    if( MSW )
      return( (V & 0xFFFF0000) >> 16 );
    
    return( V & 0x0000FFFF );
  }

  // Sets the bit of V indicated by posn to val (false or true)
  void setBit( int* V, const byte posn, const bit val )
  {
    *V = ( (*V & (0xFFFFFFFF - (1 << posn))) | (val << posn) );
  }

  // Sets the bits of V between first and fast to the least significant bits of Value
  void setBits( int* V, const byte first, const byte last, const int val )
  {
    int pos ;
    for( pos=first; pos <= last; pos++ )
      setBit( V, pos, (getBits(val, pos-first, pos-first) == 1) );
  }

  // Sets one byte of V indicated by posn to val
  void setByte( int* V, const twobits posn, const byte val )
  {
    switch( posn )
    {
      case byte0: *V = (*V & 0xFFFFFF00) | val; break ;
      case byte1: *V = (*V & 0xFFFF00FF) | (val << 8); break ;
      case byte2: *V = (*V & 0xFF00FFFF) | (val << 16); break ;
      case byte3: *V = (*V & 0x00FFFFFF) | (val << 24); break ;
    }
  }

  // Sets one word of V indicated by MSW to Value
  void setWord( int* V, const bit MSW, const word val )
  {
    if( MSW )
      *V = ( (*V & 0x0000FFFF) | (val << 16) );
    else
        *V = ( (*V & 0xFFFF0000) | val );
  }

  /*
   *  CLASSES
   *=========================================================================================================*/

  class Processor
  {
    public:
      Processor()
        : mem(this),
          ctrl(this, mem)
      {
        mnemoInit();
      }

      /*
       *   FUNCTIONS
       *=========================================================================================================*/

      // Initializes Mnemo with strings corresponding to each instruction
      // - to have a more useful display of each opCode
      void mnemoInit();

      // Read into memory a machine language program contained in a file
      bool loadProgram(string);

      // Determines the format of the instruction: return True if F1, False if F2
      bool formatF1(byte);

      // Fetch-Execute Cycle simulated
      void control();

      int  getTmpReg(int);
      void setTmpReg(int, int);
      word getMar();
      void setMar(word);
      word getMdr();
      void setMdr(word);

  // store information
  class Memory {
    public:
      // Constructor
      Memory(Processor* proc): p(proc)
      {
        memory = new byte[memorySize];
      }

      // Copies an element (Byte, Word, Long) from memory\CPU to CPU\memory.
      // Verifies if we are trying to access an address outside the range allowed for addressing [0x0000..0x1000].
      // Uses the RW (read|write) bit.
      // Parameter dsz determines the data size (byte, word, int/long).
      void access(dataSize);

      void load(byte, byte);

    private:
      byte* memory; // store the binary program
      Processor* p;
  };

  // fetch and execute
  class Controller {
    public:
      // Constructors
      Controller(Processor* proc, Memory mry)
        : p(proc), mem(mry), OpId(0), numOprd(0), dSize(byteSize), opcData(0), R1(0), R2(0),
          M1(DATA_REGISTER_DIRECT), M2(DATA_REGISTER_DIRECT), Sm(false), Dm(false), Rm(false)
      { }

      // Generic error verification function, with message display,
      // if Cond is False, display an error message (including the OpName)
      // The Halt Status bit will also be set if there is an Error.
      bool checkCond( bool, string );

      // return the "number of bytes"
      byte numBytes(dataSize);

      // Fetch the OpCode from memory
      void fetchOpCode();

      // Update the fields OpId, DS, numOprd, M1, R1, M2, R2 and Data according to given format.
      // Uses getBits()
      void decodeInstr();

      // Fetch the operands, according to their number (numOprd) & addressing modes (M1 or M2)
      void fetchOperands();

      // Status bits Z & N are often set the same way in many instructions
      // A function would be useful to do this
      void setZN( int tmpReg );

      // The calculations to find V & C are more complex but are simplified by the use of Sm, Dm, Rm
      // It would be a good Idea to make a procedure to find these values
      void setSmDmRm( int tmpSrc, int tmpDst, int tmpRes );

      // Transfer data in the required temporary register
      void fillTmpReg( int*        reg,      // tmp Register to modify - TMPS, TMPD or TMPR
                       word        opAddrNo, // address of Operand (OpAddr1 | OpAddr2), for addressMode 3
                       dataSize    dsz,      // Data Size
                       addressmode mode,     // required Addressing Mode
                       byte        regNo    ); // Register number for A[n] or D[n]

      // Transfer the contents of temporary register to Register OR Memory
      void setResult( int         tmpReg,         // Source Register (TMPD...)
                      word        OpAddrNo,       // Operand Address (OpAddr1...)
                      dataSize    dsz,        // Data Size
                      addressmode mode,    // required Addressing Mode
                      byte        RegNo        ); // Register Number for A[n] or D[n]

      /********************************************************************
        The execution of each instruction is done via its micro-program
      *********************************************************************/
      void execInstr();

      word getOpCode();
      void setOpCode(word w);

    private:
      Processor* p;
      Memory mem;

      byte OpId ; // numeric id for opCodes
      byte numOprd ;// Number of necessary operands = opCode bit P + 1
      dataSize dSize ;
      // store data from opCode for Format F2
      byte opcData ;
      // temp storage for Register # (from opCode) for operands 1 & 2
      byte R1, R2 ;
      // temp storage for address Mode (from opCode) for operands 1 & 2
      addressmode M1, M2 ;
      // Most Significant Bits of TMPS, TMPD, & TMPR
      bool Sm, Dm, Rm ;
  };

    private:
      Memory mem;
      Controller ctrl;
      friend class Memory;
      friend class Controller;
  };

} // namespace mhs_cpp_sim68k

#endif // MHS_CODEBLOCKS_CPP_SIM68K_SIMUNIT_HH
