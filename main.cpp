#include "all.h"
#include "MatchCallbackBat.h"
#include "ASTConsumerBat.h"
#include "filesystem"

namespace fs = std::filesystem;
using namespace clang::transformer;

static llvm::cl::OptionCategory ToolingSampleCategory("Tooling Sample");
static llvm::cl::OptionCategory MatcherSampleCategory("Matcher Sample");

static std::string outPutDir("E:\\AWorkSpace\\CppTransplant\\working\\outFiles\\");

class IfStmtHandler : public MatchFinder::MatchCallback
{
public:
	IfStmtHandler(Rewriter& Rewrite) : Rewrite(Rewrite) {}

	virtual void run(const MatchFinder::MatchResult& Result)
	{
		// The matched 'if' statement was bound to 'ifStmt'.
		if (const IfStmt* IfS = Result.Nodes.getNodeAs<clang::IfStmt>("ifStmt"))
		{
			const Stmt* Then = IfS->getThen();
			Rewrite.InsertText(Then->getBeginLoc(), "// the 'if' part\n", true, true);

			if (const Stmt* Else = IfS->getElse())
			{
				Rewrite.InsertText(Else->getBeginLoc(), "// the 'else' part\n", true,
					true);
			}
		}
	}

private:
	Rewriter& Rewrite;
};

class IncrementForLoopHandler : public MatchFinder::MatchCallback
{
public:
	IncrementForLoopHandler(Rewriter& Rewrite) : Rewrite(Rewrite) {}

	virtual void run(const MatchFinder::MatchResult& Result)
	{
		const VarDecl* IncVar = Result.Nodes.getNodeAs<VarDecl>("incVarName");
		Rewrite.InsertText(IncVar->getBeginLoc(), "/* increment */", true, true);
	}

private:
	Rewriter& Rewrite;
};

class ClassUsageRenamer : public MatchFinder::MatchCallback
{
public:
	ClassUsageRenamer(Rewriter& Rewrite) : Rewrite(Rewrite) {}

	virtual void run(const MatchFinder::MatchResult& Result)
	{
		if (const auto* ClassDecl =
			Result.Nodes.getNodeAs<CXXRecordDecl>("classDecl"))
		{
			// 替换类定义
			if (ClassDecl->isThisDeclarationADefinition())
			{
				SourceRange classRange = ClassDecl->getSourceRange();
				Rewrite.ReplaceText(classRange.getBegin(),
					ClassDecl->getNameAsString().length(),
					"NewClassName");
			}
		}
		else if (const auto* NewExpr =
			Result.Nodes.getNodeAs<CXXNewExpr>("newExpr"))
		{
			// 替换类的构造函数调用
			SourceLocation startLoc = NewExpr->getBeginLoc();
			if (startLoc.isValid())
			{
				Rewrite.ReplaceText(startLoc,
					NewExpr->getType()
					->getAsCXXRecordDecl()
					->getNameAsString()
					.length(),
					"NewClassName");
			}
		}
		else if (const auto* CallExpr = Result.Nodes.getNodeAs<CXXMemberCallExpr>(
			"memberCallExpr"))
		{
			// 替换成员函数调用（示例仅为了展示，实际操作可能需要更复杂的处理）
			// 这里的处理会非常依赖于具体的上下文和需求
		}
		else if (const VarDecl* VarDeclaration =
			Result.Nodes.getNodeAs<VarDecl>("arrayDecl"))
		{
			// 检查变量的类型是否确实为Carray
			QualType qt = VarDeclaration->getType();
			if (qt.getAsString() == "CArray")
			{
				// 获取变量类型的源位置范围
				SourceRange typeRange =
					VarDeclaration->getTypeSourceInfo()->getTypeLoc().getSourceRange();
				// 执行替换
				Rewrite.ReplaceText(typeRange, "std::vector<int>");
			}
		}
	}

private:
	Rewriter& Rewrite;
};

// CStringArray to QStringList
class ClassRenamer_CStringArray : public MatchFinder::MatchCallback
{
public:
	ClassRenamer_CStringArray(Rewriter& Rewrite) : Rewrite(Rewrite) {}

