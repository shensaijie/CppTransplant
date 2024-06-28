#pragma once
#include "all.h"

namespace fs = std::filesystem;
using namespace clang::transformer;

namespace cl = llvm::cl;
using namespace clang;
using namespace tooling;
using namespace clang::ast_matchers;
using namespace clang::driver;
using namespace clang::tooling;

using namespace std;

// For each source file provided to the tool, a new FrontendAction is created.
class MyFrontendAction : public ASTFrontendAction
{
public:
	MyFrontendAction() {}
	void EndSourceFileAction() override
	{
		/*TheRewriter.getEditBuffer(TheRewriter.getSourceMgr().getMainFileID()).write(llvm::outs());*/

		SourceManager& SM = TheRewriter.getSourceMgr();
		/*llvm::errs() << "\n** EndSourceFileAction for: " << SM.getFileEntryForID(SM.getMainFileID())->getName() << "\n";*/

		std::string File = SM.getFileEntryForID(SM.getMainFileID())->getName().str();

		fs::path myPath = File;
		std::error_code EC;
		llvm::raw_fd_ostream OS(outPutDir + myPath.filename().string(), EC, llvm::sys::fs::OF_Text);
		if (EC)
		{
			llvm::errs() << EC.message() << "\n";
		}
		TheRewriter.getEditBuffer(TheRewriter.getSourceMgr().getMainFileID()).write(OS);
	}

	std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance& CI, StringRef file) override
	{
		TheRewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
		return std::make_unique<MyASTConsumer>(TheRewriter);
	}

protected:
	Rewriter TheRewriter;
};

class SmallHumpAction : public MyFrontendAction
{
public:
	SmallHumpAction() {}
	std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance& CI, StringRef file) override
	{
		TheRewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
		return std::make_unique<MyASTConsumer2>(TheRewriter);
	}
};
