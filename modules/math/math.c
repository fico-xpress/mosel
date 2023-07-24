/******************************************
  File math.c
  ```````````
  Example module defining a set of
  mathematical functions and
  an int64 type

  author: Y. Colombani, rev. Apr. 2023

  (c) Copyright 2023 Fair Isaac Corporation
  
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
 
       http://www.apache.org/licenses/LICENSE-2.0
 
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

*******************************************/
/*
cl /nologo /MD /I%XPRESSDIR%\include /LD math.c /Femath.dso
gcc -Wall -o math.dso -shared -D_REENTRANT -I${XPRESSDIR}/include math.c -lm
*/
#define VMAJ 1
#define VMIN 0
#define VREL 0

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#define XPRM_NICOMPAT 5004000   /* Compatibility level: Mosel 5.4.0 */
#include "xprm_ni.h"
#include "mmath.h"

/**** Function prototypes ****/
static int chkerror(XPRMcontext ctx,const char *fct);

static int ma_tan(XPRMcontext ctx,void *libctx);
static int ma_asin(XPRMcontext ctx,void *libctx);
static int ma_acos(XPRMcontext ctx,void *libctx);
static int ma_atan2(XPRMcontext ctx,void *libctx);
static int ma_cosh(XPRMcontext ctx,void *libctx);
static int ma_sinh(XPRMcontext ctx,void *libctx);
static int ma_tanh(XPRMcontext ctx,void *libctx);
static int ma_acosh(XPRMcontext ctx,void *libctx);
static int ma_asinh(XPRMcontext ctx,void *libctx);
static int ma_atanh(XPRMcontext ctx,void *libctx);
static int ma_hypot(XPRMcontext ctx,void *libctx);
static int ma_frexp(XPRMcontext ctx,void *libctx);
static int ma_ldexp(XPRMcontext ctx,void *libctx);
static int ma_logb(XPRMcontext ctx,void *libctx);
static int ma_log1p(XPRMcontext ctx,void *libctx);
static int ma_cbrt(XPRMcontext ctx,void *libctx);
static int ma_cpysgn(XPRMcontext ctx,void *libctx);
static int ma_nextaft(XPRMcontext ctx,void *libctx);
static int ma_nextup(XPRMcontext ctx,void *libctx);
static int ma_rint(XPRMcontext ctx,void *libctx);
static int ma_scalb(XPRMcontext ctx,void *libctx);
static int ma_sign(XPRMcontext ctx,void *libctx);
static int ma_sign_i(XPRMcontext ctx,void *libctx);
static int ma_todeg(XPRMcontext ctx,void *libctx);
static int ma_torad(XPRMcontext ctx,void *libctx);
static int ma_round2d(XPRMcontext ctx,void *libctx);
static int ma_roundnd(XPRMcontext ctx,void *libctx);
static int ma_j0(XPRMcontext ctx,void *libctx);
static int ma_j1(XPRMcontext ctx,void *libctx);
static int ma_jn(XPRMcontext ctx,void *libctx);
static int ma_y0(XPRMcontext ctx,void *libctx);
static int ma_y1(XPRMcontext ctx,void *libctx);
static int ma_yn(XPRMcontext ctx,void *libctx);

static void *ma_reset(XPRMcontext ctx,void *libctx,int version);
static int ma_chkver(int);
static void ma_updvers(int,int,int*);
static int ma_chkres(int);
static size_t ma_memuse(XPRMcontext ctx,void *libctx,void *ref,int code);

					/* From bigint.c */
