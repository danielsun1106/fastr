/*
 * Copyright (c) 2014, 2018, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3 only, as
 * published by the Free Software Foundation.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 3 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 3 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 */

/* This file is "included" by the corresponding Rinternals.c in the
   truffle_nfi and truffle_llvm directories.
   The implementation must define the following functions:

   char *ensure_truffle_chararray_n(const char *x, int n)
     Ensures that the sequence of 'n' bytes starting at 'x' is in the
     appropriate representation for the implementation.

   void *ensure_string(const char *x)
     Ensures that (on the Java side of the upcall) x, which must be null-terminated,
     appears as a java.lang.String

   void *ensure_function(void *fptr)
     Ensures that (on the Java side of the upcall) fptr appears as an executable
     Truffle object.

   Any of these functions could be the identity function.
*/

// tracing/debugging support, set to 1 and recompile to enable
#define TRACE_UPCALLS 0    // trace upcalls

#define TARGp "%s(%p)\n"
#define TARGpp "%s(%p, %p)\n"
#define TARGppp "%s(%p, %p, %p)\n"
#define TARGpd "%s(%p, %d)\n"
#define TARGppd "%s(%p, %p, %d)\n"
#define TARGs "%s(\"%s\")\n"
#define TARGps "%s(%p, \"%s\")\n"
#define TARGsdd "%s(\"%s\", %d, %d)\n"

#if TRACE_UPCALLS
#define TRACE(format, ...) printf("%s " format "\n", __FUNCTION__, __VA_ARGS__)
#define TRACE0() printf("%s\n", __FUNCTION__)
#define TRACE1(x) printf("%s %p\n", __FUNCTION__, x)
#define TRACE2(x, y) printf("%s %p %p\n", __FUNCTION__, x, y)
#define TRACE3(x, y, z) printf("%s %p %p %p\n", __FUNCTION__, x, y, z)
#else
#define TRACE(format, ...)
#define TRACE0()
#define TRACE1(x)
#define TRACE2(x, y)
#define TRACE3(x, y, z)
#endif

#define UNIMPLEMENTED unimplemented(__FUNCTION__)

#define NO_FASTR_REDEFINE
#include <rffiutils.h>

// these two functions are here just to handle casting void* to void function pointers...
DL_FUNC R_ExternalPtrAddrFn(SEXP s) {
    return (DL_FUNC) R_ExternalPtrAddr(s);
}

SEXP R_MakeExternalPtrFn(DL_FUNC p, SEXP tag, SEXP prot) {
    return R_MakeExternalPtr((void *) p, tag, prot);
}

// R_GlobalEnv et al are not a variables in FASTR as they are RContext specific
SEXP FASTR_R_GlobalEnv() {
    TRACE0();
    SEXP result = ((call_R_GlobalEnv) callbacks[R_GlobalEnv_x])();
    checkExitCall();
    return result;
}

SEXP FASTR_R_BaseEnv() {
    TRACE0();
    SEXP result = ((call_R_BaseEnv) callbacks[R_BaseEnv_x])();
    checkExitCall();
    return result;
}

SEXP FASTR_R_BaseNamespace() {
    TRACE0();
    SEXP result = ((call_R_BaseNamespace) callbacks[R_BaseNamespace_x])();
    checkExitCall();
    return result;
}

SEXP FASTR_R_NamespaceRegistry() {
    TRACE0();
    SEXP result = ((call_R_NamespaceRegistry) callbacks[R_NamespaceRegistry_x])();
    checkExitCall();
    return result;
}

CTXT FASTR_GlobalContext() {
    TRACE0();
    CTXT result = ((call_R_GlobalContext) callbacks[R_GlobalContext_x])();
    checkExitCall();
    return result;
}

Rboolean FASTR_R_Interactive() {
    TRACE0();
    int result = (int) ((call_R_Interactive) callbacks[R_Interactive_x])();
    checkExitCall();
    return result;
}

SEXP CAR(SEXP e) {
    TRACE1(e);
    SEXP result = ((call_CAR) callbacks[CAR_x])(e);
    checkExitCall();
    return result;
}

SEXP CDR(SEXP e) {
    TRACE1(e);
    SEXP result = ((call_CDR) callbacks[CDR_x])(e);
    checkExitCall();
    return result;
}

int LENGTH(SEXP x) {
    TRACE1(x);
    int result = ((call_LENGTH) callbacks[LENGTH_x])(x);
    checkExitCall();
    return result;
}

SEXP Rf_ScalarString(SEXP value) {
    TRACE1(value);
    SEXP result = ((call_Rf_ScalarString) callbacks[Rf_ScalarString_x])(value);
    checkExitCall();
    return result;
}

SEXP Rf_mkString(const char *s) {
    TRACE0();
    return ScalarString(Rf_mkChar(s));
}

void Rf_gsetVar(SEXP symbol, SEXP value, SEXP rho) {
    TRACE0();
    ((call_Rf_gsetVar) callbacks[Rf_gsetVar_x])(symbol, value, rho);
    checkExitCall();
}

SEXP Rf_coerceVector(SEXP x, SEXPTYPE mode) {
    TRACE(TARGpp, x, mode);
    SEXP result = ((call_Rf_coerceVector) callbacks[Rf_coerceVector_x])(x, mode);
    checkExitCall();
    return result;
}

SEXP Rf_cons(SEXP car, SEXP cdr) {
    TRACE0();
    SEXP result = ((call_Rf_cons) callbacks[Rf_cons_x])(car, cdr);
    checkExitCall();
    return result;
}

SEXP Rf_GetOption1(SEXP tag) {
    TRACE0();
    return ((call_Rf_GetOption1) callbacks[Rf_GetOption1_x])(tag);
}

SEXP Rf_mkChar(const char *x) {
    TRACE0();
    return Rf_mkCharLenCE(x, strlen(x), CE_NATIVE);
}

SEXP Rf_mkCharCE(const char *x, cetype_t y) {
    TRACE0();
    return Rf_mkCharLenCE(x, strlen(x), y);
}

SEXP Rf_mkCharLen(const char *x, int y) {
    TRACE0();
    return Rf_mkCharLenCE(x, y, CE_NATIVE);
}

SEXP Rf_mkCharLenCE(const char *x, int len, cetype_t enc) {
    TRACE0();
    SEXP result = ((call_Rf_mkCharLenCE) callbacks[Rf_mkCharLenCE_x])(ensure_truffle_chararray_n(x, len), len, enc);
    checkExitCall();
    return result;
}

#define BUFSIZE 8192

static int Rvsnprintf(char *buf, size_t size, const char  *format, va_list ap) {
    TRACE0();
    int val;
    val = vsnprintf(buf, size, format, ap);
    buf[size-1] = '\0';
    return val;
}

