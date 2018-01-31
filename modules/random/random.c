/******************************************
  File random.c
  `````````````
  Example module defining a random number generator
  and associated distributions.
  See below for copyright notices

  author: N. Verma & Y. Colombani, 2006 

  (c) Copyright 2018 Fair Isaac Corporation
  
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
cl /nologo /MD /I%XPRESSDIR%\include /LD random.c /Ferandom.dso
gcc -Wall -o random.dso -shared -D_REENTRANT -I${XPRESSDIR}/include random.c -lm
*/

/* 
   A C-program for MT19937, with initialization improved 2002/1/26.
   Coded by Takuji Nishimura and Makoto Matsumoto.

   Before using, initialize the state by using init_genrand(seed)  
   or init_by_array(init_key, key_length).

   Copyright (C) 1997 - 2002, Makoto Matsumoto and Takuji Nishimura,
   All rights reserved.                          

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

     1. Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.

     2. Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

     3. The names of its contributors may not be used to endorse or promote 
        products derived from this software without specific prior written 
        permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER
   OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


   Any feedback is very welcome.
   http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/emt.html
   email: m-mat @ math.sci.hiroshima-u.ac.jp (remove space)
*/
/* -------------------------------------------------------------------------- 
 * This is an ANSI C library for generating random variates from six discrete 
 * distributions
 *
 *      Generator         Range (x)     Mean         Variance
 *
 *      Bernoulli(p)      x = 0,1       p            p*(1-p)
 *      Binomial(n, p)    x = 0,...,n   n*p          n*p*(1-p)
 *      Equilikely(a, b)  x = a,...,b   (a+b)/2      ((b-a+1)*(b-a+1)-1)/12
 *      Geometric(p)      x = 0,...     p/(1-p)      p/((1-p)*(1-p))
 *      Pascal(n, p)      x = 0,...     n*p/(1-p)    n*p/((1-p)*(1-p))
 *      Poisson(m)        x = 0,...     m            m
 * 
 * and seven continuous distributions
 *
 *      Uniform(a, b)     a < x < b     (a + b)/2    (b - a)*(b - a)/12 
 *      Exponential(m)    x > 0         m            m*m
 *      Erlang(n, b)      x > 0         n*b          n*b*b
 *      Normal(m, s)      all x         m            s*s
 *      Lognormal(a, b)   x > 0            see below
 *      Chisquare(n)      x > 0         n            2*n 
 *      Student(n)        all x         0  (n > 1)   n/(n - 2)   (n > 2)
 *
 * For the a Lognormal(a, b) random variable, the mean and variance are
 *
 *                        mean = exp(a + 0.5*b*b)
 *                    variance = (exp(b*b) - 1) * exp(2*a + b*b)
 *
 * Name              : rvgs.c  (Random Variate GeneratorS)
 * Author            : Steve Park & Dave Geyer
 * Language          : ANSI C
 * Latest Revision   : 10-28-98
 * http://www.cs.wm.edu/~va/software/park/park.html
 * --------------------------------------------------------------------------
 */

#include <stdlib.h>
#include <string.h>
#include <math.h>
#define MM_NICOMPAT 3000000
#include "xprm_ni.h"

/* Period parameters */  
#define N 624
#define M 397
#define MATRIX_A 0x9908b0dfU    /* constant vector a */
#define UPPER_MASK 0x80000000U  /* most significant w-r bits */
#define LOWER_MASK 0x7fffffffU  /* least significant r bits */

/**** Structures used by this module ****/
typedef struct
	{
	 unsigned int mt[N];	/* the array for the state vector  */
	 int mti;		/* mti==N+1 means mt[N] is not initialized */
	} s_rndctx;

/**** Function prototypes ****/
static int chkres(int);
static void *rnd_reset(XPRMcontext ctx,void *libctx,int version);
static void init_genrand(s_rndctx *rndctx,unsigned int s);
static int rnd_setseed(XPRMcontext ctx,void *libctx);
static int rnd_setseeda(XPRMcontext ctx,void *libctx);
static unsigned int genrand_int32(s_rndctx *rndctx);
static int rnd_int32(XPRMcontext ctx,void *libctx);
static int rnd_int32b(XPRMcontext ctx,void *libctx);
static int rnd_int31(XPRMcontext ctx,void *libctx);
static int rnd_real1(XPRMcontext ctx,void *libctx);
static int rnd_real2(XPRMcontext ctx,void *libctx);
static int rnd_real3(XPRMcontext ctx,void *libctx);
static double genrand_res53(s_rndctx *rndctx);
static int rnd_real(XPRMcontext ctx,void *libctx);
static int rnd_realb(XPRMcontext ctx,void *libctx);