static int bi_maxi64(XPRMcontext ctx,void *libctx);
static int bi_isodd(XPRMcontext ctx,void *libctx);
static int bi_abs(XPRMcontext ctx,void *libctx);
static int bi_asint(XPRMcontext ctx,void *libctx);
static int bi_asreal(XPRMcontext ctx,void *libctx);
static int bi_new0(XPRMcontext ctx,void *libctx);
static int bi_new1r(XPRMcontext ctx,void *libctx);
static int bi_new1i(XPRMcontext ctx,void *libctx);
static int bi_zero(XPRMcontext ctx,void *libctx);
static int bi_one(XPRMcontext ctx,void *libctx);
static int bi_asgn(XPRMcontext ctx,void *libctx);
static int bi_asgn_i(XPRMcontext ctx,void *libctx);
static int bi_pls(XPRMcontext ctx,void *libctx);
static int bi_pls_i(XPRMcontext ctx,void *libctx);
static int bi_pls_r(XPRMcontext ctx,void *libctx);
static int bi_neg(XPRMcontext ctx,void *libctx);
static int bi_mul(XPRMcontext ctx,void *libctx);
static int bi_mul_i(XPRMcontext ctx,void *libctx);
static int bi_mul_r(XPRMcontext ctx,void *libctx);
static int bi_div(XPRMcontext ctx,void *libctx);
static int bi_div_i1(XPRMcontext ctx,void *libctx);
static int bi_div_i2(XPRMcontext ctx,void *libctx);
static int bi_div_r1(XPRMcontext ctx,void *libctx);
static int bi_div_r2(XPRMcontext ctx,void *libctx);
static int bi_idiv(XPRMcontext ctx,void *libctx);
static int bi_idiv_i1(XPRMcontext ctx,void *libctx);
static int bi_idiv_i2(XPRMcontext ctx,void *libctx);
static int bi_mod(XPRMcontext ctx,void *libctx);
static int bi_mod_i1(XPRMcontext ctx,void *libctx);
static int bi_mod_i2(XPRMcontext ctx,void *libctx);
static int bi_eql_i(XPRMcontext ctx,void *libctx);
static int bi_eql_i2(XPRMcontext ctx,void *libctx);
static int bi_lt_i(XPRMcontext ctx,void *libctx);
static int bi_lt_i2(XPRMcontext ctx,void *libctx);
static int bi_gt_i(XPRMcontext ctx,void *libctx);
static int bi_gt_i2(XPRMcontext ctx,void *libctx);
static int bi_tostr(XPRMcontext ctx,void *,void *,char *,int,int);
static int bi_fromstr(XPRMcontext ctx,void *libctx,void *toinit,const char *str,int,const char **endptr);
static int bi_copy(XPRMcontext ctx,void *libctx,void *toinit,void *src,int typnum);
static int bi_compare(XPRMcontext ctx,void *libctx,void *c1,void *c2,int typnum);
static void *bi_create(XPRMcontext ctx,void *,void *,int);
static void bi_delete(XPRMcontext ctx,void *,void *,int);
static int64_t imci_getint64val(XPRMcontext ctx,struct MathCtx *mactx,void *ref);
static int imci_setint64val(XPRMcontext ctx,struct MathCtx *mactx,void *ref,int64_t v);

/**** Structures for passing info to Mosel ****/
/* Constants */
static const double ma_log2e=	1.4426950408889634074;	/* log_2 e */
static const double ma_log10e=	0.43429448190325182765;	/* log_10 e */
static const double ma_ln2=	0.69314718055994530942;	/* log_e 2 */
static const double ma_ln10=	2.30258509299404568402;	/* log_e 10 */
static const double ma_pi_2=	1.57079632679489661923;	/* pi/2 */
static const double ma_pi_4=	0.78539816339744830962;	/* pi/4 */
static const double ma_1_pi=	0.31830988618379067154;	/* 1/pi */
static const double ma_2_pi=	0.63661977236758134308;	/* 2/pi */
static const double ma_2_sqrtpi=1.12837916709551257390;	/* 2/sqrt(pi) */
static const double ma_sqrt2=	1.41421356237309504880;	/* sqrt(2) */
static const double ma_sqrt1_2=	0.70710678118654752440;	/* 1/sqrt(2) */