void Rf_errorcall(SEXP x, const char *format, ...) {
    TRACE0();
    // See also comments in Rf_error
    char buf[BUFSIZE];
    va_list(ap);
    va_start(ap,format);
    Rvsnprintf(buf, BUFSIZE - 1, format, ap);
    va_end(ap);
    ((call_Rf_errorcall) callbacks[Rf_errorcall_x])(x, ensure_string(buf));
    checkExitCall();
    // Should not reach here
    unimplemented("Unexpected return from Rf_errorcall, should be no return function");
}

void Rf_warningcall(SEXP x, const char *format, ...) {
    TRACE0();
    char buf[8192];
    va_list(ap);
    va_start(ap,format);
    Rvsnprintf(buf, BUFSIZE - 1, format, ap);
    va_end(ap);
    ((call_Rf_warningcall) callbacks[Rf_warningcall_x])(x, ensure_string(buf));
}

void Rf_warning(const char *format, ...) {
    TRACE0();
    char buf[8192];
    va_list(ap);
    va_start(ap, format);
    Rvsnprintf(buf, BUFSIZE - 1, format, ap);
    va_end(ap);
    ((call_Rf_warning) callbacks[Rf_warning_x])(ensure_string(buf));
}

void Rprintf(const char *format, ...) {
    TRACE0();
    char buf[8192];
    va_list(ap);
    va_start(ap,format);
    Rvsnprintf(buf, BUFSIZE - 1, format, ap);
    va_end(ap);
    ((call_Rprintf) callbacks[Rprintf_x])(ensure_string(buf));
}

void Rf_error(const char *format, ...) {
    TRACE0();
    // This is a bit tricky. The usual error handling model in Java is "throw RError.error(...)" but
    // RError.error does quite a lot of stuff including potentially searching for R condition handlers
    // and, if it finds any, does not return, but throws a different exception than RError.
    // We definitely need to exit the FFI call and we certainly cannot return to our caller.
    char buf[BUFSIZE];
    va_list(ap);
    va_start(ap,format);
    Rvsnprintf(buf, BUFSIZE - 1, format, ap);
    va_end(ap);
    ((call_Rf_error) callbacks[Rf_error_x])(ensure_string(buf));
    checkExitCall();
    // Should not reach here
    unimplemented("Unexpected return from Rf_error, should be no return function");
}

/*
  REprintf is used by the error handler do not add
  anything unless you're sure it won't
  cause problems
*/
void REprintf(const char *format, ...) {
    TRACE0();
    // TODO: determine correct target for this message
    char buf[8192];
    va_list(ap);
    va_start(ap,format);
    Rvsnprintf(buf, BUFSIZE - 1, format, ap);
    va_end(ap);
    // TODO
}

void Rvprintf(const char *format, va_list args) {
    TRACE0();
    UNIMPLEMENTED;
}

void REvprintf(const char *format, va_list args) {
    TRACE0();
    UNIMPLEMENTED;
}

SEXP Rf_ScalarInteger(int value) {
    TRACE0();
    SEXP result = ((call_Rf_ScalarInteger) callbacks[Rf_ScalarInteger_x])(value);
    checkExitCall();
    return result;
}

SEXP Rf_ScalarReal(double value) {
    TRACE0();
    SEXP result = ((call_Rf_ScalarReal) callbacks[Rf_ScalarReal_x])(value);
    checkExitCall();
    return result;
}

SEXP Rf_ScalarLogical(int value) {
    TRACE0();
    SEXP result = ((call_Rf_ScalarLogical) callbacks[Rf_ScalarLogical_x])(value);
    checkExitCall();
    return result;
}

SEXP Rf_allocVector3(SEXPTYPE t, R_xlen_t len, R_allocator_t* allocator) {
    TRACE0();
    if (allocator != NULL) {
        return UNIMPLEMENTED;
    }
    SEXP result = ((call_Rf_allocVector) callbacks[Rf_allocVector_x])(t, len);
    checkExitCall();
    return result;
}

SEXP Rf_allocArray(SEXPTYPE t, SEXP dims) {
    TRACE0();
    SEXP result = ((call_Rf_allocArray) callbacks[Rf_allocArray_x])(t, dims);
    checkExitCall();
    return result;
}

SEXP Rf_alloc3DArray(SEXPTYPE t, int x, int y, int z) {
    TRACE0();
    return UNIMPLEMENTED;
}

SEXP Rf_allocMatrix(SEXPTYPE mode, int nrow, int ncol) {
    TRACE0();
    SEXP result = ((call_Rf_allocMatrix) callbacks[Rf_allocMatrix_x])(mode, nrow, ncol);
    checkExitCall();
    return result;
}

SEXP Rf_allocList(int x) {
    TRACE0();
    return UNIMPLEMENTED;
}

SEXP Rf_allocSExp(SEXPTYPE t) {
    TRACE0();
    return UNIMPLEMENTED;
}

void Rf_defineVar(SEXP symbol, SEXP value, SEXP rho) {
    TRACE0();
    ((call_Rf_defineVar) callbacks[Rf_defineVar_x])(symbol, value, rho);
    checkExitCall();
}

void Rf_setVar(SEXP x, SEXP y, SEXP z) {
    TRACE0();
    unimplemented("Rf_setVar");
}

SEXP Rf_dimgets(SEXP x, SEXP y) {
    TRACE0();
    return unimplemented("Rf_dimgets");
}

SEXP Rf_dimnamesgets(SEXP x, SEXP y) {
    TRACE0();
    return unimplemented("Rf_dimnamesgets");
}

SEXP Rf_eval(SEXP expr, SEXP env) {
    TRACE0();
    SEXP result = ((call_Rf_eval) callbacks[Rf_eval_x])(expr, env);
    checkExitCall();
    return result;
}

SEXP Rf_findFun(SEXP symbol, SEXP rho) {
    TRACE0();
    SEXP result = ((call_Rf_findFun) callbacks[Rf_findFun_x])(symbol, rho);
    checkExitCall();
    return result;
}

SEXP Rf_findVar(SEXP sym, SEXP rho) {
    TRACE0();
    SEXP result = ((call_Rf_findVar) callbacks[Rf_findVar_x])(sym, rho);
    checkExitCall();
    return result;
}

SEXP Rf_findVarInFrame(SEXP rho, SEXP sym) {
    TRACE0();
    SEXP result = ((call_Rf_findVarInFrame) callbacks[Rf_findVarInFrame_x])(rho, sym);
    checkExitCall();
    return result;
}

SEXP Rf_findVarInFrame3(SEXP rho, SEXP sym, Rboolean b) {
    TRACE0();
    return ((call_Rf_findVarInFrame3) callbacks[Rf_findVarInFrame3_x])(rho, sym, b);
}

SEXP Rf_getAttrib(SEXP vec, SEXP name) {
    TRACE0();
    SEXP result = ((call_Rf_getAttrib) callbacks[Rf_getAttrib_x])(vec, name);
//    printf("Rf_getAttrib: %p\n", result);
    return result;
}

