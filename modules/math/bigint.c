/******************************************
  File bigint.c
  `````````````
  Example module defining a new type
    int64
  with the corresponding arithmetic operators.

  author: Y. Colombani

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
/*  to be included in 'math.c' */

#include <stdio.h>
#include <ctype.h>
#include <limits.h>

#ifdef _WIN32
#define snprintf _snprintf
#define strtoi64(a,b) _strtoi64(a,b,10)
#define FMLTI64 "%I64d"
#define MAX_INT64 LLONG_MAX
#define LABS llabs

#elif defined(__LP64__)||defined(_LP64)||defined(__ILP64__)||defined(_ILP64)
#define strtoi64(a,b) strtol(a,b,10)
#define FMLTI64 "%ld"
#define MAX_INT64 LONG_MAX
#define LABS labs
#else

#define strtoi64(a,b) strtoll(a,b,10)
#define FMLTI64 "%lld"
#define MAX_INT64 LLONG_MAX
#define LABS llabs
#endif

#define BI_CONST  (1<<30)       /* Marker for a constant int64 number */
#define BIVAL(r) (mactx->value[(size_t)(r)])
#define BIRCNT(r) (mactx->refcnt[(size_t)(r)])

static int i64isneg(s_mathctx *mactx,void *r);

/******** Functions implementing the operators ********/

/***********************/
/* Biggest int64 value */
/***********************/
static int bi_maxi64(XPRMcontext ctx,void *libctx)
{
 s_mathctx *mactx=libctx;
 void *r1;

 r1=bi_create(ctx,libctx,NULL,0);
 if(r1==NULL)
  return XPRM_RT_ERROR;
 else
 {
  BIVAL(r1)=MAX_INT64;
  XPRM_PUSH_REF(ctx,r1);
  return XPRM_RT_OK;
 }
}

/**********************/
/* isodd(int64)->bool */
/**********************/
static int bi_isodd(XPRMcontext ctx,void *libctx)
{
 s_mathctx *mactx=libctx;
 void *r1;

 r1=XPRM_POP_REF(ctx);
 XPRM_PUSH_INT(ctx,r1==NULL?0:BIVAL(r1)%2);
 return XPRM_RT_OK;
}

/*********************/
/* abs(int64)->int64 */
/*********************/
static int bi_abs(XPRMcontext ctx,void *libctx)
{
 s_mathctx *mactx=libctx;
 void *r1,*r2;

 r1=XPRM_POP_REF(ctx);
 r2=bi_create(ctx,libctx,NULL,0);
 if(r2==NULL)
  return XPRM_RT_ERROR;
 else
 {
  if(r1!=NULL)
   BIVAL(r2)=LABS(BIVAL(r1));
  XPRM_PUSH_REF(ctx,r2);
  return XPRM_RT_OK;
 }
}

/******************/
/* int64->integer */
/******************/
static int bi_asint(XPRMcontext ctx,void *libctx)
{
 s_mathctx *mactx=libctx;
 void *r1;

 r1=XPRM_POP_REF(ctx);
 XPRM_PUSH_INT(ctx,r1==NULL?0:BIVAL(r1));
 return XPRM_RT_OK;
}

/***************/
/* int64->real */
/***************/
static int bi_asreal(XPRMcontext ctx,void *libctx)
{
 s_mathctx *mactx=libctx;
 void *r1;

 r1=XPRM_POP_REF(ctx);
 XPRM_PUSH_REAL(ctx,r1==NULL?0:BIVAL(r1));
 return XPRM_RT_OK;
}

/******************/
/* Clone an int64 */
/******************/
static int bi_new0(XPRMcontext ctx,void *libctx)
{
 s_mathctx *mactx=libctx;
 void *r1,*r2;

 r1=XPRM_POP_REF(ctx);
 if(r1!=NULL)
 {
  r2=bi_create(ctx,libctx,NULL,0);
  if(r2==NULL) return XPRM_RT_ERROR;
  BIVAL(r2)=BIVAL(r1);
  XPRM_PUSH_REF(ctx,r2);
 }
 else
  XPRM_PUSH_REF(ctx,NULL);
 return XPRM_RT_OK;
}