static XPRMdsoconst tabconst[]=
    {
     XPRM_CST_REAL("M_LOG2E",ma_log2e),
     XPRM_CST_REAL("M_LOG10E",ma_log10e),
     XPRM_CST_REAL("M_LN2",ma_ln2),
     XPRM_CST_REAL("M_LN10",ma_ln10),
     XPRM_CST_REAL("M_PI_2",ma_pi_2),
     XPRM_CST_REAL("M_PI_4",ma_pi_4),
     XPRM_CST_REAL("M_1_PI",ma_1_pi),
     XPRM_CST_REAL("M_2_PI",ma_2_pi),
     XPRM_CST_REAL("M_2_SQRTPI",ma_2_sqrtpi),
     XPRM_CST_REAL("M_SQRT2",ma_sqrt2),
     XPRM_CST_REAL("M_SQRT1_2",ma_sqrt1_2)
    };

/* Subroutines */
static XPRMdsofct tabfct[]=
        {
         {"tan",1000,XPRM_TYP_REAL,1,"r",ma_tan},
         {"arcsin",1001,XPRM_TYP_REAL,1,"r",ma_asin},
         {"arccos",1002,XPRM_TYP_REAL,1,"r",ma_acos},
         {"arctan2",1003,XPRM_TYP_REAL,2,"rr",ma_atan2},
         {"cosh",1004,XPRM_TYP_REAL,1,"r",ma_cosh},
         {"sinh",1005,XPRM_TYP_REAL,1,"r",ma_sinh},
         {"tanh",1006,XPRM_TYP_REAL,1,"r",ma_tanh},
         {"arccosh",1007,XPRM_TYP_REAL,1,"r",ma_acosh},
         {"arcsinh",1008,XPRM_TYP_REAL,1,"r",ma_asinh},
         {"arctanh",1009,XPRM_TYP_REAL,1,"r",ma_atanh},

         {"hypot",1015,XPRM_TYP_REAL,2,"rr",ma_hypot},
         {"frexp",1016,XPRM_TYP_REAL,2,"rLi",ma_frexp},
         {"ldexp",1017,XPRM_TYP_REAL,2,"ri",ma_ldexp},
         {"logb",1018,XPRM_TYP_REAL,1,"r",ma_logb},
         {"log1p",1019,XPRM_TYP_REAL,1,"r",ma_log1p},
         {"cbrt",1020,XPRM_TYP_REAL,1,"r",ma_cbrt},
         {"copysign",1021,XPRM_TYP_REAL,2,"rr",ma_cpysgn},
         {"nextafter",1022,XPRM_TYP_REAL,2,"rr",ma_nextaft},
         {"nextup",1023,XPRM_TYP_REAL,1,"r",ma_nextup},
         {"rint",1024,XPRM_TYP_REAL,1,"r",ma_rint},
         {"scalb",1025,XPRM_TYP_REAL,2,"ri",ma_scalb},
         {"sign",1026,XPRM_TYP_REAL,1,"r",ma_sign},
         {"sign",1027,XPRM_TYP_INT,1,"i",ma_sign_i},
         {"todeg",1028,XPRM_TYP_REAL,1,"r",ma_todeg},
         {"torad",1029,XPRM_TYP_REAL,1,"r",ma_torad},

         {"round2d",1040,XPRM_TYP_REAL,1,"r",ma_round2d},
         {"roundnd",1041,XPRM_TYP_REAL,2,"ri",ma_roundnd},

         {"j0",1120,XPRM_TYP_REAL,1,"r",ma_j0},
         {"j1",1121,XPRM_TYP_REAL,1,"r",ma_j1},
         {"jn",1122,XPRM_TYP_REAL,2,"ir",ma_jn},
         {"y0",1123,XPRM_TYP_REAL,1,"r",ma_y0},
         {"y1",1124,XPRM_TYP_REAL,1,"r",ma_y1},
         {"yn",1125,XPRM_TYP_REAL,2,"ir",ma_yn},

         {"@&",1300,XPRM_TYP_EXTN,1,"int64:|int64|",bi_new0},
         {"@&",1301,XPRM_TYP_EXTN,1,"int64:r",bi_new1r},
         {"@&I",1302,XPRM_TYP_EXTN,1,"int64:i",bi_new1i},
         {"@0",1304,XPRM_TYP_EXTN,0,"int64:",bi_zero},
         {"@1",1305,XPRM_TYP_EXTN,0,"int64:",bi_one},
         {"@:",1306,XPRM_TYP_NOT,2,"|int64||int64|",bi_asgn},
         {"@:",1307,XPRM_TYP_NOT,2,"|int64|i",bi_asgn_i},
         {"@+",1308,XPRM_TYP_EXTN,2,"int64:|int64||int64|",bi_pls},
         {"@+",1309,XPRM_TYP_EXTN,2,"int64:|int64|i",bi_pls_i},
         {"@+",1310,XPRM_TYP_REAL,2,"|int64|r",bi_pls_r},
         {"@*",1311,XPRM_TYP_EXTN,2,"int64:|int64||int64|",bi_mul},
         {"@*",1312,XPRM_TYP_EXTN,2,"int64:|int64|i",bi_mul_i},
         {"@*",1313,XPRM_TYP_REAL,2,"|int64|r",bi_mul_r},
         {"@-",1314,XPRM_TYP_EXTN,1,"int64:|int64|",bi_neg},
         {"@/",1315,XPRM_TYP_REAL,2,"|int64||int64|",bi_div},
         {"@/",1316,XPRM_TYP_REAL,2,"|int64|i",bi_div_i1},
         {"@/",1317,XPRM_TYP_REAL,2,"i|int64|",bi_div_i2},
         {"@/",1318,XPRM_TYP_REAL,2,"|int64|r",bi_div_r1},
         {"@/",1319,XPRM_TYP_REAL,2,"r|int64|",bi_div_r2},
         {"@d",1320,XPRM_TYP_EXTN,2,"int64:|int64||int64|",bi_idiv},
         {"@d",1321,XPRM_TYP_EXTN,2,"int64:|int64|i",bi_idiv_i1},
         {"@d",1322,XPRM_TYP_INT,2,"i|int64|",bi_idiv_i2},
         {"@m",1323,XPRM_TYP_EXTN,2,"int64:|int64||int64|",bi_mod},
         {"@m",1324,XPRM_TYP_EXTN,2,"int64:|int64|i",bi_mod_i1},
         {"@m",1325,XPRM_TYP_INT,2,"i|int64|",bi_mod_i2},

         {"@=",1350,XPRM_TYP_BOOL,2,"|int64|i",bi_eql_i},
         {"@<",1351,XPRM_TYP_BOOL,2,"|int64|i",bi_lt_i},
         {"@>",1352,XPRM_TYP_BOOL,2,"|int64|i",bi_gt_i},
         {"@=",1353,XPRM_TYP_BOOL,2,"i|int64|",bi_eql_i2},
         {"@<",1354,XPRM_TYP_BOOL,2,"i|int64|",bi_lt_i2},
         {"@>",1355,XPRM_TYP_BOOL,2,"i|int64|",bi_gt_i2},

         {"MAX_INT64",1370,XPRM_TYP_EXTN,0,"int64:",bi_maxi64},
         {"isodd",1371,XPRM_TYP_BOOL,1,"|int64|",bi_isodd},
         {"abs",1372,XPRM_TYP_EXTN,1,"int64:|int64|",bi_abs},
         {"getasint",1373,XPRM_TYP_INT,1,"|int64|",bi_asint},
         {"getasreal",1374,XPRM_TYP_REAL,1,"|int64|",bi_asreal}
	};

                                     /* Table of types */
