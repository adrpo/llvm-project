//===- MemCpyOptimizer.h - memcpy optimization ------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This pass performs various transformations related to eliminating memcpy
// calls, or transforming sets of stores into memset's.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_SCALAR_MEMCPYOPTIMIZER_H
#define LLVM_TRANSFORMS_SCALAR_MEMCPYOPTIMIZER_H

#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/PassManager.h"
#include <cstdint>
#include <functional>

namespace llvm {

class AssumptionCache;
class CallInst;
class DominatorTree;
class Function;
class Instruction;
class MemCpyInst;
class MemMoveInst;
class MemoryDependenceResults;
class MemSetInst;
class StoreInst;
class TargetLibraryInfo;
class Value;

class MemCpyOptPass : public PassInfoMixin<MemCpyOptPass> {
  MemoryDependenceResults *MD = nullptr;
  TargetLibraryInfo *TLI = nullptr;
  std::function<AliasAnalysis &()> LookupAliasAnalysis;
  std::function<AssumptionCache &()> LookupAssumptionCache;
  std::function<DominatorTree &()> LookupDomTree;

public:
  MemCpyOptPass() = default;

  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM);

  // Glue for the old PM.
  bool runImpl(Function &F, MemoryDependenceResults *MD_,
               TargetLibraryInfo *TLI_,
               std::function<AliasAnalysis &()> LookupAliasAnalysis_,
               std::function<AssumptionCache &()> LookupAssumptionCache_,
               std::function<DominatorTree &()> LookupDomTree_);

private:
  // Helper functions
  bool processStore(StoreInst *SI, BasicBlock::iterator &BBI);
  bool processMemSet(MemSetInst *SI, BasicBlock::iterator &BBI);
  bool processMemCpy(MemCpyInst *M, BasicBlock::iterator &BBI);
  bool processMemMove(MemMoveInst *M);
  bool performCallSlotOptzn(Instruction *cpy, Value *cpyDst, Value *cpySrc,
                            uint64_t cpyLen, Align cpyAlign, CallInst *C);
  bool processMemCpyMemCpyDependence(MemCpyInst *M, MemCpyInst *MDep);
  bool processMemSetMemCpyDependence(MemCpyInst *M, MemSetInst *MDep);
  bool performMemCpyToMemSetOptzn(MemCpyInst *M, MemSetInst *MDep);
  bool processByValArgument(CallBase &CB, unsigned ArgNo);
  Instruction *tryMergingIntoMemset(Instruction *I, Value *StartPtr,
                                    Value *ByteVal);

  bool iterateOnFunction(Function &F);
};

} // end namespace llvm

#endif // LLVM_TRANSFORMS_SCALAR_MEMCPYOPTIMIZER_H