/*******************************/
/* Create an int64 from a real */
/*******************************/
static int bi_new1r(XPRMcontext ctx,void *libctx)
{
 s_mathctx *mactx=libctx;
 void *r1;

 r1=bi_create(ctx,libctx,NULL,0);
 if(r1==NULL) return XPRM_RT_ERROR;
 BIVAL(r1)=(int64_t)XPRM_POP_REAL(ctx);
 XPRM_PUSH_REF(ctx,r1);
 return XPRM_RT_OK;
}

/***********************************/
/* Create an int64 from an integer */
/***********************************/
static int bi_new1i(XPRMcontext ctx,void *libctx)
{
 s_mathctx *mactx=libctx;
 void *r1;

 r1=bi_create(ctx,libctx,NULL,0);
 if(r1==NULL) return XPRM_RT_ERROR;
 BIVAL(r1)=XPRM_POP_INT(ctx);
 XPRM_PUSH_REF(ctx,r1);
 return XPRM_RT_OK;
}

/********************************************/
/* Zero in int64 (used to initialise `sum') */
/********************************************/
static int bi_zero(XPRMcontext ctx,void *libctx)
{
 XPRM_PUSH_REF(ctx,bi_create(ctx,libctx,NULL,0));
 return XPRM_RT_OK;
}

/********************************************/
/* One in int64 (used to initialise `prod') */
/********************************************/
static int bi_one(XPRMcontext ctx,void *libctx)
{
 s_mathctx *mactx=libctx;
 void *r1;

 r1=bi_create(ctx,libctx,NULL,0);
 if(r1==NULL) return XPRM_RT_ERROR;
 BIVAL(r1)=1;
 XPRM_PUSH_REF(ctx,r1);
 return XPRM_RT_OK;
}

/********************************************/
/* Smallest value for int64 (used in 'max') */
/********************************************/
static int bi_imax(XPRMcontext ctx,void *libctx)
{
 s_mathctx *mactx=libctx;
 void *r1;

 r1=bi_create(ctx,libctx,NULL,0);
 if(r1==NULL) return XPRM_RT_ERROR;
 BIVAL(r1)=-MAX_INT64-1;
 XPRM_PUSH_REF(ctx,r1);
 return XPRM_RT_OK;
}

/***************************/
/* Assignment int64:=int64 */
/***************************/
static int bi_asgn(XPRMcontext ctx,void *libctx)
{
 s_mathctx *mactx=libctx;
 void *r1,*r2;

 r1=XPRM_POP_REF(ctx);
 r2=XPRM_POP_REF(ctx);
 if(r1==NULL)
 {
  mm->dispmsg(ctx,"Math: Trying to access an uninitialized int64.\n");
  return RT_ERROR;
 }
 else
 if(BIRCNT(r1)&BI_CONST)
 {
  mm->dispmsg(ctx,"Math: Trying to modify a constant.\n");
  return RT_ERROR;
 }
 else
 if(r2==NULL)
 {
  BIVAL(r1)=0;
  return XPRM_RT_OK;
 }
 else
 {
  BIVAL(r1)=BIVAL(r2);
  bi_delete(ctx,libctx,r2,0);
  return XPRM_RT_OK;
 }
}

/*****************************/
/* Assignment int64:=integer */
/*****************************/
static int bi_asgn_i(XPRMcontext ctx,void *libctx)
{
 s_mathctx *mactx=libctx;
 void *r1;

 r1=XPRM_POP_REF(ctx);
 if(r1==NULL)
 {
  mm->dispmsg(ctx,"Math: Trying to access an uninitialized int64.\n");
  return RT_ERROR;
 }
 else
 if(BIRCNT(r1)&BI_CONST)
 {
  mm->dispmsg(ctx,"Math: Trying to modify a constant.\n");
  return RT_ERROR;
 }
 else
 {
  BIVAL(r1)=XPRM_POP_INT(ctx);
  return XPRM_RT_OK;
 }
}