static XPRMdsotyp tabtyp[]=
        {
         {"int64",1,XPRM_DTYP_PNCTX|XPRM_DTYP_RFCNT|XPRM_DTYP_APPND|XPRM_DTYP_TFBIN|XPRM_DTYP_ORD|XPRM_DTYP_CONST,bi_create,bi_delete,bi_tostr,bi_fromstr,bi_copy,bi_compare}
        };

                                     /* Table of services */
static struct Mmath_imci imci=
	{
	 imci_getint64val,
	 imci_setint64val
	};
static XPRMdsoserv tabserv[]=
        {
         {XPRM_SRV_RESET,(void *)ma_reset},
	 {XPRM_SRV_CHKVER,(void*)ma_chkver},
         {XPRM_SRV_MEMUSE,(void *)ma_memuse},
         {XPRM_SRV_CHKRES,(void*)ma_chkres},
	 {XPRM_SRV_UPDVERS,(void*)ma_updvers},
	 {XPRM_SRV_IMCI,(void *)(&imci)}
        };

/* Interface structure */
static XPRMdsointer dsointer= 
        { 
         sizeof(tabconst)/sizeof(XPRMdsoconst),tabconst,
         sizeof(tabfct)/sizeof(XPRMdsofct),tabfct,
         sizeof(tabtyp)/sizeof(XPRMdsotyp),tabtyp,
         sizeof(tabserv)/sizeof(XPRMdsoserv),tabserv
        };

