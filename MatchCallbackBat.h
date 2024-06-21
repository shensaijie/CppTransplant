#pragma once
#include "all.h"

// 类批量替换
class ClassRenameBat : public MatchFinder::MatchCallback
{
public:
	ClassRenameBat(Rewriter& Rewrite, std::string oldName, std::string newName) : Rewrite(Rewrite), oName(oldName), nName(newName) {}

	virtual void run(const MatchFinder::MatchResult& Result)
	{
		if (const auto* ClassDecl = Result.Nodes.getNodeAs<CXXRecordDecl>("classDecl"))
		{
			// 替换类定义
			if (ClassDecl->isThisDeclarationADefinition())
			{
				SourceRange classRange = ClassDecl->getSourceRange();
				Rewrite.ReplaceText(classRange.getBegin(), ClassDecl->getNameAsString().length(), nName);
			}
		}
		else if (const auto* NewExpr = Result.Nodes.getNodeAs<CXXNewExpr>("newExpr"))
		{
			// 替换类的构造函数调用
			SourceLocation startLoc = NewExpr->getBeginLoc();
			if (startLoc.isValid())
			{
				Rewrite.ReplaceText(startLoc, NewExpr->getType()->getAsCXXRecordDecl()->getNameAsString().length(), nName);
			}
		}
		else if (const VarDecl* VarDeclaration = Result.Nodes.getNodeAs<VarDecl>("varDecl"))
		{
			SourceManager& SM = *Result.SourceManager;
			// 检查是否已经处理过这一行
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
	std::set<unsigned> ProcessedLines; // 用于跟踪已处理的行号
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
			// 替换类定义
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