/*********************************/
/* Addition int64+int64 -> int64 */
/*********************************/
static int bi_pls(XPRMcontext ctx,void *libctx)
{
 s_mathctx *mactx=libctx;
 void *r1,*r2;

 r1=XPRM_POP_REF(ctx);
 r2=XPRM_POP_REF(ctx);
 if(r1!=NULL)
 {
  if(r2!=NULL)
  {
   BIVAL(r1)+=BIVAL(r2);
   bi_delete(ctx,libctx,r2,0);
  }
  XPRM_PUSH_REF(ctx,r1);
 }
 else
  XPRM_PUSH_REF(ctx,r2);
 return XPRM_RT_OK;
}

/***********************************/
/* Addition int64+integer -> int64 */
/***********************************/
static int bi_pls_i(XPRMcontext ctx,void *libctx)
{
 s_mathctx *mactx=libctx;
 void *r1;

 r1=XPRM_POP_REF(ctx);
 if(r1!=NULL)
 {
  BIVAL(r1)+=XPRM_POP_INT(ctx);
  XPRM_PUSH_REF(ctx,r1);
  return XPRM_RT_OK;
 }
 else
  return bi_new1i(ctx,libctx);
}

/*******************************/
/* Addition int64+real -> real */
/*******************************/
static int bi_pls_r(XPRMcontext ctx,void *libctx)
{
 s_mathctx *mactx=libctx;
 void *r1;
 double d;

 r1=XPRM_POP_REF(ctx);
 if(r1!=NULL)
 {
  d=XPRM_POP_REAL(ctx)+(double)BIVAL(r1);
  bi_delete(ctx,libctx,r1,0);
  XPRM_PUSH_REAL(ctx,d);
  return XPRM_RT_OK;
 }
 else
  return XPRM_RT_OK;
}

/**********************************/
/* Change of sign int64 -> -int64 */
/**********************************/
static int bi_neg(XPRMcontext ctx,void *libctx)
{
 s_mathctx *mactx=libctx;
 void *r1;

 r1=XPRM_POP_REF(ctx);
 if(r1!=NULL)
  BIVAL(r1)=-BIVAL(r1);
 XPRM_PUSH_REF(ctx,r1);
 return XPRM_RT_OK;
}

/********************************/
/* Product int64*int64 -> int64 */
/********************************/
static int bi_mul(XPRMcontext ctx,void *libctx)
{
 s_mathctx *mactx=libctx;
 void *r1,*r2;

 r1=XPRM_POP_REF(ctx);
 r2=XPRM_POP_REF(ctx);
 if(r1!=NULL)
 {
  if(r2!=NULL)
   BIVAL(r1)*=BIVAL(r2);
  else
   BIVAL(r1)=0;
 }
 bi_delete(ctx,libctx,r2,0);
 XPRM_PUSH_REF(ctx,r1);
 return XPRM_RT_OK;
}

/**********************************/
/* Product int64*integer -> int64 */
/**********************************/
static int bi_mul_i(XPRMcontext ctx,void *libctx)
{
 s_mathctx *mactx=libctx;
 void *r1;
 int64_t i;

 r1=XPRM_POP_REF(ctx);
 i=XPRM_POP_INT(ctx);
 if(r1!=NULL)
  BIVAL(r1)*=i;
 XPRM_PUSH_REF(ctx,r1);
 return XPRM_RT_OK;
}

/******************************/
/* Product int64*real -> real */
/******************************/
static int bi_mul_r(XPRMcontext ctx,void *libctx)
{
 s_mathctx *mactx=libctx;
 void *r1;
 double d;

 r1=XPRM_POP_REF(ctx);
 d=XPRM_POP_REAL(ctx);
 if(r1!=NULL)
 {
  d*=(double)BIVAL(r1);
  bi_delete(ctx,libctx,r1,0);
 }
 else
  d=0;
 XPRM_PUSH_REAL(ctx,d);
 return XPRM_RT_OK;
}