static XPRMnifct mm;             /* For storing Mosel NI function table */

typedef struct MathCtx            /* Module context */
        {
         int nbmaxi64;          /* current size of the arrays */
         int nbi64;             /* current number of int64s */
         size_t firstfree;      /* first free int64 */
         unsigned int *refcnt;  /* refcnts of int64s */
         int64_t *value;        /* values of int64s */
        } s_mathctx;

/************************************************/
/* Initialize the library just after loading it */
/************************************************/
DSO_INIT math_init(XPRMnifct nifct, int *interver,int *libver, XPRMdsointer **interf)
{
 mm=nifct;                      /* Save the table of Mosel NI functions */
 *interver=XPRM_NIVERS;         /* The Mosel NI version we are using */
 *libver=XPRM_MKVER(VMAJ,VMIN,VREL); /* The version of the module */
 *interf=&dsointer;             /* Our interface */

 return 0;
}

/***********************************************/
/* Display an error message in case of failure */
/***********************************************/
static int chkerror(XPRMcontext ctx,const char *fct)
{
 if(errno!=0)
 {
  mm->dispmsg(ctx,"Math: Error while evaluating '%s'.\n",fct);
  return XPRM_RT_MATHERR;
 }
 else
  return XPRM_RT_OK;
}

/***********/
/* Tangent */
/***********/
static int ma_tan(XPRMcontext ctx,void *libctx)
{
 XPRM_TOP_ST(ctx)->real=tan(XPRM_TOP_ST(ctx)->real);
 return XPRM_RT_OK;
}

/************/
/* Arc sine */
/************/
static int ma_asin(XPRMcontext ctx,void *libctx)
{
 errno=0;
 XPRM_TOP_ST(ctx)->real=asin(XPRM_TOP_ST(ctx)->real);
 return chkerror(ctx,"arcsin");
}

/**************/
/* Arc cosine */
/**************/
static int ma_acos(XPRMcontext ctx,void *libctx)
{
 errno=0;
 XPRM_TOP_ST(ctx)->real=acos(XPRM_TOP_ST(ctx)->real);
 return chkerror(ctx,"arccos");
}

/******************************/
/* Arc tangent of 2 variables */
/******************************/
static int ma_atan2(XPRMcontext ctx,void *libctx)
{
 double x;

 x=XPRM_POP_REAL(ctx);
 XPRM_TOP_ST(ctx)->real=atan2(x,XPRM_TOP_ST(ctx)->real);
 return XPRM_RT_OK;
}

/*********************/
/* Hyperbolic cosine */
/*********************/
static int ma_cosh(XPRMcontext ctx,void *libctx)
{
 XPRM_TOP_ST(ctx)->real=cosh(XPRM_TOP_ST(ctx)->real);
 return XPRM_RT_OK;
}

/*******************/
/* Hyperbolic sine */
/*******************/
static int ma_sinh(XPRMcontext ctx,void *libctx)
{
 XPRM_TOP_ST(ctx)->real=sinh(XPRM_TOP_ST(ctx)->real);
 return XPRM_RT_OK;
}