SEXP Rf_setAttrib(SEXP vec, SEXP name, SEXP val) {
    TRACE0();
    SEXP result = ((call_Rf_setAttrib) callbacks[Rf_setAttrib_x])(vec, name, val);
    checkExitCall();
    return result;
}

SEXP Rf_duplicate(SEXP x) {
    TRACE(TARGp, x);
    SEXP result = ((call_Rf_duplicate) callbacks[Rf_duplicate_x])(x, 1);
    checkExitCall();
    return result;
}

SEXP Rf_shallow_duplicate(SEXP x) {
    TRACE(TARGp, x);
    SEXP result = ((call_Rf_duplicate) callbacks[Rf_duplicate_x])(x, 0);
    checkExitCall();
    return result;
}

R_xlen_t Rf_any_duplicated(SEXP x, Rboolean from_last) {
    TRACE0();
    R_xlen_t result = (R_xlen_t) ((call_Rf_any_duplicated) callbacks[Rf_any_duplicated_x])(x, from_last);
    checkExitCall();
    return result;
}

R_xlen_t Rf_any_duplicated3(SEXP x, SEXP incomp, Rboolean from_last) {
    TRACE0();
    R_xlen_t result = (R_xlen_t) ((call_Rf_any_duplicated3) callbacks[Rf_any_duplicated3_x])(x, incomp, from_last);
    checkExitCall();
    return result;
}

SEXP Rf_duplicated(SEXP x, Rboolean y) {
    TRACE0();
    unimplemented("Rf_duplicated");
    return NULL;
}

SEXP Rf_applyClosure(SEXP x, SEXP y, SEXP z, SEXP a, SEXP b) {
    TRACE0();
    return unimplemented("Rf_applyClosure");
}

void Rf_copyMostAttrib(SEXP x, SEXP y) {
	((call_Rf_copyMostAttrib) callbacks[Rf_copyMostAttrib_x])(x, y);
	checkExitCall();
}

void Rf_copyVector(SEXP x, SEXP y) {
    TRACE0();
    unimplemented("Rf_copyVector");
}

int Rf_countContexts(int x, int y) {
    TRACE0();
    unimplemented("Rf_countContexts");
    return 0;
}

Rboolean Rf_inherits(SEXP x, const char * klass) {
    TRACE0();
    Rboolean result = (Rboolean) ((call_Rf_inherits) callbacks[Rf_inherits_x])(x, ensure_string(klass));
    checkExitCall();
    return result;
}

Rboolean Rf_isObject(SEXP s) {
    TRACE0();
    unimplemented("Rf_isObject");
    return FALSE;
}

void Rf_PrintValue(SEXP x) {
    TRACE0();
    ((call_Rf_PrintValue) callbacks[Rf_PrintValue_x])(x);
    checkExitCall();
}

SEXP Rf_install(const char *name) {
    TRACE0();
    SEXP result = ((call_Rf_install) callbacks[Rf_install_x])(ensure_string(name));
    checkExitCall();
    return result;
}

SEXP Rf_installChar(SEXP charsxp) {
    TRACE0();
    SEXP result = ((call_Rf_installChar) callbacks[Rf_installChar_x])(charsxp);
    checkExitCall();
    return result;
}

Rboolean Rf_isNull(SEXP s) {
    TRACE0();
    Rboolean result = (Rboolean) ((call_Rf_isNull) callbacks[Rf_isNull_x])(s);
    checkExitCall();
    return result;
}

Rboolean Rf_isString(SEXP s) {
    TRACE0();
    Rboolean result = (Rboolean) ((call_Rf_isString) callbacks[Rf_isString_x])(s);
    checkExitCall();
    return result;
}

Rboolean R_cycle_detected(SEXP s, SEXP child) {
    TRACE0();
    unimplemented("R_cycle_detected");
    return 0;
}

cetype_t Rf_getCharCE(SEXP x) {
    TRACE0();
    // unimplemented("Rf_getCharCE");
    // TODO: real implementation
    return CE_NATIVE;
}

const char *Rf_reEnc(const char *x, cetype_t ce_in, cetype_t ce_out, int subst) {
    TRACE0();
    // TODO proper implementation
    return x;
}

int Rf_ncols(SEXP x) {
    TRACE1(x);
    int result = (int) ((call_Rf_ncols) callbacks[Rf_ncols_x])(x);
    checkExitCall();
    return result;
}

int Rf_nrows(SEXP x) {
    TRACE1(x);
    int result = (int) ((call_Rf_nrows) callbacks[Rf_nrows_x])(x);
    checkExitCall();
    return result;
}

SEXP Rf_protect(SEXP x) {
    TRACE1(x);
    SEXP result = ((call_Rf_protect) callbacks[Rf_protect_x])(x);
    checkExitCall();
    return result;
}

void Rf_unprotect(int x) {
    TRACE("%d", x);
    ((call_Rf_unprotect) callbacks[Rf_unprotect_x])(x);
    checkExitCall();
}

void R_ProtectWithIndex(SEXP x, PROTECT_INDEX *y) {
    TRACE1(x);
    *y = ((call_R_ProtectWithIndex) callbacks[R_ProtectWithIndex_x])(x);
    checkExitCall();
}

void R_Reprotect(SEXP x, PROTECT_INDEX y) {
    TRACE("%p %i", x, y);
    ((call_R_Reprotect) callbacks[R_Reprotect_x])(x, y);
    checkExitCall();
}

void Rf_unprotect_ptr(SEXP x) {
    TRACE1(x);
    ((call_Rf_unprotect_ptr) callbacks[Rf_unprotect_ptr_x])(x);
    checkExitCall();
}

void R_FlushConsole(void) {
    TRACE0();
    // ignored
}

void R_ProcessEvents(void) {
    TRACE0();
    unimplemented("R_ProcessEvents");
}

// Tools package support, not in public API
SEXP R_NewHashedEnv(SEXP parent, SEXP size) {
    TRACE2(parent, size);
    SEXP result = ((call_R_NewHashedEnv) callbacks[R_NewHashedEnv_x])(parent, size);
    checkExitCall();
    return result;
}

SEXP Rf_classgets(SEXP vec, SEXP klass) {
    TRACE2(vec, klass);
    SEXP result = ((call_Rf_classgets) callbacks[Rf_classgets_x])(vec, klass);
    checkExitCall();
    return result;
}

const char *Rf_translateChar(SEXP x) {
    TRACE1(x);
    // TODO: proper implementation
    const char *result = CHAR(x);
    return result;
}

const char *Rf_translateChar0(SEXP x) {
    TRACE1(x);
    // TODO: proper implementation
    const char *result = CHAR(x);
    return result;
}

const char *Rf_translateCharUTF8(SEXP x) {
    TRACE1(x);
    // TODO: proper implementation
    const char *result = CHAR(x);
    return result;
}

