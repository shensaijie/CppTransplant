#pragma once

#include "clang/Tooling/Transformer/Transformer.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Tooling/Transformer/RangeSelector.h"
#include "clang/Tooling/Transformer/RewriteRule.h"
#include "clang/Tooling/Transformer/Stencil.h"
#include "llvm/ADT/STLExtras.h"
//#include "llvm/Support/Errc.h"
//#include "llvm/Support/Error.h"
//#include "clang/AST/Type.h"
//#include <optional>
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Refactoring.h"

#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/YAMLTraits.h"

#include <iostream>

#include <cstdio>
#include <memory>
#include <sstream>
#include <string>
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Frontend/CompilerInstance.h"
#include "MatchCallbackBat.h"
#include "ASTConsumerBat.h"
#include "filesystem"

namespace fs = std::filesystem;
using namespace clang::transformer;

namespace cl = llvm::cl;
using namespace clang;
using namespace tooling;
using namespace clang::ast_matchers;
using namespace clang::driver;
using namespace clang::tooling;

using namespace std;


static llvm::cl::OptionCategory ToolingSampleCategory("Tooling Sample");
static llvm::cl::OptionCategory MatcherSampleCategory("Matcher Sample");



//#include "MatchCallbackBat.h"
//#include "ASTConsumerBat.h"
//#include "FrontendAction.h"