/**********************/
/* Hyperbolic tangent */
/**********************/
static int ma_tanh(XPRMcontext ctx,void *libctx)
{
 XPRM_TOP_ST(ctx)->real=tanh(XPRM_TOP_ST(ctx)->real);
 return XPRM_RT_OK;
}

/*************************/
/* Hyperbolic arc cosine */
/*************************/
static int ma_acosh(XPRMcontext ctx,void *libctx)
{
 errno=0;
 XPRM_TOP_ST(ctx)->real=acosh(XPRM_TOP_ST(ctx)->real);
 return chkerror(ctx,"arccosh");
/* (x>0)?log(x+sqrt(x*x-1)):-log(-x+sqrt(x*x-1)); */
}

/***********************/
/* Hyperbolic arc sine */
/***********************/
static int ma_asinh(XPRMcontext ctx,void *libctx)
{
 errno=0;
 XPRM_TOP_ST(ctx)->real=asinh(XPRM_TOP_ST(ctx)->real);
 return chkerror(ctx,"arcsinh");
/* (x>0)?log(x+sqrt(x*x+1)):-log(-x+sqrt(x*x+1)); */
}

/**************************/
/* Hyperbolic arc tangent */
/**************************/
static int ma_atanh(XPRMcontext ctx,void *libctx)
{
 errno=0;
 XPRM_TOP_ST(ctx)->real=atanh(XPRM_TOP_ST(ctx)->real);
 return chkerror(ctx,"arctanh");
/* (x>=.5)?0.5*log((1+x)/(1-x)):0.5*log((2*x)+(2*x)*x/(1-x)); */
}

/**********************/
/* Euclidean distance */
/**********************/
static int ma_hypot(XPRMcontext ctx,void *libctx)
{
 double x;

 x=XPRM_POP_REAL(ctx);
 XPRM_TOP_ST(ctx)->real=hypot(x,XPRM_TOP_ST(ctx)->real);
 return XPRM_RT_OK;
}

/**************************************/
/* Convert floating-point number to   */
/* fractional and integral components */
/**************************************/
static int ma_frexp(XPRMcontext ctx,void *libctx)
{
 double x;
 XPRMlist l;
 XPRMalltypes e;

 x=XPRM_POP_REAL(ctx);
 l=XPRM_TOP_ST(ctx)->ref;
 XPRM_TOP_ST(ctx)->real=frexp(x,&e.integer);
 if((l!=NULL)&&(mm->getlisttype(l)&XPRM_GRP_DYN))
  mm->addellist(ctx,l,XPRM_TYP_INT,&e);
 return XPRM_RT_OK;
}

/***********************************/
/* Multiply  floating-point number */
/* by integral power of 2          */
/***********************************/
static int ma_ldexp(XPRMcontext ctx,void *libctx)
{
 double x;

 x=XPRM_POP_REAL(ctx);
 XPRM_TOP_ST(ctx)->real=ldexp(x,XPRM_TOP_ST(ctx)->integer);
 return XPRM_RT_OK;
}

/******************************************/
/* Get exponent of a floating-point value */
/******************************************/
static int ma_logb(XPRMcontext ctx,void *libctx)
{
 XPRM_TOP_ST(ctx)->real=logb(XPRM_TOP_ST(ctx)->real);
 return XPRM_RT_OK;
}

/********************************/
/* Logarithm of 1 plus argument */
/********************************/
static int ma_log1p(XPRMcontext ctx,void *libctx)
{
 XPRM_TOP_ST(ctx)->real=log1p(XPRM_TOP_ST(ctx)->real);
 return XPRM_RT_OK;
}

/**********************/
/* Cube root function */
/**********************/
static int ma_cbrt(XPRMcontext ctx,void *libctx)
{
 XPRM_TOP_ST(ctx)->real=cbrt(XPRM_TOP_ST(ctx)->real);
 return XPRM_RT_OK;
}