SEXP Rf_lengthgets(SEXP x, R_len_t y) {
    TRACE1(x);
    SEXP result = ((call_Rf_lengthgets) callbacks[Rf_lengthgets_x])(x, y);
    checkExitCall();
    return result;
}

SEXP Rf_xlengthgets(SEXP x, R_xlen_t y) {
    TRACE1(x);
    return unimplemented("Rf_xlengthgets");
}

SEXP R_lsInternal(SEXP env, Rboolean all) {
    TRACE1(env);
    return R_lsInternal3(env, all, TRUE);
}

SEXP R_lsInternal3(SEXP env, Rboolean all, Rboolean sorted) {
    TRACE0();
    return ((call_R_lsInternal3) callbacks[R_lsInternal3_x])(env, all, sorted);
}

SEXP Rf_namesgets(SEXP x, SEXP y) {
    TRACE0();
	SEXP result = ((call_Rf_namesgets) callbacks[Rf_namesgets_x])(x, y);
    checkExitCall();
    return result;
}

SEXP TAG(SEXP e) {
    TRACE0();
    SEXP result = ((call_TAG) callbacks[TAG_x])(e);
    checkExitCall();
    return result;
}

SEXP PRINTNAME(SEXP e) {
    TRACE0();
    SEXP result = ((call_PRINTNAME) callbacks[PRINTNAME_x])(e);
    checkExitCall();
    return result;
}

SEXP CAAR(SEXP e) {
    TRACE0();
    SEXP result = ((call_CAAR) callbacks[CAAR_x])(e);
    checkExitCall();
    return result;
}

SEXP CDAR(SEXP e) {
    TRACE0();
    SEXP result = ((call_CDAR) callbacks[CDAR_x])(e);
    checkExitCall();
    return result;
}

SEXP CADR(SEXP e) {
    TRACE0();
    SEXP result = ((call_CADR) callbacks[CADR_x])(e);
    checkExitCall();
    return result;
}

SEXP CDDR(SEXP e) {
    TRACE0();
    SEXP result = ((call_CDDR) callbacks[CDDR_x])(e);
    checkExitCall();
    return result;
}

SEXP CDDDR(SEXP e) {
    TRACE0();
    SEXP result = ((call_CDDDR) callbacks[CDDDR_x])(e);
    checkExitCall();
    return result;
}

SEXP CADDR(SEXP e) {
    TRACE0();
    SEXP result = ((call_CADDR) callbacks[CADDR_x])(e);
    checkExitCall();
    return result;
}

SEXP CADDDR(SEXP e) {
    TRACE0();
    SEXP result = ((call_CADDDR) callbacks[CADDDR_x])(e);
    checkExitCall();
    return result;
}

SEXP CAD4R(SEXP e) {
    TRACE0();
    SEXP result = ((call_CAD4R) callbacks[CAD4R_x])(e);
    checkExitCall();
    return result;
}

int MISSING(SEXP x) {
    TRACE0();
    unimplemented("MISSING");
    return 0;
}

void SET_MISSING(SEXP x, int v) {
    TRACE0();
    unimplemented("SET_MISSING");
}

void SET_TAG(SEXP x, SEXP y) {
    TRACE0();
    ((call_SET_TAG) callbacks[SET_TAG_x])(x, y);
    checkExitCall();
}

SEXP SETCAR(SEXP x, SEXP y) {
    TRACE0();
    SEXP result = ((call_SETCAR) callbacks[SETCAR_x])(x, y);
    checkExitCall();
    return result;
}

SEXP SETCDR(SEXP x, SEXP y) {
    TRACE0();
    SEXP result = ((call_SETCDR) callbacks[SETCDR_x])(x, y);
    checkExitCall();
    return result;
}

SEXP SETCADR(SEXP x, SEXP y) {
    TRACE0();
    SEXP result = ((call_SETCADR) callbacks[SETCADR_x])(x, y);
    checkExitCall();
    return result;
}

SEXP SETCADDR(SEXP x, SEXP y) {
    TRACE0();
    // note: signature is same, we reuse call_SETCADR
    SEXP result = ((call_SETCADR) callbacks[SETCADDR_x])(x, y);
    return NULL;
}

SEXP SETCADDDR(SEXP x, SEXP y) {
    TRACE0();
    SEXP result = ((call_SETCADR) callbacks[SETCADDDR_x])(x, y);
    return NULL;
}

SEXP SETCAD4R(SEXP x, SEXP y) {
    TRACE0();
    SEXP result = ((call_SETCADR) callbacks[SETCAD4R_x])(x, y);
    return NULL;
}

SEXP FORMALS(SEXP x) {
    TRACE0();
    SEXP result = ((call_FORMALS) callbacks[FORMALS_x])(x);
    checkExitCall();
    return result;
}

SEXP BODY(SEXP x) {
    TRACE0();
    SEXP result = ((call_BODY) callbacks[BODY_x])(x);
    checkExitCall();
    return result;
}

SEXP CLOENV(SEXP x) {
    TRACE(TARGp, x);
    SEXP result = ((call_CLOENV) callbacks[CLOENV_x])(x);
    checkExitCall();
    return result;
}

int RDEBUG(SEXP x) {
    TRACE0();
    int result = ((call_RDEBUG) callbacks[RDEBUG_x])(x);
    checkExitCall();
    return result;
}

int RSTEP(SEXP x) {
    TRACE0();
    int result = ((call_RSTEP) callbacks[RSTEP_x])(x);
    checkExitCall();
    return result;
}

int RTRACE(SEXP x) {
    TRACE0();
    unimplemented("RTRACE");
    return 0;
}

void SET_RDEBUG(SEXP x, int v) {
    TRACE0();
    ((call_SET_RDEBUG) callbacks[SET_RDEBUG_x])(x, v);
    checkExitCall();
}

void SET_RSTEP(SEXP x, int v) {
    TRACE0();
    ((call_SET_RSTEP) callbacks[SET_RSTEP_x])(x, v);
    checkExitCall();
}

void SET_RTRACE(SEXP x, int v) {
    TRACE0();
    unimplemented("SET_RTRACE");
}

void SET_FORMALS(SEXP x, SEXP v) {
    TRACE0();
    ((call_SET_FORMALS) callbacks[SET_FORMALS_x])(x, v);
    checkExitCall();
}

void SET_BODY(SEXP x, SEXP v) {
    TRACE0();
    ((call_SET_FORMALS) callbacks[SET_BODY_x])(x, v);
    checkExitCall();
}

void SET_CLOENV(SEXP x, SEXP v) {
    TRACE0();
    ((call_SET_FORMALS) callbacks[SET_CLOENV_x])(x, v);
    checkExitCall();
}

SEXP SYMVALUE(SEXP x) {
    TRACE0();
    SEXP result = ((call_SYMVALUE) callbacks[SYMVALUE_x])(x);
    checkExitCall();
    return result;
}

