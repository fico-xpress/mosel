/**********************************************
  File myqxprs.c
  ``````````````
  Example module interfacing to a QCQP solver
  using the mmnl API
  * matrix generation and solver call
  * retrieving solution values
  * access to sample solver parameters
  * implementation of a solver (integer solution) callback
  * routine for matrix output by the solver

  author: Y. Colombani, S. Heipcke, 2018

  (c) Copyright 2018-2022 Fair Isaac Corporation
  
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
 
       http://www.apache.org/licenses/LICENSE-2.0
 
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

***********************************************/
/*
 gcc -Wall -g -o myqxprs.dso -shared -I${XPRESSDIR}/include myqxprs.c -L${XPRESSDIR}/lib -lxprs -lxprnls
 cl /LD /MD /Femyqxprs.dso -I%XPRESSDIR%\include myqxprs.c %XPRESSDIR%\lib\xprs.lib %XPRESSDIR%\lib\xprnls.lib
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define XPRM_NICOMPAT 6000000   /* Compatibility level: Mosel 6.0.0 */
#include "xprm_ni.h"            /* Mosel NI header file */
#include "xprnls.h"             /* Character encoding conversion routines */
#include "mmnl.h"               /* mmnl module extension */

#include "xprs.h"               /* Solver library header file */

/**** Function prototypes ****/
struct SlvCtx; 
struct SlvPb; 
/* Module services */
static void *slv_reset(XPRMcontext ctx, void *libctx);
static void slv_quitlib(void);
static int slv_findparam(const char *name, int *type, int why,
                         XPRMcontext ctx, void *libctx);
static void *slv_nextparam(void *ref, const char **name, const char **desc,
                                                                int *type);
/* Solver library function calls */
static int slv_lc_getpar(XPRMcontext ctx, void *libctx);
static int slv_lc_setpar(XPRMcontext ctx, void *libctx);
static int slv_lc_getpstat(XPRMcontext ctx, void *libctx);
static int slv_lc_maxim(XPRMcontext ctx, void *libctx);
static int slv_lc_minim(XPRMcontext ctx, void *libctx);
static int slv_lc_setcbintsol(XPRMcontext ctx, void *libctx);
static int slv_lc_setcbintsol_pr(XPRMcontext ctx,void *libctx);
static int slv_lc_writepb(XPRMcontext ctx, void *libctx);
static int slv_lc_nmaxim(XPRMcontext ctx, void *libctx);
static int slv_lc_nminim(XPRMcontext ctx, void *libctx);
/* Optimization problem handling */
static void *slv_pb_create(XPRMcontext ctx, void *libctx, void *toref, int type);
static void slv_pb_delete(XPRMcontext ctx, void *libctx, void *todel, int type);
static int slv_pb_copy(XPRMcontext ctx, void *libctx, void *toinit, void *src,
                                                                int ust);

/**** Structures for passing info to Mosel ****/
/* Constants */
static XPRMdsoconst tabconst[]=
  {
   XPRM_CST_INT("MYXP_INF", XPRM_PBINF),             /* Mosel status codes */
   XPRM_CST_INT("MYXP_OPT", XPRM_PBOPT), 
   XPRM_CST_INT("MYXP_OTH", XPRM_PBOTH), 
   XPRM_CST_INT("MYXP_UNF", XPRM_PBUNF), 
   XPRM_CST_INT("MYXP_UNB", XPRM_PBUNB), 
   XPRM_CST_INT("MYXP_LP_OPTIMAL", XPRS_LP_OPTIMAL), /* Solver status codes */
   XPRM_CST_INT("MYXP_LP_INFEAS", XPRS_LP_INFEAS), 
   XPRM_CST_INT("MYXP_LP_CUTOFF", XPRS_LP_CUTOFF), 
   XPRM_CST_INT("MYXP_MIP_NOT_LOADED", XPRS_MIP_NOT_LOADED), 
   XPRM_CST_INT("MYXP_MIP_SOLUTION", XPRS_MIP_SOLUTION), 
   XPRM_CST_INT("MYXP_MIP_INFEAS", XPRS_MIP_INFEAS) 
  };

/* Subroutines */
static XPRMdsofct tabfct[]=
  {
   {"", XPRM_FCT_GETPAR, XPRM_TYP_NOT,0,NULL, slv_lc_getpar},
   {"", XPRM_FCT_SETPAR, XPRM_TYP_NOT,0,NULL, slv_lc_setpar},
   {"getprobstat", 2000, XPRM_TYP_INT,0,NULL, slv_lc_getpstat},
   {"minimise", 2100, XPRM_TYP_NOT,1,"c", slv_lc_minim},
   {"minimize", 2100, XPRM_TYP_NOT,1,"c", slv_lc_minim},
   {"maximise", 2101, XPRM_TYP_NOT,1,"c", slv_lc_maxim},
   {"maximize", 2101, XPRM_TYP_NOT,1,"c", slv_lc_maxim},
   {"setcbintsol", 2102, XPRM_TYP_NOT,1,"s", slv_lc_setcbintsol},
  {"setcbintsol", 2103, XPRM_TYP_NOT,1,"F()", slv_lc_setcbintsol_pr},
   {"writeprob", 2104, XPRM_TYP_NOT,2,"ss", slv_lc_writepb},
   {"minimise", 2120, XPRM_TYP_NOT,1,"|nlctr|", slv_lc_nminim},
   {"minimize", 2120, XPRM_TYP_NOT,1,"|nlctr|", slv_lc_nminim},
   {"maximise", 2121, XPRM_TYP_NOT,1,"|nlctr|", slv_lc_nmaxim},
   {"maximize", 2121, XPRM_TYP_NOT,1,"|nlctr|", slv_lc_nmaxim}
  };

/* Types */
static XPRMdsotyp tabtyp[]=
  {
   {"mpproblem.mqxp", 1, XPRM_DTYP_PROB|XPRM_DTYP_APPND, slv_pb_create,
    slv_pb_delete, NULL, NULL, slv_pb_copy}
  };