/*************************/
/* Copy sign of a number */
/*************************/
static int ma_cpysgn(XPRMcontext ctx,void *libctx)
{
 double x;

 x=XPRM_POP_REAL(ctx);
 XPRM_TOP_ST(ctx)->real=copysign(x,XPRM_TOP_ST(ctx)->real);
 return XPRM_RT_OK;
}

/*******************************************/
/* Next representable floating-point value */
/*******************************************/
static int ma_nextaft(XPRMcontext ctx,void *libctx)
{
 double x;

 x=XPRM_POP_REAL(ctx);
 XPRM_TOP_ST(ctx)->real=nextafter(x,XPRM_TOP_ST(ctx)->real);
 return XPRM_RT_OK;
}

/************************************/
/* Next larger floating-point value */
/************************************/
static int ma_nextup(XPRMcontext ctx,void *libctx)
{
 XPRM_TOP_ST(ctx)->real=nextafter(XPRM_TOP_ST(ctx)->real,HUGE_VAL);
 return XPRM_RT_OK;
}

/****************************/
/* Round to nearest integer */
/****************************/
static int ma_rint(XPRMcontext ctx,void *libctx)
{
 XPRM_TOP_ST(ctx)->real=rint(XPRM_TOP_ST(ctx)->real);
 return XPRM_RT_OK;
}

/*************************************************************/
/* Multiply floating-point number by integral power of radix */
/*************************************************************/
static int ma_scalb(XPRMcontext ctx,void *libctx)
{
 double x;

 x=XPRM_POP_REAL(ctx);
 XPRM_TOP_ST(ctx)->real=scalbn(x,XPRM_TOP_ST(ctx)->integer);
 return XPRM_RT_OK;
}

/*******************************/
/* Return sign of the argument */
/*******************************/
static int ma_sign(XPRMcontext ctx,void *libctx)
{
 double x;

 x=XPRM_TOP_ST(ctx)->real;
 XPRM_TOP_ST(ctx)->real=x<0?-1:x>0;
 return XPRM_RT_OK;
}

/*************************************/
/* Return sign of the argument (int) */
/*************************************/
static int ma_sign_i(XPRMcontext ctx,void *libctx)
{
 int x;

 x=XPRM_TOP_ST(ctx)->integer;
 XPRM_TOP_ST(ctx)->integer=x<0?-1:x>0;
 return XPRM_RT_OK;
}

/************/
/* RAD->DEG */
/************/
static int ma_todeg(XPRMcontext ctx,void *libctx)
{
 XPRM_TOP_ST(ctx)->real=XPRM_TOP_ST(ctx)->real*90/ma_pi_2;
 return XPRM_RT_OK;
}

/************/
/* DEG->RAD */
/************/
static int ma_torad(XPRMcontext ctx,void *libctx)
{
 XPRM_TOP_ST(ctx)->real=XPRM_TOP_ST(ctx)->real*ma_pi_2/90;
 return XPRM_RT_OK;
}

/*********************************************/
/* Round to 2 digits after the decimal point */
/*********************************************/
static int ma_round2d(XPRMcontext ctx,void *libctx)
{
 XPRM_TOP_ST(ctx)->real=rint(100*(XPRM_TOP_ST(ctx)->real))/100;
 return XPRM_RT_OK;
}

/*********************************************/
/* Round to n digits after the decimal point */
/*********************************************/
static int ma_roundnd(XPRMcontext ctx,void *libctx)
{
 double x,y;

 x=XPRM_POP_REAL(ctx);
 y=pow(10,XPRM_TOP_ST(ctx)->integer);
 XPRM_TOP_ST(ctx)->real=rint(y*x)/y;
 return XPRM_RT_OK;
}

/*********************************/
/* Bessel: first kind of order 0 */
/*********************************/
static int ma_j0(XPRMcontext ctx,void *libctx)
{
 XPRM_TOP_ST(ctx)->real=j0(XPRM_TOP_ST(ctx)->real);
 return XPRM_RT_OK;
}