static int bernoulli(s_rndctx *rndctx,double p);
static int rnd_bernoulli(XPRMcontext ctx,void *libctx);
static int rnd_binomial(XPRMcontext ctx,void *libctx);
static int geometric(s_rndctx *rndctx,double p);
static int rnd_geometric(XPRMcontext ctx,void *libctx);
static int rnd_pascal(XPRMcontext ctx,void *libctx);
static double exponential(s_rndctx *rndctx,double m);
static int rnd_exponential(XPRMcontext ctx,void *libctx);
static int rnd_poisson(XPRMcontext ctx,void *libctx);
static int rnd_erlang(XPRMcontext ctx,void *libctx);
static double normal(s_rndctx *rndctx,double m,double s);
static int rnd_normal(XPRMcontext ctx,void *libctx);
static int rnd_lognormal(XPRMcontext ctx,void *libctx);
static double chisquare(s_rndctx *rndctx,int n);
static int rnd_chisquare(XPRMcontext ctx,void *libctx);
static int rnd_student(XPRMcontext ctx,void *libctx);
static int rnd_math_error(XPRMcontext ctx,const char *name);

/**** Structures for passing info to Mosel ****/
/* Subroutines */
static XPRMdsofct tabfct[]=
        {
         {"setmtrandseed",1000,XPRM_TYP_NOT,1,"i",rnd_setseed},
         {"setmtrandseed",1001,XPRM_TYP_NOT,1,"A.i",rnd_setseeda},
         {"mtrand_int",1002,XPRM_TYP_INT,0,NULL,rnd_int32},
         {"mtrand_int",1003,XPRM_TYP_INT,2,"ii",rnd_int32b},
         {"equilikely",1003,XPRM_TYP_INT,2,"ii",rnd_int32b},
         {"mtrand_intp",1004,XPRM_TYP_INT,0,NULL,rnd_int31},
         {"mtrand_real1",1005,XPRM_TYP_REAL,0,NULL,rnd_real1},
         {"mtrand_real2",1006,XPRM_TYP_REAL,0,NULL,rnd_real2},
         {"mtrand_real3",1007,XPRM_TYP_REAL,0,NULL,rnd_real3},
         {"mtrand_real",1008,XPRM_TYP_REAL,0,NULL,rnd_real},
         {"mtrand_real",1009,XPRM_TYP_REAL,2,"rr",rnd_realb},
         {"uniform",1009,XPRM_TYP_REAL,2,"rr",rnd_realb},

         {"bernoulli",1100,XPRM_TYP_INT,1,"r",rnd_bernoulli},
         {"binomial",1101,XPRM_TYP_INT,2,"ir",rnd_binomial},
         {"chisquare",1102,XPRM_TYP_REAL,1,"i",rnd_chisquare},
         {"erlang",1103,XPRM_TYP_REAL,2,"ir",rnd_erlang},
         {"exponential",1104,XPRM_TYP_REAL,1,"r",rnd_exponential},
         {"geometric",1105,XPRM_TYP_INT,1,"r",rnd_geometric},
         {"lognormal",1106,XPRM_TYP_REAL,2,"rr",rnd_lognormal},
         {"normal",1107,XPRM_TYP_REAL,2,"rr",rnd_normal},
         {"pascal",1108,XPRM_TYP_INT,2,"ir",rnd_pascal},
         {"poisson",1109,XPRM_TYP_INT,1,"r",rnd_poisson},
         {"student",1110,XPRM_TYP_REAL,1,"i",rnd_student}
	};

/* Services */
static XPRMdsoserv tabserv[]=
        {
         {XPRM_SRV_RESET,(void *)rnd_reset},
	 {XPRM_SRV_CHKRES,(void*)chkres}
        };

/* Interface structure */
static XPRMdsointer dsointer= 
        { 
         0,NULL,
         sizeof(tabfct)/sizeof(XPRMdsofct),tabfct,
         0,NULL,
         sizeof(tabserv)/sizeof(XPRMdsoserv),tabserv
        };

static XPRMnifct mm;             /* For storing Mosel NI function table */

