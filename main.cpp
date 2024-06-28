#include "all.h"
#include "MatchCallbackBat.h"
#include "ASTConsumerBat.h"
#include "FrontendAction.h"

namespace fs = std::filesystem;
using namespace clang::transformer;

namespace cl = llvm::cl;
using namespace clang;
using namespace tooling;
using namespace clang::ast_matchers;
using namespace clang::driver;
using namespace clang::tooling;

using namespace std;



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
	//AdjustedArgs.push_back("-DWIN32");
	//AdjustedArgs.push_back("-D_DEBUG");
	//AdjustedArgs.push_back("-D_WINDOWS");
	//AdjustedArgs.push_back("-D_WINDLL");
	//AdjustedArgs.push_back("-D_MBCS");
	//AdjustedArgs.push_back("-D_AFXEXT");
	AdjustedArgs.push_back("-D_AFXDLL");
	AdjustedArgs.push_back("-D_DLL");
	AdjustedArgs.push_back("-D_MT");
	//AdjustedArgs.push_back("-D_M_ALPHA");
	AdjustedArgs.push_back("-U_MSC_VER");
	AdjustedArgs.push_back("-D_MSC_VER=1200");
	AdjustedArgs.push_back("-std=c++98");
	//AdjustedArgs.push_back("-D_AFXWIN_INLINE");
	AdjustedArgs.push_back("-D__RPC_FAR=int");
	AdjustedArgs.push_back("-D_M_PPC");
	//AdjustedArgs.push_back("-D_MAC");
	AdjustedArgs.push_back("-D_SIZE_T_DEFINED");
	// 将ArgumentsAdjuster应用于ClangTool
	Tool.appendArgumentsAdjuster(getInsertArgumentAdjuster(AdjustedArgs, ArgumentInsertPosition::END));

	Tool.run(newFrontendActionFactory<MyFrontendAction>().get());
	//Tool.run(newFrontendActionFactory<MyFrontendAction2>().get());
	return 0;
}