	virtual void run(const MatchFinder::MatchResult& Result)
	{
		if (const auto* NewExpr = Result.Nodes.getNodeAs<CXXNewExpr>("newExpr"))
		{
			// 替换类的构造函数调用
			SourceLocation startLoc = NewExpr->getBeginLoc();
			if (startLoc.isValid())
			{
				Rewrite.ReplaceText(startLoc,
					NewExpr->getType()
					->getAsCXXRecordDecl()
					->getNameAsString()
					.length(),
					"NewClassName");
			}
		}
		else if (const auto* CallExpr = Result.Nodes.getNodeAs<CXXMemberCallExpr>(
			"memberCallExpr"))
		{
			if (const CXXMethodDecl* MD = CallExpr->getMethodDecl())
			{
				std::cout << "pi------------\n";
				auto FuncName = MD->getNameAsString();

				if (FuncName == "Add")
					Rewrite.ReplaceText(CallExpr->getCallee()->getExprLoc(), 3, "push_back");
				else if (FuncName == "GetSize")
				{
					Rewrite.ReplaceText(CallExpr->getCallee()->getExprLoc(), 7, "size");
				}
			}

		}
		else if (const VarDecl* VarDeclaration = Result.Nodes.getNodeAs<VarDecl>("varDecl"))
		{
			SourceRange typeRange =
				VarDeclaration->getTypeSourceInfo()->getTypeLoc().getSourceRange();
			Rewrite.ReplaceText(typeRange, "std::vector<std::string>");
		}
	}

private:
	Rewriter& Rewrite;
};

// 函数转小驼峰
class FunctionCallHandler : public MatchFinder::MatchCallback
{
public:
	explicit FunctionCallHandler(Rewriter& Rewrite) : Rewrite(Rewrite) {}

	virtual void run(const MatchFinder::MatchResult& Result)
	{
		std::string FuncName;

		if (const CallExpr* Call = Result.Nodes.getNodeAs<clang::CallExpr>("call"))
		{
			llvm::outs() << "Matched variable: " << Call->getCalleeDecl()<< "\n";
			// Rewrite.InsertText(loc, "X");
			// Rewrite.InsertText(Call->getRParenLoc(), "Y");
			// Rewrite.InsertText(Call->getCallee()->getExprLoc(), "Z");
			// Rewrite.InsertTextAfter(Call->getCallee()->getEndLoc(), "K");
			// Rewrite.InsertTextBefore(Call->getCallee()->getEndLoc(), "K");
			// Rewrite.InsertTextAfterToken(Call->getCallee()->getEndLoc(), "F");
			if (const FunctionDecl* FD = Call->getDirectCallee())
			{
				FuncName = FD->getNameInfo().getName().getAsString();
			}

			if (FuncName.substr(0, 8).compare("operator") == 0)
				return;

			if (FuncName.size())
			{
				FuncName[0] = std::tolower(FuncName[0]);
				Rewrite.ReplaceText(Call->getCallee()->getExprLoc(), FuncName.size(), FuncName);
			}
		}
		else if (const FunctionDecl* FD = Result.Nodes.getNodeAs<clang::FunctionDecl>("funcDecl"))
		{
			llvm::outs() << "Matched function: " << FD->getNameInfo().getName().getAsString() << "\n";
			if (Result.Context->getSourceManager().isInMainFile(FD->getBeginLoc()))
			{
				//llvm::outs() << "Function name: " << FD->getNameAsString() << "\n";
				FuncName = FD->getNameInfo().getName().getAsString();
				if (FuncName.size())
				{
					FuncName[0] = std::tolower(FuncName[0]);
					//std::cout << FuncName;
					Rewrite.ReplaceText(FD->getNameInfo().getSourceRange(), FuncName);
				}
			}
		}
	}

private:
	Rewriter& Rewrite;
};