/************************************************/
/* Initialize the library just after loading it */
/************************************************/
DSO_INIT random_init(XPRMnifct nifct, int *interver,int *libver, XPRMdsointer **interf)
{
 mm=nifct;                      /* Save the table of Mosel NI functions */
 *interver=XPRM_NIVERS;         /* The Mosel NI version we are using */
 *libver=XPRM_MKVER(0,0,1);     /* The version of the module: 0.0.1 */
 *interf=&dsointer;             /* Our interface */

 return 0;
}

/******************** Services ********************/

/****************************************************************/
/* Check restrictions (this module implements all restrictions) */
/****************************************************************/
static int chkres(int r)
{ return 0; }

/**************************************/
/* Reset the random module for a run  */
/**************************************/
static void *rnd_reset(XPRMcontext ctx,void *libctx,int version)
{
 s_rndctx *rndctx;
 int jdn,t;

 if(libctx==NULL)               /* libctx==NULL => initialisation */
 {
  rndctx=malloc(sizeof(s_rndctx));
  if(rndctx==NULL)
  {
   mm->dispmsg(ctx,"RANDOM: Out of memory.\n");
   return NULL;
  }
  memset(rndctx,0,sizeof(s_rndctx));
  mm->time(ctx,&jdn,&t,XPRM_TIME_LOCAL);
  init_genrand(rndctx,jdn*t);
  return rndctx;
 }
 else                           /* otherwise release the resources we use */
 {
  free(libctx);
  return NULL;
 }
}

/******** Functions implementing subroutines ********/

/* initializes mt[N] with a seed */
static void init_genrand(s_rndctx *rndctx,unsigned int s)
{
    rndctx->mt[0]= s;
    for (rndctx->mti=1; rndctx->mti<N; rndctx->mti++) {
        rndctx->mt[rndctx->mti] = (1812433253U * 
		(rndctx->mt[rndctx->mti-1]^(rndctx->mt[rndctx->mti-1] >> 30))+
		rndctx->mti); 
        /* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
        /* In the previous versions, MSBs of the seed affect   */
        /* only MSBs of the array mt[].                        */
        /* 2002/01/09 modified by Makoto Matsumoto             */
    }
}

static int rnd_setseed(XPRMcontext ctx,void *libctx)
{
 init_genrand(libctx,XPRM_POP_INT(ctx));
 return RT_OK;
}

/* initialize by an array with array-length */
/* init_key is the array for initializing keys */
/* key_length is its length */
/* slight change for C++, 2004/2/26 */
static int rnd_setseeda(XPRMcontext ctx,void *libctx)
{
    s_rndctx *rndctx;
    XPRMarray arrseeds;
    int key_length,dim,indices[15];
    XPRMalltypes init_key;
    int i, j, k;

    rndctx=libctx;
    arrseeds=XPRM_POP_REF(ctx);
    if((arrseeds==NULL)||((key_length=mm->getarrsize(arrseeds))<1)||
       ((dim=mm->getarrdim(arrseeds))>15))
    {
     mm->dispmsg(ctx,"RANDOM: invalid data for `setmtrandseed'.\n");
     return RT_ERROR;
    }

    mm->getfirstarrtruentry(arrseeds,indices);
    init_genrand(rndctx,19650218U);
    i=1;j=0;
    k = (N>key_length ? N : key_length);
    for (; k; k--) {
        mm->getarrval(arrseeds,indices,&init_key);
        rndctx->mt[i] = (rndctx->mt[i]^ 
			((rndctx->mt[i-1]^(rndctx->mt[i-1] >> 30))*1664525U))+
			init_key.integer + j; /* non linear */
        i++; j++;
        if (i>=N) { rndctx->mt[0] = rndctx->mt[N-1]; i=1; }
        if(mm->getnextarrtruentry(arrseeds,indices))
	{
	 mm->getfirstarrtruentry(arrseeds,indices);
	 j=0;
        }
    }
    for (k=N-1; k; k--) {
        rndctx->mt[i] = (rndctx->mt[i]^
			((rndctx->mt[i-1]^(rndctx->mt[i-1]>>30))*1566083941U))-
			i; /* non linear */
        i++;
        if (i>=N) { rndctx->mt[0] = rndctx->mt[N-1]; i=1; }
    }

    rndctx->mt[0] = 0x80000000U; /* MSB is 1; assuring non-zero initial array */
    return RT_OK;
}