/* Module dependency/implied list: modules required by this module */
static const char *slv_deplist[]={"mmnl",NULL};
static const char *slv_implst[]={"mmnl",NULL};

/* Services */
static XPRMdsoserv tabserv[]=
  {
   {XPRM_SRV_PARAM, (void *)slv_findparam},
   {XPRM_SRV_PARLST, (void *)slv_nextparam},
   {XPRM_SRV_RESET, (void *)slv_reset},
   {XPRM_SRV_UNLOAD, (void *)slv_quitlib},
   {XPRM_SRV_DEPLST, (void *)slv_deplist},
   {XPRM_SRV_IMPLST, (void *)slv_implst}
  };

/* Interface structure */
static XPRMdsointer dsointer=
  {
   sizeof(tabconst)/sizeof(XPRMdsoconst), tabconst,
   sizeof(tabfct)/sizeof(XPRMdsofct), tabfct,
   sizeof(tabtyp)/sizeof(XPRMdsotyp), tabtyp,
   sizeof(tabserv)/sizeof(XPRMdsoserv), tabserv
  };

/**** Structures used by this module ****/
static XPRMnifct mm;       /* For storing the Mosel NI function table */
static mmnl_imci mmnl;     /* mmnl API */

/* The "problem" type is used to record solution information and solver ref */
/* Mosel will maintain 1 instance of this type for each mpproblem object */
typedef struct SlvPb
  {
   struct SlvCtx *slctx;
   XPRSprob xpb;           /* Solver problem */
   int have;
   double *solval;         /* Structures for storing solution values */
   double *dualval;
   double *rcostval;
   double *slackval;
   XPRMproc cb_intsol;     /* Solver callback (INTSOL)  */
   XPRMcontext saved_ctx;  /* Mosel context (used by callbacks) */
   struct SlvPb *prev,*next;
  } s_slvpb;

typedef struct SlvCtx      /* A context for this module */
  {
   int pbid;               /* ID of type "mpproblem.mqxp" */
   int options;            /* Runtime options */
   s_slvpb *probs;         /* List of created problems */
   void **nlctx;           /* Execution context of 'mmnl' */
  } s_slvctx;

static struct              /* Parameters published by this module */
  {
   char *name;
   int type;
  } myxprsparams[]=
  {
   {"myxp_verbose", XPRM_TYP_BOOL|XPRM_CPAR_READ|XPRM_CPAR_WRITE},
   {"myxp_maxtime", XPRM_TYP_INT|XPRM_CPAR_READ|XPRM_CPAR_WRITE},
   {"myxp_lpstatus", XPRM_TYP_INT|XPRM_CPAR_READ},
   {"myxp_mipstatus", XPRM_TYP_INT|XPRM_CPAR_READ},
   {"myxp_lpobjval", XPRM_TYP_REAL|XPRM_CPAR_READ},
   {"myxp_loadnames", XPRM_TYP_BOOL|XPRM_CPAR_READ|XPRM_CPAR_WRITE}
  };

#define SLV_NBPARAM (sizeof(myxprsparams)/sizeof(myxprsparams[0]))

/* 'have' in 'slvpb': available solution info */
#define HAVESOL    1
#define HAVERCS    2
#define HAVEDUA    4
#define HAVESLK    8
#define HAVEMIPSOL 16

/* For decoding 'options' in the module context 'slvctx' */
#define OPT_VERBOSE   1
#define OPT_LOADNAMES 2

#define OBJ_MINIMIZE  0
#define OBJ_MAXIMIZE  1

/* Get problem pointer from module context */
#define SLVCTX2PB(o) ((s_slvpb*)(ctx->pbctx[(o)->pbid]))

/* Align on 128 items */
#define ALIGN128(c) ((c)*(((c)+128-1)/128))

/* Prototypes of module-internal functions */
static XPRMnlpdata slv_loadmat(XPRMcontext ctx, s_slvctx *slctx, 
  XPRMlinctr obj, XPRMnlctr nlobj);
static void slv_loadnames(XPRMcontext ctx, s_slvpb *slpb, XPRMnlpdata nlpdata);
static int slv_optim(XPRMcontext ctx, struct SlvCtx *slctx, int objsense,
  XPRMlinctr obj, XPRMnlctr nlobj);
static void slv_clearsol(XPRMcontext ctx, struct SlvPb *slpb);
        /* MMNL solver interface */
static double slv_getsol_v(XPRMcontext ctx, void *mctx, int what, int col);
static double slv_getsol_c(XPRMcontext ctx, void *mctx, int what, int row);
        /* Solver callback funtions */
static int XPRS_CC slv_cb_stopxprs(XPRSprob prob, s_slvpb *slpb);
static void XPRS_CC slv_cb_output(XPRSprob prob, void *p, const char *ch,
  int n, int errtype);
static void XPRS_CC slv_cb_intsol(XPRSprob opt_prob, s_slvpb *slpb);
        /* Memory management */
static void Free(void *ad);

/************************************************/
/* Initialize the library just after loading it */
/************************************************/
DSO_INIT myqxprs_init(XPRMnifct nifct, int *interver, int *libver,
                      XPRMdsointer **interf)
{
 int r;

 *interver=XPRM_NIVERS;         /* The interface version we are using */
 *libver=XPRM_MKVER(0,0,1);     /* The version of the module: 0.0.1 */
 *interf=&dsointer;             /* Our module interface structure */
 r=XPRSinit(NULL);              /* Initialize the solver */
 if((r!=0)&&(r!=32))
 {
  nifct->dispmsg(NULL,"myqxprs: I cannot initialize Xpress Optimizer.\n");
  return 1;
 }
 mm=nifct;                      /* Retrieve the Mosel NI function table */
 return 0;
}

        /********************/
        /***** Services *****/
        /********************/

