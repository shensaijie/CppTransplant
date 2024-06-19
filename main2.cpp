#include "all.h"
using namespace clang::transformer;
using namespace clang;
using namespace tooling;
using namespace ast_matchers;



std::vector<clang::tooling::AtomicChange> Changes; // �洢���������
int ErrorCount = 0; // ���������
std::vector<std::string> ErrorMessages; // �洢������Ϣ������

Transformer::ChangeSetConsumer consumer()
{
	return [](llvm::Expected<llvm::MutableArrayRef<clang::tooling::AtomicChange>> C) {
		if (C) {
			// �ɹ�ʱ��������ƶ��� Changes ������
			Changes.insert(Changes.end(), std::make_move_iterator(C->begin()),
				std::make_move_iterator(C->end()));

			std::cout << "1111\n";
		}
		else {
			// �������󣺽�������Ϣ�洢���������������Ǵ�ӡ
			auto Err = C.takeError();
			ErrorMessages.push_back(llvm::toString(std::move(Err)));
			++ErrorCount;
		}
		};
}

int main(int argc, const char** argv)
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
	int main()
	{
		//CArray array;
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


	auto transformer = std::make_unique<Transformer>(std::move(Rule), consumer());

	transformer->registerMatchers(&matchFinder);
	auto Factory = clang::tooling::newFrontendActionFactory(&matchFinder);

	std::string Code = ("#include \"header.h\"\n" + Input);
	// ʹ���޸ĺ�Ĵ������й���
	if (!clang::tooling::runToolOnCodeWithArgs(
		Factory->create(), Code, std::vector<std::string>(), "input.cc",
		"clang-tool", std::make_shared<clang::PCHContainerOperations>(),
		FileContents)) {
		llvm::errs() << "���й���ʧ�ܡ�\n";
		//return 0; // ʧ��ʱ��ǰ����
	}

	// ���ռ����ĸ���Ӧ���ڴ���
	auto ChangedCode = clang::tooling::applyAtomicChanges(
		"input.cc", Code, Changes, clang::tooling::ApplyChangesSpec());

	// ����Ƿ�ɹ�Ӧ�ø���
	if (!ChangedCode) {
		llvm::errs() << "Ӧ�ø���ʧ�ܣ�"
			<< llvm::toString(ChangedCode.takeError()) << "\n";
		//return std::nullopt; // ʧ��ʱ��ǰ����
	}

	// ������в��趼�ɹ����򷵻�ת����Ĵ���
	std::cout << "���н����\n" << ChangedCode.get();

	return 0;
}
