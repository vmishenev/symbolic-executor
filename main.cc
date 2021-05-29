/* main.cc */

#include <queue>
#include <stack>
#include <cvc4/cvc4.h>
#include "heading.h"
#include "sygma.h"

using namespace CVC4;

// prototype of bison-generated parser function
int yyparse();

const int BOUND = 10; // max call depth




std::map<string, func_def*> ftable;
std::map<string, Expr> vartable;
std::map<string, int> depth_counter;

ExprManager em;
SmtEngine gl_smt(&em);
Type integer_t = em.integerType();
Expr nil = em.mkVar("<null>", integer_t);

void handle_b(block * b, crnt_state* crnt);
Expr handle_exp2(expr_t* op, crnt_state * crnt, std::list<Expr>::iterator &it);
bool handle_assert(asserop* assert_op, crnt_state * crnt);
bool handle_ite(ifop* ite_op, crnt_state * crnt);
bool handle_assign(assignop* assign_op, crnt_state * crnt);
bool handle_ret(returnop* ret_op, crnt_state * crnt);

int main(int argc, char **argv) {
	if ((argc > 1) && (freopen(argv[1], "r", stdin) == NULL)) {
		cerr << argv[0] << ": File " << argv[1] << " cannot be opened.\n";
		exit(1);
	}

	yyparse();
	block *b = ftable["main"]->b;

	gl_smt.setOption("produce-unsat-cores", "true");
	//smt.setOption("produce-assignments", "true");
	gl_smt.setOption("produce-models", true);
	gl_smt.setLogic("LIA");
	///smt.setOption("incremental", SExpr("true"));

	sygma *sg = new sygma();
	crnt_state *new_state = new crnt_state;
	new_state->sg = sg;

	handle_b(b, new_state);

	delete sg;
	delete new_state;
	return 0;
}

Expr get_var(string name, sygma *sg) {
	auto seqrch_sg = sg->m.find(name);
	if (seqrch_sg != sg->m.end()) {
		return seqrch_sg->second;
	}

	auto search = vartable.find(name);
	if (search != vartable.end()) {
		return search->second;
	} else {
		Expr e = em.mkVar(name, integer_t);
		vartable.emplace(std::make_pair(name, e));
		return e;
	}
}

bool check_sat(Expr e) {
	//SmtEngine smt(&em);
	//smt.setLogic("LIA");
	Result result = gl_smt.checkSat(e);
	return result == Result::SAT;
}

Expr not_expr(Expr e) {
	return em.mkExpr((Kind) 22, e);;

}

Expr handle_call(funcall* call_op, crnt_state * crnt,
		std::list<Expr>::iterator &it) {
	if (it != crnt->expr_call_queue.end()) {
		Expr exp = *it;
		it++;
		return exp;
	} else {
		string name = call_op->name;
		//[+]	count the depth
		auto search_depth = depth_counter.find(name);
		if(search_depth != depth_counter.end() && search_depth->second > BOUND) {
			return em.mkConst(Rational(0));
		} else{
			depth_counter[name] = 1;
		}
		search_depth->second++;
		//[-]	count the depth

		if(name=="print") {
			Expr e = handle_exp2(call_op->args.front(), crnt, it);
			cout<<"print("<<e<<")"<<endl;
			return nil;
		}
		func_def *fund = ftable[name];
		if (!fund) {
			std::cerr << "unknown function " << name << std::endl;
			exit(-1);
		}

		if (fund->largs->size() != call_op->args.size()) {
			std::cerr << "different call cardinality of " << name << std::endl;
			exit(-1);
		}

		sygma *sg_call = new sygma; //TODO leak
		auto it_expr = call_op->args.begin();
		for (string name_arg : *(fund->largs)) {
			Expr e = handle_exp2(*it_expr, crnt, it);
			sg_call->insert_var(name_arg, e);
			it_expr++;
		}
		crnt_state * call_state = new crnt_state;
		call_state->sg = sg_call;
		call_state->pi = crnt->pi; //TODO it's slow
		call_state->parent = crnt;


		handle_b(fund->b, call_state);
		return nil;
	}
}

Expr handle_exp(expr_t* op, crnt_state * crnt) {
	auto it = crnt->expr_call_queue.begin();
	return handle_exp2(op, crnt, it);
}

Expr handle_exp2(expr_t* op, crnt_state * crnt, std::list<Expr>::iterator &it) {
	binary* bin_op = dynamic_cast<binary*>(op);
	if (bin_op) {
		Expr left = handle_exp2(bin_op->arg1, crnt, it);
		if (left == nil)
			return nil;
		Expr right = handle_exp2(bin_op->arg2, crnt, it);
		if (right == nil)
			return nil;

		string t = bin_op->op;
		if (t == "*") {
			return em.mkExpr((Kind) 35, left, right);	//Kind::MULT
		} else if (t == "+") {
			return em.mkExpr((Kind) 34, left, right);	//Kind::PLUS
		} else if (t == "-") {
			return em.mkExpr((Kind) 37, left, right);	//Kind::MINUS
		} else if (t == "/") {
			return em.mkExpr((Kind) 39, left, right);	//Kind::DIVIDE
		} else if (t == "<=") {
			return em.mkExpr((Kind) 52, left, right);	//Kind::LEQ
		} else if (t == ">=") {
			return em.mkExpr((Kind) 54, left, right);	//kind::GEQ
		} else if (t == "<") {
			return em.mkExpr((Kind) 51, left, right);	//kind::LT
		} else if (t == ">") {
			return em.mkExpr((Kind) 53, left, right);	//kind::GT
		} else if (t == "!=") {
			return em.mkExpr((Kind) 22, em.mkExpr((Kind) 8, left, right));
		} else if (t == "==") {
			return em.mkExpr((Kind) 8, left, right); //kind::EQUAL
		}
	}
	value* val = dynamic_cast<value*>(op);
	if (val) {
		return get_var(val->text, crnt->sg);

	}
	value_int* ival = dynamic_cast<value_int*>(op);
	if (ival) {
		return em.mkConst(Rational(ival->i));
	}

	funcall* call_op = dynamic_cast<funcall*>(op);
	if (call_op) {
		return handle_call(call_op, crnt, it);
	}
	return nil;
}

