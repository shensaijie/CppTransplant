#include "clang/Tooling/Transformer/Transformer.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Tooling/Transformer/RangeSelector.h"
#include "clang/Tooling/Transformer/RewriteRule.h"
#include "clang/Tooling/Transformer/Stencil.h"
#include "llvm/ADT/STLExtras.h"
//#include "llvm/Support/Errc.h"
//#include "llvm/Support/Error.h"
#include "clang/AST/Type.h"
//#include <optional>
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Refactoring.h"

#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/YAMLTraits.h"

#include "llvm/Support/CommandLine.h"
#include <iostream>

#include <cstdio>
#include <memory>
#include <sstream>
#include <string>

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/TargetOptions.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Parse/ParseAST.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Rewrite/Frontend/Rewriters.h"
//#include "llvm/Support/Host.h"
#include "llvm/Support/raw_ostream.h"


#include "clang/Frontend/CommandLineSourceLoc.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Refactoring.h"
#include "clang/Tooling/Refactoring/RefactoringAction.h"
#include "clang/Tooling/Refactoring/RefactoringOptions.h"
#include "clang/Tooling/Refactoring/Rename/RenamingAction.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/raw_ostream.h"
#include <optional>
#include <string>


using namespace clang;
using namespace tooling;
namespace cl = llvm::cl;



namespace {
	using ::clang::transformer::addInclude;
	using ::clang::transformer::applyFirst;
	using ::clang::transformer::before;
	using ::clang::transformer::cat;
	using ::clang::transformer::changeTo;
	using ::clang::transformer::editList;
	using ::clang::transformer::makeRule;
	using ::clang::transformer::member;
	using ::clang::transformer::name;
	using ::clang::transformer::node;
	using ::clang::transformer::noEdits;
	using ::clang::transformer::remove;
	using ::clang::transformer::rewriteDescendants;
	using ::clang::transformer::RewriteRule;
	using ::clang::transformer::RewriteRuleWith;
	using ::clang::transformer::statement;
    using namespace clang;
    using namespace clang::driver;
    using namespace clang::tooling;

    static llvm::cl::OptionCategory ToolingSampleCategory("Tooling Sample");

   //------------------------------------------------------------------------------
    // Clang rewriter sample. Demonstrates:
    //
    // * How to use RecursiveASTVisitor to find interesting AST nodes.
    // * How to use the Rewriter API to rewrite the source code.
    //
    // Eli Bendersky (eliben@gmail.com)
    // This code is in the public domain
    //------------------------------------------------------------------------------



} // namespace
    
//------------------------------------------------------------------------------
// AST matching sample. Demonstrates:
//
// * How to write a simple source tool using libTooling.
// * How to use AST matchers to find interesting AST nodes.
// * How to use the Rewriter API to rewrite the source code.
//
// Eli Bendersky (eliben@gmail.com)
// This code is in the public domain
//------------------------------------------------------------------------------
#include <string>

#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::driver;
using namespace clang::tooling;

static llvm::cl::OptionCategory MatcherSampleCategory("Matcher Sample");

class IfStmtHandler : public MatchFinder::MatchCallback {
public:
  IfStmtHandler(Rewriter &Rewrite) : Rewrite(Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &Result) {
    // The matched 'if' statement was bound to 'ifStmt'.
    if (const IfStmt *IfS = Result.Nodes.getNodeAs<clang::IfStmt>("ifStmt")) {
      const Stmt *Then = IfS->getThen();
      Rewrite.InsertText(Then->getBeginLoc(), "// the 'if' part\n", true, true);

      if (const Stmt *Else = IfS->getElse()) {
        Rewrite.InsertText(Else->getBeginLoc(), "// the 'else' part\n", true,
                           true);
      }
    }
  }

private:
  Rewriter &Rewrite;
};

class IncrementForLoopHandler : public MatchFinder::MatchCallback {
public:
  IncrementForLoopHandler(Rewriter &Rewrite) : Rewrite(Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &Result) {
    const VarDecl *IncVar = Result.Nodes.getNodeAs<VarDecl>("incVarName");
    Rewrite.InsertText(IncVar->getBeginLoc(), "/* increment */", true, true);
  }

private:
  Rewriter &Rewrite;
};

