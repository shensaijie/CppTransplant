#pragma once
#include <format>

namespace fs = std::filesystem;
using namespace clang::transformer;

namespace cl = llvm::cl;
using namespace clang;
using namespace tooling;
using namespace clang::ast_matchers;
using namespace clang::driver;
using namespace clang::tooling;

using namespace std;

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

		batAddClassRename("CStringArray", "std::vector<std::string>");
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
	void batFunctionRename(string in, string out, std::string formatStr, std::vector<int> argv)
	{
		//std::format(formatStr, 1, 2);

		/*auto args = std::make_format_args(1.0);
		auto args2 = std::make_format_args(1.0);
		args + args2;
		std::string result = std::format("{}1 {}2 {}3", 1);
		result = std::format("{}1 {}2 {}3", result);
		result = std::format("{}1 {}2 {}3", 1);*/

		//if (const CallExpr* callExpr = Result.Nodes.getNodeAs<CallExpr>("functionCall"))
		//{
		//	SourceManager& srcMgr = *Result.SourceManager;
		//	LangOptions langOpts = Result.Context->getLangOpts();

		//	// 获取函数调用的范围
		//	SourceRange callRange = callExpr->getSourceRange();
		//	std::string callText = Lexer::getSourceText(CharSourceRange::getTokenRange(callRange), srcMgr, langOpts).str();

		//	// 使用正则表达式替换函数调用
		//	std::regex expr("oldFunction\\((.*)\\)");
		//	std::string replacedText = std::regex_replace(callText, expr, "newFunction($1)");

		//	// 输出替换后的代码
		//	llvm::outs() << "Original call: " << callText << "\n";
		//	llvm::outs() << "Replaced call: " << replacedText << "\n";

		//	// 当然，你可以在这里写入新的代码文件或进行其他处理
		//}
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
		//Matcher.addMatcher(callExpr().bind("call"), new FunctionCallHandler(R));
		//Matcher.addMatcher(functionDecl(isDefinition()).bind("funcDecl"), new FunctionCallHandler(R));
		//Matcher.addMatcher(callExpr(callee(functionDecl(hasName("GetDlgItem")))).bind("funcDecl"), new FunctionCallHandler(R));
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