/********************************/
/* Division int64/int64 -> real */
/********************************/
static int bi_div(XPRMcontext ctx,void *libctx)
{
 s_mathctx *mactx=libctx;
 void *r1,*r2;

 r1=XPRM_POP_REF(ctx);
 r2=XPRM_POP_REF(ctx);
 if((r2==NULL)||(BIVAL(r2)==0))
 {
  mm->dispmsg(ctx,"Math: Division by 0.\n");
  XPRM_PUSH_REAL(ctx,strtod(i64isneg(mactx,r1)?"-inf":"inf",NULL));
  bi_delete(ctx,libctx,r1,0);
  bi_delete(ctx,libctx,r2,0);
  return XPRM_RT_MATHERR;
 }
 else
 {
  if(r1!=NULL)
  {
   XPRM_PUSH_REAL(ctx,(double)BIVAL(r1)/(double)BIVAL(r2));
   bi_delete(ctx,libctx,r1,0);
  }
  else
   XPRM_PUSH_REAL(ctx,0);
  bi_delete(ctx,libctx,r2,0);
  return XPRM_RT_OK;
 }
}

/**********************************/
/* Division int64/integer -> real */
/**********************************/
static int bi_div_i1(XPRMcontext ctx,void *libctx)
{
 s_mathctx *mactx=libctx;
 void *r1;
 double i;

 r1=XPRM_POP_REF(ctx);
 i=XPRM_POP_INT(ctx);
 if(i==0)
 {
  mm->dispmsg(ctx,"Math: Division by 0.\n");
  XPRM_PUSH_REAL(ctx,strtod(i64isneg(mactx,r1)?"-inf":"inf",NULL));
  bi_delete(ctx,libctx,r1,0);
  return XPRM_RT_MATHERR;
 }
 else
 {
  if(r1==NULL)
   XPRM_PUSH_REAL(ctx,0);
  else
  {
   XPRM_PUSH_REAL(ctx,(double)BIVAL(r1)/i);
   bi_delete(ctx,libctx,r1,0);
  }
  return XPRM_RT_OK;
 }
}

/**********************************/
/* Division integer/int64 -> real */
/**********************************/
static int bi_div_i2(XPRMcontext ctx,void *libctx)
{
 s_mathctx *mactx=libctx;
 void *r1;
 double i;

 i=XPRM_POP_INT(ctx);
 r1=XPRM_POP_REF(ctx);
 if((r1==NULL)||(BIVAL(r1)==0))
 {
  mm->dispmsg(ctx,"Math: Division by 0.\n");
  XPRM_PUSH_REAL(ctx,strtod(i<0?"-inf":"inf",NULL));
  bi_delete(ctx,libctx,r1,0);
  return XPRM_RT_MATHERR;
 }
 else
 {
  XPRM_PUSH_REAL(ctx,i/(double)BIVAL(r1));
  bi_delete(ctx,libctx,r1,0);
  return XPRM_RT_OK;
 }
}

/*******************************/
/* Division int64/real -> real */
/*******************************/
static int bi_div_r1(XPRMcontext ctx,void *libctx)
{
 s_mathctx *mactx=libctx;
 void *r1;
 double d;

 r1=XPRM_POP_REF(ctx);
 d=XPRM_POP_REAL(ctx);
 if(d==0)
 {
  mm->dispmsg(ctx,"Math: Division by 0.\n");
  XPRM_PUSH_REAL(ctx,strtod(i64isneg(mactx,r1)?"-inf":"inf",NULL));
  bi_delete(ctx,libctx,r1,0);
  return XPRM_RT_MATHERR;
 }
 else
 {
  if(r1==NULL)
   XPRM_PUSH_REAL(ctx,0);
  else
  {
   XPRM_PUSH_REAL(ctx,(double)BIVAL(r1)/d);
   bi_delete(ctx,libctx,r1,0);
  }
  return XPRM_RT_OK;
 }
}

/*******************************/
/* Division real/int64 -> real */
/*******************************/
static int bi_div_r2(XPRMcontext ctx,void *libctx)
{
 s_mathctx *mactx=libctx;
 void *r1;
 double d;

 d=XPRM_POP_REAL(ctx);
 r1=XPRM_POP_REF(ctx);
 if((r1==NULL)||(BIVAL(r1)==0))
 {
  mm->dispmsg(ctx,"Math: Division by 0.\n");
  XPRM_PUSH_REAL(ctx,strtod(d<0?"-inf":"inf",NULL));
  bi_delete(ctx,libctx,r1,0);
  return XPRM_RT_MATHERR;
 }
 else
 {
  XPRM_PUSH_REAL(ctx,d/(double)BIVAL(r1));
  bi_delete(ctx,libctx,r1,0);
  return XPRM_RT_OK;
 }
}