/* generates a random number on [0,0xffffffff]-interval */
static unsigned int genrand_int32(s_rndctx *rndctx)
{
    unsigned int y;
    static unsigned int mag01[2]={0x0U, MATRIX_A};
    /* mag01[x] = x * MATRIX_A  for x=0,1 */

    if (rndctx->mti >= N) { /* generate N words at one time */
        int kk;

        if (rndctx->mti == N+1)   /* if init_genrand() has not been called, */
            init_genrand(rndctx,5489U); /* a default initial seed is used */

        for (kk=0;kk<N-M;kk++) {
            y = (rndctx->mt[kk]&UPPER_MASK)|(rndctx->mt[kk+1]&LOWER_MASK);
            rndctx->mt[kk] = rndctx->mt[kk+M] ^ (y >> 1) ^ mag01[y & 0x1U];
        }
        for (;kk<N-1;kk++) {
            y = (rndctx->mt[kk]&UPPER_MASK)|(rndctx->mt[kk+1]&LOWER_MASK);
            rndctx->mt[kk] = rndctx->mt[kk+(M-N)] ^ (y >> 1) ^ mag01[y & 0x1U];
        }
        y = (rndctx->mt[N-1]&UPPER_MASK)|(rndctx->mt[0]&LOWER_MASK);
        rndctx->mt[N-1] = rndctx->mt[M-1] ^ (y >> 1) ^ mag01[y & 0x1U];

        rndctx->mti = 0;
    }
  
    y = rndctx->mt[rndctx->mti++];

    /* Tempering */
    y ^= (y >> 11);
    y ^= (y << 7) & 0x9d2c5680U;
    y ^= (y << 15) & 0xefc60000U;
    y ^= (y >> 18);

    return y;
}

static int rnd_int32(XPRMcontext ctx,void *libctx)
{
 XPRM_PUSH_INT(ctx,(signed)genrand_int32(libctx));
 return RT_OK;
}

static int rnd_int32b(XPRMcontext ctx,void *libctx)
{
 int lo,up;

 lo=XPRM_POP_INT(ctx);
 up=XPRM_TOP_ST(ctx)->integer;
 if(lo!=up)
 {
  if(lo>up)
  {
   up=lo;
   lo=XPRM_TOP_ST(ctx)->integer;
  }
  XPRM_TOP_ST(ctx)->integer=lo+(signed)(genrand_int32(libctx)%(up-lo+1));
 }
 return RT_OK;
}

/* generates a random number on [0,0x7fffffff]-interval */
static int rnd_int31(XPRMcontext ctx,void *libctx)
{
 XPRM_PUSH_INT(ctx,(int)(genrand_int32(libctx)>>1));
 return RT_OK;
}

/* generates a random number on [0,1]-real-interval */
static int rnd_real1(XPRMcontext ctx,void *libctx)
{
 XPRM_PUSH_REAL(ctx,genrand_int32(libctx)*(1.0/4294967295.0)); 
    /* divided by 2^32-1 */ 
 return RT_OK;
}

/* generates a random number on [0,1)-real-interval */
static int rnd_real2(XPRMcontext ctx,void *libctx)
{
 XPRM_PUSH_REAL(ctx,genrand_int32(libctx)*(1.0/4294967296.0)); 
    /* divided by 2^32 */
 return RT_OK;
}

/* generates a random number on (0,1)-real-interval */
static int rnd_real3(XPRMcontext ctx,void *libctx)
{
 XPRM_PUSH_REAL(ctx,(((double)genrand_int32(libctx))+0.5)*(1.0/4294967296.0)); 
    /* divided by 2^32 */
 return RT_OK;
}

/* generates a random number on [0,1) with 53-bit resolution*/
static double genrand_res53(s_rndctx *rndctx) 
{ 
    unsigned int a=genrand_int32(rndctx)>>5, b=genrand_int32(rndctx)>>6; 
    return(a*67108864.0+b)*(1.0/9007199254740992.0); 
} 

static int rnd_real(XPRMcontext ctx,void *libctx)
{
 XPRM_PUSH_REAL(ctx,genrand_res53(libctx)); 
 return RT_OK;
}

static int rnd_realb(XPRMcontext ctx,void *libctx)
{
 double lo,up;

 lo=XPRM_POP_REAL(ctx);
 up=XPRM_TOP_ST(ctx)->real;
 if(lo!=up)
 {
  if(lo>up)
  {
   up=lo;
   lo=XPRM_TOP_ST(ctx)->real;
  }
  XPRM_TOP_ST(ctx)->real=lo+(genrand_res53(libctx)*(up-lo));
 }
 return RT_OK;
}