/*********************************/
/* Bessel: first kind of order 1 */
/*********************************/
static int ma_j1(XPRMcontext ctx,void *libctx)
{
 XPRM_TOP_ST(ctx)->real=j1(XPRM_TOP_ST(ctx)->real);
 return XPRM_RT_OK;
}

/*********************************/
/* Bessel: first kind of order n */
/*********************************/
static int ma_jn(XPRMcontext ctx,void *libctx)
{
 int n;

 n=XPRM_POP_INT(ctx);
 XPRM_TOP_ST(ctx)->real=jn(n,XPRM_TOP_ST(ctx)->real);
 return XPRM_RT_OK;
}

/**********************************/
/* Bessel: second kind of order 0 */
/**********************************/
static int ma_y0(XPRMcontext ctx,void *libctx)
{
 XPRM_TOP_ST(ctx)->real=y0(XPRM_TOP_ST(ctx)->real);
 return XPRM_RT_OK;
}

/**********************************/
/* Bessel: second kind of order 1 */
/**********************************/
static int ma_y1(XPRMcontext ctx,void *libctx)
{
 XPRM_TOP_ST(ctx)->real=y1(XPRM_TOP_ST(ctx)->real);
 return XPRM_RT_OK;
}

/**********************************/
/* Bessel: second kind of order n */
/**********************************/
static int ma_yn(XPRMcontext ctx,void *libctx)
{
 int n;

 n=XPRM_POP_INT(ctx);
 XPRM_TOP_ST(ctx)->real=yn(n,XPRM_TOP_ST(ctx)->real);
 return XPRM_RT_OK;
}

/******************** Services ********************/

/***********************************/
/* Reset the math module for a run */
/***********************************/
static void *ma_reset(XPRMcontext ctx,void *libctx,int version)
{
 s_mathctx *mactx;

 if(libctx==NULL)               /* libctx==NULL => initialisation */
 {
  mactx=malloc(sizeof(s_mathctx));
  if(mactx==NULL)
  {
   mm->dispmsg(ctx,"Math: Out of memory error.\n");
   return NULL;
  }
  else
  {
   memset(mactx,0,sizeof(s_mathctx));
   return mactx;
  }
 }
 else
 {
  mactx=libctx;

  free(mactx->value);
  free(mactx);
  return NULL;
 }
}

/****************************************************************/
/* Check restrictions (this module implements all restrictions) */
/****************************************************************/
static int ma_chkres(int r)
{
 return 0;
}

/**********************************/
/* Handle compatibility checkings */
/**********************************/
static int ma_chkver(int v)
{
 return (v>MM_MKVER(VMAJ,VMIN,VREL))||(v<MM_MKVER(0,0,3));
}

/********************************************/
/* Update version number during compilation */
/********************************************/
static void ma_updvers(int type,int what,int *v)
{
 switch(type)
 {
  case XPRM_UPDV_INIT: if(*v <XPRM_MKVER(0,0,3)) *v=XPRM_MKVER(0,0,3); break;
  case XPRM_UPDV_FUNC:
	if(*v<XPRM_MKVER(1,0,0))
	{
	 if((what>=1300)&&(what<=1374))	/* int64 routines */
	 { *v=XPRM_MKVER(1,0,0); return; }
	}
	break;
  case XPRM_UPDV_TYPE:
	if(*v<XPRM_MKVER(1,0,0))
	 *v=XPRM_MKVER(1,0,0);
	break;
 }
}

/*****************************/
/* Memory used by the module */
/*****************************/
static size_t ma_memuse(XPRMcontext ctx,void *libctx,void *ref,int code)
{
 s_mathctx *mactx=libctx;

 switch(code)
 {
  case 0:
    return sizeof(s_mathctx)+mactx->nbmaxi64*(sizeof(int)+sizeof(int64_t));
  case 1:
    return sizeof(int64_t)+sizeof(int);
  default:
    return -1;
 }
}

#include "bigint.c"
