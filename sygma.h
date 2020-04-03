#pragma once

#include <map>
#include <string>
#include <iostream>
#include <list>
#include <cvc4/cvc4.h>


//it maps variable name to (CVC4::Expr)
struct sygma {
	map<std::string, CVC4::Expr> m;
	sygma() {
	}

	sygma(map<std::string, CVC4::Expr> &m) :
			m(m) {
	}

	/*sygma clone() {
		return (sygma(m));

	}*/

	sygma *clone_ptr() {
		sygma *ptr = new sygma(m);
		return ptr;

	}
	void insert_var(string name, CVC4::Expr &e) {
		auto seqrch_sg = m.find(name);
		if (seqrch_sg != m.end()) {
			seqrch_sg->second = e;
		} else
			m.emplace(std::make_pair(name, e));
	}
};

struct crnt_state {
	int pos_oper = 0;

	std::list<CVC4::Expr> expr_call_queue;
	std::list<CVC4::Expr> pi;
	sygma *sg;
	block *bl;
	crnt_state *parent = nullptr;

	//crnt_state()  {}
	crnt_state *clone() {
		crnt_state *p = new crnt_state;
		p->sg = sg->clone_ptr();

		p->expr_call_queue = expr_call_queue; //????
		p->pi = pi; ///????

		p->pos_oper = pos_oper;
		p->bl = bl;
		//p->deep = deep;

		if (parent)
			p->parent = parent->clone(); //deep clone
		return p;
	}
	void reset_pos() {
		pos_oper = 0;

	}

};