/* These real versions are due to Isaku Wada, 2002/01/09 added */

#define RANDOM genrand_res53(rndctx)
/* ========================================================
 * Returns 1 with probability p or 0 with probability 1 - p. 
 * NOTE: use 0.0 < p < 1.0                                   
 * ========================================================
 */ 
static int bernoulli(s_rndctx *rndctx,double p)
{ return ((RANDOM < (1.0 - p)) ? 0 : 1); }

static int rnd_bernoulli(XPRMcontext ctx,void *libctx)
{
 double p;

 p=XPRM_TOP_ST(ctx)->real;
 if((p<=0)||(p>=1))
  return rnd_math_error(ctx,"bernoulli");
 else
 {
  XPRM_TOP_ST(ctx)->integer=bernoulli(libctx,p);
  return XPRM_RT_OK;
 }
}

/* ================================================================ 
 * Returns a binomial distributed integer between 0 and n inclusive. 
 * NOTE: use n > 0 and 0.0 < p < 1.0
 * ================================================================
 */
static int rnd_binomial(XPRMcontext ctx,void *libctx)
{
 int n,x;
 double p;

 n=XPRM_POP_INT(ctx);
 p=XPRM_TOP_ST(ctx)->real;
 if((n<=0)||(p<=0)||(p>=1))
  return rnd_math_error(ctx,"binomial");
 else
 {
  x=0;
  for(;n>0;n--)
   x+=bernoulli(libctx,p);
  XPRM_TOP_ST(ctx)->integer=x;
  return XPRM_RT_OK;
 }
}

/* ====================================================
 * Returns a geometric distributed non-negative integer.
 * NOTE: use 0.0 < p < 1.0
 * ====================================================
 */
static int geometric(s_rndctx *rndctx,double p)
{ return ((int) (log(1.0 - RANDOM) / log(p))); }

static int rnd_geometric(XPRMcontext ctx,void *libctx)
{
 double p;

 p=XPRM_TOP_ST(ctx)->real;
 if((p<=0)||(p>=1))
  return rnd_math_error(ctx,"geometric");
 else
 {
  XPRM_TOP_ST(ctx)->integer=geometric(libctx,p);
  return XPRM_RT_OK;
 }
}

/* ================================================= 
 * Returns a Pascal distributed non-negative integer. 
 * NOTE: use n > 0 and 0.0 < p < 1.0
 * =================================================
 */
static int rnd_pascal(XPRMcontext ctx,void *libctx)
{
 int n,x;
 double p;

 n=XPRM_POP_INT(ctx);
 p=XPRM_TOP_ST(ctx)->real;
 if((n<=0)||(p<=0)||(p>=1))
  return rnd_math_error(ctx,"pascal");
 else
 {
  x=0;
  for(;n>0;n--)
   x+=geometric(libctx,p);
  XPRM_TOP_ST(ctx)->integer=x;
  return XPRM_RT_OK;
 }
}

/* =========================================================
 * Returns an exponentially distributed positive real number. 
 * NOTE: use m > 0.0
 * =========================================================
 */
static double exponential(s_rndctx *rndctx,double m)
{ return (-m * log(1.0 - RANDOM)); }

static int rnd_exponential(XPRMcontext ctx,void *libctx)
{
 double m;

 m=XPRM_TOP_ST(ctx)->real;
 if(m<=0)
  return rnd_math_error(ctx,"exponential");
 else
 {
  XPRM_TOP_ST(ctx)->real=exponential(libctx,m);
  return XPRM_RT_OK;
 }
}

/* ================================================== 
 * Returns a Poisson distributed non-negative integer. 
 * NOTE: use m > 0
 * ==================================================
 */
static int rnd_poisson(XPRMcontext ctx,void *libctx)
{
 double m,t;
 int x;

 m=XPRM_TOP_ST(ctx)->real;
 if(m<=0)
  return rnd_math_error(ctx,"poisson");
 else
 {
  t=0;
  x=0;
  while (t < m) {
    t += exponential(libctx,1.0);
    x++;
  }
  XPRM_TOP_ST(ctx)->integer=x-1;
  return XPRM_RT_OK;
 }
}

/* ================================================== 
 * Returns an Erlang distributed positive real number.
 * NOTE: use n > 0 and b > 0.0
 * ==================================================
 */
