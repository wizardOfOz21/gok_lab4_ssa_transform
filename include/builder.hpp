#pragma once

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/StandardInstrumentations.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/Scalar/DCE.h"
#include "llvm/Transforms/Scalar/Reassociate.h"
#include "llvm/Transforms/Scalar/SimplifyCFG.h"

#include "llvm/Transforms/Utils.h"
#include "llvm/Transforms/Utils/Mem2Reg.h"

#include <algorithm>
#include <iostream>
#include <cassert>
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

using namespace llvm;
using std::make_unique;
using std::map;
using std::string;
using std::vector;
using std::unique_ptr;

using snapshot = map<std::string, AllocaInst *>;

extern string main_func_name;
extern unique_ptr<LLVMContext> TheContext;
// Модуль, в который вставляется весь код
extern unique_ptr<Module> TheModule;
// Глобальный билдер, через API которого создаются функции и инструкции
extern unique_ptr<IRBuilder<>> Builder;
// Словарь имен переменных и функций
extern snapshot NamedValues;
// Менеджер пасов – используется для подключения оптимизаций
extern unique_ptr<FunctionPassManager> TheFPM;
extern unique_ptr<LoopAnalysisManager> TheLAM;
extern unique_ptr<FunctionAnalysisManager> TheFAM;
extern unique_ptr<CGSCCAnalysisManager> TheCGAM;
extern unique_ptr<ModuleAnalysisManager> TheMAM;
// MainFunction - функция в IR, в которой объявляются глобальные переменные
extern Function *MainFunc;
// Определенная с помощью "entry = " в коде функция, 
// вызов которой вставляется в конец MainFunc и начинает исполнение программы
extern string entry_function_name;
// Стек словарей с предыдущими значениями переопределенных переменных скоупа
extern vector<snapshot> snapshots;

void init(bool);

void LogError(const string &msg);

Value *LogErrorV(const string &msg);

void in_func_name_print();

AllocaInst *CreateEntryBlockAlloca(Function *TheFunction,
                                   const std::string &VarName);

Function *CreateMainFunction();
