#include "all.h"

static llvm::cl::OptionCategory ToolingSampleCategory("Tooling Sample");
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

class ClassUsageRenamer : public MatchFinder::MatchCallback {
public:
	ClassUsageRenamer(Rewriter &Rewrite) : Rewrite(Rewrite) {}

	virtual void run(const MatchFinder::MatchResult &Result) {
		if (const auto *ClassDecl = Result.Nodes.getNodeAs<CXXRecordDecl>("classDecl")) {
			// 替换类定义
			if (ClassDecl->isThisDeclarationADefinition()) {
				SourceRange classRange = ClassDecl->getSourceRange();
				Rewrite.ReplaceText(classRange.getBegin(), ClassDecl->getNameAsString().length(), "NewClassName");
			}
		}
		else if (const auto *NewExpr = Result.Nodes.getNodeAs<CXXNewExpr>("newExpr")) {
			// 替换类的构造函数调用
			SourceLocation startLoc = NewExpr->getBeginLoc();
			if (startLoc.isValid()) {
				Rewrite.ReplaceText(startLoc, NewExpr->getType()->getAsCXXRecordDecl()->getNameAsString().length(), "NewClassName");
			}
		}
		else if (const auto *CallExpr = Result.Nodes.getNodeAs<CXXMemberCallExpr>("memberCallExpr")) {
			// 替换成员函数调用（示例仅为了展示，实际操作可能需要更复杂的处理）
			// 这里的处理会非常依赖于具体的上下文和需求
		}
		else if (const VarDecl *VarDeclaration = Result.Nodes.getNodeAs<VarDecl>("arrayDecl")) {
			// 检查变量的类型是否确实为Carray
			QualType qt = VarDeclaration->getType();
			if (qt.getAsString() == "CArray") {
				// 获取变量类型的源位置范围
				SourceRange typeRange = VarDeclaration->getTypeSourceInfo()->getTypeLoc().getSourceRange();
				// 执行替换
				Rewrite.ReplaceText(typeRange, "std::vector<int>");
			}
		}
	}

private:
	Rewriter &Rewrite;
};

// CStringArray to QStringList
class ClassRenamer_CStringArray : public MatchFinder::MatchCallback {
public:
	ClassRenamer_CStringArray(Rewriter &Rewrite) : Rewrite(Rewrite) {}

	virtual void run(const MatchFinder::MatchResult &Result) {
		if (const auto *NewExpr = Result.Nodes.getNodeAs<CXXNewExpr>("newExpr")) {
			// 替换类的构造函数调用
			SourceLocation startLoc = NewExpr->getBeginLoc();
			if (startLoc.isValid()) {
				Rewrite.ReplaceText(startLoc, NewExpr->getType()->getAsCXXRecordDecl()->getNameAsString().length(), "NewClassName");
			}
		}
		else if (const auto *CallExpr = Result.Nodes.getNodeAs<CXXMemberCallExpr>("memberCallExpr")) {
			// 替换成员函数调用（示例仅为了展示，实际操作可能需要更复杂的处理）
			// 这里的处理会非常依赖于具体的上下文和需求
		}
		else if (const VarDecl *VarDeclaration = Result.Nodes.getNodeAs<VarDecl>("varDecl")) {
			SourceRange typeRange = VarDeclaration->getTypeSourceInfo()->getTypeLoc().getSourceRange();
			Rewrite.ReplaceText(typeRange, "QStringList");
			
		}
	}

private:
	Rewriter &Rewrite;
};