static int rnd_erlang(XPRMcontext ctx,void *libctx)
{
 int n;
 double b,x;

 n=XPRM_POP_INT(ctx);
 b=XPRM_TOP_ST(ctx)->real;
 if((n<=0)||(b<=0))
  return rnd_math_error(ctx,"erlang");
 else
 {
  x=0;
  for(;n>0;n--)
   x+=exponential(libctx,b);
  XPRM_TOP_ST(ctx)->real=x;
  return XPRM_RT_OK;
 }
}

/* ========================================================================
 * Returns a normal (Gaussian) distributed real number.
 * NOTE: use s > 0.0
 *
 * Uses a very accurate approximation of the normal idf due to Odeh & Evans, 
 * J. Applied Statistics, 1974, vol 23, pp 96-97.
 * ========================================================================
 */
static double normal(s_rndctx *rndctx,double m,double s)
{
  const double p0 = 0.322232431088;     const double q0 = 0.099348462606;
  const double p1 = 1.0;                const double q1 = 0.588581570495;
  const double p2 = 0.342242088547;     const double q2 = 0.531103462366;
  const double p3 = 0.204231210245e-1;  const double q3 = 0.103537752850;
  const double p4 = 0.453642210148e-4;  const double q4 = 0.385607006340e-2;
  double u, t, p, q, z;

  u   = RANDOM;
  if (u < 0.5)
    t = sqrt(-2.0 * log(u));
  else
    t = sqrt(-2.0 * log(1.0 - u));
  p   = p0 + t * (p1 + t * (p2 + t * (p3 + t * p4)));
  q   = q0 + t * (q1 + t * (q2 + t * (q3 + t * q4)));
  if (u < 0.5)
    z = (p / q) - t;
  else
    z = t - (p / q);
  return (m + s * z);
}

static int rnd_normal(XPRMcontext ctx,void *libctx)
{
 double m,s;

 m=XPRM_POP_REAL(ctx);
 s=XPRM_TOP_ST(ctx)->real;
 if(s<=0)
  return rnd_math_error(ctx,"normal");
 else
 {
  XPRM_TOP_ST(ctx)->real=normal(libctx,m,s);
  return XPRM_RT_OK;
 }
}

/* ==================================================== 
 * Returns a lognormal distributed positive real number. 
 * NOTE: use b > 0.0
 * ====================================================
 */
static int rnd_lognormal(XPRMcontext ctx,void *libctx)
{
 double a,b;

 a=XPRM_POP_REAL(ctx);
 b=XPRM_TOP_ST(ctx)->real;
 if(b<=0)
  return rnd_math_error(ctx,"lognormal");
 else
 {
  XPRM_TOP_ST(ctx)->real=(exp(a + b * normal(libctx,0.0, 1.0)));
  return XPRM_RT_OK;
 }
}

/* =====================================================
 * Returns a chi-square distributed positive real number. 
 * NOTE: use n > 0
 * =====================================================
 */
static double chisquare(s_rndctx *rndctx,int n)
{
  long   i;
  double z, x = 0.0;

  for (i = 0; i < n; i++) {
    z  = normal(rndctx,0.0, 1.0);
    x += z * z;
  }
  return (x);
}

static int rnd_chisquare(XPRMcontext ctx,void *libctx)
{
 int n;

 n=XPRM_TOP_ST(ctx)->integer;
 if(n<=0)
  return rnd_math_error(ctx,"chisquare");
 else
 {
  XPRM_TOP_ST(ctx)->real=chisquare(libctx,n);
  return XPRM_RT_OK;
 }
}

/* =========================================== 
 * Returns a student-t distributed real number.
 * NOTE: use n > 0
 * ===========================================
 */
static int rnd_student(XPRMcontext ctx,void *libctx)
{
 int n;

 n=XPRM_TOP_ST(ctx)->integer;
 if(n<=0)
  return rnd_math_error(ctx,"student");
 else
 {
  XPRM_TOP_ST(ctx)->real=normal(libctx,0.0,1.0)/sqrt(chisquare(libctx,n)/n);
  return XPRM_RT_OK;
 }
}

/*************************************/
/* Display an error message and fail */
/*************************************/
static int rnd_math_error(XPRMcontext ctx,const char *name)
{
 mm->dispmsg(ctx,"RANDOM: Invalid parameters for `%s'\n",name);
 return XPRM_RT_MATHERR;
}
