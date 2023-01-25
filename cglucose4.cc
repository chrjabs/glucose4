/*
 * Author: Christoph Jabs - christoph.jabs@helsinki.fi
 *
 * Copyright © 2022 Christoph Jabs, University of Helsinki
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the “Software”), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#include "core/Solver.h"
#include "simp/SimpSolver.h"

#define IPASIR2MS(il) (mkLit(abs(il) - 1, il < 0))

namespace Glucose {

// Note: the memory layout of Wrapper and SimpWrapper needs to be the same
// because we are casting SimpWrapper into Wrapper

struct Wrapper {
  Solver *solver;
  vec<Lit> clause{};
  vec<Lit> assumps{};

  Wrapper() : solver(new Solver()) {}

  ~Wrapper() { delete solver; }
};

struct SimpWrapper {
  SimpSolver *solver;
  vec<Lit> clause{};
  vec<Lit> assumps{};

  SimpWrapper() : solver(new SimpSolver()) {}

  ~SimpWrapper() { delete solver; }
};

} // namespace Glucose

using namespace Glucose;

extern "C" {

#include "cglucose4.h"

const char *cglucose4_signature(void) { return "Glucose 4.2.1"; }

CGlucose4 *cglucose4_init(void) { return (CGlucose4 *)new Wrapper(); }

CGlucoseSimp4 *cglucosesimp4_init(void) {
  return (CGlucoseSimp4 *)new SimpWrapper();
}

void cglucose4_release(CGlucose4 *handle) { delete (Wrapper *)handle; }

void cglucosesimp4_release(CGlucoseSimp4 *handle) {
  delete (SimpWrapper *)handle;
}

void cglucose4_add(CGlucose4 *handle, int lit) {
  Wrapper *wrapper = (Wrapper *)handle;
  if (lit) {
    int var = abs(lit) - 1;
    while (var >= wrapper->solver->nVars())
      wrapper->solver->newVar();
    wrapper->clause.push(IPASIR2MS(lit));
    return;
  }
  wrapper->solver->addClause_(wrapper->clause);
  wrapper->clause.clear();
}

void cglucosesimp4_add(CGlucoseSimp4 *handle, int lit) {
  cglucose4_add((CGlucose4 *)handle, lit);
}

void cglucose4_assume(CGlucose4 *handle, int lit) {
  ((Wrapper *)handle)->assumps.push(IPASIR2MS(lit));
}

void cglucosesimp4_assume(CGlucoseSimp4 *handle, int lit) {
  cglucose4_assume((CGlucose4 *)handle, lit);
}

int cglucose4_solve(CGlucose4 *handle) {
  Wrapper *wrapper = (Wrapper *)handle;
  lbool res = wrapper->solver->solveLimited(wrapper->assumps);
  wrapper->assumps.clear();
  if (res == l_True) {
    return 10;
  }
  if (res == l_False) {
    return 20;
  }
  return 0;
}

int cglucosesimp4_solve(CGlucoseSimp4 *handle) {
  return cglucose4_solve((CGlucose4 *)handle);
}

int cglucose4_val(CGlucose4 *handle, int lit) {
  Wrapper *wrapper = (Wrapper *)handle;
  Var v = abs(lit);
  lbool val = wrapper->solver->modelValue(v);
  if (val == l_True) {
    return v;
  }
  if (val == l_False) {
    return -v;
  }
  return 0;
}

int cglucosesimp4_val(CGlucoseSimp4 *handle, int lit) {
  return cglucose4_solve((CGlucose4 *)handle);
}

int cglucose4_failed(CGlucose4 *handle, int lit) {
  Wrapper *wrapper = (Wrapper *)handle;
  Lit ms_lit = IPASIR2MS(-lit);
  for (int i = 0; i < wrapper->solver->conflict.size(); ++i) {
    if (wrapper->solver->conflict[i] == ms_lit) {
      return 1;
    }
  }
  return 0;
}

int cglucosesimp4_failed(CGlucoseSimp4 *handle, int lit) {
  return cglucose4_failed((CGlucose4 *)handle, lit);
}

int cglucose4_n_assigns(CGlucose4 *handle) {
  return ((Wrapper *)handle)->solver->nAssigns();
}

int cglucosesimp4_n_assigns(CGlucoseSimp4 *handle) {
  return ((SimpWrapper *)handle)->solver->nAssigns();
}

int cglucose4_n_clauses(CGlucose4 *handle) {
  return ((Wrapper *)handle)->solver->nClauses();
}

int cglucosesimp4_n_clauses(CGlucoseSimp4 *handle) {
  return ((SimpWrapper *)handle)->solver->nClauses();
}

int cglucose4_n_learnts(CGlucose4 *handle) {
  return ((Wrapper *)handle)->solver->nLearnts();
}

int cglucosesimp4_n_learnts(CGlucoseSimp4 *handle) {
  return ((SimpWrapper *)handle)->solver->nLearnts();
}

int cglucose4_n_vars(CGlucose4 *handle) {
  return ((Wrapper *)handle)->solver->nVars();
}

int cglucosesimp4_n_vars(CGlucoseSimp4 *handle) {
  return ((SimpWrapper *)handle)->solver->nVars();
}

void cglucose4_set_conf_limit(CGlucose4 *handle, int64_t limit) {
  ((Wrapper *)handle)->solver->setConfBudget(limit);
}

void cglucosesimp4_set_conf_limit(CGlucoseSimp4 *handle, int64_t limit) {
  ((SimpWrapper *)handle)->solver->setConfBudget(limit);
}

void cglucose4_set_prop_limit(CGlucose4 *handle, int64_t limit) {
  ((Wrapper *)handle)->solver->setPropBudget(limit);
}

void cglucosesimp4_set_prop_limit(CGlucoseSimp4 *handle, int64_t limit) {
  ((SimpWrapper *)handle)->solver->setPropBudget(limit);
}

void cglucose4_set_no_limit(CGlucose4 *handle) {
  ((Wrapper *)handle)->solver->budgetOff();
}

void cglucosesimp4_set_no_limit(CGlucoseSimp4 *handle) {
  ((SimpWrapper *)handle)->solver->budgetOff();
}

void cglucose4_interrupt(CGlucose4 *handle) {
  ((Wrapper *)handle)->solver->interrupt();
}

void cglucosesimp4_interrupt(CGlucoseSimp4 *handle) {
  ((SimpWrapper *)handle)->solver->interrupt();
}

void cglucosesimp4_set_frozen(CGlucoseSimp4 *handle, int var, bool frozen) {
  ((SimpWrapper *)handle)->solver->setFrozen(var, frozen);
}

int cglucosesimp4_is_eliminated(CGlucoseSimp4 *handle, int var) {
  return ((SimpWrapper *)handle)->solver->isEliminated(var);
}
}