/*****************************************/
/* Reset the myqxprs interface for a run */
/*****************************************/
static void *slv_reset(XPRMcontext ctx, void *libctx)
{
 XPRMdsolib dso;
 s_slvctx *slctx;

 /* End of execution: release context */
 if(libctx!=NULL)
 {
  slctx=libctx;

  /* Release all remaining problems */
  while(slctx->probs!=NULL)
  {
   slv_pb_delete(ctx,slctx,slctx->probs,-1);
  }
  free(slctx);
  return NULL;
 }
 else
 {
  /* Begin of execution: create context */
  if((slctx=malloc(sizeof(s_slvctx)))==NULL)
  {
   mm->dispmsg(ctx,"myqxprs: Out of memory error.\n");
   return NULL;
  }
  memset(slctx,0,sizeof(s_slvctx));

  /* Load interface functions and context of 'mmnl' */
  if(((dso=mm->finddso("mmnl"))==NULL)||
     ((slctx->nlctx=mm->getdsoctx(ctx,dso,(void **)(&(mmnl))))==NULL))
  {
   mm->dispmsg(ctx,"myqxprs: mmnl not found.\n");
   free(slctx);
   return NULL;
  }

  /* Record the problem ID of our problem type */
  mm->gettypeprop(ctx,mm->findtypecode(ctx,"mpproblem.mqxp"),XPRM_TPROP_PBID,
              (XPRMalltypes*)&(slctx->pbid));
  return (void *)slctx;
 }
}

/*************************************/
/* Called when unloading the library */
/*************************************/
static void slv_quitlib(void)
{
 if(mm!=NULL)
 {
  XPRSfree();
  mm=NULL;
 }
}

/****************************/
/* Find a control parameter */
/****************************/
static int slv_findparam(const char *name, int *type, int why, XPRMcontext ctx,
                void *libctx)
{
 int n;

 for(n=0;n<SLV_NBPARAM;n++)
 {
  if(strcmp(name,myxprsparams[n].name)==0)
  {
   *type=myxprsparams[n].type;
   return n;
  }
 }

 return -1;
}

/*********************************************/
/* Return the next parameter for enumeration */
/*********************************************/
static void *slv_nextparam(void *ref, const char **name, const char **desc,
                int *type)
{
 size_t cst;

 cst=(size_t)ref;
 if((cst<0)||(cst>=SLV_NBPARAM))
  return NULL;
 else
 {
  *name=myxprsparams[cst].name;
  *type=myxprsparams[cst].type;
  *desc=NULL;
  return (void *)(cst+1);
 }
}

        /*************************************/
        /***** Subroutines of the module *****/
        /*************************************/

/*******************************/
/* Getting a control parameter */
/*******************************/
static int slv_lc_getpar(XPRMcontext ctx, void *libctx)
{
 s_slvctx *slctx;
 int n;
 double r;

 slctx=libctx;
 n=XPRM_POP_INT(ctx);
 switch(n)
 {
  case 0:
   XPRM_PUSH_INT(ctx,(slctx->options&OPT_VERBOSE)?1:0);
   break;
  case 1:
   XPRSgetintcontrol(SLVCTX2PB(slctx)->xpb,XPRS_MAXTIME,&n);
   XPRM_PUSH_INT(ctx,n);
   break;
  case 2:
   XPRSgetintattrib(SLVCTX2PB(slctx)->xpb,XPRS_LPSTATUS,&n);
   XPRM_PUSH_INT(ctx,n);
   break;
  case 3:
   XPRSgetintattrib(SLVCTX2PB(slctx)->xpb,XPRS_MIPSTATUS,&n);
   XPRM_PUSH_INT(ctx,n);
   break;
  case 4:
   XPRSgetdblattrib(SLVCTX2PB(slctx)->xpb,XPRS_LPOBJVAL,&r);
   XPRM_PUSH_REAL(ctx,r);
   break;
  case 5:
   XPRM_PUSH_INT(ctx,(slctx->options&OPT_LOADNAMES)?1:0);
   break;
  default:
   mm->dispmsg(ctx,"myqxprs: Wrong control parameter number.\n");
   return XPRM_RT_ERROR;
 }
 return XPRM_RT_OK;
}

/*******************************/
/* Setting a control parameter */
/*******************************/
static int slv_lc_setpar(XPRMcontext ctx, void *libctx)
{
 s_slvctx *slctx;
 int n;

 slctx=libctx;
 n=XPRM_POP_INT(ctx);
 switch(n)
 {
  case 0:
    slctx->options=XPRM_POP_INT(ctx)?(slctx->options|OPT_VERBOSE):(slctx->options&~OPT_VERBOSE);
    break;
  case 1:
    XPRSsetintcontrol(SLVCTX2PB(slctx)->xpb,XPRS_MAXTIME,XPRM_POP_INT(ctx));
    break;
  case 5:
    slctx->options=XPRM_POP_INT(ctx)?(slctx->options|OPT_LOADNAMES):(slctx->options&~OPT_LOADNAMES);
    break;
  default:
    mm->dispmsg(ctx,"myqxprs: Wrong control parameter number.\n");
    return XPRM_RT_ERROR;
 }

 return XPRM_RT_OK;
}

/*****************************/
/* Return the problem status */
/*****************************/
static int slv_lc_getpstat(XPRMcontext ctx, void *libctx)
{
 XPRM_PUSH_INT(ctx,mm->getprobstat(ctx)&XPRM_PBRES);
 return XPRM_RT_OK;
}

/**********************/
/* Maximize a problem */
/**********************/
static int slv_lc_maxim(XPRMcontext ctx, void *libctx)
{
 XPRMlinctr obj;

 obj=XPRM_POP_REF(ctx);
 return slv_optim(ctx,(s_slvctx *)libctx,OBJ_MAXIMIZE,obj,NULL);
}

/**********************/
/* Minimize a problem */
/**********************/
static int slv_lc_minim(XPRMcontext ctx, void *libctx)
{
 XPRMlinctr obj;

 obj=XPRM_POP_REF(ctx);
 return slv_optim(ctx,(s_slvctx *)libctx,OBJ_MINIMIZE,obj,NULL);
}

