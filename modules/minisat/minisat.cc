/********************************************************
  File minisat.cc  
  ```````````````
  Example module interfacing to a SAT solver using the Mosel NI
  * problem representation via logical clauses
  * solver call
  * retrieval of solution values

  author: T. Berthold, Z. Csizmadia

  (c) Copyright 2019 Fair Isaac Corporation
 
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
 
       http://www.apache.org/licenses/LICENSE-2.0
 
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

********************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <float.h>
#include <math.h>
#include <ctype.h>

#include "core/Solver.h"        // Solver library header file 
#define XPRM_NICOMPAT 4000000   // Compatibility level: Mosel 4.0.0 
#include "xprm_ni.h"            // Mosel NI header file 

#include <vector>
#include <set>

//**** Function prototypes ****
// Module subroutines
static int xt_minisat_solve(mm_context ctx, void* libctx);
static int xt_minisat_duplicate(mm_context ctx, void* libctx);
static int xt_minisat_negate(mm_context ctx, void* libctx);
static int xt_minisat_getsol(mm_context ctx, void* libctx);
// Type handling
void* minisat_logical_create(mm_context ctx, void* libctx, void* ref, int typnum);
void minisat_logical_delete(mm_context ctx, void* libctx, void* todel, int typnum);
int minisat_logical_tostring(mm_context ctx, void* libctx, void* obj, char* dest, int maxsize, int typnum);
// Module services
void* minisat_reset(mm_context ctx, void* libctx, int Versnum);

//**** Structures for passing info to Mosel ****
//! Types
static XPRMdsotyp tabtyp[] = {
  {(char*)"logical",5,XPRM_DTYP_PNCTX|XPRM_DTYP_RFCNT, minisat_logical_create, minisat_logical_delete, minisat_logical_tostring, NULL}  // a logical
};

//! Subroutines defined by the module
static mm_dsofct tabfct[] =
{
  { (char*)"@&",          1000,XPRM_TYP_EXTN, 1,(char*)"logical:|logical|",    xt_minisat_duplicate },
  { (char*)"@n",          1001,XPRM_TYP_EXTN, 1,(char*)"logical:|logical|",    xt_minisat_negate },
  { (char*)"solve",       1010,XPRM_TYP_INT,  1,(char*)"e",                    xt_minisat_solve },
  { (char*)"getsol",      1011,XPRM_TYP_BOOL, 1,(char*)"|logical|",            xt_minisat_getsol }
};

//! Module services
static XPRMdsoserv tabserv[] = {
  {MM_SRV_RESET, (void*)minisat_reset},  
  {MM_SRV_PROVIDER,(void*)"Fair Isaac"}
};


//! Interface structure passed to Mosel
static mm_dsointer dsointer =
{
  0,NULL,
  sizeof(tabfct) / sizeof(mm_dsofct),tabfct,
  sizeof(tabtyp) / sizeof(XPRMdsotyp),tabtyp,
  sizeof(tabserv) / sizeof(XPRMdsoserv),tabserv
};

//**** Structures used by this module ****
typedef struct s_logical {
  int ID;          //!< Integer id of the logical; minus if it represents the negated version
  int solved;      //!< If was involved in a solve
  int refcnt;      //!< Reference count
  char value;      //!< Solve value
  struct s_logical* negated;   //!< Pointer to the negated version
} minisat_logical;

// Problem specific data (context)
typedef struct {  
  int logical_type;
  int globalNextID; //! Global ID counter; each logical gets a new positive value
  std::set<minisat_logical *> *logicals;
} minisat_problem;

static mm_nifct mm;                // For storing the Mosel NI function table

/************************************************/
/* Initialise the library just after loading it */
/************************************************/
extern "C" DSO_INIT minisat_init(mm_nifct nifct, int* interver, int* libver, mm_dsointer** interf)
{
  mm = nifct;                      // Retrieve the Mosel NI function table
  *interver = XPRM_NIVERS;         // The interface version we are using
  *libver = XPRM_MKVER(0, 0, 1);   // The version of the module: 0.0.1
  *interf = &dsointer;             // Our module interface structure
  return RT_OK;
}

        /********************/
        /***** Services *****/
        /********************/

//! Initialize and reset the module and the problem
void* minisat_reset(mm_context ctx, void* libctx, int Versnum) {
  
  minisat_problem* minisatProb = NULL;
  std::set<minisat_logical *>::iterator LogicIterator;
  
  if (libctx == NULL) { /* entry call */

    minisatProb = (minisat_problem *) calloc(1, sizeof(minisat_problem));
    if (minisatProb) {
      minisatProb->globalNextID = 1;
      minisatProb->logical_type = mm->findtypecode(ctx, "logical");
      try {
        minisatProb->logicals = new(std::set<minisat_logical*>);
      }
      catch (...) {
        // Out of memory
        free(minisatProb);
        minisatProb = NULL;
      }
    }
    return(minisatProb);
  }

  // Free problem structure
  minisatProb = (minisat_problem *) libctx;

  // Free any logicals we may hold; let's opt not making the logical type a class, just free the C memory
  for (LogicIterator = minisatProb->logicals->begin(); LogicIterator != minisatProb->logicals->end(); LogicIterator++) {
    free(*LogicIterator);
  }
  delete(minisatProb->logicals);

  free(minisatProb);

  return(NULL);
}

        /*************************/
        /***** Type handling *****/
        /*************************/