SEXP INTERNAL(SEXP x) {
    TRACE0();
    return unimplemented("INTERNAL");
}

int DDVAL(SEXP x) {
    TRACE0();
    unimplemented("DDVAL");
    return 0;
}

void SET_DDVAL(SEXP x, int v) {
    TRACE0();
    unimplemented("SET_DDVAL");
}

void SET_SYMVALUE(SEXP x, SEXP v) {
    TRACE0();
    ((call_SET_SYMVALUE) callbacks[SET_SYMVALUE_x])(x, v);
    checkExitCall();
}

void SET_INTERNAL(SEXP x, SEXP v) {
    TRACE0();
    unimplemented("SET_INTERNAL");
}

SEXP FRAME(SEXP x) {
    TRACE0();
    return unimplemented("FRAME");
}

SEXP ENCLOS(SEXP x) {
    TRACE0();
    SEXP result = ((call_ENCLOS) callbacks[ENCLOS_x])(x);
    checkExitCall();
    return result;
}

SEXP HASHTAB(SEXP x) {
    TRACE0();
    return unimplemented("HASHTAB");
}

int ENVFLAGS(SEXP x) {
    TRACE0();
    unimplemented("ENVFLAGS");
    return 0;
}

void SET_ENVFLAGS(SEXP x, int v) {
    TRACE0();
    unimplemented("SET_ENVFLAGS");
}

void SET_FRAME(SEXP x, SEXP v) {
    TRACE0();
    unimplemented("SET_FRAME");
}

void SET_ENCLOS(SEXP x, SEXP v) {
    TRACE0();
    unimplemented("SET_ENCLOS");
}

void SET_HASHTAB(SEXP x, SEXP v) {
    TRACE0();
    unimplemented("SET_HASHTAB");
}

SEXP PRCODE(SEXP x) {
    TRACE0();
    SEXP result = ((call_PRCODE) callbacks[PRCODE_x])(x);
    checkExitCall();
    return result;
}

SEXP PRENV(SEXP x) {
    TRACE0();
    SEXP result = ((call_PRENV) callbacks[PRENV_x])(x);
    checkExitCall();
    return result;
}

SEXP PRVALUE(SEXP x) {
    TRACE0();
    SEXP result = ((call_PRVALUE) callbacks[PRVALUE_x])(x);
    checkExitCall();
    return result;
}

int PRSEEN(SEXP x) {
    TRACE0();
    int result = ((call_PRSEEN) callbacks[PRSEEN_x])(x);
    checkExitCall();
    return result;
}

void SET_PRSEEN(SEXP x, int v) {
    TRACE0();
    unimplemented("SET_PRSEEN");
}

void SET_PRENV(SEXP x, SEXP v) {
    TRACE0();
    unimplemented("SET_PRENV");
}

void SET_PRVALUE(SEXP x, SEXP v) {
    TRACE0();
    unimplemented("SET_PRVALUE");
}

void SET_PRCODE(SEXP x, SEXP v) {
    TRACE0();
    unimplemented("SET_PRCODE");
}

int TRUELENGTH(SEXP x) {
    TRACE(TARGp, x);
    // TODO do not throw an error for now
    return 0;
}

void SETLENGTH(SEXP x, int v) {
    TRACE0();
    unimplemented("SETLENGTH");
}

void SET_TRUELENGTH(SEXP x, int v) {
    TRACE0();
    unimplemented("SET_TRUELENGTH");
}

R_xlen_t XLENGTH(SEXP x) {
    TRACE0();
    // xlength seems to be used for long vectors (no such thing in FastR at the moment)
    return LENGTH(x);
}

R_xlen_t XTRUELENGTH(SEXP x) {
    TRACE0();
    unimplemented("XTRUELENGTH");
    return 0;
}

int IS_LONG_VEC(SEXP x) {
    TRACE0();
    unimplemented("IS_LONG_VEC");
    return 0;
}

int LEVELS(SEXP x) {
    TRACE0();
    unimplemented("LEVELS");
    return 0;
}

int SETLEVELS(SEXP x, int v) {
    TRACE0();
    unimplemented("SETLEVELS");
    return 0;
}

int *FASTR_INTEGER(SEXP x) {
    TRACE(TARGp, x);
    int *result = ((call_INTEGER) callbacks[INTEGER_x])(x);
    checkExitCall();
    return result;
}

double *FASTR_REAL(SEXP x){
    TRACE(TARGp, x);
    double *result = ((call_REAL) callbacks[REAL_x])(x);
    checkExitCall();
    return result;
}

Rcomplex *COMPLEX(SEXP x) {
    TRACE0();
    Rcomplex *result = ((call_COMPLEX) callbacks[COMPLEX_x])(x);
    checkExitCall();
    return result;
}

int *LOGICAL(SEXP x){
    TRACE0();
    int *result = ((call_LOGICAL) callbacks[LOGICAL_x])(x);
    checkExitCall();
    return result;
}

Rbyte *RAW(SEXP x) {
    TRACE0();
    Rbyte *result = ((call_RAW) callbacks[RAW_x])(x);
    checkExitCall();
    return result;
}

const char * R_CHAR(SEXP x) {
    TRACE0();
    SEXP result = ((call_R_CHAR) callbacks[R_CHAR_x])(x);
    checkExitCall();
    return result;
}

SEXP STRING_ELT(SEXP x, R_xlen_t i) {
    TRACE0();
    SEXP result = ((call_STRING_ELT) callbacks[STRING_ELT_x])(x, i);
    checkExitCall();
    return result;
}

SEXP VECTOR_ELT(SEXP x, R_xlen_t i) {
    TRACE0();
    SEXP result = ((call_VECTOR_ELT) callbacks[VECTOR_ELT_x])(x, i);
    checkExitCall();
    return result;
}

void SET_STRING_ELT(SEXP x, R_xlen_t i, SEXP v) {
    TRACE0();
    ((call_SET_STRING_ELT) callbacks[SET_STRING_ELT_x])(x, i, v);
    checkExitCall();
}

SEXP SET_VECTOR_ELT(SEXP x, R_xlen_t i, SEXP v) {
    TRACE0();
    SEXP result = ((call_SET_VECTOR_ELT) callbacks[SET_VECTOR_ELT_x])(x, i, v);
    checkExitCall();
    return result;
}

SEXP *STRING_PTR(SEXP x) {
    TRACE0();
    unimplemented("STRING_PTR");
    return NULL;
}

SEXP * NORET VECTOR_PTR(SEXP x) {
    TRACE0();
    unimplemented("VECTOR_PTR");
}

SEXP Rf_asChar(SEXP x) {
    TRACE0();
    SEXP result = ((call_Rf_asChar) callbacks[Rf_asChar_x])(x);
    checkExitCall();
    return result;
}

