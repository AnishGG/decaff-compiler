#ifndef BLOCKTABLE
#define BLOCKTABLE
#include<llvm/IR/Verifier.h>
#include<llvm/IR/DerivedTypes.h>
#include<llvm/IR/IRBuilder.h>
#include<llvm/IR/LLVMContext.h>
#include<llvm/IR/Module.h>
#include<list>
#include<unordered_map>
#include<stack>

class BlockVariables{
    private:
        void initialize_block(llvm::BasicBlock *block){
            this->block = block;
        }
    public:
        llvm::BasicBlock *block;
        std::unordered_map<std::string, llvm::Value *> localVariables;    // value required to differentiate from global and alloca stact declarations
        BlockVariables(llvm::BasicBlock *block){
            initialize_block(block);
        }
        ~BlockVariables(){}
};

#endif
