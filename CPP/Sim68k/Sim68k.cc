/*
 * Sim68k.cc
 *   Ported from Sim68k.pas - a Pascal program originally created in Nov 1999 for CSI2111
 *                          - simulates the actions of the Motorola 68000 microprocessor
 *   Author: Mark Sattolo <epistemik@gmail.com>
 * ------------------------------------------------------------------------------------------------------
 *   $File: //depot/Eclipse/CPP/Workspace/Sim68k/src/Sim68k.c $
 *   $Revision: #23 $
 *   $Change: 114 $
 *   $DateTime: 2011/02/21 15:14:53 $   
 */

#include "SimUnit.hh" // Library containing useful functions
#include <fstream>
#include <sstream>
#include <unistd.h>  // for sleep()

namespace mhs_cpp_sim68k
{
  /*
   *  VARIABLES
   *=========================================================================================================*/

  string Mnemo[ iHLT + 1 ]; // Mnemonic string for opCodes

  // The CPU registers
  word PC ;               // Program Counter
  int TMPD, TMPR, TMPS ;  // Temporary Registers D, R, S
  word OpCode ;           // OPCODE of the current instruction
  word OpAddr1, OpAddr2 ; // Operand Addresses

  // status bits
  bit C ; // Carry
  bit V ; // Overflow
  bit Z ; // Zero
  bit N ; // Negative
  bit H ; // Halt

  int  DR[2] ; // Data Registers
  word AR[2] ; // Address Registers

  word MAR ;  // Memory Address Register
  int  MDR ;  // Memory Data Register
  bit  RW ;   // read/write
  dataSize DS ;

  /*
   *   FUNCTIONS
   *=========================================================================================================*/

  // Read into memory a machine language program contained in a file
  bool Processor::loadProgram( string filename )
  {
    bool inComment = false ;
    bool inHex = false;
    int numHex = 0;

    word address = 0 ;
    char ch ;
    string hex1, hex2 ;
    long hexVal = 0 ;

    string msg ; // error handling

    try
    {
      ifstream inputfile( filename.c_str() );
      if( !inputfile )
      {
        msg = string( "\nError occurred opening file: " ) + string( filename );
        throw msg ;
      }
      else
          cout << "Processing file '" << filename << "'" << endl;

      // get the data from the file
      while( !inputfile.eof() )
      {
        inputfile >> ch ; // get a character
        if( nDebugLevel > 2 )
          cout << "Read character: " << ch << endl;

        // beginning & end of comment sections
        if( ch == COMMENT_MARKER )
        {
          inComment = ! inComment ;
          continue ;
        }

        // skip comment
        if( inComment )
          continue ;

        // skip EOL char; also, comment sections end at EOL
        if( ch == EOL )
        {
          inComment = false ;
          continue ;
        }

        // process hex input
        if( inHex )
        {
          numHex++ ;
          if( nDebugLevel > 1 )
            cout << "In Hex processing #" << numHex << endl;
          if( numHex == 2 ) {
            hex2 = string(1, ch) ;
            hexVal = strtol( (hex1 + hex2).c_str(), (char**)NULL, 16 );
            if( nDebugLevel > 0 ) {
              cout.setf(ios_base::hex, ios_base::basefield);
              cout << "Processor::loadProgram(): Read value '" << hexVal ;
              cout.setf(ios_base::dec, ios_base::basefield);
              cout << "' into memory at location: " << address << endl;
            }
            mem.load(address, (byte)hexVal);
            address++ ;
            inHex = false ;
            numHex = 0 ;
          }
          else {
            hex1 = string(1, ch) ;
          }
        }

        // hex input next
        if( ch == HEX_MARKER )
        {
          inHex = true ;
        }

      }// while( !inputfile.eof() )

      inputfile.close();
    }
    catch( string& msg )
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
    }

    cout << "Program loaded '" << address << "' bytes in memory." << endl << endl;