/*************************************/
/* Division int64 div int64 -> int64 */
/*************************************/
static int bi_idiv(XPRMcontext ctx,void *libctx)
{
 s_mathctx *mactx=libctx;
 void *r1,*r2;

 r1=XPRM_POP_REF(ctx);
 r2=XPRM_POP_REF(ctx);
 if((r2==NULL)||(BIVAL(r2)==0))
 {
  mm->dispmsg(ctx,"Math: Division by 0.\n");
  bi_delete(ctx,libctx,r2,0);
  if((r1==NULL)&&((r1=bi_create(ctx,libctx,NULL,0))==NULL))
   return XPRM_RT_ERROR;
  else
  {
   BIVAL(r1)=BIVAL(r1)<0?-MAX_INT64-1:MAX_INT64;
   XPRM_PUSH_REF(ctx,r1);
   return XPRM_RT_MATHERR;
  }
 }
 else
 {
  if(r1!=NULL)
   BIVAL(r1)=BIVAL(r1)/BIVAL(r2);
  XPRM_PUSH_REF(ctx,r1);
  bi_delete(ctx,libctx,r2,0);
  return XPRM_RT_OK;
 }
}

/***************************************/
/* Division int64 div integer -> int64 */
/***************************************/
static int bi_idiv_i1(XPRMcontext ctx,void *libctx)
{
 s_mathctx *mactx=libctx;
 void *r1;
 int64_t i;

 r1=XPRM_POP_REF(ctx);
 i=XPRM_POP_INT(ctx);
 if(i==0)
 {
  mm->dispmsg(ctx,"Math: Division by 0.\n");
  if((r1==NULL)&&((r1=bi_create(ctx,libctx,NULL,0))==NULL))
   return XPRM_RT_ERROR;
  else
  {
   BIVAL(r1)=BIVAL(r1)<0?-MAX_INT64-1:MAX_INT64;
   XPRM_PUSH_REF(ctx,r1);
   return XPRM_RT_MATHERR;
  }
 }
 else
 {
  if(r1!=NULL)
   BIVAL(r1)/=i;
  XPRM_PUSH_REF(ctx,r1);
  return XPRM_RT_OK;
 }
}

/*****************************************/
/* Division integer div int64 -> integer */
/*****************************************/
static int bi_idiv_i2(XPRMcontext ctx,void *libctx)
{
 s_mathctx *mactx=libctx;
 void *r1;
 int64_t i;

 i=XPRM_POP_INT(ctx);
 r1=XPRM_POP_REF(ctx);
 if((r1==NULL)||(BIVAL(r1)==0))
 {
  mm->dispmsg(ctx,"Math: Division by 0.\n");
  XPRM_PUSH_INT(ctx,i<0?-INT_MAX-1:INT_MAX);
  bi_delete(ctx,libctx,r1,0);
  return XPRM_RT_MATHERR;
 }
 else
 {
  XPRM_PUSH_INT(ctx,(int)(i/BIVAL(r1)));
  bi_delete(ctx,libctx,r1,0);
  return XPRM_RT_OK;
 }
}

/*************************************/
/* Division int64 mod int64 -> int64 */
/*************************************/
static int bi_mod(XPRMcontext ctx,void *libctx)
{
 s_mathctx *mactx=libctx;
 void *r1,*r2;

 r1=XPRM_POP_REF(ctx);
 r2=XPRM_POP_REF(ctx);
 if((r2==NULL)||(BIVAL(r2)==0))
 {
  mm->dispmsg(ctx,"Math: Division by 0.\n");
  bi_delete(ctx,libctx,r2,0);
  if((r1==NULL)&&((r1=bi_create(ctx,libctx,NULL,0))==NULL))
   return XPRM_RT_ERROR;
  else
  {
   BIVAL(r1)=BIVAL(r1)<0?-MAX_INT64-1:MAX_INT64;
   XPRM_PUSH_REF(ctx,r1);
   return XPRM_RT_MATHERR;
  }
 }
 else
 {
  if(r1!=NULL)
   BIVAL(r1)=BIVAL(r1)%BIVAL(r2);
  XPRM_PUSH_REF(ctx,r1);
  bi_delete(ctx,libctx,r2,0);
  return XPRM_RT_OK;
 }
}

