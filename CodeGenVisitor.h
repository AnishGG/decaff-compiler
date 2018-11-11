#ifndef CODE_GENERATION
#define CODE_GENERATION

#include <llvm/IR/Verifier.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/LegacyPassManagers.h>
#include <llvm/IR/IRPrintingPasses.h>
#include <llvm/AsmParser/Parser.h> 
#include <llvm/IR/Instructions.h>
#include <llvm/IR/CallingConv.h>
#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Pass.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/CallingConv.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/InlineAsm.h>
#include <llvm/Support/FormattedStream.h>
#include <llvm/Support/MathExtras.h>
#include <iostream>
#include "BlockTable.h"
#include "ast.h"
#include "Visitor.h"

class CodeGenVisitor : public Visitor
{
    private:
        llvm::Module* module;
        ASTProgram* start;
        llvm::Function* mainFunction;                   // To create a function, initial function type of main is known
        std::list<BlockVariables> blockTable;           // List to store all the blocks
        void initiateModule(ASTProgram* start){         // Initiating top level container for all other LLVM IR objects
            std::string module_name = start->getID();
            llvm::LLVMContext &context = llvm::getGlobalContext();
            this->module = new llvm::Module(module_name, context);
            this->start = start;
        }
        void set_architecture(){
            this->module->setTargetTriple("x86_64-pc-linux-gnu");   // target architecture for which the code is generated
        }
    public:
        /**************************************************************/
        // These are all the block handling functions
        std::unordered_map<std::string, llvm::Value *> getLocalVariables(){
            return blockTable.front().localVariables;
        }
        void setLocalVariables(std::unordered_map<std::string, llvm::Value *> variables){
            blockTable.front().localVariables.insert(variables.begin(), variables.end());
        }
        bool lookupLocalVariables(std::string name){
            std::unordered_map<std::string, llvm::Value *> it; 
            it = getLocalVariables();
            bool ret = it.find(name) != it.end();
            return ret;
        }
        void declareLocalVariables(std::string name, llvm::Value *value){
            if(lookupLocalVariables(name) == false){
                std::pair<std::string, llvm::Value *> inp = std::pair<std::string, llvm::Value *>(name, value);
                blockTable.front().localVariables.insert(inp);
            }
            else{
                std::cerr<<"Variable "<<name<<" already declared";
                exit(0);
            }
        }
        bool lookupGlobalVariables(std::string name){
            bool ret = (returnLocalVariables(name) == NULL);
            return !ret;
        }
        llvm::Value *returnLocalVariables(std::string name){
            std::list<BlockVariables>::iterator it;
            for(it = blockTable.begin(); it != blockTable.end(); it++){
                std::unordered_map<std::string, llvm::Value *>::iterator found_or_end;
                found_or_end = it->localVariables.find(name);
                if(found_or_end != it->localVariables.end()){
                    llvm::Value *ret = found_or_end->second;
                    return ret;
                }
            }
            return NULL;
        }
        void pushBlock(llvm::BasicBlock *block){
            blockTable.push_front(BlockVariables(block));
        }
        void popBlock(){
            blockTable.pop_front();
        }
        llvm::BasicBlock *topBlock(){
            std::list<BlockVariables>::iterator it;
            for(it = blockTable.begin(); it != blockTable.end(); it++){
                if(it->block != NULL){  // some scopes might be useless
                    return it->block;
                }
            }
            return blockTable.front().block;
        }
        llvm::BasicBlock *bottomBlock(){
            return blockTable.back().block;
        }
        /*************************************************************/
        CodeGenVisitor(ASTProgram* start){
            initiateModule(start);
            set_architecture();
            llvm::FunctionType *ftype = llvm::FunctionType::get(llvm::Type::getVoidTy(llvm::getGlobalContext()), false);    // Initialising the return type of the main function i.e. main
            mainFunction = llvm::Function::Create(ftype, llvm::GlobalValue::ExternalLinkage, "main", module);
        }
        ~CodeGenVisitor(){}
        void handleBlock(llvm::BasicBlock *block){
            block = topBlock();
            popBlock();
        }
        void buildPass(llvm::BasicBlock *block){
            llvm::legacy::PassManager PM;       // Manages what all passes do we want to run on our module
            llvm::ReturnInst::Create(llvm::getGlobalContext(), block);
            llvm::verifyModule(*this->module);        // Performs various checks like to determine if our compiler is doing everything right
            PM.add(llvm::createPrintModulePass(llvm::outs()));  // To generate the IR from the ast we gave
            PM.run(*this->module);                    // Executes the scheduled pass
        }
        void codeGen() {
            llvm::BasicBlock *block = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", this->mainFunction, 0);   // 0 to insert at the end of the function
            pushBlock(block);
            this->visit(start);
            handleBlock(block);
            buildPass(block);
        }
};
#endif