/********************************************/
/* Define integer callback (procedure name) */
/********************************************/
static int slv_lc_setcbintsol(XPRMcontext ctx, void *libctx)
{
 s_slvctx *slctx;
 s_slvpb *slpb;
 XPRMalltypes result;
 const char *procname,*partyp;
 int nbpar,type;

 slctx=libctx;
 slpb=SLVCTX2PB(slctx);
 procname=XPRM_POP_REF(ctx);

 if(slpb->cb_intsol!=NULL)
 {
  mm->delref(ctx,XPRM_STR_PROC,slpb->cb_intsol);
  slpb->cb_intsol=NULL;
 }
 if(procname!=NULL)
 {                  /* The specified entity must be a procedure */
  if(XPRM_STR(mm->findident(ctx,procname,&result,XPRM_FID_NOLOC))!=XPRM_STR_PROC)
  {
   mm->dispmsg(ctx,"myqxprs: Wrong subroutine type for callback `intsol'.\n");
   return XPRM_RT_ERROR;
  }
  do
  {                 /* The specified procedure must not have any arguments */
   mm->getprocinfo(result.proc,&partyp,&nbpar,&type);
   type=XPRM_TYP(type);
   if((type==XPRM_TYP_NOT)&&(nbpar==0)) break;
   result.proc=mm->getnextproc(result.proc);
  } while(result.proc!=NULL);
  if(result.proc==NULL)
  {
   mm->dispmsg(ctx,"myqxprs: Wrong procedure type for callback `intsol'.\n");
   return XPRM_RT_ERROR;
  }
  else
   slpb->cb_intsol=mm->newref(ctx,XPRM_STR_PROC,result.proc);
 }
 return XPRM_RT_OK;
}

/*************************************************/
/* Define integer callback (procedure reference) */
/*************************************************/
static int slv_lc_setcbintsol_pr(XPRMcontext ctx,void *libctx)
{
 s_slvctx *slctx;
 s_slvpb *slpb;
 mm_proc proc;

 slctx=libctx;
 slpb=SLVCTX2PB(slctx);
 proc=XPRM_POP_REF(ctx);

 if(proc==NULL)
 {
  mm->dispmsg(ctx,"myxprs: NULL reference.\n");
  return XPRM_RT_ERROR;
 }
 else
 {
  if(slpb->cb_intsol!=NULL)
   mm->delref(ctx,XPRM_STR_PROC,slpb->cb_intsol);
  slpb->cb_intsol=mm->newref(ctx,XPRM_STR_PROC,proc);
 }
 return XPRM_RT_OK;
}

/******************/
/* Call writeprob */
/******************/
static int slv_lc_writepb(XPRMcontext ctx, void *libctx)
{
 s_slvpb *slpb;
 int rts;
 char *dname,*options;
 char ename[MM_MAXPATHLEN];

 slpb=SLVCTX2PB((s_slvctx*)libctx);
 dname=MM_POP_REF(ctx);
 options=MM_POP_REF(ctx);
 if((dname!=NULL)&&            /* Make sure that the file can be created */
    (mm->pathcheck(ctx,dname,ename,MM_MAXPATHLEN,MM_RCHK_WRITE|MM_RCHK_IODRV)==0))
 {
  slpb->saved_ctx=ctx;         /* Save current context for callbacks */
  rts=XPRSwriteprob(slpb->xpb,XNLSconvstrto(XNLS_ENC_FNAME,ename,-1,NULL),options);
  slpb->saved_ctx=NULL;
  if(rts)
  {
   mm->dispmsg(ctx,"myqxprs: Error when executing `writeprob'.\n");
   return XPRM_RT_IOERR;
  }
  else
   return XPRM_RT_OK;
 }
 else
 {
  mm->dispmsg(ctx,"myqxprs: Cannot write to '%s'.\n",dname!=NULL?dname:"");
  return XPRM_RT_IOERR;
 }
}

/*************************************/
/* Maximize a problem (nl objective) */
/*************************************/
static int slv_lc_nmaxim(XPRMcontext ctx, void *libctx)
{
 XPRMnlctr obj;

 obj=XPRM_POP_REF(ctx);
 return slv_optim(ctx,(s_slvctx *)libctx,OBJ_MAXIMIZE,NULL,obj);
}

/*************************************/
/* Minimize a problem (nl objective) */
/*************************************/
static int slv_lc_nminim(XPRMcontext ctx, void *libctx)
{
 XPRMnlctr obj;

 obj=XPRM_POP_REF(ctx);
 return slv_optim(ctx,(s_slvctx *)libctx,OBJ_MINIMIZE,NULL,obj);
}

        /******************************/
        /***** Problem management *****/
        /******************************/

/**************************/
/* Create a new "problem" */
/**************************/
static void *slv_pb_create(XPRMcontext ctx, void *libctx, void *toref, int type)
{
 s_slvctx *slctx;
 s_slvpb *slpb;
 int i;

 slctx=libctx;
 if((slpb=malloc(sizeof(s_slvpb)))==NULL)
 {
  mm->dispmsg(ctx,"myqxprs: Out of memory error.\n");
  return NULL;
 }
 memset(slpb,0,sizeof(s_slvpb));
 i=XPRScreateprob(&(slpb->xpb));
 if((i!=0)&&(i!=32))
 {
  mm->dispmsg(ctx,"myqxprs: I cannot create the problem.\n");
  free(slpb);
  return NULL;
 }
 slpb->slctx=slctx;
 /* Redirect solver messages to the Mosel streams */
 XPRSaddcbmessage(slpb->xpb,slv_cb_output,slpb,0);
 XPRSsetintcontrol(slpb->xpb,XPRS_OUTPUTLOG,1);

 /* Define intsol callback */
 XPRSaddcbintsol(slpb->xpb,(void*)slv_cb_intsol,slpb,0);

 /* Define log callbacks to report program interruption */
 XPRSaddcblplog(slpb->xpb,(void*)slv_cb_stopxprs,slpb,0);
 XPRSaddcbcutlog(slpb->xpb,(void*)slv_cb_stopxprs,slpb,0);
 XPRSaddcbgloballog(slpb->xpb,(void*)slv_cb_stopxprs,slpb,0);
 XPRSaddcbbarlog(slpb->xpb,(void*)slv_cb_stopxprs,slpb,0);

 if(slctx->probs!=NULL)
 {
  slpb->next=slctx->probs;
  slctx->probs->prev=slpb;
 }
 /* else we are creating the main problem */

 slctx->probs=slpb;
 return slpb;
}

