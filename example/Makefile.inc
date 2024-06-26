# This file is automatically generated after the make install step.

EXTRAE_HOME=@sub_PREFIXDIR@
MPI_HOME=@sub_MPI_HOME@
MPI_CFLAGS=@sub_MPI_CFLAGS@
MPI_LDFLAGS=@sub_MPI_LDFLAGS@
MPI_LIBS=@sub_MPI_LIBS@
PAPI_HOME=@sub_PAPI_HOME@
XML2_HOME= @sub_XML2_HOME@
XML2_LDFLAGS=@sub_XML2_LDFLAGS@ @sub_XML2_RPATH@
XML2_LIBS=@sub_XML2_LIBS@
UNWIND_HOME=@sub_UNWIND_HOME@
BFD_HOME=@sub_BFD_HOME@
LIBERTY_HOME=@sub_LIBERTY_HOME@
BFD_LIBS=@sub_BFD_LDFLAGS@ @sub_BFD_RPATH@ @sub_BFD_LIBS@ @sub_LIBERTY_LDFLAGS@ @sub_LIBERTY_RPATH@ @sub_LIBERTY_LIBS@
UNWIND_LIBS=@sub_UNWIND_LDFLAGS@ @sub_UNWIND_RPATH@ @sub_UNWIND_LIBS@
PAPI_LIBS=@sub_PAPI_LDFLAGS@ @sub_PAPI_RPATH@ @sub_PAPI_LIBS@
POSIX_CLOCK_LIBS=@sub_POSIX_CLOCK@
LIBEXECINFO_LIBS=@sub_libexecinfo_libs@ # FreeBSD specific
ZLIB_HOME=@sub_ZLIB_HOME@
ZLIB_LIBS=@sub_ZLIB_LDFLAGS@ @sub_ZLIB_RPATH@ @sub_ZLIB_LIBS@
DL_LIBS=@sub_DL_LIBS@
OPENCL_HOME=@sub_OPENCL_HOME@
OPENCL_LDFLAGS=@sub_OPENCL_LDFLAGS@
GASPI_CFLAGS=@sub_GASPI_CFLAGS@
GASPI_LIBS=-@sub_GASPI_LDFLAGS@ @sub_GASPI_RPATH@ @sub_GASPI_LIBS@
OPENACC_HOME=@sub_OPENACC_HOME@

CC=@sub_CC@
FC=@sub_FC@
CXX=@sub_CXX@
MPICC=@sub_MPICC@
MPIF77=@sub_MPIF77@
MPIF90=@sub_MPIF90@
NVCC=@sub_NVCC@
JAVAC=@sub_JAVAC@
JAR=@sub_JAR@

# For statically linked apps or systems

EXTRAE_LDFLAGS_LIBSEQTRACE=-L$(EXTRAE_HOME)/lib -lseqtrace $(ZLIB_LDFLAGS) $(ZLIB_LIBS) $(PAPI_LDFLAGS) $(PAPI_LIBS) $(XML2_LDFLAGS) $(XML2_LIBS) $(BFD_LDFLAGS) $(BFD_LIBS)

EXTRAE_LDFLAGS_LIBMPITRACE=-L$(EXTRAE_HOME)/lib -lmpitrace $(ZLIB_LDFLAGS) $(ZLIB_LIBS) $(PAPI_LDFLAGS) $(PAPI_LIBS) $(XML2_LDFLAGS) $(XML2_LIBS) $(BFD_LDFLAGS) $(BFD_LIBS)

EXTRAE_LDFLAGS_LIBMPITRACEF=-L$(EXTRAE_HOME)/lib -lmpitracef $(ZLIB_LDFLAGS) $(ZLIB_LIBS) $(PAPI_LDFLAGS) $(PAPI_LIBS) $(XML2_LDFLAGS) $(XML2_LIBS) $(BFD_LDFLAGS) $(BFD_LIBS)