/***************************************/
/* Division int64 mod integer -> int64 */
/***************************************/
static int bi_mod_i1(XPRMcontext ctx,void *libctx)
{
 s_mathctx *mactx=libctx;
 void *r1;
 int64_t i;

 r1=XPRM_POP_REF(ctx);
 i=XPRM_POP_INT(ctx);
 if(i==0)
 {
  mm->dispmsg(ctx,"Math: Division by 0.\n");
  if((r1==NULL)&&((r1=bi_create(ctx,libctx,NULL,0))==NULL))
   return XPRM_RT_ERROR;
  else
  {
   BIVAL(r1)=BIVAL(r1)<0?-MAX_INT64-1:MAX_INT64;
   XPRM_PUSH_REF(ctx,r1);
   return XPRM_RT_MATHERR;
  }
 }
 else
 {
  if(r1!=NULL)
   BIVAL(r1)%=i;
  XPRM_PUSH_REF(ctx,r1);
  return XPRM_RT_OK;
 }
}

/*****************************************/
/* Division integer mod int64 -> integer */
/*****************************************/
static int bi_mod_i2(XPRMcontext ctx,void *libctx)
{
 s_mathctx *mactx=libctx;
 void *r1;
 int64_t i;

 i=XPRM_POP_INT(ctx);
 r1=XPRM_POP_REF(ctx);
 if((r1==NULL)||(BIVAL(r1)==0))
 {
  mm->dispmsg(ctx,"Math: Division by 0.\n");
  XPRM_PUSH_INT(ctx,i<0?-INT_MAX-1:INT_MAX);
  bi_delete(ctx,libctx,r1,0);
  return XPRM_RT_MATHERR;
 }
 else
 {
  XPRM_PUSH_INT(ctx,(int)(i % BIVAL(r1)));
  bi_delete(ctx,libctx,r1,0);
  return XPRM_RT_OK;
 }
}

/****************************/
/* Comparison int64=integer */
/****************************/
static int bi_eql_i(XPRMcontext ctx,void *libctx)
{
 s_mathctx *mactx=libctx;
 void *r1;
 int i;
 int b;

 r1=XPRM_POP_REF(ctx);
 i=XPRM_POP_INT(ctx);
 if(r1!=NULL)
  b=BIVAL(r1)==i;
 else
  b=(0==i);
 XPRM_PUSH_INT(ctx,b);
 return XPRM_RT_OK;
}

/****************************/
/* Comparison integer=int64 */
/****************************/
static int bi_eql_i2(XPRMcontext ctx,void *libctx)
{
 s_mathctx *mactx=libctx;
 void *r1;
 int i;
 int b;

 i=XPRM_POP_INT(ctx);
 r1=XPRM_POP_REF(ctx);
 if(r1!=NULL)
  b=BIVAL(r1)==i;
 else
  b=(0==i);
 XPRM_PUSH_INT(ctx,b);
 return XPRM_RT_OK;
}

/****************************/
/* Comparison int64<integer */
/****************************/
static int bi_lt_i(XPRMcontext ctx,void *libctx)
{
 s_mathctx *mactx=libctx;
 void *r1;
 int i;
 int b;

 r1=XPRM_POP_REF(ctx);
 i=XPRM_POP_INT(ctx);
 if(r1!=NULL)
  b=BIVAL(r1)<i;
 else
  b=(0<i);
 XPRM_PUSH_INT(ctx,b);
 return XPRM_RT_OK;
}

/****************************/
/* Comparison integer<int64 */
/****************************/
static int bi_lt_i2(XPRMcontext ctx,void *libctx)
{
 s_mathctx *mactx=libctx;
 void *r1;
 int i;
 int b;

 i=XPRM_POP_INT(ctx);
 r1=XPRM_POP_REF(ctx);
 if(r1!=NULL)
  b=BIVAL(r1)>i;
 else
  b=(0>i);
 XPRM_PUSH_INT(ctx,b);
 return XPRM_RT_OK;
}