/**********************/
/* Delete a "problem" */
/**********************/
static void slv_pb_delete(XPRMcontext ctx, void *libctx, void *todel, int type)
{
 s_slvctx *slctx;
 s_slvpb *slpb;

 slctx=libctx;
 slpb=todel;
 slv_clearsol(ctx,slpb);
 XPRSdestroyprob(slpb->xpb);
 if(slpb->next!=NULL)  /* Last in list */
  slpb->next->prev=slpb->prev;
 if(slpb->prev==NULL)  /* First in list */
  slctx->probs=slpb->next;
 else
  slpb->prev->next=slpb->next;
 free(slpb);
}

/***********************************************************/
/* Copy/reset/append: simply clear data of the destination */
/***********************************************************/
static int slv_pb_copy(XPRMcontext ctx, void *libctx, void *toinit, void *src, 
  int ust)
{
 s_slvpb *slpb;

 slpb=toinit;
 if(XPRM_CPY(ust)<XPRM_CPY_APPEND) slv_clearsol(ctx,slpb);
 return 0;
}

        /**************************************/
        /**** Solver & solution management ****/
        /**************************************/
/**************************************/
/* Load the matrix into the optimizer */
/**************************************/
static XPRMnlpdata slv_loadmat(XPRMcontext ctx, s_slvctx *slctx, XPRMlinctr obj,
  XPRMnlctr nlobj)
{
 XPRMnlpdata nlpdata;
 s_slvpb *slpb;
 static char ctcvr[7]={'N','G','L','E','R','1','2'};
 int c,r;
 char pbname[80];
 double *dqe;
 int *mqc1,*mqc2,nb_qp;
 int *qcrows,*qcnqtr,*qcmqc1,*qcmqc2;
 double *qcdqe;
 double objval;

 slpb=SLVCTX2PB(slctx);
 /* Call 'mmnl' function 'loadprob' to generate the matrix */
 nlpdata=NULL;
 c=mmnl->loadprob(ctx,*(slctx->nlctx),&nlpdata,
    NLOPT_COLWJAC|NLOPT_QUADLIN|
    ((slctx->options&OPT_VERBOSE)?NLOPT_VERBOSE:0)|
    ((slctx->options&OPT_LOADNAMES)?NLOPT_NAMES:0),
    obj,nlobj,NULL,NULL,NULL);
 if(c!=0)
  return NULL;

 if(nlpdata->nbqctr<nlpdata->nbnlctr)
 {
  mm->dispmsg(ctx,"myqxprs: Problem contains nonlinear constraints.\n");
  mmnl->strip(ctx,nlpdata,NLSTRIP_ALL);
  return NULL;
 }
 if(nlpdata->objtype==1)        /* Objective is nonlinear */
 {
  mm->dispmsg(ctx,"myqxprs: Objective is nonlinear.\n");
  mmnl->strip(ctx,nlpdata,NLSTRIP_ALL);
  return NULL;
 }

 /* Define the callback functions for exposing the solution values */
 /* The solution arrays will be stored into our problem object (slpb) */
 nlpdata->fctctx=slpb;
 nlpdata->getsol_v=slv_getsol_v;
 nlpdata->getsol_c=slv_getsol_c;

 /* Xpress Optimizer uses a character code for each variable type */
 for(c=0;c<nlpdata->nbctr;c++)
  nlpdata->ctrtype[c]=ctcvr[(int)(nlpdata->ctrtype[c])];

 /* Generate names for the linear part of the problem */
 if(slctx->options&OPT_LOADNAMES)
  mm->genmpnames(ctx,MM_KEEPOBJ,NULL,0);

 if(nlpdata->objtype==2)        /* Quadratic objective */
 {
  XPRMnlqterm qtls;

  r=mmnl->buildqtls(ctx,*(slctx->nlctx),nlpdata->nlobj,1,&nb_qp,&qtls,NULL);
  if((r<0)||(r>=4))
  {
   mmnl->freeqtls(ctx,*(slctx->nlctx),qtls);
   mmnl->strip(ctx,nlpdata,NLSTRIP_ALL);
   return NULL;
  }

  if(nb_qp>0)
  {
   if((dqe=malloc((sizeof(double)+sizeof(int)*2)*nb_qp))==NULL)
   {
    mmnl->freeqtls(ctx,*(slctx->nlctx),qtls);
    mm->dispmsg(ctx,"myqxprs: Out of memory error.\n");
    mmnl->strip(ctx,nlpdata,NLSTRIP_ALL);
    return NULL;
   }
   mqc1=(int *)(dqe+nb_qp);
   mqc2=mqc1+nb_qp;
   for(c=0;c<nb_qp;c++)
   {
    mqc1[c]=mm->getvarnum(ctx,qtls[c].v1);
    mqc2[c]=mm->getvarnum(ctx,qtls[c].v2);
    dqe[c]=(qtls[c].v1==qtls[c].v2)?qtls[c].coeff*2:qtls[c].coeff;
   }
  }
  else
  {
   mqc1=mqc2=NULL;
   dqe=NULL;
  }
  mmnl->freeqtls(ctx,*(slctx->nlctx),qtls);
 }
 else
 {
  nb_qp=0;
  mqc1=mqc2=NULL;
  dqe=NULL;
 }

 qcrows=qcnqtr=qcmqc1=qcmqc2=NULL;
 qcdqe=NULL;
 if(nlpdata->nbqctr>0)          /* Quadratic constraints */
 {
  XPRMnlqterm qtls;
  int maxterms,curt;
  double rhs;

  qcrows=(int *)malloc(sizeof(int)*2*nlpdata->nbqctr);
  if(qcrows==NULL)
  {
   mm->dispmsg(ctx,"myqxprs: Out of memory error.\n");
   free(dqe);
   mmnl->strip(ctx,nlpdata,NLSTRIP_ALL);
   return NULL;
  }
  qcnqtr=qcrows+nlpdata->nbqctr;

  maxterms=0;
  curt=0;
  for(c=0;c<nlpdata->nbqctr;c++)
  {
   qcrows[c]=c;
   r=mmnl->buildqtls(ctx,*(slctx->nlctx),nlpdata->nlctr[c],1,
                                                     &(qcnqtr[c]),&qtls,&rhs);
   if((r<0)||(r>=4))            /* Should not be possible... */
   {
    mmnl->freeqtls(ctx,*(slctx->nlctx),qtls);
    free(qcdqe);
    free(qcmqc2);
    free(qcmqc1);
    free(qcrows);
    free(dqe);
    mmnl->strip(ctx,nlpdata,NLSTRIP_ALL);
    return NULL;
   }

   if(qcnqtr[c]+curt>maxterms)
   {
    int *newp1,*newp2;
    double *newp3;
    int newsize;

    newsize=maxterms+ALIGN128(qcnqtr[c]);
    newp1=newp2=NULL;
    newp3=NULL;
    if(((newp1=(int *)realloc(qcmqc1,newsize*sizeof(int)))==NULL)||
       ((newp2=(int *)realloc(qcmqc2,newsize*sizeof(int)))==NULL)||
       ((newp3=(double *)realloc(qcdqe,newsize*sizeof(double)))==NULL))
    {
     mm->dispmsg(ctx,"myqxprs: Out of memory error.\n");
     mmnl->freeqtls(ctx,*(slctx->nlctx),qtls);
     free((newp3!=NULL)?newp3:qcdqe);
     free((newp2!=NULL)?newp2:qcmqc2);
     free((newp1!=NULL)?newp1:qcmqc1);
     free(qcrows);
     free(dqe);
     mmnl->strip(ctx,nlpdata,NLSTRIP_ALL);
     return NULL;
    }
    else
    {
     qcmqc1=newp1;
     qcmqc2=newp2;
     qcdqe=newp3;
     maxterms=newsize;
    }
   }

   for(r=0;r<qcnqtr[c];r++,curt++)
   {
    qcmqc1[curt]=mm->getvarnum(ctx,qtls[r].v1);
    qcmqc2[curt]=mm->getvarnum(ctx,qtls[r].v2);
    qcdqe[curt]=(qtls[r].v1==qtls[r].v2)?qtls[r].coeff:qtls[r].coeff/2;
   }
   nlpdata->rhs[c]=-rhs;
   mmnl->freeqtls(ctx,*(slctx->nlctx),qtls);
  }
 }

 /* Load the matrix into the optimizer */
 sprintf(pbname,"xpb%p",slpb);
 r=XPRSloadqcqpglobal(slpb->xpb,pbname,nlpdata->nbvar,
         nlpdata->nbctr,
         nlpdata->ctrtype,nlpdata->rhs,nlpdata->range,nlpdata->grdobj,
         nlpdata->colstart,NULL,nlpdata->cref,
         nlpdata->jac,nlpdata->dlb,nlpdata->dub,
         nb_qp,mqc1,mqc2,dqe,
         nlpdata->nbqctr,qcrows,qcnqtr,qcmqc1,qcmqc2,qcdqe,
         nlpdata->ngents,nlpdata->nsos,nlpdata->qgtype,
         nlpdata->mgcols,nlpdata->mplim,
         nlpdata->qstype,
         nlpdata->msstart,nlpdata->mscols,nlpdata->dref);
 free(qcdqe);
 free(qcmqc2);
 free(qcmqc1);
 free(qcrows);
 free(dqe);

 if(!r)
 {
  /* Set objective constant term */
  c=-1;
  objval=-nlpdata->fixobj;
  XPRSchgobj(slpb->xpb,1,&c,&objval);
  
  /* Load names if requested */
  if(slctx->options&OPT_LOADNAMES)
   slv_loadnames(ctx,slpb,nlpdata);
 }

 /* Release matrix information - it is no longer required */
 mmnl->strip(ctx,nlpdata,NLSTRIP_ALL);
 return nlpdata;
}