void handle_b(block * b, crnt_state * crnt) {

	crnt->bl = b;

	for (unsigned int i = crnt->pos_oper; i < b->ops.size(); i++) {
		crnt->pos_oper = i;
		oper_t *op = b->ops[i];

		if (assignop* assign_op = dynamic_cast<assignop*>(op)) {
			if (!handle_assign(assign_op, crnt))
				return;
			continue;
		}

		if (asserop* assert_op = dynamic_cast<asserop*>(op)) {
			if (!handle_assert(assert_op, crnt))
				return;
			continue;
		}

		ifop* ite_op = dynamic_cast<ifop*>(op);
		if (ite_op) {
			ite_op->thenops->parent = b;
			ite_op->thenops->pos = i;
			if (!handle_ite(ite_op, crnt))
				return;
			continue;
		}
		exprop* expr_op = dynamic_cast<exprop*>(op);
		if (expr_op) {
			handle_exp(expr_op->expr, crnt);
			continue;
		}
		returnop* ret_op = dynamic_cast<returnop*>(op);
		if (ret_op) {
			handle_ret(ret_op, crnt);
			return;
		}
	}
	if (b->parent) {
		crnt->pos_oper = b->pos + 1;
		handle_b(b->parent, crnt);
	}
}

bool handle_assign(assignop* assign_op, crnt_state * crnt) {
	Expr right = handle_exp(assign_op->value, crnt);
	if (right == nil)
		return false;
	crnt->expr_call_queue.clear();
	crnt->sg->insert_var(assign_op->name, right);
	return true;
}

bool handle_ret(returnop* ret_op, crnt_state * crnt) {
	Expr exp = handle_exp(ret_op->arg, crnt);
	if (crnt->parent) {
		crnt_state *prnt = crnt->parent;
		prnt->expr_call_queue.push_back(exp);
		prnt->pi = crnt->pi;
		handle_b(crnt->parent->bl, crnt->parent);
	}
	return false;
}

bool handle_ite(ifop* ite_op, crnt_state * crnt) {
	Expr exp = handle_exp(ite_op->cond, crnt);

	if (check_sat(exp)) {
		gl_smt.push();
		gl_smt.assertFormula(exp);
		crnt_state *new_state = crnt->clone();
		new_state->pi.push_back(exp);
		new_state->reset_pos();
		handle_b(ite_op->thenops, new_state);
		gl_smt.pop();
	}
	Expr not_exp = em.mkExpr((Kind) 22, exp);
	if (check_sat(not_exp)) {
		gl_smt.push();
		gl_smt.assertFormula(not_exp);
		crnt->pi.push_back(not_exp);
		crnt->reset_pos();
		handle_b(ite_op->elseops, crnt);
		gl_smt.pop();
	}
	return true;
}

bool handle_assert(asserop* assert_op, crnt_state * crnt) {
	Expr exp2 = em.mkExpr((Kind) 22, handle_exp(assert_op->expr, crnt));

	/*for (auto e : crnt->pi) {
		exp2 = em.mkExpr((Kind) 23, exp2, e);

	}*/
	//smt.assertFormula(exp2);
	gl_smt.push();
	std::cout << exp2 << " is " << std::endl;
	Result result = gl_smt.checkSat(exp2);
	cout << "Result from CVC4 is: " << result << endl;

	enum Result::Sat sat_result = result.isSat();
	if (sat_result == Result::UNSAT) {
		cout << "unsat (";
		UnsatCore unsat_core = gl_smt.getUnsatCore();
		std::vector<Expr>::const_iterator it = unsat_core.begin();
		std::vector<Expr>::const_iterator it_end = unsat_core.end();
		for (; it != it_end; ++it) {
			cout << Expr(*it);
		}
		cout << ")" << endl;
	} else if (sat_result == Result::SAT) {
		for(pair<string, Expr> it : vartable) {
			cout << it.first<< " := " << gl_smt.getValue(it.second)  << endl;
		}
		std::cerr<<"Program's unsafe"<<std::endl;
		/*std::vector<std::pair<Expr, Expr> >  vec = smt.getAssignment();
		 for(auto it=vec.begin(); it!=vec.end(); it++) {
			 cout<<it->first.toString() <<" "<<it->second.toString() <<endl;
		 }*/
	}
	gl_smt.pop();
	return true;
}