SEXP Rf_PairToVectorList(SEXP x) {
    TRACE0();
    SEXP result = ((call_Rf_PairToVectorList) callbacks[Rf_PairToVectorList_x])(x);
    checkExitCall();
    return result;
}

SEXP Rf_VectorToPairList(SEXP x){
	SEXP result = ((call_Rf_VectorToPairList) callbacks[Rf_VectorToPairList_x])(x);
    checkExitCall();
    return result;
}

SEXP Rf_asCharacterFactor(SEXP x){
	SEXP result = ((call_Rf_asCharacterFactor) callbacks[Rf_asCharacterFactor_x])(x);
    checkExitCall();
    return result;
}

int Rf_asLogical(SEXP x) {
    TRACE0();
    int result = ((call_Rf_asLogical) callbacks[Rf_asLogical_x])(x);
    checkExitCall();
    return result;
}

int Rf_asInteger(SEXP x) {
    TRACE0();
    int result = ((call_Rf_asInteger) callbacks[Rf_asInteger_x])(x);
    checkExitCall();
    return result;
}

double Rf_asReal(SEXP x) {
    TRACE0();
    double result = ((call_Rf_asReal) callbacks[Rf_asReal_x])(x);
    checkExitCall();
    return result;
}

Rcomplex Rf_asComplex(SEXP x) {
    TRACE0();
    unimplemented("Rf_asComplex");
    Rcomplex c; return c;
}

int TYPEOF(SEXP x) {
    TRACE0();
    int result = (int) ((call_TYPEOF) callbacks[TYPEOF_x])(x);
    checkExitCall();
    return result;
}

SEXP ATTRIB(SEXP x) {
    TRACE0();
    SEXP result = ((call_ATTRIB) callbacks[ATTRIB_x])(x);
    checkExitCall();
    return result;
}

int OBJECT(SEXP x) {
    TRACE0();
    int result = (int) ((call_OBJECT) callbacks[OBJECT_x])(x);
    checkExitCall();
    return result;
}

int MARK(SEXP x) {
    TRACE0();
    unimplemented("MARK");
    return 0;
}

int NAMED(SEXP x) {
    TRACE0();
    int result = (int) ((call_NAMED) callbacks[NAMED_x])(x);
    checkExitCall();
    return result;
}

int REFCNT(SEXP x) {
    TRACE0();
    unimplemented("REFCNT");
    return 0;
}

void SET_OBJECT(SEXP x, int v) {
    TRACE0();
    ((call_SET_OBJECT) callbacks[SET_OBJECT_x])(x, v);
}

void SET_TYPEOF(SEXP x, int v) {
    TRACE0();
    ((call_SET_TYPEOF) callbacks[SET_TYPEOF_x])(x, v);
    checkExitCall();
}

void SET_NAMED(SEXP x, int v) {
    TRACE(TARGpd, x, v);
    ((call_SET_NAMED) callbacks[SET_NAMED_FASTR_x])(x, v);
    checkExitCall();
}

void SET_ATTRIB(SEXP x, SEXP v) {
    TRACE0();
    ((call_SET_ATTRIB) callbacks[SET_ATTRIB_x])(x, v);
}

void DUPLICATE_ATTRIB(SEXP to, SEXP from) {
    TRACE0();
    ((call_DUPLICATE_ATTRIB) callbacks[DUPLICATE_ATTRIB_x])(to, from);
    checkExitCall();
}

R_len_t R_BadLongVector(SEXP x, const char *y, int z) {
    TRACE0();
    unimplemented("R_BadLongVector");
    exit(1);
    // "no return" function
}

int IS_S4_OBJECT(SEXP x) {
    TRACE0();
    int result = (int) ((call_IS_S4_OBJECT) callbacks[IS_S4_OBJECT_x])(x);
    checkExitCall();
    return result;
}

void SET_S4_OBJECT(SEXP x) {
    TRACE0();
    ((call_SET_S4_OBJECT) callbacks[SET_S4_OBJECT_x])(x);
}

void UNSET_S4_OBJECT(SEXP x) {
    TRACE0();
    ((call_UNSET_S4_OBJECT) callbacks[UNSET_S4_OBJECT_x])(x);
}

Rboolean R_ToplevelExec(void (*fun)(void *), void *data) {
    TRACE0();

    // reset handler stack
    SEXP saved_handler_stack = ((call_R_ToplevelExec) callbacks[R_ToplevelExec_x])();
    checkExitCall();
    fun(data);
    ((call_restoreHandlerStack) callbacks[restoreHandlerStacks_x])(saved_handler_stack);
    checkExitCall();

    // TODO detect errors
    return TRUE;
}

SEXP R_ExecWithCleanup(SEXP (*fun)(void *), void *data,
               void (*cleanfun)(void *), void *cleandata) {
    TRACE0();
    return unimplemented("R_ExecWithCleanup");
}

/* Environment and Binding Features */
void R_RestoreHashCount(SEXP rho) {
    TRACE0();
    unimplemented("R_RestoreHashCount");
}

Rboolean R_IsPackageEnv(SEXP rho) {
    TRACE0();
    unimplemented("R_IsPackageEnv");
}

SEXP R_PackageEnvName(SEXP rho) {
    TRACE0();
    return unimplemented("R_PackageEnvName");
}

SEXP R_FindPackageEnv(SEXP info) {
    TRACE0();
    return unimplemented("R_FindPackageEnv");
}

Rboolean R_IsNamespaceEnv(SEXP rho) {
    TRACE0();
    return (Rboolean) unimplemented("R_IsNamespaceEnv");
}

SEXP R_FindNamespace(SEXP info) {
    TRACE0();
    SEXP result = ((call_R_FindNamespace) callbacks[R_FindNamespace_x])(info);
    checkExitCall();
    return result;
}

SEXP R_NamespaceEnvSpec(SEXP rho) {
    TRACE0();
    return unimplemented("R_NamespaceEnvSpec");
}

void R_LockEnvironment(SEXP env, Rboolean bindings) {
    TRACE0();
    unimplemented("R_LockEnvironment");
}

Rboolean R_EnvironmentIsLocked(SEXP env) {
    TRACE0();
    unimplemented("");
}

void R_LockBinding(SEXP sym, SEXP env) {
    TRACE0();
    ((call_R_LockBinding) callbacks[R_LockBinding_x])(sym, env);
    checkExitCall();
}

void R_unLockBinding(SEXP sym, SEXP env) {
    TRACE0();
    ((call_R_unLockBinding) callbacks[R_unLockBinding_x])(sym, env);
    checkExitCall();
}

void R_MakeActiveBinding(SEXP sym, SEXP fun, SEXP env) {
    TRACE0();
    unimplemented("R_MakeActiveBinding");
}

Rboolean R_BindingIsLocked(SEXP sym, SEXP env) {
    TRACE0();
    Rboolean result = (Rboolean) ((call_R_BindingIsLocked) callbacks[R_BindingIsLocked_x])(sym, env);
    checkExitCall();
    return result;
}

