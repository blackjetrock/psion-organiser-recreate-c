////////////////////////////////////////////////////////////////////////////////
//
// NewOPL types
//
// Contains utility functions related to the types in OPL
//
//
////////////////////////////////////////////////////////////////////////////////

// OPL has these types:
//
//
// Integer Variable
//   Format: HH LL
//   16 bit integer, first the high byte then the low byte.
//   Signed
//   Size: 2
//   Type Byte: 00
// 
//   Example: 13579
//   35 0B
//
// Float Variable
//   Format: MM MM MM MM MM MM EE SS
//   6-byte mantissa (least to most significant), Exponent byte, Sign byte
//   Size: 8
//   Type Byte: 01
// 
//   Mantissa is binary coded decimal, most significant digit in high nibble.
//   Exponent is in range -99 to 99, with 0 meaning 1 digit before decimal point.
//   Sign byte: sign in bit 7, rest clear, so $80 if negative, $00 if positive.
// 
//   Example: -123.456789
//   00 90 78 56 34 12 02 80
// 
// Float constants inside a procedure are stored in a shortened form:
//   LL [MM] EE
//   Length and sign byte LL, a number of mantissa bytes, an exponent byte.
//   
//   The missing least significant mantissa bytes are considered to be 0.
//   The sign bit is stored in the high bit of LL.
//   
//   Example: -123.4
//   83 34 12 02
// 
// String Variable
//   Format: MM; LL [characters]
//   Declared maximum length, Actual length, ASCII contents
//   Size: MM+2
//   Type Byte: 02
//   
//   MM is the maximum length of string, i.e. the number of bytes reserved
//   for contents as stated in the LOCAL or GLOBAL declaration.
//   LL is actual length of string.
//   In the MM character bytes that follow, the first LL are actual contents,
//   the rest are indeterminate.
// 
//   Note that the address of the string variable is that of the length byte.
// 
//   Example: LOCAL A$(10) : A$ = "ABCD"
//   0A; 04 41 42 43 44 ?? ?? ?? ?? ?? ??
// 
//   String constants inside a procedure are stored simply as a length byte
//   followed by the contents.
// 
//   Example: "ABCD"
//   04 41 42 43 44
// 
// Integer Array Variable
//   Format: LL LL [integers]
//   Array size, list of integers
//   Size: LLLL*2+2
//   Type Byte: 03
//   
//   LLLL is a word (16-bit integer) containing the number of integers in the array.
//   This is followed by the array contents, two bytes for each integer.
// 
//   In OPL, ADDR(array%()) returns address of first integer in the array. In QCode,
//   the array variable's address is the address of the array size word.
// 
//   Example: LOCAL A%(2) : A%(1)=123 : A%(2)=-1234
//   00 02, 00 7B, FB 2E
// 
// Float Array Variable
//   Format: LL LL [floats]
//   Array size, list of floats
//   Size: LLLL*8+2
//   Type Byte: 04
//   
//   LLLL is the number of integers in the array.
//   This is followed by the array contents, eight bytes for each float.
// 
//   In OPL, ADDR() returns address of first float. In QCode, the array address
//   is the address of the array size word.
// 
//   Example: LOCAL A(2) : A(1)=1234567 : A(2)=-123.456789
//   00 02, 00 00 70 56 34 12 06 00, 00 90 78 56 34 12 02 80
// 
// String Array Variable
//   Format: MM; LL LL [strings]
//   Declared maximum length, Array size, list of strings
//   Size: LLLL*(MM+1)+3
//   Type Byte: 05
// 
//   MM is shared maximum length of the strings.
//   LLLL is the number of strings in the array.
//   This is followed by the array contents, MM+1 bytes of storage space for
//   each string of which the first is its length byte.
// 
//   In OPL, ADDR() returns address of first string. In QCode, the array address is the address of the array size word.
//   
//   Example: LOCAL A$(2,4) : A$(1)="ABC" : A$(2)="1234"
//   04; 00 02, 03 41 42 43 ??, 04 31 32 33 34
// 

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>

#include "nopl.h"