/****************************/
/* Comparison int64>integer */
/****************************/
static int bi_gt_i(XPRMcontext ctx,void *libctx)
{
 s_mathctx *mactx=libctx;
 void *r1;
 int i;
 int b;

 r1=XPRM_POP_REF(ctx);
 i=XPRM_POP_INT(ctx);
 if(r1!=NULL)
  b=BIVAL(r1)>i;
 else
  b=(0>i);
 XPRM_PUSH_INT(ctx,b);
 return XPRM_RT_OK;
}

/****************************/
/* Comparison integer>int64 */
/****************************/
static int bi_gt_i2(XPRMcontext ctx,void *libctx)
{
 s_mathctx *mactx=libctx;
 void *r1;
 int i;
 int b;

 i=XPRM_POP_INT(ctx);
 r1=XPRM_POP_REF(ctx);
 if(r1!=NULL)
  b=BIVAL(r1)<i;
 else
  b=(0<i);
 XPRM_PUSH_INT(ctx,b);
 return XPRM_RT_OK;
}

/****************************/
/* Allocate an int64 number */
/****************************/
static void *bi_create(XPRMcontext ctx,void *libctx,void *todup,int typnum)
{
 s_mathctx *mactx=libctx;
 size_t newi;

 if((todup!=NULL)&&(XPRM_CREATE(typnum)==XPRM_CREATE_NEW))
 {
  BIRCNT(todup)++;
  return todup;
 }
 else
 {
  if(mactx->nbmaxi64>mactx->nbi64)
  {
   if(mactx->firstfree!=0)
   {
    newi=mactx->firstfree;
    mactx->firstfree=(size_t)BIVAL(newi);
   }
   else
    newi=mactx->nbi64;
  }
  else
  {
   int newnbm;
   int64_t *newv;

   if(mactx->nbmaxi64==0)
    newnbm=340;
   else
   if(mactx->nbmaxi64<10000)
    newnbm=mactx->nbmaxi64*2;
   else
    newnbm=mactx->nbmaxi64+10000;
   newv=realloc(mactx->value,newnbm*(sizeof(int)+sizeof(int64_t)));
   if(newv==NULL)
   {
    mm->dispmsg(ctx,"Math: Out of memory error.\n");
    return NULL;
   }
   else
   {
    mactx->value=newv;
    mactx->refcnt=(unsigned int*)(newv+newnbm);
    if(mactx->nbi64==0) /* the first entry is not used */
    {
     BIVAL(0)=0;
     BIRCNT(0)=0;
     newi=mactx->nbi64=1;
    }
    else
    {
     memmove(mactx->refcnt,mactx->value+mactx->nbmaxi64,mactx->nbmaxi64*sizeof(int));
     newi=mactx->nbi64;
    }
    mactx->nbmaxi64=newnbm;
   }
  }
  mactx->nbi64++;
  if(XPRM_CREATE(typnum)==XPRM_CREATE_CST)
  {
   BIVAL(newi)=BIVAL(todup);
   BIRCNT(newi)=1|BI_CONST;
  }
  else
  {
   BIVAL(newi)=0;
   BIRCNT(newi)=1;
  }
  return (void*)newi;
 }
}

/******************************/
/* Deallocate an int64 number */
/******************************/
static void bi_delete(XPRMcontext ctx,void *libctx,void *todel,int typnum)
{
 s_mathctx *mactx=libctx;

 if((todel!=NULL)&&(((--BIRCNT(todel))&~BI_CONST)<1))
 {
  BIRCNT(todel)=0;
  BIVAL(todel)=(int64_t)(mactx->firstfree);
  mactx->firstfree=(size_t)todel;
  mactx->nbi64--;
 }
}

/*******************/
/* int64 -> String */
/*******************/
static int bi_tostr(XPRMcontext ctx,void *libctx,void *toprt,char *str,int len,int typnum)
{
 s_mathctx *mactx=libctx;

 if(typnum&XPRM_TFSTR_BIN)
 {
  if(len>=sizeof(int64_t))
  {
   if(toprt==NULL)
    memset(str,0,sizeof(int64_t));
   else
   {
    /* We assume that all supported platforms are little endian */
    memcpy(str,&BIVAL(toprt),sizeof(int64_t));
   }
  }
  return sizeof(int64_t);
 }
 else
 if(toprt==NULL)
 {
  strncpy(str,"0",len);
  return 1;
 }
 else
  return snprintf(str,len,FMLTI64,BIVAL(toprt));
}