//! Create a logical type object
void* minisat_logical_create(mm_context ctx, void* libctx, void* ref, int typnum) {
  minisat_logical *L;
  minisat_problem* minisatProb = (minisat_problem *) libctx;

  if (ref != NULL) {
    L = (minisat_logical*)ref;
    L->refcnt++;
  }
  else {
    L = (minisat_logical*)calloc(1, sizeof(minisat_logical));  
    if (L == NULL) {
      free(L); 
      mm->dispmsg(ctx, "Minisat: Out of memory.\n");
      return NULL;
    }

    L->refcnt=1;
    L->ID = minisatProb->globalNextID;
    minisatProb->globalNextID++;

    // Add a referece to the created logical
    try {
      minisatProb->logicals->insert(L);
    } 
    catch (...) {
      free(L);
      mm->dispmsg(ctx, "Minisat: Out of memory.\n");
      return NULL;
    }
  }

  return L;
}

//! Create the negated version of a logical
int minisat_logical_create_no(mm_context ctx, void* libctx, minisat_logical* L) {
  minisat_logical* L_negated;
  minisat_problem* minisatProb = (minisat_problem*)libctx;

  L_negated = (minisat_logical*)calloc(1, sizeof(minisat_logical));
  if (L_negated == NULL) {
    mm->dispmsg(ctx, "Minisat: Out of memory.\n");
    return RT_ERROR;
  }

  L_negated->ID = -L->ID;
  L->negated = L_negated;
  L_negated->negated = L;

  try {
    minisatProb->logicals->insert(L_negated);
  }
  catch (...) {
    free(L_negated);
    mm->dispmsg(ctx, "Minisat: Out of memory.\n");
    return RT_ERROR;
  }

  return RT_OK;
}

//! Delete a logical object
void minisat_logical_delete(mm_context ctx, void* libctx, void* todel, int typnum) {
  minisat_problem* minisatProb = (minisat_problem*)libctx;
  minisat_logical* L = (minisat_logical*) todel;

  if ( (L != NULL) && (--(L->refcnt) < 1) ) { 
    minisat_logical* L_negated = (minisat_logical*) L->negated;

    minisatProb->logicals->erase(L);
    free(L);
    // Orphan the logical on it's negated
    if (L_negated) {
      L_negated->negated = NULL;
    }
  }
}

//! local snprintf implementation
int minisat_snprintf(char* target_buffer, int buffersize, const char* format, ...)
{
  va_list argptr;
  int i;

  va_start(argptr, format);
  i = vsnprintf(target_buffer, buffersize, format, argptr);
  va_end(argptr);

  // make utterly sure it is terminated by a '0'
  if ((i < 0) || (i >= buffersize)) i = buffersize - 1;
  target_buffer[i] = '\0';

  return(i);
}

//! Convert logical type to string, so we are able to 'writeln' it
int minisat_logical_tostring(mm_context ctx, void* libctx, void* obj, char* dest, int maxsize, int typnum) {
  return (minisat_snprintf(dest, maxsize, "%i", obj==NULL?0:((minisat_logical*)obj)->ID));
}

        /*************************************/
        /***** Subroutines of the module *****/
        /*************************************/