// Implementation of the ASTConsumer interface for reading an AST produced
// by the Clang parser. It registers a couple of matchers and runs them on
// the AST.
class MyASTConsumer : public ASTConsumer {
public:
	MyASTConsumer(Rewriter &R) : HandlerForIf(R), HandlerForFor(R), Renamer(R){
		// Add a simple matcher for finding 'if' statements.
		//Matcher.addMatcher(ifStmt().bind("ifStmt"), &HandlerForIf);

		//// Add a complex matcher for finding 'for' loops with an initializer set
		//// to 0, < comparison in the codition and an increment. For example:
		////
		////  for (int i = 0; i < N; ++i)
		//Matcher.addMatcher(
		//	forStmt(hasLoopInit(declStmt(hasSingleDecl(
		//		varDecl(hasInitializer(integerLiteral(equals(0))))
		//		.bind("initVarName")))),
		//		hasIncrement(unaryOperator(
		//			hasOperatorName("++"),
		//			hasUnaryOperand(declRefExpr(to(
		//				varDecl(hasType(isInteger())).bind("incVarName")))))),
		//		hasCondition(binaryOperator(
		//			hasOperatorName("<"),
		//			hasLHS(ignoringParenImpCasts(declRefExpr(to(
		//				varDecl(hasType(isInteger())).bind("condVarName"))))),
		//			hasRHS(expr(hasType(isInteger()))))))
		//		.bind("forLoop"),
		//	&HandlerForFor);

		// 替换类调用
		
		Matcher.addMatcher(cxxRecordDecl(hasName("CArray")).bind("classDecl"), &Renamer);
		Matcher.addMatcher(cxxNewExpr(has(declRefExpr(to(cxxRecordDecl(hasName("CArray")))))).bind("newExpr"), &Renamer);
		Matcher.addMatcher(varDecl(hasType(namedDecl(hasName("CArray")))).bind("arrayDecl"), &Renamer);
		
		
		Matcher.addMatcher(varDecl(hasType(namedDecl(hasName("CStringArray")))).bind("varDecl"), new ClassRenamer_CStringArray(R));
	}

	void HandleTranslationUnit(ASTContext &Context) override {
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
class MyFrontendAction : public ASTFrontendAction {
public:
	MyFrontendAction() {}
	void EndSourceFileAction() override {
		TheRewriter.getEditBuffer(TheRewriter.getSourceMgr().getMainFileID())
			.write(llvm::outs());

		SourceManager &SM = TheRewriter.getSourceMgr();
		/*llvm::errs() << "\n** EndSourceFileAction for: "
			<< SM.getFileEntryForID(SM.getMainFileID())->getName() << "\n";*/

		std::string File =
			SM.getFileEntryForID(SM.getMainFileID())->getName().str() +
			std::string("Out.cpp");
		std::error_code EC;
		llvm::raw_fd_ostream OS(File, EC, llvm::sys::fs::OF_Text);
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

	argv[1] = "E:\\AWorkSpace\\CppTransplant\\test\\QuickNote.cpp";
	auto ExpectedParser =
		CommonOptionsParser::create(argc, argv, cl::getGeneralCategory());

	ClangTool Tool(ExpectedParser->getCompilations(),
				   ExpectedParser->getSourcePathList());

	// 添加额外的包含路径
	CommandLineArguments AdjustedArgs;
	AdjustedArgs.push_back("-IE:\\AWorkSpace\\Eps2020\\include\\BaseInclude");
	AdjustedArgs.push_back("-IE:\\AWorkSpace\\Eps2020\\include");
	AdjustedArgs.push_back("-IE:\\AWorkSpace\\Eps2020\\includesm");
	AdjustedArgs.push_back("-IC:\\Program Files (x86)\\Microsoft Visual Studio\\VC98\\Include");
	AdjustedArgs.push_back("-IC:\\Program Files(x86)\\Microsoft Visual Studio\\VC98\\MFC\\Include");
	AdjustedArgs.push_back("-IE:\\AWorkSpace\\Eps2020\\DLL_C\\SSExtend");

	// 将ArgumentsAdjuster应用于ClangTool
	Tool.appendArgumentsAdjuster(getInsertArgumentAdjuster(AdjustedArgs, ArgumentInsertPosition::END));

	return Tool.run(newFrontendActionFactory<MyFrontendAction>().get());
}
