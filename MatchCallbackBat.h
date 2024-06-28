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

static std::string outPutDir("E:\\AWorkSpace\\CppTransplant\\working\\outFiles\\");

// �������滻
class ClassRenameBat : public MatchFinder::MatchCallback
{
public:
	ClassRenameBat(Rewriter& Rewrite, std::string oldName, std::string newName) : Rewrite(Rewrite), oName(oldName), nName(newName) {}

	virtual void run(const MatchFinder::MatchResult& Result)
	{
		if (const auto* ClassDecl = Result.Nodes.getNodeAs<CXXRecordDecl>("classDecl"))
		{
			// �滻�ඨ��
			if (ClassDecl->isThisDeclarationADefinition())
			{
				SourceRange classRange = ClassDecl->getSourceRange();
				Rewrite.ReplaceText(classRange.getBegin(), ClassDecl->getNameAsString().length(), nName);
			}
		}
		else if (const auto* NewExpr = Result.Nodes.getNodeAs<CXXNewExpr>("newExpr"))
		{
			// �滻��Ĺ��캯������
			SourceLocation startLoc = NewExpr->getBeginLoc();
			if (startLoc.isValid())
			{
				Rewrite.ReplaceText(startLoc, NewExpr->getType()->getAsCXXRecordDecl()->getNameAsString().length(), nName);
			}
		}
		else if (const VarDecl* VarDeclaration = Result.Nodes.getNodeAs<VarDecl>("varDecl"))
		{
			SourceManager& SM = *Result.SourceManager;
			// ����Ƿ��Ѿ��������һ��
			auto startLoc = VarDeclaration->getTypeSourceInfo()->getTypeLoc().getBeginLoc();
			if (ProcessedLines.insert(SM.getExpansionLineNumber(startLoc)).second)
			{
				SourceRange typeRange = VarDeclaration->getTypeSourceInfo()->getTypeLoc().getSourceRange();
				Rewrite.ReplaceText(typeRange, nName);
			}
			
		}
	}
private:
	Rewriter& Rewrite;
	std::string oName, nName;
	std::set<unsigned> ProcessedLines; // ���ڸ����Ѵ�����к�
};

class FunctionCallBat : public MatchFinder::MatchCallback
{
public:
	explicit FunctionCallBat(Rewriter& Rewrite, string in, string out, std::vector<int> iargv, std::vector<int> oargv) 
		: Rewrite(Rewrite), in(in), out(out), iargv(iargv), oargv(oargv){}

	virtual void run(const MatchFinder::MatchResult& Result)
	{

	}
private:
	Rewriter& Rewrite;
	string in;
	string out;
	std::vector<int> iargv;
	std::vector<int> oargv;
};



class ClassRewriteBat : public MatchFinder::MatchCallback
{
public:
	ClassRewriteBat(Rewriter& Rewrite) : Rewrite(Rewrite) {}

	virtual void run(const MatchFinder::MatchResult& Result)
	{
		if (const auto* ClassDecl =
		Result.Nodes.getNodeAs<CXXRecordDecl>("classDecl"))
		{
			// �滻�ඨ��
			if (ClassDecl->isThisDeclarationADefinition())
			{
				SourceRange classRange = ClassDecl->getSourceRange();
				Rewrite.ReplaceText(classRange.getBegin(),
					ClassDecl->getNameAsString().length(),
					"NewClassName");
			}
		}
		else if (const auto* NewExpr = Result.Nodes.getNodeAs<CXXNewExpr>("newExpr"))
		{
			// �滻��Ĺ��캯������
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
			// �滻�ඨ��
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
			// �滻��Ĺ��캯������
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
			// �滻��Ա�������ã�ʾ����Ϊ��չʾ��ʵ�ʲ���������Ҫ�����ӵĴ���
			// ����Ĵ����ǳ������ھ���������ĺ�����
		}
		else if (const VarDecl* VarDeclaration =
			Result.Nodes.getNodeAs<VarDecl>("arrayDecl"))
		{
			// �������������Ƿ�ȷʵΪCarray
			QualType qt = VarDeclaration->getType();
			if (qt.getAsString() == "CArray")
			{
				// ��ȡ�������͵�Դλ�÷�Χ
				SourceRange typeRange =
					VarDeclaration->getTypeSourceInfo()->getTypeLoc().getSourceRange();
				// ִ���滻
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
			// �滻��Ĺ��캯������
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

// ����תС�շ�
class FunctionCallHandler : public MatchFinder::MatchCallback
{
public:
	explicit FunctionCallHandler(Rewriter& Rewrite) : Rewrite(Rewrite) {}

	virtual void run(const MatchFinder::MatchResult& Result)
	{
		std::string FuncName;

		if (const CallExpr* Call = Result.Nodes.getNodeAs<clang::CallExpr>("call"))
		{
			// Rewrite.InsertText(loc, "X");
			// Rewrite.InsertText(Call->getRParenLoc(), "Y");
			// Rewrite.InsertText(Call->getCallee()->getExprLoc(), "Z");
			Rewrite.InsertTextAfter(Call->getCallee()->getEndLoc(), "@");
			// Rewrite.InsertTextBefore(Call->getCallee()->getEndLoc(), "K");
			// Rewrite.InsertTextAfterToken(Call->getCallee()->getEndLoc(), "F");
			if (const FunctionDecl* FD = Call->getDirectCallee())
			{
				FuncName = FD->getNameInfo().getName().getAsString();
			}
			llvm::outs() << "Matched variable: " << Call->getCalleeDecl() << " " << FuncName << "\n";

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
			// ��������
			//llvm::outs() << "Matched function: " << FD->getNameInfo().getName().getAsString() << "\n";
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
			Rewrite.InsertTextAfter(FD->getNameInfo().getEndLoc(), "$");

			if (FD->hasBody())
			{
				std::error_code EC;
				llvm::raw_fd_ostream OS(outPutDir + "funs.txt", EC, llvm::sys::fs::OF_Append);
				if (EC)
				{
					llvm::errs() << EC.message() << "\n";
				}


				// ʹ��Result.Context��ȡAST������

				// ��ȡ��ǰƥ�䵽�Ľڵ��λ��
				if (FD->hasBody())
				{
					ASTContext* Context = Result.Context;
					FullSourceLoc FullLocation = Context->getFullLoc(FD->getBeginLoc());
					if (FullLocation.isValid())
					{
						OS << " in file: " << FullLocation.getManager().getFilename(FullLocation)
							<< " at " << FullLocation.getSpellingLineNumber() << ":"
							<< FullLocation.getSpellingColumnNumber() << "\n";
					}
				}


				// �������������
				const DeclContext* DC = FD->getDeclContext();
				if (const NamespaceDecl* ND = dyn_cast<NamespaceDecl>(DC))
				{
					OS << "Namespace: " << ND->getNameAsString() << "::";
				}
				else if (const RecordDecl* RD = dyn_cast<RecordDecl>(DC))
				{
					OS << "Class/Struct: " << RD->getNameAsString() << "::";
				}
				OS << "" << FD->getNameInfo().getName().getAsString() << "\t";

				// �����������
				QualType returnType = FD->getReturnType();
				OS << "Return type: " << returnType.getAsString() << "\t";

				// �����������
				for (unsigned int i = 0; i < FD->getNumParams(); ++i)
				{
					const ParmVarDecl* PVD = FD->getParamDecl(i);
					QualType paramType = PVD->getType();
					OS << "Param " << i + 1 << " type: " << paramType.getAsString() << "\t";
				}

				OS << "\n";
			}

		}
	}

private:
	Rewriter& Rewrite;
};