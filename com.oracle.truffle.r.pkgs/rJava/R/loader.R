##
 # This material is distributed under the GNU General Public License
 # Version 2. You may review the terms of this license at
 # http://www.gnu.org/licenses/gpl-2.0.html
 #
 # Copyright (c) 2006 Simon Urbanek <simon.urbanek@r-project.org>
 # Copyright (c) 2018, Oracle and/or its affiliates
 #
 # All rights reserved.
##

.jaddClassPath <- function(path) {
    if (!length(path)) return(invisible(NULL))
    # FASTR <<<<<
    # if (!is.jnull(.rJava.class.loader))
    #    invisible(.jcall(.rJava.class.loader,"V","addClassPath",as.character(path)))
    # else {
    #    cpr <- try(.jmergeClassPath(paste(path,collapse=.Platform$path.sep)), silent=TRUE)
    #    invisible(!inherits(cpr, "try-error"))
    #}
    invisible(java.addToClasspath(path))
    # FASTR >>>>>
}

.jclassPath <- function() {
    # FASTR <<<<<
    # if (is.jnull(.rJava.class.loader)) {
    #     cp <- .jcall("java/lang/System", "S", "getProperty", "java.class.path")
    #     unlist(strsplit(cp, .Platform$path.sep))
    # } else {
    #     .jcall(.rJava.class.loader,"[Ljava/lang/String;","getClassPath")
    # }
    
    # not provided, do nothing

    # FASTR >>>>>
}

.jaddLibrary <- function(name, path) {
    # FASTR TODO 
    stop(".jaddLibrary overriden but not yet implemented")
    # if (!is.jnull(.rJava.class.loader))
    #     invisible(.jcall(.rJava.class.loader, "V", "addRLibrary", as.character(name)[1], as.character(path)[1]))
}

.jrmLibrary <- function(name) {
  ## FIXME: unimplemented
}

.jclassLoader <- function() {
    # FASTR TODO 
    stop(".jclassLoader overriden but not yet implemented")
    #.rJava.class.loader
}

.jpackage <- function(name, jars='*', morePaths='', nativeLibrary=FALSE, lib.loc=NULL) {
  if (!.jniInitialized) .jinit()
  classes <- system.file("java", package=name, lib.loc=lib.loc)
  if (nchar(classes)) {
    .jaddClassPath(classes)
    if (length(jars)) {
      if (length(jars)==1 && jars=='*') {
        jars <- grep(".*\\.jar",list.files(classes,full.names=TRUE),TRUE,value=TRUE)
        if (length(jars)) .jaddClassPath(jars)
      } else .jaddClassPath(paste(classes,jars,sep=.Platform$file.sep))
    }
  }  
  if (any(nchar(morePaths))) {
    cl <- as.character(morePaths)
    cl <- cl[nchar(cl)>0]
    .jaddClassPath(cl)
  }
  if (is.logical(nativeLibrary)) {
    if (nativeLibrary) {
      libs <- "libs"
      if (nchar(.Platform$r_arch)) lib <- file.path("libs", .Platform$r_arch)
      lib <- system.file(libs, paste(name, .Platform$dynlib.ext, sep=''), package=name, lib.loc=lib.loc)
      if (nchar(lib))
        .jaddLibrary(name, lib)
      else
        warning("Native library for `",name,"' could not be found.")
    }
  } else {
    .jaddLibrary(name, nativeLibrary)
  }
  invisible(TRUE)
}