// Implementation of the ASTConsumer interface for reading an AST produced
// by the Clang parser. It registers a couple of matchers and runs them on
// the AST.
class MyASTConsumer : public ASTConsumer
{
public:
	MyASTConsumer(Rewriter& R) : HandlerForIf(R), HandlerForFor(R), Renamer(R), Rewrite(R)
	{
		// Add a simple matcher for finding 'if' statements.
		// Matcher.addMatcher(ifStmt().bind("ifStmt"), &HandlerForIf);

		// 替换类调用
		// Matcher.addMatcher(cxxRecordDecl(hasName("CArray")).bind("classDecl"), &Renamer);
		// Matcher.addMatcher(cxxNewExpr(has(declRefExpr(to(cxxRecordDecl(hasName("CArray")))))).bind("newExpr"),&Renamer);
		// Matcher.addMatcher(varDecl(hasType(namedDecl(hasName("CArray")))).bind("arrayDecl"),&Renamer);

		//Matcher.addMatcher(cxxMemberCallExpr(callee(cxxMethodDecl(ofClass(hasName("CStringArray"))))).bind("memberCallExpr"),new ClassRenamer_CStringArray(R));
		//Matcher.addMatcher(varDecl(hasType(namedDecl(hasName("CStringArray")))).bind("varDecl"), new ClassRenamer_CStringArray(R));

		//Matcher.addMatcher(varDecl(hasType(namedDecl(hasName("CStringArray")))).bind("varDecl"), new ClassRenameBat(R, string("CStringArray"), string("std::vector<std::string>")));


		//Matcher.addMatcher(callExpr().bind("call"), new FunctionCallHandler(R));
		//Matcher.addMatcher(functionDecl(isDefinition()).bind("funcDecl"), new FunctionCallHandler(R));

		//batAddClassRename("CStringArray", "std::vector<std::string>");
		batAddClassRename("CString", "std::string");
	}

	void HandleTranslationUnit(ASTContext& Context) override
	{
		// Run the matchers when we have the whole TU parsed.
		Matcher.matchAST(Context);
	}

private:
	void batAddClassRename(string in, string out)
	{
		Matcher.addMatcher(varDecl(hasType(namedDecl(hasName(in)))).bind("varDecl"), new ClassRenameBat(Rewrite, in, out));
	}

private:
	IfStmtHandler HandlerForIf;
	IncrementForLoopHandler HandlerForFor;
	ClassUsageRenamer Renamer;
	MatchFinder Matcher;
	Rewriter& Rewrite;
};

class MyASTConsumer2 : public ASTConsumer
{
public:
	MyASTConsumer2(Rewriter& R) : HandlerForIf(R), HandlerForFor(R), Renamer(R)
	{
		Matcher.addMatcher(callExpr().bind("call"), new FunctionCallHandler(R));
		Matcher.addMatcher(functionDecl(isDefinition()).bind("funcDecl"), new FunctionCallHandler(R));
	}

	void HandleTranslationUnit(ASTContext& Context) override
	{
		// Run the matchers when we have the whole TU parsed.
		Matcher.matchAST(Context);
	}

private:
	IfStmtHandler HandlerForIf;
	IncrementForLoopHandler HandlerForFor;
	ClassUsageRenamer Renamer;
	MatchFinder Matcher;
};


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

	std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance& CI,
		StringRef file) override
	{
		TheRewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
		return std::make_unique<MyASTConsumer>(TheRewriter);
	}

protected:
	Rewriter TheRewriter;
};

class MyFrontendAction2 : public MyFrontendAction
{
public:
	MyFrontendAction2() {}
	std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance& CI,
		StringRef file) override
	{
		TheRewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
		return std::make_unique<MyASTConsumer2>(TheRewriter);
	}
};

