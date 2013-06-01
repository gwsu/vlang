//===--- LiteralSupport.h ---------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the NumericLiteralParser, CharLiteralParser, and
// StringLiteralParser interfaces.
//
//===----------------------------------------------------------------------===//

#ifndef VLANG_LITERALSUPPORT_H
#define VLANG_LITERALSUPPORT_H

#include "vlang/Basic/CharInfo.h"
#include "vlang/Basic/LLVM.h"
#include "vlang/Basic/TokenKinds.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/DataTypes.h"

namespace vlang {

class DiagnosticsEngine;
class Preprocessor;
class Token;
class SourceLocation;
class TargetInfo;
class SourceManager;
class LangOptions;

/// NumericLiteralParser - This performs strict semantic analysis of the content
/// of a ppnumber, classifying it as either integer, floating, or erroneous,
/// determines the radix of the value and can convert it to a useful value.
class NumericLiteralParser {
  Preprocessor &PP; // needed for diagnostics

  const char *const ThisTokBegin;
  const char *const ThisTokEnd;
  const char *DigitsBegin, *DigitsEnd; // markers
  const char *s; // cursor

  unsigned radix;

  bool saw_period;

public:
  NumericLiteralParser(StringRef TokSpelling,
                       SourceLocation TokLoc,
                       Preprocessor &PP);
  bool hadError;
  bool isUnsigned;
  bool isReal;       // 1.0f

  bool isIntegerLiteral() const {
    return !saw_period;
  }
  bool isFloatingLiteral() const {
    return saw_period;
  }

  unsigned getRadix() const { return radix; }

  /// GetIntegerValue - Convert this numeric literal value to an APInt that
  /// matches Val's input width.  If there is an overflow (i.e., if the unsigned
  /// value read is larger than the APInt's bits will hold), set Val to the low
  /// bits of the result and return true.  Otherwise, return false.
  bool GetIntegerValue(llvm::APInt &Val);

  /// GetFloatValue - Convert this numeric literal to a floating value, using
  /// the specified APFloat fltSemantics (specifying float, double, etc).
  /// The optional bool isExact (passed-by-reference) has its value
  /// set to true if the returned APFloat can represent the number in the
  /// literal exactly, and false otherwise.
  llvm::APFloat::opStatus GetFloatValue(llvm::APFloat &Result);

private:

  /// SkipHexDigits - Read and skip over any hex digits, up to End.
  /// Return a pointer to the first non-hex digit or End.
  const char *SkipHexDigits(const char *ptr) {
    while (ptr != ThisTokEnd && isHexDigit(*ptr))
      ptr++;
    return ptr;
  }

  /// SkipOctalDigits - Read and skip over any octal digits, up to End.
  /// Return a pointer to the first non-hex digit or End.
  const char *SkipOctalDigits(const char *ptr) {
    while (ptr != ThisTokEnd && ((*ptr >= '0') && (*ptr <= '7')))
      ptr++;
    return ptr;
  }

  /// SkipDigits - Read and skip over any digits, up to End.
  /// Return a pointer to the first non-hex digit or End.
  const char *SkipDigits(const char *ptr) {
    while (ptr != ThisTokEnd && isDigit(*ptr))
      ptr++;
    return ptr;
  }

  /// SkipBinaryDigits - Read and skip over any binary digits, up to End.
  /// Return a pointer to the first non-binary digit or End.
  const char *SkipBinaryDigits(const char *ptr) {
    while (ptr != ThisTokEnd && (*ptr == '0' || *ptr == '1'))
      ptr++;
    return ptr;
  }

};

/// StringLiteralParser - This decodes string escape characters and performs
/// wide string analysis and Translation Phase #6 (concatenation of string
/// literals) (C99 5.1.1.2p1).
class StringLiteralParser {
  const SourceManager &SM;
  const LangOptions &Features;
  DiagnosticsEngine *Diags;
  
  unsigned MaxTokenLength;
  unsigned SizeBound;
  unsigned CharByteWidth;
  tok::TokenKind Kind;
  SmallString<512> ResultBuf;
  char *ResultPtr; // cursor
  SmallString<32> UDSuffixBuf;
  unsigned UDSuffixToken;
  unsigned UDSuffixOffset;
public:
  StringLiteralParser(const Token *StringToks, unsigned NumStringToks,
                      Preprocessor &PP, bool Complain = true);
  StringLiteralParser(const Token *StringToks, unsigned NumStringToks,
                      const SourceManager &sm, const LangOptions &features,
                      DiagnosticsEngine *diags = 0)
    : SM(sm), Features(features), Diags(diags),
      MaxTokenLength(0), SizeBound(0), CharByteWidth(0), Kind(tok::unknown),
      ResultPtr(ResultBuf.data()), hadError(false){
    init(StringToks, NumStringToks);
  }
    

  bool hadError;

  StringRef GetString() const {
    return StringRef(ResultBuf.data(), GetStringLength());
  }
  unsigned GetStringLength() const { return ResultPtr-ResultBuf.data(); }

  unsigned GetNumStringChars() const {
    return GetStringLength() / CharByteWidth;
  }
  /// getOffsetOfStringByte - This function returns the offset of the
  /// specified byte of the string data represented by Token.  This handles
  /// advancing over escape sequences in the string.
  ///
  /// If the Diagnostics pointer is non-null, then this will do semantic
  /// checking of the string literal and emit errors and warnings.
  unsigned getOffsetOfStringByte(const Token &TheTok, unsigned ByteNo) const;

  bool isAscii() const { return Kind == tok::string_literal; }

  StringRef getUDSuffix() const { return UDSuffixBuf; }

  /// Get the index of a token containing a ud-suffix.
  unsigned getUDSuffixToken() const {
    assert(!UDSuffixBuf.empty() && "no ud-suffix");
    return UDSuffixToken;
  }
  /// Get the spelling offset of the first byte of the ud-suffix.
  unsigned getUDSuffixOffset() const {
    assert(!UDSuffixBuf.empty() && "no ud-suffix");
    return UDSuffixOffset;
  }

private:
  void init(const Token *StringToks, unsigned NumStringToks);
  bool CopyStringFragment(const Token &Tok, const char *TokBegin,
                          StringRef Fragment);
  void DiagnoseLexingError(SourceLocation Loc);
};

}  // end namespace vlang

#endif