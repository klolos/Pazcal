
/******************************************************************************
 *  C++ code file : opt.cpp
 *  Project       : Pazcal Compiler
 *  Written by    : Lolos Konstantinos, Podimata Charikleia
 *                  (lolos.kostis@gmail.com, charapod@gmail.com)
 *  Date          : 2014-2015
 *  Description   : Intermediate code optimizations, header file
 ******************************************************************************/

#ifndef __OPTIMIZATIONS_H__
#define __OPTIMIZATIONS_H__

#include "stdio.h"
#include "general.h"
#include "error.h"
#include "symbol.h"
#include "gc/include/gc.h"
#include <stdlib.h>
#include <string.h>
#include <list>
#include <vector>
#include <string>


void optimize();
void remove_no_ops();

#endif