/*********************************/
/* Load names into the optimizer */
/*********************************/
static void slv_loadnames(XPRMcontext ctx, s_slvpb *slpb, XPRMnlpdata nlpdata)
{
 char *names,*n;
 size_t totlen,totlen2;
 size_t l;
 int c,r;

 totlen=0;
 for(c=0;c<nlpdata->nbvar;c++)
 {
  l=strlen(mm->getmpname(ctx,MM_MPNAM_COL,c));
  totlen+=l+1;
 }
 totlen2=0;
 for(c=0;c<nlpdata->nb_nlctrs;c++)
 {
  l=strlen(nlpdata->names[c]);
  totlen2+=l+1;
 }
 for(;c<nlpdata->nbctr;c++)
 {
  l=strlen(mm->getmpname(ctx,MM_MPNAM_ROW,c-nlpdata->nb_nlctrs));
  totlen2+=l+1;
 }
 if(totlen<totlen2) totlen=totlen2;
 totlen2=0;
 for(c=0;c<nlpdata->nsos;c++)
 {
  l=strlen(mm->getmpname(ctx,MM_MPNAM_SOS,c));
  totlen2+=l+1;
 }
 if(totlen<totlen2) totlen=totlen2;

 if((names=malloc(totlen))==NULL)
  mm->dispmsg(ctx,"myqxprs: Not enough memory for loading the names.\n");
 else
 {
  n=names;
  for(c=0;c<nlpdata->nbvar;c++)
   n+=strlen(strcpy(n,mm->getmpname(ctx,MM_MPNAM_COL,c)))+1;
  if((r=XPRSaddnames(slpb->xpb,2,names,0,nlpdata->nbvar-1))!=0)
   mm->dispmsg(ctx,"myqxprs: Error when executing `%s'.\n","addnames");
  if(!r && (nlpdata->nbctr>0))
  {
   n=names;
   for(c=0;c<nlpdata->nb_nlctrs;c++)
    n+=strlen(strcpy(n,nlpdata->names[c]))+1;
   for(;c<nlpdata->nbctr;c++)
    n+=strlen(strcpy(n,mm->getmpname(ctx,MM_MPNAM_ROW,c-nlpdata->nb_nlctrs)))+1;
   if((r=XPRSaddnames(slpb->xpb,1,names,0,nlpdata->nbctr-1))!=0)
    mm->dispmsg(ctx,"myqxprs: Error when executing `%s'.\n","addnames");
  }
  if(!r && (nlpdata->nsos>0))
  {
   n=names;
   for(c=0;c<nlpdata->nsos;c++)
    n+=strlen(strcpy(n,mm->getmpname(ctx,MM_MPNAM_SOS,c)))+1;
   if((r=XPRSaddnames(slpb->xpb,3,names,0,nlpdata->nsos-1))!=0)
    mm->dispmsg(ctx,"myqxprs: Error when executing `%s'.\n","addnames");
  }
  free(names);
 }
}

