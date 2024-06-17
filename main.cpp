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
#include <clang/Tooling/CommonOptionsParser.h>

#include "llvm/Support/CommandLine.h"
#include <iostream>

using namespace clang;
using namespace tooling;
using namespace ast_matchers;

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

	std::vector<clang::tooling::AtomicChange> Changes; // 存储变更的容器
	int ErrorCount = 0; // 错误计数器
	std::vector<std::string> ErrorMessages; // 存储错误信息的容器

	Transformer::ChangeSetConsumer consumer() 
	{
		return [](llvm::Expected<llvm::MutableArrayRef<clang::tooling::AtomicChange>> C) {
			if (C) {
				// 成功时，将变更移动到 Changes 容器中
				Changes.insert(Changes.end(), std::make_move_iterator(C->begin()),
					std::make_move_iterator(C->end()));

				std::cout << "1111\n";
			}
			else {
				// 处理错误：将错误信息存储起来，而不仅仅是打印
				auto Err = C.takeError();
				ErrorMessages.push_back(llvm::toString(std::move(Err)));
				++ErrorCount;
			}
			};
	}

	int main2(int argc, const char** argv)
	{

	std::vector<std::unique_ptr<Transformer>> Transformers;
	clang::ast_matchers::MatchFinder matchFinder;
	// Records whether any errors occurred in individual changes.
	int ErrorCount = 0;
	std::vector<std::string> StringMetadata;
	FileContentMappings FileContents = { {"header.h", ""} };
	
	StringRef Flag = "flag";
	RewriteRule Rule = makeRule(
		cxxMemberCallExpr(on(expr(hasType(cxxRecordDecl(
			hasName("proto::ProtoCommandLineFlag"))))
			.bind(Flag)),
			unless(callee(cxxMethodDecl(hasName("GetProto"))))),
		changeTo(node(std::string(Flag)), cat("EXPR")));

	std::string Input = R"cc(
namespace proto {
    class ProtoCommandLineFlag{
	public:
		int foo() {return 0;}
	};
}

    proto::ProtoCommandLineFlag flag;
    int x = flag.foo();
    //int y = flag.GetProto().foo();
  )cc";
	std::string Expected = R"cc(
    proto::ProtoCommandLineFlag flag;
    int x = EXPR.foo();
    int y = flag.GetProto().foo();
  )cc";


	auto transformer =std::make_unique<Transformer>(std::move(Rule), consumer());

	transformer->registerMatchers(&matchFinder);
	auto Factory = clang::tooling::newFrontendActionFactory(&matchFinder);

	std::string Code = ("#include \"header.h\"\n" + Input);
	// 使用修改后的代码运行工具
	if (!clang::tooling::runToolOnCodeWithArgs(
		Factory->create(), Code, std::vector<std::string>(), "input.cc",
		"clang-tool", std::make_shared<clang::PCHContainerOperations>(),
		FileContents)) {
		llvm::errs() << "运行工具失败。\n";
		//return 0; // 失败时提前返回
	}

	// 将收集到的更改应用于代码
	auto ChangedCode = clang::tooling::applyAtomicChanges(
		"input.cc", Code, Changes, clang::tooling::ApplyChangesSpec());

	// 检查是否成功应用更改
	if (!ChangedCode) {
		llvm::errs() << "应用更改失败："
			<< llvm::toString(ChangedCode.takeError()) << "\n";
		//return std::nullopt; // 失败时提前返回
	}

	// 如果所有步骤都成功，则返回转换后的代码
	std::cout << "运行结果：\n" << ChangedCode.get();

	return 0;

	/*llvm::cl::OptionCategory MyToolCategory("My Tool Options");

	auto OptionsParser = clang::tooling::CommonOptionsParser::create(argc, argv, MyToolCategory);
	clang::tooling::ClangTool Tool(OptionsParser->getCompilations(), OptionsParser->getSourcePathList());

	clang::ast_matchers::MatchFinder Finder;

	Finder.addMatcher(clang::ast_matchers::functionDecl().bind(Flag), transformer.get());

	return Tool.run(clang::tooling::newFrontendActionFactory(&Finder).get());
	

	return 0;*/
	}
}

int main(int argc, const char** argv)
{
	return main2(argc, argv);
	return 0;
}