    return true ;
  }

  void Processor::Memory::load( byte location, byte data )
  {
    if( nDebugLevel > 2 ) {
      cout.setf(ios_base::hex, ios_base::basefield);
      cout << data << endl;
      cout << location << endl;
      cout.setf(ios_base::dec, ios_base::basefield);
      cout << "Memory::load(): Read value '" << data << "' into memory at location: " << location << endl;
    }
    memory[location] = data ;
  }

  // Copies an element (Byte, Word, Long) from memory\CPU to CPU\memory.
  // Verifies if we are trying to access an address outside the range allowed for addressing [0x0000..0x1000].
  // Uses the RW (read|write) bit.
  // Parameter dsz determines the data size (byte, word, int/long).
  void Processor::Memory::access( dataSize dsz )
  {
    cout.setf(ios_base::hex, ios_base::basefield);

//      MAR = 0x1001 ; // debug
    if( (MAR >= 0) && (MAR < memorySize) ) // Valid Memory Address
    {
      if( RW ) // Read = copy an element from memory to CPU
      {
        switch( dsz )
        {
          case byteSize: MDR = memory[MAR] ; break;

          case wordSize: MDR = memory[MAR] * 0x100 + memory[MAR+1] ; break;

          case longSize: MDR = ( (memory[MAR]   * 0x1000000) & 0xFF000000 ) |
                               ( (memory[MAR+1] * 0x10000)   & 0x00FF0000 ) |
                               ( (memory[MAR+2] * 0x100)     & 0x0000FF00 ) |
                               (  memory[MAR+3]              & 0x000000FF ); break;

          default: cout << "*** ERROR >> Memory::access() received invalid data size '" << sizeName[dsz]
                        << "' at PC = " << PC-2 << endl;
          H = true ;
        }
        if( nDebugLevel > 0 )
          cout << "Memory::access(" << sizeName[dsz] << ") READ: MDR now has value: " << MDR << endl;
      }
      else
  //      if( RW == Write ) // Write = copy an element from the CPU to memory
        {
          switch( dsz )
          {
            case byteSize:
                     memory[MAR] = MDR % 0x100 ; // LSB: 8 last bits
                     if( nDebugLevel > 0 )
                       cout << "Memory::access(" << sizeName[dsz] << ") WRITE: memory[" << MAR
                            << "] now has value = " << (int)memory[MAR] << endl;
                     break;
            case wordSize:
                     memory[MAR] = (MDR / 0x100) % 0x100 ; // MSB: 8 first bits
                     memory[MAR+1] = MDR % 0x100 ; // LSB: 8 last bits
                     if( nDebugLevel > 0 )
                       cout << "Memory::access(" << sizeName[dsz] << ") WRITE: memory["
                            << MAR << "] now has value = " << (int)memory[MAR]
                            << " memory[" << MAR+1 << "] now has value = " << (int)memory[MAR+1] << endl;
                     break;
            case longSize:
                     memory[MAR]   = (MDR >> 24) & 0x000000FF ; // MSB: 8 first bits
                     memory[MAR+1] = (MDR >> 16) & 0x000000FF ;
                     memory[MAR+2] = (MDR >> 8 ) & 0x000000FF ;
                     memory[MAR+3] =  MDR % 0x100 ; break;
                     if( nDebugLevel > 0 )
                       cout << "Memory::access(" << sizeName[dsz] << ") WRITE: memory["
                            << MAR << "] now has value = " << (int)memory[MAR]
                            << " memory[" << MAR+1 << "] now has value = " << (int)memory[MAR+1]
                            << " memory[" << MAR+2 << "] now has value = " << (int)memory[MAR+2]
                            << " memory[" << MAR+2 << "] now has value = " << (int)memory[MAR+3] << endl;
                     break;
            default:
                   cout << "*** ERROR >> Memory::access() received invalid data size '" << dsz
                        << "' at PC = " << PC-2 << endl;
                   H = true ;
          }
       }
    }
    else // Invalid Memory Address
      {
        cout << "*** ERROR >> Memory::access() uses the invalid address " << MAR << " at PC = " << PC-2 << endl;
        H = true ; // End of simulation...!
      }
  }

  /****************************************************************************
    Since many instructions will make local fetches between temporary registers
    (TMPS, TMPD, TMPR) & memory or the Dn & An registers it would be
    useful to create procedures to transfer the words/bytes between them.
    Here are 2 suggestions of procedures to do this.
  *****************************************************************************/

  // Transfer data to the specified temporary register
  void Processor::Controller::fillTmpReg(
                   int*        tmpReg,   // tmp Register to modify - TMPS, TMPD or TMPR
                   word        opAddrNo, // address of Operand (OpAddr1 | OpAddr2), for addressMode 3
                   dataSize    dsz,      // Data Size
                   addressmode mode,     // required Addressing Mode
                   byte        regNo   ) // Register number for A[n] or D[n]
  {
    RW = true ;

    switch( mode )
    {
      case DATA_REGISTER_DIRECT:
             *tmpReg = DR[regNo];
             if( dsz == byteSize )
               setByte( tmpReg, byte1, 0 );
             if( dsz <= wordSize )
               setWord( tmpReg, MOST, 0 );
             break;

      case ADDRESS_REGISTER_DIRECT:
             *tmpReg = AR[regNo];
             break;

      case RELATIVE_ABSOLUTE:
             // We need to access memory, except for branching & MOVA.
             MAR = opAddrNo;
             mem.access( dsz );
             *tmpReg = MDR ;
             break;

      case ADDRESS_REGISTER_INDIRECT:
             // We need to access memory.
             MAR = AR[regNo];
             mem.access( dsz );
             *tmpReg = MDR ;
             break;

      case ADDRESS_REGISTER_INDIRECT_POSTINC:
             // We need to access memory.
             MAR = AR[regNo];
             mem.access( dsz );
             *tmpReg = MDR ;
             AR[regNo] = AR[regNo] + numBytes(dsz);
             break;

      case ADDRESS_REGISTER_INDIRECT_PREDEC:
             // We need to access memory.
             AR[regNo] = AR[regNo] - numBytes(dsz);
             MAR = AR[regNo];
             mem.access( dsz );
             *tmpReg = MDR ;
             break;

      default: // This error should never occur, but just in case...!
               cout << "*** ERROR >> Controller::fillTmpReg() has Invalid Addressing Mode '" << mode
                    << "' at PC = " << PC-2 << endl;
               H = true ;
    }// switch mode
  }

  // Generic error verification function, with message display,
  // if Cond is False, display an error message (including the OpName)
  // The Halt Status bit will also be set if there is an Error.
  bool Processor::Controller::checkCond( bool Cond, string Message )
  {
    if( Cond )
      return true ;

    cout << "*** ERROR: '" << Message << "' at PC = " << PC-2 << " for operation " << Mnemo[OpId] << endl;
    H = true ; // program will halt

    return false ;
  }

  // return the "number of bytes"
  byte Processor::Controller::numBytes( dataSize Size )
  {
    return( (Size == longSize) ? 4 : (Size + 1) );
  }

  // Fetch the OpCode from memory
  void Processor::Controller::fetchOpCode()
  {
    if( nDebugLevel > 0 )
      cout << "\nFetchOpCode(): at PC = " << PC << endl;

    RW  = true ;
    MAR = PC ;
    PC += 2 ;
    mem.access( wordSize );
    OpCode = getWord(MDR, LEAST); // get LSW from MDR
  }

  // Update the fields OpId, DS, numOprd, M1, R1, M2, R2 and Data according to given format
  void Processor::Controller::decodeInstr()
  {
      DS    = (dataSize)getBits( OpCode,  9, 10 );
     OpId   = getBits( OpCode, 11, 15 );
    numOprd = getBits( OpCode,  8,  8 ) + 1 ;

    if( nDebugLevel > 0 )
      cout << "DecodeInstr(OpCode " << OpCode << "): at PC = " << PC-2 << " : OpId = " << Mnemo[OpId]
           << ", size = " << sizeName[DS] << ", numOprnd = " << (int)numOprd << endl;

    if( numOprd > 0 ) // SHOULD ALWAYS BE TRUE!
    {
      M2 = (addressmode)getBits( OpCode, 1, 3 );
      R2 = getBits( OpCode, 0, 0 );
      
      if( p->formatF1(OpId) )
      {
        if( (OpId < iDSR) )
        {
          M1 = (addressmode)getBits( OpCode, 5, 7 );
          R1 = getBits( OpCode, 4, 4 );
        }
        else { // NEED to reset these for iDSR and iHLT !
            M1 = M2 = DATA_REGISTER_DIRECT ;
            R1 = R2 = 0 ;
        }
      }
      else // Format F2
          opcData = getBits( OpCode, 4, 7 );
    }
    else
      {
        cout << "*** ERROR: DecodeInstr() received invalid number of operands '" << (int)numOprd
             << "' at PC = " << PC-2 << endl;
        H = true ;
      }
  }

  // Fetch the operands, according to their number (numOprd) & addressing modes (M1 or M2)
  void Processor::Controller::fetchOperands()
  {
    if( nDebugLevel > 0 )
      cout << "Controller::fetchOperands(" << (int)numOprd << "): at PC = " << PC-2
           << " : M1 = " << M1 << ", M2 = " << M2 << endl;

    RW = true ;

    // Fetch the address of 1st operand (in OpAddr1)
    if( p->formatF1(OpId) && (M1 == RELATIVE_ABSOLUTE) )
    {
      MAR = PC ;
      mem.access( wordSize );
      OpAddr1 = getWord( MDR, LEAST ); // get LSW of MDR
      PC += 2 ;
    }

    // Fetch the address of 2nd operand, if F1 & 2 operands.
    // OR, operand of an instruction with format F2 put in OpAddr2
    if( M2 == RELATIVE_ABSOLUTE )
    {
      MAR = PC ;
      mem.access( wordSize );
      OpAddr2 = getWord( MDR, LEAST ); // get LSW of MDR
      PC += 2 ;
    }

    // Check invalid number of operands.
    if( (numOprd == 2) && (!p->formatF1(OpId)) )
    {
      cout << "*** ERROR >> Controller::fetchOperands() has an invalid number of operands for " << Mnemo[OpId]
           << " at PC = " << PC-2 << endl;
      H = true ;
    }

  }

  // Transfer the contents of temporary register to Register OR Memory
  void Processor::Controller::setResult(
                int  tmpReg,            // Source Register (TMPD...)
                word OpAddrNo,          // Operand Address (OpAddr1...)  
                enum dataSize dsz,      // Data Size                    
                enum addressmode mode,  // required Addressing Mode     
                byte RegNo            ) // Register Number for A[n] or D[n] 
  {
    RW = false ;
  
    // Depends on Addressing Mode
    switch( mode )
    {
      case DATA_REGISTER_DIRECT:
             switch( dsz )
             {
               case byteSize: setBits( &(DR[RegNo]), 0, 7, tmpReg ); break;
               case wordSize: setWord( &(DR[RegNo]), LEAST, getWord(tmpReg, LEAST) ); break;
               case longSize: DR[RegNo] = tmpReg; break;

               default: cout << "*** ERROR >> Controller::setResult() received invalid data size '" << dsz
                             << "' at PC = " << PC-2 << endl;
                        H = true ;
             }
             break;

      case ADDRESS_REGISTER_DIRECT:
             AR[RegNo] = getWord( tmpReg, LEAST );
             break;

      case RELATIVE_ABSOLUTE:
             // We need to access memory, except for branching & MOVA.
             MAR = OpAddrNo;
             MDR = tmpReg;
             mem.access( dsz );
             break;

      case ADDRESS_REGISTER_INDIRECT:
             // We need to access memory.
             MAR = AR[RegNo];
             MDR = tmpReg;
             mem.access( dsz );
             break;

      case ADDRESS_REGISTER_INDIRECT_POSTINC:
             // We need to access memory.
             // ATTENTION: for some instructions, the address register has already been incremented by fillTmpReg()
             // DO NOT increment it a 2nd time here
             MAR = AR[RegNo] - numBytes(dsz);
             MDR = tmpReg;
             mem.access( dsz );
             break;

      case ADDRESS_REGISTER_INDIRECT_PREDEC:
             // We need to access memory.
             // ATTENTION: for some instructions, the address register has already been decremented by fillTmpReg
             // DO NOT decrement it a 2nd time here
             MAR = AR[RegNo];
             MDR = tmpReg;
             mem.access( dsz );
             break;
             
      default: // invalid addressMode
              cout << "*** ERROR >> Controller::setResult() has Invalid Addressing Mode '" << mode
                   << "' at PC = " << PC-2 << endl;
              H = true ;
    }
  }

  // Status bits Z & N are often set the same way in many instructions
  // A function would be useful to do this
  void Processor::Controller::setZN( int tmpReg )
  {
    switch( DS )
    {
      case byteSize: Z = ( (getBits(getWord(tmpReg, LEAST), 0,  7) | 0) == 0 );
                     N = (  getBits(getWord(tmpReg, LEAST), 7,  7) == 1 );
                     break;

      case wordSize: Z = ( (getBits(getWord(tmpReg, LEAST),  0, 15) | 0) == 0 );
                     N = (  getBits(getWord(tmpReg, LEAST), 15, 15) == 1 );
                     break;

      case longSize: Z = ( (tmpReg | 0x00000000) == 0 );
                     N = ( getBits(getWord(tmpReg, MOST), 15, 15) == 1 );
                     break;

      default: cout << "*** ERROR >> Controller::setZN() received invalid data size '" << DS
                    << "' at PC = " << PC-2 << endl;
               H = true ;
    }
  }

  // The calculations to find V & C are more complex but are simplified by the use of Sm, Dm, Rm
  // It would be a good Idea to make a procedure to find these values
  void Processor::Controller::setSmDmRm( int tmpSrc, int tmpDst, int tmpRes )
  {
    byte mostSigBit = 15 ; // wordSize
    switch( DS )
    {
      case byteSize: mostSigBit =  7 ; break;
      case wordSize: break;
      case longSize: mostSigBit = 31 ; break;

      default: cout << "*** ERROR >> Controller::setSmDmRm() received invalid data size '" << DS
                    << "' at PC = " << PC-2 << endl;
               H = true ;
    }
    
    Sm = ( getBits(tmpSrc, mostSigBit, mostSigBit) == 1 );
    Dm = ( getBits(tmpDst, mostSigBit, mostSigBit) == 1 );
    Rm = ( getBits(tmpRes, mostSigBit, mostSigBit) == 1 );
  }

  /********************************************************************
    The execution of each instruction is done via its micro-program
  *********************************************************************/
  void Processor::Controller::execInstr()
  {
    byte i ; // counter
    word tmpA ;
    string input ;

    cout.setf(ios_base::hex, ios_base::basefield);
    if( nDebugLevel > 0 )
      cout << "Controller::execInstr(" << Mnemo[OpId] << "." << sizeName[DS]
           << "): OpAd1 = " << OpAddr1 << ", OpAd2 = " << OpAddr2
           << ", M1 = " << M1 << ", R1 = " << (int)R1 << ", M2 = " << M2 << ", R2 = " << (int)R2 << endl;

    // Execute the instruction according to opCode
    // Use a CASE structure where each case corresponds to an instruction & its micro-program
    switch( OpId )
    {
      // addition
      case iADD:
               /* EXAMPLE micro-program according to step 2.4.1 in section 3  */
               // 1. Fill TMPS if necessary 
               fillTmpReg( &TMPS, OpAddr1, DS, M1, R1 );
               // 2. Fill TMPD if necessary 
               fillTmpReg( &TMPD, OpAddr2, DS, M2, R2 );
               // 3. Compute TMPR using TMPS & TMPD
               TMPR = TMPS + TMPD ;
               if( nDebugLevel > 0 )
                 cout << "TMPR(" << TMPR << ") = TMPS(" << TMPS << ") + TMPD(" << TMPD << ")" << endl;
               // 4. Update status bits HZNVC if necessary  
               setZN( TMPR );
               setSmDmRm( TMPS, TMPD, TMPR );
               V = ( Sm & Dm & ~Rm ) | ( ~Sm & ~Dm & Rm );
               C = ( Sm & Dm ) | ( ~Rm & Dm ) | ( Sm & ~Rm );
               // 5. Store the result in the destination if necessary 
               setResult( TMPR, OpAddr2, DS, M2, R2 );
               break ;
      // add quick
      case iADDQ:
                fillTmpReg( &TMPD, OpAddr2, DS, M2, R2 );
                TMPS = 0 ;
                setByte( &TMPS, byte0, opcData );
                // Sign extension if W or L ??
                TMPR = TMPD + TMPS ;
                if( nDebugLevel > 0 )
                  cout << "TMPR(" << TMPR << ") = TMPS(" << TMPS << ") + TMPD(" << TMPD << ")" << endl;
                setZN( TMPR );
                setSmDmRm( TMPS, TMPD, TMPR );
                V = ( Sm & Dm & ~Rm ) | ( ~Sm & ~Dm & Rm );
                C = ( Sm & Dm ) | ( ~Rm & Dm ) | ( Sm & ~Rm );
                setResult( TMPR, OpAddr2, DS, M2, R2 );
                break;
      // subtraction
      case iSUB:
               fillTmpReg( &TMPS, OpAddr1, DS, M1, R1 );
               fillTmpReg( &TMPD, OpAddr2, DS, M2, R2 );
               TMPR = TMPD - TMPS ;
               if( nDebugLevel > 0 )
                 cout << "TMPR(" << TMPR << ") = TMPD(" << TMPD << ") - TMPS(" << TMPS << ")" << endl;
               setZN( TMPR );
               setSmDmRm( TMPS, TMPD, TMPR );
               V = ( ~Sm & Dm & ~Rm ) | ( Sm & ~Dm & Rm );
               C = ( Sm & ~Dm ) | ( Rm & ~Dm ) | ( Sm & Rm );
               setResult( TMPR, OpAddr2, DS, M2, R2 );
               break;
      // sub quick
      case iSUBQ:
                fillTmpReg( &TMPD, OpAddr2, DS, M2, R2 );
                TMPS = 0;
                setByte( &TMPS, byte0, opcData );
                // Sign extension if W or L ??
                TMPR = TMPD - TMPS;
                if( nDebugLevel > 0 )
                  cout << "TMPR(" << TMPR << ") = TMPD(" << TMPD << ") - TMPS(" << TMPS << ")" << endl;
                setZN( TMPR );
                setSmDmRm( TMPS, TMPD, TMPR );
                V = ( ~Sm & Dm & ~Rm ) | ( Sm & ~Dm & Rm );
                C = ( Sm & ~Dm ) | ( Rm & ~Dm ) | ( Sm & Rm );
                setResult( TMPR, OpAddr2, DS, M2, R2 );
                break;
      // signed
      case iMULS:
                if( checkCond( (DS == wordSize), "Invalid Data Size" ) )
                {
                  fillTmpReg( &TMPS, OpAddr1, DS, M1, R1 );
                  fillTmpReg( &TMPD, OpAddr2, DS, M2, R2 );
                  if( getBits(TMPS,15,15) == 1 )
                    TMPS = TMPS | 0xFFFF0000 ;
                  if( getBits(TMPD,15,15) == 1 )
                    TMPD = TMPD | 0xFFFF0000 ;
                  TMPR = TMPD * TMPS;
                  if( nDebugLevel > 0 )
                    cout << "TMPR(" << TMPR << ") = TMPD(" << TMPD << ") * TMPS(" << TMPS << ")" << endl;
                  setZN( TMPR );
                  V = false;
                  C = false;
                  setResult( TMPR, OpAddr2, longSize, M2, R2 );
                }// if size = 1
                break;
      // signed
      case iDIVS:
                if( checkCond( (DS == longSize), "Invalid Data Size" ) )
                {
                  fillTmpReg( &TMPS, OpAddr1, wordSize, M1, R1);
                  if( checkCond( (TMPS != 0), "Division by Zero" ) )
                  {
                    fillTmpReg( &TMPD, OpAddr2, DS, M2, R2 );
                    V = ( (TMPD / TMPS) < -32768 ) | ( (TMPD / TMPS) > 32767 );
                    if( TMPS > 0x8000 )
                    {
                      i = 1;
                      TMPS = (TMPS ^ 0xFFFF) + 1;
                      TMPD = (TMPD ^ 0xFFFFFFFF) + 1;
                    };
                    if( ((TMPD / TMPS) == 0) && (i == 1) )
                    {
                      setWord( &TMPR, LEAST, 0 );
                      TMPD = (TMPD ^ 0xFFFFFFFF) + 1 ;
                      setWord( &TMPR, MOST, TMPD % TMPS );
                    }
                    else
                      {
                        TMPR = TMPD / getWord(TMPS, LEAST);
                        setWord( &TMPR, MOST, (TMPD % getWord(TMPS, LEAST)) );
                      };
                    if( nDebugLevel > 0 )
                      cout << "TMPR(" << TMPR << ") = TMPD(" << TMPD << ") / TMPS(" << TMPS << ")" << endl;
                    setZN( TMPR );
                    C = false ;
                    setResult( TMPR, OpAddr2, DS, M2, R2 );
                  }// if not div by 0
                }// if longSize
                break;
      // negate
      case iNEG:
               fillTmpReg( &TMPD, OpAddr1, DS, M1, R1 );
               TMPR = -TMPD;
               setZN( TMPR );
               setSmDmRm( TMPS, TMPD, TMPR );
               V = Dm & Rm ;
               C = Dm | Rm ;
               setResult( TMPR, OpAddr1, DS, M1, R1 );
               break;
      // clear
      case iCLR:
               TMPD = 0 ;
               setZN( TMPD );
               V = false;
               C = false;
               setResult( TMPD, OpAddr1, DS, M1, R1 );
               break;
      // bitwise
      case iNOT:
               fillTmpReg( &TMPD, OpAddr1, DS, M1, R1 );
               TMPR = ~TMPD ;
               setZN( TMPR );
               V = false;
               C = false;
               setResult( TMPR, OpAddr1, DS, M1, R1 );
               break;
      // bitwise
      case iAND:
               fillTmpReg( &TMPS, OpAddr1, DS, M1, R1 );
               fillTmpReg( &TMPD, OpAddr2, DS, M2, R2 );
               TMPR = TMPD & TMPS ;
               if( nDebugLevel > 0 )
                 cout << "TMPR(" << TMPR << ") = TMPD(" << TMPD << ") & TMPS(" << TMPS << ")" << endl;
               setZN( TMPR );
               V = false;
               C = false;
               setResult( TMPR, OpAddr2, DS, M2, R2 );
               break;
      // bitwise
      case iOR:
              fillTmpReg( &TMPS, OpAddr1, DS, M1, R1 );
              fillTmpReg( &TMPD, OpAddr2, DS, M2, R2 );
              TMPR = TMPD | TMPS ;
              if( nDebugLevel > 0 )
                cout << "TMPR(" << TMPR << ") = TMPD(" << TMPD << ") | TMPS(" << TMPS << ")" << endl;
              setZN( TMPR );
              V = false;
              C = false;
              setResult( TMPR, OpAddr2, DS, M2, R2 );
              break;
      // xor
      case iEOR:
               fillTmpReg( &TMPS, OpAddr1, DS, M1, R1 );
               fillTmpReg( &TMPD, OpAddr2, DS, M2, R2 );
               TMPR = TMPD ^ TMPS;
               if( nDebugLevel > 0 )
                 cout << "TMPR(" << TMPR << ") = TMPD(" << TMPD << ") ^ TMPS(" << TMPS << ")" << endl;
               setZN( TMPR );
               V = false;
               C = false;
               setResult( TMPR, OpAddr2, DS, M2, R2 );
               break;
      // shift left
      case iLSL:
               fillTmpReg( &TMPD, OpAddr2, DS, M2, R2 );
               TMPR = TMPD << opcData;
               setZN( TMPR );
               V = false;
               if( opcData > 0 )
               {
                 C = ( getBits(TMPD, numBytes(DS)*8-opcData, numBytes(DS)*8-opcData) == 1 ) ? true : false ;
               }
               else
                   C = false;
               setResult( TMPR, OpAddr2, DS, M2, R2 );
               break;
      // shift right
      case iLSR:
               // In Turbo Pascal, SHR did NOT sign-extend, whereas >> in C is "machine-dependent" and in this platform
               // (Linux x86_64) it does sign extension for variables, so DO NOT get the proper answer for some operations.
               // We want to ensure there is NEVER any sign extension, to duplicate the Pascal results, HOWEVER C does NOT
               // sign extend CONSTANTS, so CANNOT correct for sign extension by doing this:
               // TMPR = TMPS & ~( 0x80000000 >> (opcData-1) ) as the constant 0x80000000 is NOT extended
               // Instead, need to put 0x80000000 in a register and proceed as below:
               fillTmpReg( &TMPD, OpAddr2, DS, M2, R2 );
               if( nDebugLevel > 0 )
                 cout << "TMPD = " << TMPD << "; opcData = " << opcData << endl;
               TMPS = 0x80000000 ;
               TMPR = TMPS >> (opcData-1) ;
               if( nDebugLevel > 0 )
                 cout << "TMPS = " << TMPS << "; TMPR = TMPS >> " << opcData-1 << " = " << TMPR << endl;
               TMPS = ~TMPR ;
               if( nDebugLevel > 0 )
                 cout << "TMPS = ~TMPR = " << TMPS << endl;
               TMPR = (TMPD >> opcData) & TMPS ;
               setZN( TMPR );
               V = false;
               C = ( opcData > 0 ) ? ( getBits(TMPD, opcData-1, opcData-1) == 1 ) : false ;
               setResult( TMPR, OpAddr2, DS, M2, R2 );
               break;
      // rotate left
      case iROL:
               fillTmpReg( &TMPD, OpAddr2, DS, M2, R2 );
               opcData = opcData % (8 * numBytes(DS) );
               TMPR = TMPD << opcData ;
               TMPS = TMPD >> ( (8*numBytes(DS)) - opcData );
               setBits( &TMPR, 0, opcData-1, TMPS );
               setZN( TMPR );
               V = false;
               C = ( opcData > 0 ) ? ( getBits(TMPD, (numBytes(DS)*8)-opcData, (numBytes(DS)*8)-opcData) == 1 ) : false ;
               setResult( TMPR, OpAddr2, DS, M2, R2 );
               break;
      // rotate right
      case iROR:
               fillTmpReg( &TMPD, OpAddr2, DS, M2, R2 );
               opcData = opcData % ( 8*numBytes(DS) );
               TMPR = TMPD >> opcData;
               setBits( &TMPR, (8*numBytes(DS))-opcData, (8*numBytes(DS))-1, TMPD );
               setZN( TMPR );
               V = false;
               C = ( opcData > 0 ) ? ( getBits(TMPD, opcData-1, opcData-1) == 1 ) : false ;
               setResult( TMPR, OpAddr2, DS, M2, R2 );
               break;
      // compare
      case iCMP:
               fillTmpReg( &TMPS, OpAddr1, DS, M1, R1 );
               fillTmpReg( &TMPD, OpAddr2, DS, M2, R2 );
               TMPR = TMPD - TMPS ;
               setZN( TMPR );
               setSmDmRm( TMPS, TMPD, TMPR );
               V = ( ~Sm & Dm & ~Rm ) | ( Sm & ~Dm & Rm );
               C = ( Sm & ~Dm ) | ( Rm & ~Dm ) | ( Sm & Rm );
               break;
      // test
      case iTST:
               fillTmpReg( &TMPD, OpAddr1, DS, M1, R1 );
               setZN( TMPD );
               V = false ;
               C = false ;
               break;
      // branch
      case iBRA:
               if( checkCond( (M1 == RELATIVE_ABSOLUTE), "Invalid Addressing Mode" )
                   && checkCond( (DS == wordSize), "Invalid Data Size" ) )
                PC = OpAddr1 ;
               break;
      // branch if overflow
      case iBVS:
               if( checkCond( (M1 == RELATIVE_ABSOLUTE), "Invalid Addressing Mode" )
                   && checkCond( (DS == wordSize), "Invalid Data Size" ) )
                 if( V ) PC = OpAddr1 ;
               break;
      // branch if equal
      case iBEQ:
               if( checkCond( (M1 == RELATIVE_ABSOLUTE), "Invalid Addressing Mode" )
                   && checkCond( (DS == wordSize), "Invalid Data Size" ) )
                 if( Z ) PC = OpAddr1 ;
               break;
      // branch if carry
      case iBCS:
               if( checkCond( (M1 == RELATIVE_ABSOLUTE), "Invalid Addressing Mode" )
                   && checkCond( (DS == wordSize), "Invalid Data Size" ) )
                 if( C ) PC = OpAddr1 ;
               break;
      // branch if GTE
      case iBGE:
               if( checkCond( (M1 == RELATIVE_ABSOLUTE), "Invalid Addressing Mode" )
                   && checkCond( (DS == wordSize), "Invalid Data Size" ) )
                 if( !(N^V) ) PC = OpAddr1 ;
               break;
      // branch if LTE
      case iBLE:
               if( checkCond( (M1 == RELATIVE_ABSOLUTE), "Invalid Addressing Mode" )
                   && checkCond( (DS == wordSize), "Invalid Data Size" ) )
                 if( (N^V) ) PC = OpAddr1 ;
               break;
      // move
      case iMOV:
               fillTmpReg( &TMPS, OpAddr1, DS, M1, R1 );
               setResult( TMPS, OpAddr2, DS, M2, R2 );
               break;
      // move quick
      case iMOVQ:
                fillTmpReg( &TMPD, OpAddr2, DS, M2, R2 );
                setByte( &TMPD, byte0, opcData );
                // Sign extension if W or L ??
                setZN( TMPD );
                V = false;
                C = false;
                setResult( TMPD, OpAddr2, DS, M2, R2 );
                break;
      // exchange
      case iEXG:
               if( checkCond( ((M1 <= ADDRESS_REGISTER_DIRECT) && (M2 <= ADDRESS_REGISTER_DIRECT)),
                              "Invalid Addressing Mode" ) )
               {
                 fillTmpReg( &TMPS, OpAddr1, DS, M1, R1 );
                 fillTmpReg( &TMPD, OpAddr2, DS, M2, R2 );
                 setResult( TMPS, OpAddr1, DS, M2, R2 );
                 setResult( TMPD, OpAddr2, DS, M1, R1 );
                 V = false;
                 C = false;
               }
               break;
      // move to address
      case iMOVA:
                if( checkCond( ((M1 == RELATIVE_ABSOLUTE) && (M2 == ADDRESS_REGISTER_DIRECT)),
                               "Invalid Addressing Mode" )
                    && checkCond( (DS == wordSize), "Invalid Data Size" ) )
                  setResult( OpAddr1, OpAddr2, DS, M2, R2 );
                break;
      // input
      case iINP:
               cout << "Enter a value ";
               switch( DS )
               {
                 case byteSize: cout << "(" << sizeName[byteSize] << ") for "; break;
                 case wordSize: cout << "(" << sizeName[wordSize] << ") for "; break;
                 case longSize: cout << "(" << sizeName[longSize] << ") for "; break;
                 
                 default: cout << "\n*** ERROR >> ExecInstr() invalid data size '" << DS
                               << "' for instruction '" << OpId << "' at PC = " << PC-2 << endl;
                          H = true ;
                          return ;
               }
               switch( M1 )
               {
                 case DATA_REGISTER_DIRECT:    cout << "the register DR" << (int)R1 ; break;
                 case ADDRESS_REGISTER_DIRECT: cout << "the register AR" << (int)R1 ; break;
                 
                 case ADDRESS_REGISTER_INDIRECT: 
                 case ADDRESS_REGISTER_INDIRECT_PREDEC: 
                 case ADDRESS_REGISTER_INDIRECT_POSTINC: cout << "the memory address " << AR[R1]/*:4*/; break;
                 
                 case RELATIVE_ABSOLUTE: cout << "the memory address " << OpAddr1 ; break;
                 
                 default: cout << "\n*** ERROR >> ExecInstr() invalid mode type '" << M1
                               << "' for instruction '" << OpId << "' at PC = " << PC-2 << endl;
                          H = true ;
                          return ;
               }
               cout << ": " ;
               cin >> input ;
               TMPD = (int)strtol( input.c_str(), (char**)NULL, 0 );
               setZN( TMPD );
               C = false;
               V = false;
               setResult( TMPD, OpAddr1, DS, M1, R1 );
               break;
      // display
      case iDSP:
               fillTmpReg( &TMPS, OpAddr1, DS, M1, R1 );
               switch( M1 )
               {
                 case DATA_REGISTER_DIRECT:      cout << "[ DR" << (int)R1 << " ] = "; break;
                 case ADDRESS_REGISTER_DIRECT:   cout << "[ AR" << (int)R1 << " ] = "; break;

                 case ADDRESS_REGISTER_INDIRECT: cout << "[$" << AR[R1] << " ] = "; break;
                 
                 case ADDRESS_REGISTER_INDIRECT_POSTINC: // numBytes(DS) subtracted to compensate post-incrementation
                         tmpA = AR[R1] - numBytes(DS);
                         cout << "[$" << tmpA << " ] = ";
                         break;
                         
                 case ADDRESS_REGISTER_INDIRECT_PREDEC: cout << "[$" << AR[R1] << "] = "; break;

                 case RELATIVE_ABSOLUTE: cout << "[$" << OpAddr1 << "] = "; break;
                 
                 default: cout << "\n*** ERROR >> ExecInstr() invalid address mode '" << M1
                               << "' for instruction '" << Mnemo[OpId] << "' at PC = " << PC-2 << endl;
                          H = true ;
                          return ;
               }
               switch( DS )
               {
                 case byteSize: cout << "$" << (TMPS & 0xff)   << " (" << sizeName[byteSize] << ")" << endl; break;
                 case wordSize: cout << "$" << (TMPS & 0xffff) << " (" << sizeName[wordSize] << ")" << endl; break;
                 case longSize: cout << "$" <<  TMPS           << " (" << sizeName[longSize] << ")" << endl; break;
                 
                 default: cout << "\n*** ERROR >> ExecInstr() invalid data size '" << DS
                               << "' for instruction '" << Mnemo[OpId] << "' at PC = " << PC-2 << endl;
                          H = true ;
                          return ;
               }
               break;
      // display status register
      case iDSR:
               cout << "Status Bits = H:" << bitValue[H] << " N:" << bitValue[N] << " Z:" << bitValue[Z]
                    << " V:" << bitValue[V] << " C:" << bitValue[C] << endl;
               break;
      // halt
      case iHLT:
               H = true; // Set the Halt bit to true (stops program)
               break;

      default:
             cout << "*** ERROR >> ExecInstr() received invalid instruction '" << Mnemo[OpId]
                  << "' at PC = " << PC-2 << endl;
             H = true ;
                
    }
  }

  // Determines the format of the instruction: return True if F1, False if F2
  bool Processor::formatF1( byte opid )
  {
    if( (opid == iADDQ) || (opid == iSUBQ) || ((opid >= iLSL) && (opid <= iROR)) || (opid == iMOVQ) )
      return false ;

    return true ;
  }

  // Initializes Mnemo with strings corresponding to each instruction
  // - to have a more useful display of each opCode
  void Processor::mnemoInit()
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
  }

  // Fetch-Execute Cycle simulated
  void Processor::control()
  {
    do // Repeat the Fetch-Execute Cycle until the Halt bit becomes true
    {
      ctrl.fetchOpCode();
      ctrl.decodeInstr();
      ctrl.fetchOperands();
      if( !H )
//        break;
        ctrl.execInstr();
    }
    while( !H );

    cout << endl << "\tEnd of Fetch-Execute Cycle" << endl;
  }

} // namespace mhs_cpp_sim68k