/*******************/
/* String -> int64 */
/*******************/
static int bi_fromstr(XPRMcontext ctx,void *libctx,void *toinit,const char *str,int typnum,const char **endptr)
{
 s_mathctx *mactx=libctx;

 if(BIRCNT(toinit)&BI_CONST)
 {
  mm->dispmsg(ctx,"Math: Trying to modify a constant.\n");
  return XPRM_RT_ERROR;
 }
 else
 if(typnum&XPRM_TFSTR_BIN)
 {
  if(*endptr-str!=sizeof(int64_t))
   return XPRM_RT_ERROR;
  else
  {
   /* We assume that all supported platforms are little endian */
   memcpy(&BIVAL(toinit),str,sizeof(int64_t));
   return XPRM_RT_OK;
  }
 }
 else
 {
  char *ep;

  BIVAL(toinit)=strtoi64(str,&ep);
  if(endptr!=NULL) *endptr=ep;
  if((ep==str)|| !isdigit(ep[-1]))
   return XPRM_RT_ERROR;
  else
   return XPRM_RT_OK;
 }
}

/*****************/
/* Copy an int64 */
/*****************/
static int bi_copy(XPRMcontext ctx,void *libctx,void *toinit,void *src,int typnum)
{
 s_mathctx *mactx=libctx;

 switch(XPRM_CPY(typnum))
 {
  case XPRM_CPY_COPY:
  case XPRM_CPY_RESET:
      if(BIRCNT(toinit)&BI_CONST)
       return 1;
      else
      {
       BIVAL(toinit)=(src!=NULL)?BIVAL(src):0;
       return 0;
      }
  case XPRM_CPY_APPEND:
      if(src!=NULL)
       BIVAL(toinit)+=BIVAL(src);
      return 0;
  case XPRM_CPY_HASH:
      {
       if(src==NULL)
       {
        int64_t z=0;

        *(unsigned int *)toinit=mm->hashmix(ctx,0,&z,sizeof(int64_t));
       }
       else
        *(unsigned int *)toinit=mm->hashmix(ctx,0,&BIVAL(src),sizeof(int64_t));
       return 0;
      }
  default:
      return 1;
 }
}

/**************************/
/* Compare 2 int64 values */
/**************************/
static int bi_compare(XPRMcontext ctx,void *libctx,void *r1,void *r2,int typnum)
{
 s_mathctx *mactx=libctx;
 int64_t i1,i2;

 i1=(r1==NULL)?0:BIVAL(r1);
 i2=(r2==NULL)?0:BIVAL(r2);
 
 switch(XPRM_COMPARE(typnum))
 {
  case XPRM_COMPARE_EQ: return i1==i2;
  case XPRM_COMPARE_NEQ: return i1!=i2;
  case XPRM_COMPARE_LTH: return i1<i2;
  case XPRM_COMPARE_LEQ: return i1<=i2;
  case XPRM_COMPARE_GEQ: return i1>=i2;
  case XPRM_COMPARE_GTH: return i1>i2;
  case XPRM_COMPARE_CMP:
    if(i1==i2)
     return 0;
    else
     return (i1<i2)?-1:1;
  default:
    return XPRM_COMPARE_ERROR;
 }
}

/************************************/
/* IMCI: retrieve value of an int64 */
/************************************/
static int64_t imci_getint64val(XPRMcontext ctx,s_mathctx *mactx,void *ref)
{
 if(ref==NULL)
  return 0;
 else
  return BIVAL(ref);
}

/*******************************/
/* IMCI: set value of an int64 */
/*******************************/
static int imci_setint64val(XPRMcontext ctx,s_mathctx *mactx,void *ref,int64_t v)
{
 if(ref==NULL)
  return 1;
 else
 if(BIRCNT(ref)&BI_CONST)
  return 2;
 else
 {
  BIVAL(ref)=v;
  return 0;
 }
}

/**************************************/
/* Check whether an int64 is negative */
/**************************************/
static int i64isneg(s_mathctx *mactx,void *r)
{
 return (r!=NULL)&&(BIVAL(r)<0);
}