Rboolean R_BindingIsActive(SEXP sym, SEXP env) {
    TRACE0();
    // TODO: for now, I believe all bindings are false
    return (Rboolean)0;
}

Rboolean R_HasFancyBindings(SEXP rho) {
    TRACE0();
    return (Rboolean) unimplemented("R_HasFancyBindings");
}

Rboolean Rf_isS4(SEXP x) {
    TRACE0();
    return IS_S4_OBJECT(x);
}

SEXP Rf_asS4(SEXP x, Rboolean b, int i) {
    TRACE0();
    unimplemented("Rf_asS4");
}

static SEXP R_tryEvalInternal(SEXP x, SEXP y, int *ErrorOccurred, int silent) {
    TRACE0();
    if (ErrorOccurred) {
       *ErrorOccurred = 0;
    }
    return ((call_R_tryEval) callbacks[R_tryEval_x])(x, y, ErrorOccurred, silent);
}

SEXP R_tryEval(SEXP x, SEXP y, int *ErrorOccurred) {
    TRACE0();
    return R_tryEvalInternal(x, y, ErrorOccurred, 0);
}

SEXP R_tryEvalSilent(SEXP x, SEXP y, int *ErrorOccurred) {
    TRACE0();
    return R_tryEvalInternal(x, y, ErrorOccurred, 1);
}
/*
double R_atof(const char *str) {
    TRACE0();
    unimplemented("R_atof");
    return 0;
}

double R_strtod(const char *c, char **end) {
    TRACE0();
    unimplemented("R_strtod");
    return 0;
}*/

SEXP R_PromiseExpr(SEXP x) {
    TRACE0();
    SEXP result = ((call_R_PromiseExpr) callbacks[R_PromiseExpr_x])(x);
    checkExitCall();
    return result;
}

SEXP R_ClosureExpr(SEXP x) {
    TRACE0();
    return unimplemented("R_ClosureExpr");
}

SEXP R_forceAndCall(SEXP e, int n, SEXP rho) {
    TRACE0();
    return unimplemented("R_forceAndCall");
}

SEXP R_MakeExternalPtr(void *p, SEXP tag, SEXP prot) {
    TRACE0();
    SEXP result = ((call_R_MakeExternalPtr) callbacks[R_MakeExternalPtr_x])(p, tag, prot);
    checkExitCall();
    return result;
}

void *R_ExternalPtrAddr(SEXP s) {

    TRACE0();
    SEXP result = ((call_R_ExternalPtrAddr) callbacks[R_ExternalPtrAddr_x])(s);
    checkExitCall();
    return result;
}

SEXP R_ExternalPtrTag(SEXP s) {
    TRACE0();
    SEXP result = ((call_R_ExternalPtrTag) callbacks[R_ExternalPtrTag_x])(s);
    checkExitCall();
    return result;
}

SEXP R_ExternalPtrProtected(SEXP s) {
    TRACE0();
    SEXP result = ((call_R_ExternalPtrProtected) callbacks[R_ExternalPtrProtected_x])(s);
    checkExitCall();
    return result;
}

void R_SetExternalPtrAddr(SEXP s, void *p) {
    TRACE0();
    ((call_R_SetExternalPtrAddr) callbacks[R_SetExternalPtrAddr_x])(s, p);
}

void R_SetExternalPtrTag(SEXP s, SEXP tag) {
    TRACE0();
    ((call_R_SetExternalPtrTag) callbacks[R_SetExternalPtrTag_x])(s, tag);
}

void R_SetExternalPtrProtected(SEXP s, SEXP p) {
    TRACE0();
    ((call_R_SetExternalPtrProtected) callbacks[R_SetExternalPtrProtected_x])(s, p);
}

void R_ClearExternalPtr(SEXP s) {
    TRACE0();
    R_SetExternalPtrAddr(s, NULL);
}

void R_RegisterFinalizer(SEXP s, SEXP fun) {
    TRACE0();
    // TODO implement, but not fail for now
}
void R_RegisterCFinalizer(SEXP s, R_CFinalizer_t fun) {
    TRACE0();
    // TODO implement, but not fail for now
}

void R_RegisterFinalizerEx(SEXP s, SEXP fun, Rboolean onexit) {
    TRACE0();
    // TODO implement, but not fail for now

}

void R_RegisterCFinalizerEx(SEXP s, R_CFinalizer_t fun, Rboolean onexit) {
    TRACE0();
    // TODO implement, but not fail for now
}

void R_RunPendingFinalizers(void) {
    TRACE0();
    // TODO implement, but not fail for now
}

SEXP R_MakeWeakRef(SEXP key, SEXP val, SEXP fin, Rboolean onexit) {
    TRACE0();
    unimplemented("R_MakeWeakRef");
}

SEXP R_MakeWeakRefC(SEXP key, SEXP val, R_CFinalizer_t fin, Rboolean onexit) {
    TRACE0();
    unimplemented("R_MakeWeakRefC");
}

SEXP R_WeakRefKey(SEXP w) {
    TRACE0();
    unimplemented("R_WeakRefKey");
}

SEXP R_WeakRefValue(SEXP w) {
    TRACE0();
    unimplemented("R_WeakRefValue");
}

void R_RunWeakRefFinalizer(SEXP w) {
    TRACE0();
    // TODO implement, but not fail for now
}

SEXP R_do_slot(SEXP obj, SEXP name) {
    TRACE0();
    SEXP result = ((call_R_do_slot) callbacks[R_do_slot_x])(obj, name);
    checkExitCall();
    return result;
}

SEXP R_do_slot_assign(SEXP obj, SEXP name, SEXP value) {
    TRACE0();
    SEXP result = ((call_R_do_slot_assign) callbacks[R_do_slot_assign_x])(obj, name, value);
    checkExitCall();
    return result;
}

int R_has_slot(SEXP obj, SEXP name) {
    TRACE(TARGpp, obj, name);
    int result = ((call_R_has_slot) callbacks[R_has_slot_x])(obj, name);
    checkExitCall();
    return result;
}

SEXP R_do_MAKE_CLASS(const char *what) {
    TRACE0();
    SEXP result = ((call_R_do_MAKE_CLASS) callbacks[R_do_MAKE_CLASS_x])(what);
    checkExitCall();
    return result;
}

SEXP R_getClassDef (const char *what) {
    TRACE(TARGs, what);
    SEXP result = ((call_R_getClassDef) callbacks[R_getClassDef_x])(what);
    checkExitCall();
    return result;
}

SEXP R_do_new_object(SEXP class_def) {
    TRACE0();
    SEXP result = ((call_R_do_new_object) callbacks[R_do_new_object_x])(class_def);
    checkExitCall();
    return result;
}