// Implementation of the ASTConsumer interface for reading an AST produced
// by the Clang parser. It registers a couple of matchers and runs them on
// the AST.
class MyASTConsumer : public ASTConsumer {
public:
  MyASTConsumer(Rewriter &R) : HandlerForIf(R), HandlerForFor(R) {
    // Add a simple matcher for finding 'if' statements.
    Matcher.addMatcher(ifStmt().bind("ifStmt"), &HandlerForIf);

    // Add a complex matcher for finding 'for' loops with an initializer set
    // to 0, < comparison in the codition and an increment. For example:
    //
    //  for (int i = 0; i < N; ++i)
    Matcher.addMatcher(
        forStmt(hasLoopInit(declStmt(hasSingleDecl(
                    varDecl(hasInitializer(integerLiteral(equals(0))))
                        .bind("initVarName")))),
                hasIncrement(unaryOperator(
                    hasOperatorName("++"),
                    hasUnaryOperand(declRefExpr(to(
                        varDecl(hasType(isInteger())).bind("incVarName")))))),
                hasCondition(binaryOperator(
                    hasOperatorName("<"),
                    hasLHS(ignoringParenImpCasts(declRefExpr(to(
                        varDecl(hasType(isInteger())).bind("condVarName"))))),
                    hasRHS(expr(hasType(isInteger()))))))
            .bind("forLoop"),
        &HandlerForFor);
  }

  void HandleTranslationUnit(ASTContext &Context) override {
    // Run the matchers when we have the whole TU parsed.
    Matcher.matchAST(Context);
  }

private:
  IfStmtHandler HandlerForIf;
  IncrementForLoopHandler HandlerForFor;
  MatchFinder Matcher;
};

// For each source file provided to the tool, a new FrontendAction is created.
class MyFrontendAction : public ASTFrontendAction {
public:
  MyFrontendAction() {}
  void EndSourceFileAction() override {
    TheRewriter.getEditBuffer(TheRewriter.getSourceMgr().getMainFileID())
        .write(llvm::outs());

    SourceManager &SM = TheRewriter.getSourceMgr();
    llvm::errs() << "\n** EndSourceFileAction for: "
                 << SM.getFileEntryForID(SM.getMainFileID())->getName() << "\n";
    
    std::string File =
        SM.getFileEntryForID(SM.getMainFileID())->getName().str() +
        std::string("Out.cpp");
    std::error_code EC;
    llvm::raw_fd_ostream OS(File, EC, llvm::sys::fs::OF_TextWithCRLF);
    if (EC) {
      llvm::errs() << EC.message() << "\n";
      //return true;
    }
    TheRewriter.getEditBuffer(TheRewriter.getSourceMgr().getMainFileID()).write(OS);
  }

  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                 StringRef file) override {
    TheRewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
    return std::make_unique<MyASTConsumer>(TheRewriter);
  }

private:
  Rewriter TheRewriter;
};

int main(int argc, const char **argv) {
  auto ExpectedParser =
      CommonOptionsParser::create(argc, argv, cl::getGeneralCategory());
      
  ClangTool Tool(ExpectedParser->getCompilations(),
                 ExpectedParser->getSourcePathList());

  return Tool.run(newFrontendActionFactory<MyFrontendAction>().get());
}


//------------------------------------------------------------------------------
// Tooling sample. Demonstrates:
//std::vector<std::unique_ptr<Transformer>> Transformers;
//clang::ast_matchers::MatchFinder matchFinder;
//// Records whether any errors occurred in individual changes.
//int ErrorCount = 0;
//std::vector<std::string> StringMetadata;
//FileContentMappings FileContents = {{"header.h", ""}};
//
//StringRef Flag = "flag";
//RewriteRule Rule = makeRule(
//    cxxMemberCallExpr(
//        on(expr(hasType(cxxRecordDecl(hasName("proto::ProtoCommandLineFlag"))))
//               .bind(Flag)),
//        unless(callee(cxxMethodDecl(hasName("GetProto"))))),
//    changeTo(node(std::string(Flag)), cat("EXPR")));
//
//Rewriter TheRewriter;
//
//auto transformer =
//    std::make_unique<Transformer>(std::move(Rule), MyASTConsumer(TheRewriter));
//
//transformer->registerMatchers(&matchFinder);
//auto Factory = clang::tooling::newFrontendActionFactory(&matchFinder);
// This code is in the public domain
//------------------------------------------------------------------------------