using namespace mhs_cpp_sim68k ;

/*
 *  MAIN
 **************************************************************************/
int main( int argc, char* argv[] )
{
  char option ; // option chosen from the menu by the user
  string input ;
  
  if( argc > 1 )
    nDebugLevel = strtol( argv[1], (char**)NULL, 0 );
  
  // info on system data sizes
  if( nDebugLevel > 1 )
  {
    cout << "sizeof( bool ) == " << sizeof(bool) << endl;
    cout << "sizeof( char ) == " << sizeof(char) << endl;
    cout << "sizeof( short ) == " << sizeof(short) << endl;
    cout << "sizeof( int ) == " << sizeof(int) << endl;
    cout << "sizeof( long ) == " << sizeof(long) << endl;
    cout << "sizeof( bit ) == " << sizeof(bit) << endl;
    cout << "sizeof( twobits ) == " << sizeof(enum twobits) << endl;
    cout << "sizeof( dataSize ) == " << sizeof(enum dataSize) << endl;
    cout << "sizeof( addressmode ) == " << sizeof(enum addressmode) << endl;
    cout << "sizeof( byte ) == " << sizeof(byte) << endl;
    cout << "sizeof( word ) == " << sizeof(word) << endl;
    cout << "sizeof( string ) == " << sizeof(string) << endl;
  }

  Processor proc;

  // Menu 
  while( option != QUIT )
  {
    if( ! isspace(option) )
      cout << "Your Option ('" << EXECUTE << "' to execute a program, '" << QUIT << "' to quit): " << endl;
    
    cin >> option ;
    cout << "\nYour entry: " << option << endl;
    if( option != QUIT  &&  option != EXECUTE ) {
      cout << "\nYour input '" << option << "' was NOT acceptable. Please try again." << endl;
      continue ;
    }
    switch( option )
    {
      case EXECUTE :
                    // execution on the simulator
                    cout << "Name of the 68k binary program ('.68b' will be added automatically): " << endl;
                    cin >> input ;
                    if( proc.loadProgram(input + ".68b") ) {
                      // Start the processor
                      proc.control();
                    }
                    else
                        cout << "\nYour input '" << input << "' was NOT acceptable. Please try again." << endl;
                    break;
                    
      case QUIT : cout << "Bye!" << endl;
                  break;
                  
      case EOL : break;
      
      default: cout << "Invalid Option. Please enter '" << EXECUTE << "' or '" << QUIT << "'" << endl;
    }
    
  }
  
  cout << "\tPROGRAM ENDED" << endl;
  return 0 ;

}