static SEXP nfiGetMethodsNamespace() {
    TRACE0();
    SEXP result = ((call_R_MethodsNamespace) callbacks[R_MethodsNamespace_x])();
    checkExitCall();
    return result;
}

int R_check_class_etc (SEXP x, const char **valid) {
    TRACE0();
    return R_check_class_etc_helper(x, valid, nfiGetMethodsNamespace);
}

void R_PreserveObject(SEXP x) {
    TRACE0();
    ((call_R_PreserveObject) callbacks[R_PreserveObject_x])(x);
}

void R_ReleaseObject(SEXP x) {
    TRACE0();
    // this function is sometimes called after the engine has shut down (e.g., from C++ destructors in Rcpp)
    if(callbacks != NULL) {
    	((call_R_ReleaseObject) callbacks[R_ReleaseObject_x])(x);
    }
}

void R_dot_Last(void) {
    TRACE0();
    unimplemented("R_dot_Last");
}

Rboolean R_compute_identical(SEXP x, SEXP y, int flags) {
    TRACE0();
    Rboolean result = (Rboolean) ((call_R_compute_identical) callbacks[R_compute_identical_x])(x, y, flags);
    checkExitCall();
    return result;
}

void Rf_copyListMatrix(SEXP s, SEXP t, Rboolean byrow) {
    TRACE0();
    ((call_Rf_copyListMatrix) callbacks[Rf_copyListMatrix_x])(s, t, byrow);
    checkExitCall();
}

void Rf_copyMatrix(SEXP s, SEXP t, Rboolean byrow) {
    TRACE0();
    ((call_Rf_copyMatrix) callbacks[Rf_copyMatrix_x])(s, t, byrow);
    checkExitCall();
}

int FASTR_getConnectionChar(SEXP conn) {
    TRACE0();
    int result = ((call_FASTR_getConnectionChar) callbacks[FASTR_getConnectionChar_x])(conn);
    checkExitCall();
    return result;
}

SEXPTYPE Rf_str2type(const char *s) {
    TRACE0();
    SEXPTYPE result = ((call_Rf_str2type) callbacks[Rf_str2type_x])(s);
    checkExitCall();
    return result;
}

// Must match ordinal value for DLL.NativeSymbolType
#define C_NATIVE_TYPE 0
#define CALL_NATIVE_TYPE 1
#define FORTRAN_NATIVE_TYPE 2
#define EXTERNAL_NATIVE_TYPE 3

int
R_registerRoutines(DllInfo *info, const R_CMethodDef * const croutines,
           const R_CallMethodDef * const callRoutines,
           const R_FortranMethodDef * const fortranRoutines,
           const R_ExternalMethodDef * const externalRoutines) {
    TRACE0();
    int num;
    if (croutines) {
    TRACE0();
        for(num = 0; croutines[num].name != NULL; num++) {;}
        ((call_registerRoutines) callbacks[registerRoutines_x])(info, C_NATIVE_TYPE, num, croutines);
    }
    if (callRoutines) {
    TRACE0();
        for(num = 0; callRoutines[num].name != NULL; num++) {;}
        ((call_registerRoutines) callbacks[registerRoutines_x])(info, CALL_NATIVE_TYPE, num, callRoutines);
    }
    if (fortranRoutines) {
    TRACE0();
        for(num = 0; fortranRoutines[num].name != NULL; num++) {;}
        ((call_registerRoutines) callbacks[registerRoutines_x])(info, FORTRAN_NATIVE_TYPE, num, fortranRoutines);
    }
    if (externalRoutines) {
    TRACE0();
        for(num = 0; externalRoutines[num].name != NULL; num++) {;}
        ((call_registerRoutines) callbacks[registerRoutines_x])(info, EXTERNAL_NATIVE_TYPE, num, externalRoutines);
    }
    return 1;
}

DllInfo *R_getEmbeddingDllInfo() {
    TRACE0();
    DllInfo *result = ((call_getEmbeddingDLLInfo) callbacks[getEmbeddingDLLInfo_x])();
    checkExitCall();
    return result;
}

Rboolean R_useDynamicSymbols(DllInfo *dllInfo, Rboolean value) {
    TRACE0();
    Rboolean result = ((call_useDynamicSymbols) callbacks[useDynamicSymbols_x])(dllInfo, value);
    checkExitCall();
    return result;
}

Rboolean R_forceSymbols(DllInfo *dllInfo, Rboolean value) {
    TRACE0();
    Rboolean result = ((call_forceSymbols) callbacks[forceSymbols_x])(dllInfo, value);
    checkExitCall();
    return result;
}

void *Rdynload_setSymbol(DllInfo *info, int nstOrd, void* routinesAddr, int index) {
    TRACE0();
    const char *name;
    void *fun;
    int numArgs;
    switch (nstOrd) {
    TRACE0();
    case C_NATIVE_TYPE: {
        R_CMethodDef *croutines = (R_CMethodDef *) routinesAddr;
        name = croutines[index].name;
        fun = croutines[index].fun;
        numArgs = croutines[index].numArgs;
        break;
    }
    case CALL_NATIVE_TYPE: {
        R_CallMethodDef *callRoutines = (R_CallMethodDef *) routinesAddr;
        name = callRoutines[index].name;
        fun = callRoutines[index].fun;
        numArgs = callRoutines[index].numArgs;
        break;
    }
    case FORTRAN_NATIVE_TYPE: {
        R_FortranMethodDef * fortranRoutines = (R_FortranMethodDef *) routinesAddr;
        name = fortranRoutines[index].name;
        fun = fortranRoutines[index].fun;
        numArgs = fortranRoutines[index].numArgs;
        break;
    }
    case EXTERNAL_NATIVE_TYPE: {
        R_ExternalMethodDef * externalRoutines = (R_ExternalMethodDef *) routinesAddr;
        name = externalRoutines[index].name;
        fun = externalRoutines[index].fun;
        numArgs = externalRoutines[index].numArgs;
        break;
    }
    }
    void *result = ((call_setDotSymbolValues) callbacks[setDotSymbolValues_x])(info, ensure_string(name), (DL_FUNC) ensure_function(fun), numArgs);
    checkExitCall();
    return result;
}

void R_RegisterCCallable(const char *package, const char *name, DL_FUNC fptr) {
    TRACE0();
    ((call_registerCCallable) callbacks[registerCCallable_x])(ensure_string(package), ensure_string(name), (void *)fptr);
    checkExitCall();
}

DL_FUNC R_GetCCallable(const char *package, const char *name) {
    TRACE0();
    SEXP result = ((call_getCCallable) callbacks[getCCallable_x])(ensure_string(package), ensure_string(name));
    checkExitCall();
    return result;
}

DL_FUNC R_FindSymbol(char const *name, char const *pkg, R_RegisteredNativeSymbol *symbol) {
    TRACE0();
    return unimplemented("R_FindSymbol");
}