int main(int argc, const char** argv)
{
	//argv[1] = "E:\\AWorkSpace\\NexGenEps2\\DLL_C\\SSEdit\\QuickNoteIn.h";
	auto ExpectedParser = CommonOptionsParser::create(argc, argv, cl::getGeneralCategory());

	ClangTool Tool(ExpectedParser->getCompilations(), ExpectedParser->getSourcePathList());

	// 添加额外的包含路径
	CommandLineArguments AdjustedArgs;
	AdjustedArgs.push_back("-IE:\\AWorkSpace\\Eps2020\\include\\BaseInclude");
	AdjustedArgs.push_back("-IE:\\AWorkSpace\\Eps2020\\include\\AdjustBase");
	AdjustedArgs.push_back("-IE:\\AWorkSpace\\Eps2020\\include\\ado");
	AdjustedArgs.push_back("-IE:\\AWorkSpace\\Eps2020\\include\\BaseInclude");
	AdjustedArgs.push_back("-IE:\\AWorkSpace\\Eps2020\\include\\BCGCBPro");
	AdjustedArgs.push_back("-IE:\\AWorkSpace\\Eps2020\\include\\cairo");
	AdjustedArgs.push_back("-IE:\\AWorkSpace\\Eps2020\\include\\DataGrid");
	AdjustedArgs.push_back("-IE:\\AWorkSpace\\Eps2020\\include\\EditInclude");
	AdjustedArgs.push_back("-IE:\\AWorkSpace\\Eps2020\\include\\epshouse");
	AdjustedArgs.push_back("-IE:\\AWorkSpace\\Eps2020\\include\\ExchangeInclude");
	AdjustedArgs.push_back("-IE:\\AWorkSpace\\Eps2020\\include\\FunctionCore");
	AdjustedArgs.push_back("-IE:\\AWorkSpace\\Eps2020\\include\\GDIPlus");
	AdjustedArgs.push_back("-IE:\\AWorkSpace\\Eps2020\\include\\glbase");
	AdjustedArgs.push_back("-IE:\\AWorkSpace\\Eps2020\\include\\InterfaceInclude");
	AdjustedArgs.push_back("-IE:\\AWorkSpace\\Eps2020\\include\\oci");
	AdjustedArgs.push_back("-IE:\\AWorkSpace\\Eps2020\\include\\ssadobase");
	AdjustedArgs.push_back("-IE:\\AWorkSpace\\Eps2020\\include\\SSctrlbar");
	AdjustedArgs.push_back("-IE:\\AWorkSpace\\Eps2020\\include\\SSDBCore");
	AdjustedArgs.push_back("-IE:\\AWorkSpace\\Eps2020\\include\\SSEditBase");
	AdjustedArgs.push_back("-IE:\\AWorkSpace\\Eps2020\\include\\SSPipe");
	AdjustedArgs.push_back("-IE:\\AWorkSpace\\Eps2020\\include\\SSProject");
	AdjustedArgs.push_back("-IE:\\AWorkSpace\\Eps2020\\include\\UserFunc");
	AdjustedArgs.push_back("-IE:\\AWorkSpace\\Eps2020\\include\\VSInterface");
	AdjustedArgs.push_back("-IE:\\AWorkSpace\\Eps2020\\include\\_SSEnvironment");
	AdjustedArgs.push_back("-IE:\\AWorkSpace\\Eps2020\\include\\_UserInterface");
	AdjustedArgs.push_back("-IE:\\AWorkSpace\\Eps2020\\includesm");
	// AdjustedArgs.push_back("-IC:\\Program Files(x86)\\Microsoft Visual
	// Studio\\VC98\\atl\\Include"); AdjustedArgs.push_back("-IC:\\Program
	// Files(x86)\\Microsoft Visual Studio\\VC98\\MFC\\Include");
	// AdjustedArgs.push_back("-IC:\\Program Files (x86)\\Microsoft Visual
	// Studio\\VC98\\Include");
	// AdjustedArgs.push_back("-IE:\\AWorkSpace\\Eps2020\\DLL_C\\SSExtend");
	AdjustedArgs.push_back("-IE:\\AWorkSpace\\CppTransplant\\working");
	// AdjustedArgs.push_back("-IE:\\AWorkSpace\\Eps2020\\include\\BCGCBPro");
	AdjustedArgs.push_back("-DWIN32");
	AdjustedArgs.push_back("-D_DEBUG");
	AdjustedArgs.push_back("-D_WINDOWS");
	AdjustedArgs.push_back("-D_WINDLL");
	AdjustedArgs.push_back("-D_MBCS");
	AdjustedArgs.push_back("-D_AFXEXT");
	AdjustedArgs.push_back("-D_AFXDLL");
	AdjustedArgs.push_back("-D_DLL");
	AdjustedArgs.push_back("-D_MT");
	AdjustedArgs.push_back("-D_M_ALPHA");
	AdjustedArgs.push_back("-D_SIZE_T_DEFINED");
	// 将ArgumentsAdjuster应用于ClangTool
	Tool.appendArgumentsAdjuster(getInsertArgumentAdjuster(AdjustedArgs, ArgumentInsertPosition::END));

	Tool.run(newFrontendActionFactory<MyFrontendAction2>().get());
	//Tool.run(newFrontendActionFactory<MyFrontendAction2>().get());
	return 0;
}
