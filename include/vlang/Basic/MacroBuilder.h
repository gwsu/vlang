//===--- MacroBuilder.h - CPP Macro building utility ------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Defines the vlang::MacroBuilder utility class.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_VLANG_BASIC_MACROBUILDER_H
#define LLVM_VLANG_BASIC_MACROBUILDER_H

#include "vlang/Basic/LLVM.h"
#include "llvm/ADT/Twine.h"
#include "llvm/Support/raw_ostream.h"

namespace vlang {

class MacroBuilder {
  raw_ostream &Out;
public:
  MacroBuilder(raw_ostream &Output) : Out(Output) {}

  /// Append a \`define line for macro of the form "\`define Name Value\n".
  void defineMacro(const Twine &Name, const Twine &Value = "1") {
    Out << "`define " << Name << ' ' << Value << '\n';
  }

  /// Append a \`undef line for Name.  Name should be of the form XXX
  /// and we emit "\`undef XXX".
  void undefineMacro(const Twine &Name) {
    Out << "`undef " << Name << '\n';
  }

  /// Directly append Str and a newline to the underlying buffer.
  void append(const Twine &Str) {
    Out << Str << '\n';
  }
};

}  // end namespace vlang

#endif
