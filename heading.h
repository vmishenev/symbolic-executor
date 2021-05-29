/* heading.h */
#pragma once

#define YY_NO_UNPUT

using namespace std;
#include <map>
#include <list>
#include <vector>
#include <iostream>
#include <stdio.h>
#include <string>
#include <stdlib.h>

//TODO Visitor: class visitor { };

    class oper_t { // abstract
    protected: oper_t() {}
    public: virtual ~oper_t() {}
        virtual void print(int indent=0) =0;
    };

    class expr_t { // abstract
    protected: expr_t() {}
    public: virtual ~expr_t() {}
        virtual void print() =0;
    };

    class block : public oper_t {
    public:
        block *parent=nullptr;
        int pos=0;
        std::vector<oper_t*> ops;

    public:
        block() {}
        block(oper_t* op) { append(op); }
        block(oper_t* op1, oper_t* op2) { append(op1); append(op2); }
        //std::vector<oper_t*>  *get_ops() { return &ops;}
        void append(oper_t* op) {
            if(!op)return;
            /* block* b = dynamic_cast<block*>(op);
                                if(b) {
                                    ops.splice(ops.end(), b->ops, b->ops.begin(), b->ops.end());
                                    delete b;
                        }
                        else*/ ops.push_back(op);
        }
        int size() { return ops.size(); }
        virtual void print(int indent=0) {
            for(auto i: ops) {
                std::cout << std::string(indent, '\t');
                i->print(indent);
            }
        }
        virtual ~block() { for(auto i: ops) delete i; }
    };

    class exprop : public oper_t {
    public: expr_t* expr;
    public: exprop(expr_t* expr) : expr(expr) {}
        virtual void print(int indent=0) {
            expr->print();
            std::cout << ";" << std::endl;
        }
        virtual ~exprop() { delete expr; }
    };

    class asserop : public oper_t {
    public: expr_t* expr;
    public: asserop(expr_t* expr) : expr(expr) {}
        virtual void print(int indent=0) {
            cout<<"assert (";
            expr->print();
            std::cout << ");" << std::endl;
        }
        virtual ~asserop() { delete expr; }
    };

    class ifop : public oper_t {
    public:
        block *parent=nullptr;
        int pos=0;
        expr_t* cond;
        block *thenops, *elseops;
    public: ifop(expr_t* cond, oper_t* thenops, oper_t* elseops) :
            cond(cond), thenops(dynamic_cast<block *>(thenops)), elseops(dynamic_cast<block *>(elseops)) {}
        virtual void print(int indent=0) {
            std::cout << "if "; cond->print();  std::cout << " {" << std::endl;
            thenops->print(indent+1);
            if (elseops->size()) {
                std::cout << std::string(indent, '\t') << "} else {" << std::endl;
                elseops->print(indent+1);
            }
            std::cout << std::string(indent, '\t') << "}" << std::endl;
        }
        virtual ~ifop() { delete cond; }
    };


    class returnop : public oper_t {
    public: expr_t *arg;
    public: returnop(expr_t *arg_a) :
            arg(arg_a) {}
        virtual void print(int indent=0) { std::cout << "return ";  arg->print(); std::cout<<";"<< std::endl; }
    };


    class assignop : public oper_t {
    public:
        std::string name;
        expr_t* value;
    public:
        assignop( std::string& name, expr_t* value) :
            name(name), value(value) {}
        virtual void print(int indent=0) { std::cout<<name<<" := "; value->print(); }
        virtual ~assignop() { delete value; }
    };

    class binary : public expr_t {
    public:
        const char* op;
        expr_t *arg1, *arg2;
    public: binary(const char* op, expr_t *arg1, expr_t *arg2) :
            op(op), arg1(arg1), arg2(arg2) {}
        virtual void print() {
            std::cout<<"(";
            arg1->print();
            std::cout<<op;
            arg2->print();
            std::cout<<")";
        }
        virtual ~binary() { delete arg1; delete arg2; }
    };

    class unary : public expr_t {
        const char* op;
        expr_t* arg;
    public: unary(const char* op, expr_t* arg) : op(op), arg(arg) {}
        virtual void print(int indent=0 ) { std::cout<<op; arg->print(); }
        virtual ~unary() { delete arg; }
    };

    class funcall : public expr_t {
    public: std::string name;
        std::list<expr_t*> args;
    public: funcall(const std::string& name,
                    const std::list<expr_t*>& args) :
            name(name), args(args) {}
        virtual void print() {
            std::cout<<name<<"(";
            for(auto i:args) {
                // if (i!=args.begin())
                std::cout<<", ";
                i->print();
            }
            std::cout<<")";
        }
        virtual ~funcall() { for(auto i:args) delete i; }
    };

    class value : public expr_t {
    public: std::string text;
    public: value(const std::string& text) : text(text) {}
        virtual void print() { std::cout<<text; }
    };

    class value_int : public expr_t {
    public: int i;
    public: value_int(int n) : i(n) {}
        virtual void print() { std::cout<<i; }
    };

    struct func_def {
        std::string name;
        std::list<std::string> *largs;
        block *b;
        func_def(std::string name, std::list<std::string> *args, block *b) : name(name), 				largs(args), b(b){}
    };