/*****************************************************/
/* Optimize a problem: load it, then call the solver */
/*****************************************************/
static int slv_optim(XPRMcontext ctx, s_slvctx *slctx, int objsense,
  XPRMlinctr obj, XPRMnlctr nlobj)
{
 int c,i;
 XPRMnlpdata nlpdata;
 s_slvpb *slpb;
 int result;
 double objval;

 slpb=SLVCTX2PB(slctx);
 slpb->saved_ctx=ctx;    /* Save current context for callbacks */
 slv_clearsol(ctx,slpb);

 /* Call function 'loadmat' to generate and load the matrix */
 nlpdata=slv_loadmat(ctx,slctx,obj,nlobj);

 if(nlpdata==NULL)
 {
  mm->dispmsg(ctx,"myqxprs: loadprob failed.\n");
  slpb->saved_ctx=NULL;
  return XPRM_RT_ERROR;
 }

 /* Set optimization direction */
 XPRSchgobjsense(slpb->xpb,(objsense==OBJ_MINIMIZE)?XPRS_OBJ_MINIMIZE:XPRS_OBJ_MAXIMIZE);

 mmnl->setsolstat(ctx,nlpdata,XPRM_PBSOL,0);  /* Solution available for callbacks */
 if((nlpdata->ngents==0) && (nlpdata->nsos==0))
 {
  /* Solve an LP problem */
  c=XPRSlpoptimize(slpb->xpb,"");
  if(c!=0)
  {
   mm->dispmsg(ctx,"myqxprs: optimisation failed.\n");
   slpb->saved_ctx=NULL;
   return XPRM_RT_ERROR;
  }

  /* Retrieve solution status */
  XPRSgetintattrib(slpb->xpb,XPRS_PRESOLVESTATE,&i);
  if(i&128)
  {
   XPRSgetdblattrib(slpb->xpb,XPRS_LPOBJVAL,&objval);
   result=XPRM_PBSOL;
  }
  else
  {
   objval=0;
   result=0;
  }
  XPRSgetintattrib(slpb->xpb,XPRS_LPSTATUS,&i);
  switch(i)
  {
   case XPRS_LP_OPTIMAL:        result|=XPRM_PBOPT; break;
   case XPRS_LP_INFEAS:         result|=XPRM_PBINF; break;
   case XPRS_LP_CUTOFF:         result|=XPRM_PBOTH; break;
   case XPRS_LP_UNFINISHED:     result|=XPRM_PBUNF; break;
   case XPRS_LP_UNBOUNDED:      result|=XPRM_PBUNB; break;
   case XPRS_LP_CUTOFF_IN_DUAL: result|=XPRM_PBOTH; break;
   case XPRS_LP_UNSOLVED:       result|=XPRM_PBOTH; break;
  }
 }
 else
 {
  /* Solve an MIP problem */
  c=XPRSmipoptimize(slpb->xpb,"");
  if(c!=0)
  {
   mm->dispmsg(ctx,"myqxprs: optimisation failed.\n");
   slpb->saved_ctx=NULL;
   return XPRM_RT_ERROR;
  }

  /* Retrieve solution status */
  XPRSgetintattrib(slpb->xpb,XPRS_MIPSTATUS,&i);
  switch(i)
  {
   case XPRS_MIP_LP_NOT_OPTIMAL:
      objval=0;
      result=XPRM_PBUNF;
      break;
   case XPRS_MIP_LP_OPTIMAL:
      objval=0;
      result=XPRM_PBUNF;
      break;
   case XPRS_MIP_NO_SOL_FOUND:  /* Search incomplete: no solution */
      objval=0;
      result=XPRM_PBUNF;
      break;
   case XPRS_MIP_SOLUTION:     /* Search incomplete: there is a solution */
      XPRSgetdblattrib(slpb->xpb,XPRS_MIPOBJVAL,&objval);
      result=XPRM_PBUNF|XPRM_PBSOL;
      slpb->have|=HAVEMIPSOL;
      break;
   case XPRS_MIP_INFEAS:       /* Search complete: no solution */
      objval=0;
      result=XPRM_PBINF;
      break;
   case XPRS_MIP_OPTIMAL:      /* Search complete: best solution available */
      XPRSgetdblattrib(slpb->xpb,XPRS_MIPOBJVAL,&objval);
      result=XPRM_PBSOL|XPRM_PBOPT;
      slpb->have|=HAVEMIPSOL;
      break;
   case XPRS_MIP_UNBOUNDED:
      objval=0;
      result=XPRM_PBUNB;
      break;
   default:
      result=XPRM_PBOTH;
  }
  if(!(result&XPRM_PBSOL))
  {
   /* If no MIP solution try to get an LP solution */
   XPRSgetintattrib(slpb->xpb,XPRS_PRESOLVESTATE,&i);
   if(i&128)
   {
    XPRSgetdblattrib(slpb->xpb,XPRS_LPOBJVAL,&objval);
    result|=XPRM_PBSOL;
   }
  }
 }

 /* Record solution status and objective value */
 mmnl->setsolstat(ctx,nlpdata,result,objval);
 slpb->saved_ctx=NULL;
 return 0;
}


