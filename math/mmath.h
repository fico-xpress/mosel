/******************************************
  File mmath.h
  ````````````
  IMCI interface to module math

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

#ifndef _MMATH_H
#define _MMATH_H

#include <stdint.h>

struct MathCtx;

typedef struct Mmath_imci
	{
	 int64_t (*getint64val)(struct Vimactx *ctx,struct MathCtx *mactx,void *ref);
	 int (*setint64val)(struct Vimactx *ctx,struct MathCtx *mactx,void *ref,int64_t v);
	} *mmath_imci;
#endif