//! SOLVE!
static int xt_minisat_solve(mm_context ctx, void* libctx) {
  XPRMset sets;
  int type;
  int solutionStatus = 0;
  XPRMalltypes typeinfo;
  minisat_problem* minisatProb = (minisat_problem*)libctx;
  int nbcls;

  sets = (XPRMset)XPRM_POP_REF(ctx);

  if (sets == NULL) {
    mm->dispmsg(ctx, "Minisat: Set NULL reference.\n");
    return RT_ERROR;
  }
  
  if (minisatProb->logical_type < 1) {  // Can this happen?
    mm->dispmsg(ctx, "Minisat: Invalid parameter for `solve', expecting a set of set of logical.\n");
    return RT_ERROR;
  }
  type = XPRM_TYP(mm->getsettype(sets));
  if ((mm->gettypeprop(ctx, type, XPRM_TPROP_EXP, &typeinfo) != 0) ||
    ((typeinfo.integer & (XPRM_MSK_STR | XPRM_MSK_TYP)) != (XPRM_STR_SET | minisatProb->logical_type))) {
    mm->dispmsg(ctx, "Minisat: Invalid parameter for `solve', expecting a set of set of logical.\n");
    return RT_ERROR;
  }

  mm->printf(ctx, "Solving for\n");
  if ((nbcls = mm->getsetsize(sets)) > 0) {
    XPRMalltypes gpv, cls;
    Minisat::Solver minisat;
    Minisat::vec<Minisat::Lit> clause;
    Minisat::vec<Minisat::Lit> dummy;
    Minisat::lbool returnCode;
    std::vector<minisat_logical*> reverseLookup;
    std::vector<int> ifSavedNegated;

    // loop over arguments and copy them
    try {
      for (int c = 1; c <= nbcls; c++) {
        int nbv;

        mm->getelsetval(ctx, sets, c, &cls);
        if ((cls.set != NULL) && ((nbv = mm->getsetsize(cls.set)) > 0)) {
          for (int v = 1; v <= nbv; v++) {
            minisat_logical* L = (minisat_logical*)(mm->getelsetval(ctx, cls.set, v, &gpv)->ref);
            if (L ==NULL) {
              mm->dispmsg(ctx,"Minisat: NULL reference in problem definition (ignored).\n");
            }
            else {
              int literal = L->ID;
              unsigned int variable = abs(literal);
            
              if (reverseLookup.size() < variable) {
                reverseLookup.resize((size_t)variable + 1000);
                ifSavedNegated.resize((size_t)variable + 1000);
              }
              reverseLookup[variable] = L; // We may end up saving multiple times, which is fine
              if (L->ID < 0) {
                ifSavedNegated[variable] = 1;
              }
              else {
                ifSavedNegated[variable] = 0;
              }
            
              // create missing variables
              while ((int)variable >= minisat.nVars())
                minisat.newVar();
              clause.push((literal > 0) ? Minisat::mkLit(variable) : ~Minisat::mkLit(variable));
              mm->printf(ctx, "%i ", L->ID);
              L->solved = 1;
            }
          }
          minisat.addClause(clause);
          clause.clear();
          // new set
          mm->printf(ctx, " \n");
          mm->printf(ctx, "We got %d columns and %d rows \n", minisat.nVars(), minisat.nClauses());
        }
      }
    }
    catch (...) {
      mm->dispmsg(ctx, "Minisat: Out of memory.\n");
      return RT_ERROR;
    }

    /* presolving */
    solutionStatus = minisat.simplify();

    returnCode = minisat.solveLimited(dummy);
    solutionStatus = toInt(returnCode);
    mm->printf(ctx, solutionStatus == 0 ? "Satisfiable\n" : solutionStatus == 1 ? "Unsatisfiable\n" : "Unsolved\n");

    if (solutionStatus == 0) {
      for (int var = 1; var < minisat.nVars(); var++) {
        minisat_logical* L = reverseLookup[var];
        assert(toInt(minisat.model[var]) != 2);
        mm->printf(ctx, "x%d = %s  ", var, toInt(minisat.model[var]) == 0 ? "TRUE" : "FALSE");        
        if (toInt(minisat.model[var]) == 0) {
          L->value = 1 - ifSavedNegated[var];
          if (L->negated) ((minisat_logical*)L->negated)->value = ifSavedNegated[var];
        }
        else {
          L->value = ifSavedNegated[var];
          if (L->negated) ((minisat_logical*)L->negated)->value = 1 - ifSavedNegated[var];
        }
      }
      mm->printf(ctx, "\n");
    }
  }
  mm->printf(ctx, "\n");

  mm->fflush(ctx);

  XPRM_PUSH_INT(ctx, solutionStatus);

  return RT_OK;
}

//! Duplicate a logical (in fact we only increase its refcnt)
static int xt_minisat_duplicate(mm_context ctx, void* libctx) {
  if (XPRM_TOP_ST(ctx)->ref == NULL) {
      mm->dispmsg(ctx, "Minisat: NULL reference.\n");
      return RT_ERROR;
  }
  else {
    XPRM_TOP_ST(ctx)->ref=minisat_logical_create(ctx,libctx,XPRM_TOP_ST(ctx)->ref,0);
    return RT_OK;
  }
}

//! Negate a logical
static int xt_minisat_negate(mm_context ctx, void* libctx) {
  minisat_logical* L;

  L = (minisat_logical*)XPRM_POP_REF(ctx);

  if (L == NULL) {
    mm->dispmsg(ctx, "Minisat: NULL reference.\n");
    return RT_ERROR;
  }
  else {
    // Create or return existing negated pair
    if (L->negated) {
      XPRM_PUSH_REF(ctx, L->negated);
    }
    else {
      if (minisat_logical_create_no(ctx, libctx, L)) {
        return RT_ERROR;
      }
      XPRM_PUSH_REF(ctx, L->negated);
    }

    // Add a referece to the negated version, which we are now apparently using
    L->negated->refcnt++;
    minisat_logical_delete(ctx,libctx,L,0);
    return RT_OK;
  }
}

//! Return solution value
static int xt_minisat_getsol(mm_context ctx, void* libctx) {
  minisat_logical* L;

  L = (minisat_logical*)XPRM_POP_REF(ctx);

  if (L == NULL) {
    mm->dispmsg(ctx, "Minisat: NULL reference.\n");
    return RT_ERROR;
  }
  else if (L->solved == 0) {
    mm->dispmsg(ctx, "Minisat: Not yet solved for '%i'.\n", L->ID);
    return RT_ERROR;
  }
  else {
    XPRM_PUSH_INT(ctx, L->value);
    return RT_OK;
  }
}

///////////////////////////////////////////////////////////////////////////////
// END
///////////////////////////////////////////////////////////////////////////////

// EOF  