/**********************************/
/* Get a solution (variable part) */
/**********************************/
static double slv_getsol_v(XPRMcontext ctx, void *mctx, int what, int col)
{
 s_slvpb *slpb;

 slpb=mctx;
 if(what)
 {
  if(!(slpb->have&HAVERCS))
  {
   if(slpb->rcostval==NULL)
   {
    int ncol;

    XPRSgetintattrib(slpb->xpb,XPRS_ORIGINALCOLS,&ncol);
    if((slpb->rcostval=malloc(ncol*sizeof(double)))==NULL)
    {
     mm->dispmsg(ctx,"myqxprs: Out of memory error.\n");
     return 0;
    }
   }
   if(slpb->have&HAVEMIPSOL)  /* No rcost for a MIP => 0 */
   {
    int ncol;

    XPRSgetintattrib(slpb->xpb,XPRS_ORIGINALCOLS,&ncol);
    memset(slpb->rcostval,0,ncol*sizeof(double));
   }
   else
    XPRSgetlpsol(slpb->xpb,NULL,NULL,NULL,slpb->rcostval);
   slpb->have|=HAVERCS;
  }
  return slpb->rcostval[col];
 }
 else
 {
  if(!(slpb->have&HAVESOL))
  {
   if(slpb->solval==NULL)
   {
    int ncol;

    XPRSgetintattrib(slpb->xpb,XPRS_ORIGINALCOLS,&ncol);
    if((slpb->solval=malloc(ncol*sizeof(double)))==NULL)
    {
     mm->dispmsg(ctx,"myqxprs: Out of memory error.\n");
     return 0;
    }
   }
   if(slpb->have&HAVEMIPSOL)
    XPRSgetmipsol(slpb->xpb,slpb->solval,NULL);
   else
    XPRSgetlpsol(slpb->xpb,slpb->solval,NULL,NULL,NULL);
   slpb->have|=HAVESOL;
  }
  return slpb->solval[col];
 }
}

/************************************/
/* Get a solution (constraint part) */
/************************************/
static double slv_getsol_c(XPRMcontext ctx, void *mctx, int what, int row)
{
 s_slvpb *slpb;

 slpb=mctx;
 if(what)
 {
  if(!(slpb->have&HAVEDUA))
  {
   if(slpb->dualval==NULL)
   {
    int nrow;

    XPRSgetintattrib(slpb->xpb,XPRS_ORIGINALROWS,&nrow);
    if((slpb->dualval=malloc(nrow*sizeof(double)))==NULL)
    {
     mm->dispmsg(ctx,"myqxprs: Out of memory error.\n");
     return 0;
    }
   }
   if(slpb->have&HAVEMIPSOL)  /* No dual for a MIP => 0 */
   {
    int nrow;

    XPRSgetintattrib(slpb->xpb,XPRS_ORIGINALROWS,&nrow);
    memset(slpb->dualval,0,nrow*sizeof(double));
   }
   else
    XPRSgetlpsol(slpb->xpb,NULL,NULL,slpb->dualval,NULL);
   slpb->have|=HAVEDUA;
  }
  return slpb->dualval[row];
 }
 else
 {
  if(!(slpb->have&HAVESLK))
  {
   if(slpb->slackval==NULL)
   {
    int nrow;

    XPRSgetintattrib(slpb->xpb,XPRS_ORIGINALROWS,&nrow);
    if((slpb->slackval=malloc(nrow*sizeof(double)))==NULL)
    {
     mm->dispmsg(ctx,"myqxprs: Out of memory error.\n");
     return 0;
    }
   }
   if(slpb->have&HAVEMIPSOL)
    XPRSgetmipsol(slpb->xpb,NULL,slpb->slackval);
   else
    XPRSgetlpsol(slpb->xpb,NULL,slpb->slackval,NULL,NULL);
   slpb->have|=HAVESLK;
  }
  return slpb->slackval[row];
 }
}

/*******************************/
/* Delete solution information */
/*******************************/
static void slv_clearsol(XPRMcontext ctx, s_slvpb *slpb)
{
 Free(&(slpb->solval));
 Free(&(slpb->dualval));
 Free(&(slpb->rcostval));
 Free(&(slpb->slackval));
 slpb->have=0;
}

/*****************************************/
/* XPRS callback: log (for interruption) */
/*****************************************/
static int XPRS_CC slv_cb_stopxprs(XPRSprob prob, s_slvpb *slpb)
{
 return mm->chkinterrupt(slpb->saved_ctx);
}

/*************************/
/* XPRS callback: Output */
/*************************/
static void XPRS_CC slv_cb_output(XPRSprob prob, void *p, const char *ch, int n,
  int errtype)
{
 s_slvpb *slpb;

 slpb=(s_slvpb *)p;
 switch(errtype)
 {
  case 1:
  case 2:
  case 3:
       if(slpb->slctx->options&OPT_VERBOSE)
       {
        if(slpb->saved_ctx!=NULL)
         mm->printf(slpb->saved_ctx,"%.*s\n",n,ch);
        else
         mm->dispmsg(NULL,"%.*s\n",n,ch);
       }
       break;
  case 4:
       mm->dispmsg(slpb->saved_ctx,"%.*s\n",n,ch);
       break;
  default:
       if((slpb->slctx->options&OPT_VERBOSE) && (slpb->saved_ctx!=NULL))
        mm->fflush(slpb->saved_ctx);
 }
}

/*************************/
/* XPRS callback: IntSol */
/*************************/
static void XPRS_CC slv_cb_intsol(XPRSprob opt_prob, s_slvpb *slpb)
{
 XPRMalltypes result;
 XPRSprob xpb_save;

 if(slpb->cb_intsol!=NULL)
 {
  xpb_save=slpb->xpb;
  slpb->xpb=opt_prob;
  slpb->have=0;
  if(mm->callproc(slpb->saved_ctx,slpb->cb_intsol,&result)!=0)
  {
   mm->stoprun(slpb->saved_ctx);
   XPRSinterrupt(opt_prob,XPRS_STOP_CTRLC);
  }
  slpb->xpb=xpb_save;
 }
}

/****************/
/* Free + reset */
/****************/
static void Free(void *ad)
{
 free(*(void **)ad);
 *(void **)ad=NULL;
}
