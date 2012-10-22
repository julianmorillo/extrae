/*****************************************************************************\
 *                        ANALYSIS PERFORMANCE TOOLS                         *
 *                                   Extrae                                  *
 *              Instrumentation package for parallel applications            *
 *****************************************************************************
 *     ___     This library is free software; you can redistribute it and/or *
 *    /  __         modify it under the terms of the GNU LGPL as published   *
 *   /  /  _____    by the Free Software Foundation; either version 2.1      *
 *  /  /  /     \   of the License, or (at your option) any later version.   *
 * (  (  ( B S C )                                                           *
 *  \  \  \_____/   This library is distributed in hope that it will be      *
 *   \  \__         useful but WITHOUT ANY WARRANTY; without even the        *
 *    \___          implied warranty of MERCHANTABILITY or FITNESS FOR A     *
 *                  PARTICULAR PURPOSE. See the GNU LGPL for more details.   *
 *                                                                           *
 * You should have received a copy of the GNU Lesser General Public License  *
 * along with this library; if not, write to the Free Software Foundation,   *
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA          *
 * The GNU LEsser General Public License is contained in the file COPYING.   *
 *                                 ---------                                 *
 *   Barcelona Supercomputing Center - Centro Nacional de Supercomputacion   *
\*****************************************************************************/

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- *\
 | @file: $HeadURL$
 | @last_commit: $Date$
 | @version:     $Revision$
\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
#include "common.h"

#ifdef HAVE_DLFCN_H
# define __USE_GNU
# include <dlfcn.h>
# undef __USE_GNU
#endif
#ifdef HAVE_STDARG_H
# include <stdarg.h>
#endif
#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif
#ifdef HAVE_STDIO_H
# include <stdio.h>
#endif
#ifdef HAVE_PTHREAD_H
# include <pthread.h>
#endif

#include "threadid.h"
#include "omp-common.h"

//#define DEBUG

static char UNUSED rcsid[] = "$Id$";

#define INC_IF_NOT_NULL(ptr,cnt) (cnt = (ptr == NULL)?cnt:cnt+1)

struct __kmpv_location_t
{
	int reserved_1;
	int flags;
	int reserved_2;
	int reserved_3;
	char *location;
};

struct __kmp_task_t
{
	void *shareds;
	void *routine;
	int part_id;
};

static pthread_mutex_t extrae_map_kmpc_mutex;

static void (*__kmpc_fork_call_real)(void*,int,void*,...) = NULL;
static void (*__kmpc_barrier_real)(void*,int) = NULL;
static void (*__kmpc_critical_real)(void*,int,void*) = NULL;
static void (*__kmpc_end_critical_real)(void*,int,void*) = NULL;
static int (*__kmpc_dispatch_next_4_real)(void*,int,int*,int*,int*,int*) = NULL;
static int (*__kmpc_dispatch_next_8_real)(void*,int,int*,long long *,long long *, long long *) = NULL;
static int (*__kmpc_single_real)(void*,int) = NULL;
static void (*__kmpc_end_single_real)(void*,int) = NULL;
static void (*__kmpc_dispatch_init_4_real)(void*,int,int,int,int,int,int) = NULL;
static void (*__kmpc_dispatch_init_8_real)(void*,int,int,long long,long long,long long,long long) = NULL;
static void (*__kmpc_dispatch_fini_4_real)(void*,int) = NULL;
static void (*__kmpc_dispatch_fini_8_real)(void*,long long) = NULL; /* Don't sure about this! */

static void* (*__kmpc_omp_task_alloc_real)(void*,int,int,size_t,size_t,void*) = NULL;
static void (*__kmpc_omp_task_begin_if0_real)(void*,int,void*) = NULL;
static void (*__kmpc_omp_task_complete_if0_real)(void*,int,void*) = NULL;
static int (*__kmpc_omp_taskwait_real)(void*,int) = NULL;

int intel_kmpc_11_hook_points (int rank)
{
	int count = 0;

	/* Create mutex to protect intel omp tasks allocation calls */
	pthread_mutex_init (&extrae_map_kmpc_mutex, NULL);


	/* Careful, do not overwrite the pointer to the real call if DynInst has
	   already done it */
	if (__kmpc_fork_call_real == NULL)
	{
		/* Obtain @ for __kmpc_fork_call */
		__kmpc_fork_call_real =
			(void(*)(void*,int,void*,...))
			dlsym (RTLD_NEXT, "__kmpc_fork_call");
		if (__kmpc_fork_call_real == NULL && rank == 0)
			fprintf (stderr, PACKAGE_NAME": Unable to find __kmpc_fork_call in DSOs!!\n");
		INC_IF_NOT_NULL(__kmpc_fork_call_real,count);
	}

	/* Obtain @ for __kmpc_barrier */
	__kmpc_barrier_real =
		(void(*)(void*,int))
		dlsym (RTLD_NEXT, "__kmpc_barrier");
	if (__kmpc_barrier_real == NULL && rank == 0)
		fprintf (stderr, PACKAGE_NAME": Unable to find __kmpc_barrier in DSOs!!\n");
	INC_IF_NOT_NULL(__kmpc_barrier_real,count);

	/* Obtain @ for __kmpc_critical */
	__kmpc_critical_real =
		(void(*)(void*,int,void*))
		dlsym (RTLD_NEXT, "__kmpc_critical");
	if (__kmpc_critical_real == NULL && rank == 0)
		fprintf (stderr, PACKAGE_NAME": Unable to find __kmpc_critical in DSOs!!\n");
	INC_IF_NOT_NULL(__kmpc_critical_real,count);

	/* Obtain @ for __kmpc_end_critical */
	__kmpc_end_critical_real =
		(void(*)(void*,int,void*))
		dlsym (RTLD_NEXT, "__kmpc_end_critical");
	if (__kmpc_end_critical_real == NULL && rank == 0)
		fprintf (stderr, PACKAGE_NAME": Unable to find __kmpc_end_critical in DSOs!!\n");
	INC_IF_NOT_NULL(__kmpc_end_critical_real,count);

	/* Obtain @ for __kmpc_dispatch_next_4 */
	__kmpc_dispatch_next_4_real =
		(int(*)(void*,int,int*,int*,int*,int*))
		dlsym (RTLD_NEXT, "__kmpc_dispatch_next_4");
	if (__kmpc_dispatch_next_4_real == NULL && rank == 0)
		fprintf (stderr, PACKAGE_NAME": Unable to find __kmpc_dispatch_next_4 in DSOs!!\n");
	INC_IF_NOT_NULL(__kmpc_dispatch_next_4_real,count);

	/* Obtain @ for __kmpc_dispatch_next_8 */
	__kmpc_dispatch_next_8_real =
		(int(*)(void*,int,int*,long long *,long long *, long long *))
		dlsym (RTLD_NEXT, "__kmpc_dispatch_next_8");
	if (__kmpc_dispatch_next_8_real == NULL && rank == 0)
		fprintf (stderr, PACKAGE_NAME": Unable to find __kmpc_dispatch_next_8 in DSOs!!\n");
	INC_IF_NOT_NULL(__kmpc_dispatch_next_8_real,count);

	/* Obtain @ for __kmpc_dispatch_next_8 */
	__kmpc_single_real =
		(int(*)(void*,int)) dlsym (RTLD_NEXT, "__kmpc_single");
	if (__kmpc_single_real == NULL && rank == 0)
		fprintf (stderr, PACKAGE_NAME": Unable to find __kmpc_single in DSOs!!\n");
	INC_IF_NOT_NULL(__kmpc_single_real,count);

	/* Obtain @ for __kmpc_dispatch_next_8 */
	__kmpc_end_single_real =
		(void(*)(void*,int)) dlsym (RTLD_NEXT, "__kmpc_end_single");
	if (__kmpc_end_single_real == NULL && rank == 0)
		fprintf (stderr, PACKAGE_NAME": Unable to find __kmpc_end_single in DSOs!!\n");
	INC_IF_NOT_NULL(__kmpc_end_single_real,count);

	/* Obtain @ for __kmpc_dispatch_init_4 */
	__kmpc_dispatch_init_4_real =
		(void(*)(void*,int,int,int,int,int,int)) dlsym (RTLD_NEXT, "__kmpc_dispatch_init_4");
	if (__kmpc_dispatch_init_4_real == NULL && rank == 0)
		fprintf (stderr, PACKAGE_NAME": Unable to find __kmpc_dispatch_init_4 in DSOs!!\n");
	INC_IF_NOT_NULL(__kmpc_dispatch_init_4_real,count);

	/* Obtain @ for __kmpc_dispatch_init_8 */
	__kmpc_dispatch_init_8_real =
		(void(*)(void*,int,int,long long,long long,long long,long long)) dlsym (RTLD_NEXT, "__kmpc_dispatch_init_8");
	if (__kmpc_dispatch_init_8_real == NULL && rank == 0)
		fprintf (stderr, PACKAGE_NAME": Unable to find __kmpc_dispatch_init_8 in DSOs!!\n");
	INC_IF_NOT_NULL(__kmpc_dispatch_init_8_real,count);

	/* Obtain @ for __kmpc_dispatch_fini_4 */
	__kmpc_dispatch_fini_4_real =
		(void(*)(void*,int)) dlsym (RTLD_NEXT, "__kmpc_dispatch_fini_4");
	if (__kmpc_dispatch_fini_4_real == NULL && rank == 0)
		fprintf (stderr, PACKAGE_NAME": Unable to find __kmpc_dispatch_fini_4 in DSOs!!\n");
	INC_IF_NOT_NULL(__kmpc_dispatch_fini_4_real,count);

	/* Obtain @ for __kmpc_dispatch_fini_8 */
	__kmpc_dispatch_fini_8_real =
		(void(*)(void*,long long)) dlsym (RTLD_NEXT, "__kmpc_dispatch_fini_8");
	if (__kmpc_dispatch_fini_8_real == NULL && rank == 0)
		fprintf (stderr, PACKAGE_NAME": Unable to find __kmpc_dispatch_fini_8 in DSOs!!\n");
	INC_IF_NOT_NULL(__kmpc_dispatch_fini_8_real,count);

	/* Obtain @ for __kmpc_omp_task_alloc */
	__kmpc_omp_task_alloc_real =
		(void*(*)(void*,int,int,size_t,size_t,void*)) dlsym (RTLD_NEXT, "__kmpc_omp_task_alloc");
	if (__kmpc_omp_task_alloc_real == NULL)
		fprintf (stderr, PACKAGE_NAME": Unable to find  __kmpc_omp_task_alloc in DSOs!!\n");

	/* Obtain @ for __kmpc_omp_task_begin_if0 */
	__kmpc_omp_task_begin_if0_real =
		(void(*)(void*,int,void*)) dlsym (RTLD_NEXT, "__kmpc_omp_task_begin_if0");
	if (__kmpc_omp_task_begin_if0_real == NULL)
		fprintf (stderr, PACKAGE_NAME": Unable to find  __kmpc_omp_task_begin_if0 in DSOs!!\n");

	/* Obtain @ for __kmpc_omp_task_complete_if0 */
	__kmpc_omp_task_complete_if0_real =
		(void(*)(void*,int,void*)) dlsym (RTLD_NEXT, "__kmpc_omp_task_complete_if0");
	if (__kmpc_omp_task_complete_if0_real == NULL)
		fprintf (stderr, PACKAGE_NAME": Unable to find  __kmpc_omp_task_complete_if0 in DSOs!!\n");

	/* Obtain @ for __kmpc_omp_taskwait */
	__kmpc_omp_taskwait_real = (int(*)(void*,int)) dlsym (RTLD_NEXT, "__kmpc_omp_taskwait");
	if (__kmpc_omp_taskwait_real == NULL)
		fprintf (stderr, PACKAGE_NAME": Unable to find  __kmpc_omp_taskwait in DSOs!!\n");

	/* Any hook point? */
	return count > 0;
}

static void *par_func;

#include "intel-kmpc-11-intermediate.c"

#if defined(DYNINST_MODULE)
void Extrae_intel_kmpc_runtime_init_dyninst (void *fork_call)
{
#if defined(DEBUG)
	fprintf (stderr, PACKAGE_NAME" DEBUG: Extrae_intel_kmpc_runtime_init_dyninst:\n");
	fprintf (stderr, PACKAGE_NAME" DEBUG: fork_call = %p\n", fork_call);
#endif

	__kmpc_fork_call_real = (void(*)(void*,int,void*,...)) fork_call;
}
#endif

/*
 * kmpc_fork_call / kmpc_fork_call_extrae_dyninst
 *   dlsym does not seem to work under dyninst and we can't replace this
 *   function by itself (opposite to MPI, OpenMP does not have something like
 *   PMPI). Thus, we need to pass the address of the original __kmpc_fork_call
 *   (through Extrae_intel_kmpc_runtime_init_dyninst) and let the new 
 *   __kmpc_fork_call_extrae_dyninst do the work by finally calling to
 *   __kmpc_fork_call passed.
 */

#if !defined(DYNINST_MODULE)
void __kmpc_fork_call (void *p1, int p2, void *p3, ...)
#else
void __kmpc_fork_call_extrae_dyninst (void *p1, int p2, void *p3, ...)
#endif
{
	void *params[64];
	va_list ap;
	int i;

#if defined(DEBUG)
#if defined(DYNINST_MODULE)
	fprintf (stderr, PACKAGE_NAME": THREAD %d: __kmpc_fork_call_extrae_dyninst is at %p\n", THREADID, __kmpc_fork_call_extrae_dyninst);
#endif
	fprintf (stderr, PACKAGE_NAME": THREAD %d: __kmpc_fork_call is at %p\n", THREADID, __kmpc_fork_call_real);
	fprintf (stderr, PACKAGE_NAME": THREAD %d: __kmpc_fork_call params %p %d %p (and more to come ... )\n", THREADID, p1, p2, p3);
#endif

	if (__kmpc_fork_call_real != NULL)
	{
		Extrae_OpenMP_ParRegion_Entry ();

		/* Grab parameters */
		va_start (ap, p3);
		for (i = 0; i < p2; i++)
			params[i] = va_arg (ap, void*);
		va_end (ap);

		par_func = p3;

		switch (p2)
		{
			/* This big switch is handled by this file generated automatically by  genstubs-kmpc-11.sh */
#include "intel-kmpc-11-intermediate-switch.c"

			default:
				fprintf (stderr, PACKAGE_NAME": Error! Unhandled __kmpc_fork_call with %d arguments! Quitting!\n", p2);
				exit (-1);
				break;
		}

		Extrae_OpenMP_ParRegion_Exit ();	
	}
	else
	{
		fprintf (stderr, PACKAGE_NAME": __kmpc_fork_call is not hooked! exiting!!\n");
		exit (0);
	}
}

#if !defined(DYNINST_MODULE)
void __kmpc_barrier (void *p1, int p2)
{
#if defined(DEBUG)
	fprintf (stderr, PACKAGE_NAME": THREAD %d: __kmpc_barrier is at %p\n", THREADID, __kmpc_barrier_real);
	fprintf (stderr, PACKAGE_NAME": THREAD %d: __kmpc_barrier params %p %d\n", THREADID, p1, p2);
#endif

	if (__kmpc_barrier_real != NULL)
	{
		Extrae_OpenMP_Barrier_Entry ();
		__kmpc_barrier_real (p1, p2);
		Extrae_OpenMP_Barrier_Exit ();
	}
	else
	{
		fprintf (stderr, PACKAGE_NAME": __kmpc_barrier is not hooked! exiting!!\n");
		exit (0);
	}
}

void __kmpc_critical (void *p1, int p2, void *p3)
{
#if defined(DEBUG)
	fprintf (stderr, PACKAGE_NAME": THREAD %d: __kmpc_critical is at %p\n", THREADID, __kmpc_critical_real);
	fprintf (stderr, PACKAGE_NAME": THREAD %d: __kmpc_critical params %p %d %p\n", THREADID, p1, p2, p3);
#endif

	if (__kmpc_critical_real != NULL)
	{
		Extrae_OpenMP_Named_Lock_Entry ();
		__kmpc_critical_real (p1, p2, p3);
		Extrae_OpenMP_Named_Lock_Exit ();
	}
	else
	{
		fprintf (stderr, PACKAGE_NAME": __kmpc_critical is not hooked! exiting!!\n");
		exit (0);
	}
}

void __kmpc_end_critical (void *p1, int p2, void *p3)
{
#if defined(DEBUG)
	fprintf (stderr, PACKAGE_NAME": THREAD %d: __kmpc_end_critical is at %p\n", THREADID, __kmpc_end_critical_real);
	fprintf (stderr, PACKAGE_NAME": THREAD %d: __kmpc_end_critical params %p %d %p\n", THREADID, p1, p2, p3);
#endif

	if (__kmpc_end_critical_real != NULL)
	{
		Extrae_OpenMP_Named_Unlock_Entry ();
		__kmpc_end_critical_real (p1, p2, p3);
		Extrae_OpenMP_Named_Unlock_Exit ();
	}
	else
	{
		fprintf (stderr, PACKAGE_NAME": __kmpc_critical is not hooked! exiting!!\n");
		exit (0);
	}
}

int __kmpc_dispatch_next_4 (void *p1, int p2, int *p3, int *p4, int *p5, int *p6)
{
	int res;

#if defined(DEBUG)
	fprintf (stderr, PACKAGE_NAME": THREAD %d: __kmpc_dispatch_next_4 is at %p\n", THREADID, __kmpc_dispatch_next_4_real);
	fprintf (stderr, PACKAGE_NAME": THREAD %d: __kmpc_dispatch_next_4 params %p %d %p %p %p %p\n", THREADID, p1, p2, p3, p4, p5, p6);
#endif

	if (__kmpc_dispatch_next_8_real != NULL)
	{
		Extrae_OpenMP_Work_Entry();
		res = __kmpc_dispatch_next_4_real (p1, p2, p3, p4, p5, p6);
		Extrae_OpenMP_Work_Exit();

		if (res == 0) /* Alternative to call __kmpc_dispatch_fini_4 which seems not to be called ? */
		{
			Extrae_OpenMP_UF_Exit ();
			Extrae_OpenMP_DO_Exit ();
		}
	}
	else
	{
		fprintf (stderr, PACKAGE_NAME": __kmpc_dispatch_next_8 is not hooked! exiting!!\n");
		exit (0);
	}
	return res;
}

int __kmpc_dispatch_next_8 (void *p1, int p2, int *p3, long long *p4, long long *p5, long long *p6)
{
	int res;

#if defined(DEBUG)
	fprintf (stderr, PACKAGE_NAME": THREAD %d: __kmpc_dispatch_next_8 is at %p\n", THREADID, __kmpc_dispatch_next_8_real);
	fprintf (stderr, PACKAGE_NAME": THREAD %d: __kmpc_dispatch_next_8 params %p %d %p %p %p %p\n", THREADID, p1, p2, p3, p4, p5, p6);
#endif
	if (__kmpc_dispatch_next_8_real != NULL)
	{
		Extrae_OpenMP_Work_Entry();
		res = __kmpc_dispatch_next_8_real (p1, p2, p3, p4, p5, p6);
		Extrae_OpenMP_Work_Exit();

		if (res == 0) /* Alternative to call __kmpc_dispatch_fini_8 which seems not to be called ? */
		{
			Extrae_OpenMP_UF_Exit ();
			Extrae_OpenMP_DO_Exit ();
		}
	}
	else
	{
		fprintf (stderr, PACKAGE_NAME": __kmpc_dispatch_next_8 is not hooked! exiting!!\n");
		exit (0);
	}
	return res;
}

int __kmpc_single (void *p1, int p2)
{
	int res = 0;

#if defined(DEBUG)
	fprintf (stderr, PACKAGE_NAME": THREAD %d: __kmpc_single is at %p\n", THREADID, __kmpc_single_real);
	fprintf (stderr, PACKAGE_NAME": THREAD %d: __kmpc_single params %p %d\n", THREADID, p1, p2);
#endif

	if (__kmpc_single_real != NULL)
	{
		Extrae_OpenMP_Single_Entry ();

		res = __kmpc_single_real (p1, p2);

		if (res) /* If the thread entered in the single region, track it */
		{
			struct __kmpv_location_t *loc = (struct __kmpv_location_t*) p1;
			// printf ("loc->location = %s\n", loc->location);
			Extrae_OpenMP_UF_Entry ((UINT64) loc->location);
		}
		else
		{
			Extrae_OpenMP_Single_Exit ();
		}
	}
	else
	{
		fprintf (stderr, PACKAGE_NAME": __kmpc_critical is not hooked! exiting!!\n");
		exit (0);
	}

	return res;
}

void __kmpc_end_single (void *p1, int p2)
{
#if defined(DEBUG)
	fprintf (stderr, PACKAGE_NAME": THREAD %d: __kmpc_end_single is at %p\n", THREADID, __kmpc_single_real);
	fprintf (stderr, PACKAGE_NAME": THREAD %d: __kmpc_end_single params %p %d\n", THREADID, p1, p2);
#endif

	if (__kmpc_single_real != NULL)
	{
		/* This is only executed by the thread that entered the single region */
		Extrae_OpenMP_UF_Exit ();
		__kmpc_end_single_real (p1, p2);
		Extrae_OpenMP_Single_Exit ();
	}
	else
	{
		fprintf (stderr, PACKAGE_NAME": __kmpc_critical is not hooked! exiting!!\n");
		exit (0);
	}
}

void __kmpc_dispatch_init_4 (void *p1, int p2, int p3, int p4, int p5, int p6,
	int p7)
{
#if defined(DEBUG)
	fprintf (stderr, PACKAGE_NAME": THREAD %d: __kmpc_dispatch_init_4 is at %p\n", THREADID, __kmpc_dispatch_init_4_real);
	fprintf (stderr, PACKAGE_NAME": THREAD %d: __kmpc_dispatch_init_4 params are %p %d %d %d %d %d %d\n", THREADID, p1, p2, p3, p4, p5, p6, p7);
#endif

	if (__kmpc_dispatch_init_4_real != NULL)
	{
		Extrae_OpenMP_DO_Entry ();
		__kmpc_dispatch_init_4_real (p1, p2, p3, p4, p5, p6, p7);
		Extrae_OpenMP_UF_Entry ((UINT64) par_func /*(UINT64)p1*/); /* p1 cannot be translated with bfd? */
	}
	else
	{
		fprintf (stderr, PACKAGE_NAME":__kmpc_dispatch_init_4 is not hooked! exiting!!\n");
		exit (0);
	}
}

void __kmpc_dispatch_init_8 (void *p1, int p2, int p3, long long p4,
	long long p5, long long p6, long long p7)
{
#if defined(DEBUG)
	fprintf (stderr, PACKAGE_NAME": THREAD %d: __kmpc_dispatch_init_8 is at %p\n", THREADID, __kmpc_dispatch_init_8_real);
	fprintf (stderr, PACKAGE_NAME": THREAD %d: __kmpc_dispatch_init_8 params are %p %d %d %lld %lld %lld %lld\n", THREADID, p1, p2, p3, p4, p5, p6, p7);
#endif

	if (__kmpc_dispatch_init_8_real != NULL)
	{
		Extrae_OpenMP_DO_Entry ();
		__kmpc_dispatch_init_8_real (p1, p2, p3, p4, p5, p6, p7);
		Extrae_OpenMP_UF_Entry ((UINT64) par_func /*(UINT64)p1*/); /* p1 cannot be translated with bfd? */
	}
	else
	{
		fprintf (stderr, PACKAGE_NAME":__kmpc_dispatch_init_8 is not hooked! exiting!!\n");
		exit (0);
	}
}

void __kmpc_dispatch_fini_4 (void *p1, int p2)
{
#if defined(DEBUG)
	fprintf (stderr, PACKAGE_NAME": THREAD %d: __kmpc_dispatch_fini_4 is at %p\n", THREADID, __kmpc_dispatch_fini_4_real);
	fprintf (stderr, PACKAGE_NAME": THREAD %d: __kmpc_dispatch_fini_4 params are %p %d\n", THREADID, p1, p2);
#endif

	if (__kmpc_dispatch_fini_4_real != NULL)
	{
		Extrae_OpenMP_DO_Exit ();
		__kmpc_dispatch_fini_4_real (p1, p2);
		Extrae_OpenMP_UF_Exit ();
	}
	else
	{
		fprintf (stderr, PACKAGE_NAME":__kmpc_dispatch_fini_4 is not hooked! exiting!!\n");
		exit (0);
	}
}

void __kmpc_dispatch_fini_8 (void *p1, long long p2)
{
#if defined(DEBUG)
	fprintf (stderr, PACKAGE_NAME": THREAD %d: __kmpc_dispatch_fini_8 is at %p\n", THREADID, __kmpc_dispatch_fini_8_real);
	fprintf (stderr, PACKAGE_NAME": THREAD %d: __kmpc_dispatch_fini_8 params are %p %lld\n", THREADID, p1, p2);
#endif

	if (__kmpc_dispatch_fini_8_real != NULL)
	{
		Extrae_OpenMP_DO_Exit ();
		__kmpc_dispatch_fini_8_real (p1, p2);
		Extrae_OpenMP_UF_Exit ();
	}
	else
	{
		fprintf (stderr, PACKAGE_NAME":__kmpc_dispatch_fini_8 is not hooked! exiting!!\n");
		exit (0);
	}
}

#define EXTRAE_MAP_KMPC_TASK_SIZE 1024*1024

struct extrae_map_kmpc_task_function_st
{
	void *kmpc_task;
	void *function;
};

static struct extrae_map_kmpc_task_function_st extrae_map_kmpc_task_function[EXTRAE_MAP_KMPC_TASK_SIZE];
static unsigned extrae_map_kmpc_count = 0;

static void __extrae_add_kmpc_task_function (void *kmpc_task, void *function)
{
	unsigned u = 0;

	pthread_mutex_lock (&extrae_map_kmpc_mutex);
	if (extrae_map_kmpc_count < EXTRAE_MAP_KMPC_TASK_SIZE)
	{
		/* Look for a free place in the table */
		while (extrae_map_kmpc_task_function[u].kmpc_task != NULL)
			u++;

		/* Add the pair and aggregate to the count */
		extrae_map_kmpc_task_function[u].function = function;
		extrae_map_kmpc_task_function[u].kmpc_task = kmpc_task;
		extrae_map_kmpc_count++;
	}
	else
	{
		fprintf (stderr, PACKAGE_NAME": THREAD %d Error number of on-the-fly allocated tasks is above EXTRAE_MAP_KMPC_TASK_SIZE (%s:%d)\n", THREADID,  __FILE__, __LINE__);
		exit (0);
	}
	pthread_mutex_unlock (&extrae_map_kmpc_mutex);
}

static void * __extrae_remove_kmpc_task_function (void *kmpc_task)
{
	void *res = NULL;
	unsigned u = 0;

	pthread_mutex_lock (&extrae_map_kmpc_mutex);
	if (extrae_map_kmpc_count > 0)
	{
		while (u < EXTRAE_MAP_KMPC_TASK_SIZE)
		{
			if (extrae_map_kmpc_task_function[u].kmpc_task == kmpc_task)
			{
				res = extrae_map_kmpc_task_function[u].function;
				extrae_map_kmpc_task_function[u].kmpc_task = NULL;
				extrae_map_kmpc_task_function[u].function = NULL;
				break;
			}
			u++;
		}
	}
	pthread_mutex_unlock (&extrae_map_kmpc_mutex);

	return res;
}

static void __extrae_kmpc_task_substitute (int p1, void *p2)
{
	void (*__kmpc_task_substituted_func)(int,void*) = (void(*)(int,void*)) __extrae_remove_kmpc_task_function (p2);

	if (__kmpc_task_substituted_func != NULL)
	{
		Extrae_OpenMP_TaskUF_Entry ((UINT64) __kmpc_task_substituted_func);
		__kmpc_task_substituted_func (p1, p2); /* Original code execution */
		Extrae_OpenMP_TaskUF_Exit ();
	}
	else
	{
		fprintf (stderr, PACKAGE_NAME": THREAD %d did not find task substitution (%s:%d)\n", THREADID,  __FILE__, __LINE__);
		exit (0);
	}
}

void * __kmpc_omp_task_alloc (void *p1, int p2, int p3, size_t p4, size_t p5, void *p6)
{
	void *res;

#if defined(DEBUG)
	fprintf (stderr, PACKAGE_NAME": THREAD %d: __kmpc_omp_task_alloc_real is at %p\n", THREADID, __kmpc_omp_task_alloc_real);
	fprintf (stderr, PACKAGE_NAME": THREAD %d: __kmpc_omp_task_alloc params %p %d %d %d %d %p\n", THREADID, p1, p2, p3, p4, p5, p6);
#endif

	if (__kmpc_omp_task_alloc_real != NULL)
	{
		Extrae_OpenMP_Task_Entry ((UINT64)p6);
		res = __kmpc_omp_task_alloc_real (p1, p2, p3, p4, p5, __extrae_kmpc_task_substitute);
		__extrae_add_kmpc_task_function (res, p6);
		Extrae_OpenMP_Task_Exit ();
	}
	else
	{
		fprintf (stderr, PACKAGE_NAME": THREAD %d __kmpc_omp_task_alloc was not hooked. Exiting!\n", THREADID);
		exit (0);
	}
	
	return res;
}

void __kmpc_omp_task_begin_if0 (void *p1, int p2, void *p3)
{
#if defined(DEBUG)
	fprintf (stderr, PACKAGE_NAME": THREAD %d: __kmpc_omp_task_begin_if0_real is at %p\n", THREADID, __kmpc_omp_task_begin_if0_real);
	fprintf (stderr, PACKAGE_NAME": THREAD %d: __kmpc_omp_task_begin_if0 params %p %d %p\n", THREADID, p1, p2, p3);
#endif

	void (*__kmpc_task_substituted_func)(int,void*) = (void(*)(int,void*)) __extrae_remove_kmpc_task_function (p3);

	if (__kmpc_task_substituted_func != NULL)
	{
		if (__kmpc_omp_task_begin_if0_real != NULL)
		{
			Extrae_OpenMP_TaskUF_Entry ((UINT64) __kmpc_task_substituted_func);
			__kmpc_omp_task_begin_if0_real (p1, p2, p3);
		}
		else
		{
			fprintf (stderr, PACKAGE_NAME": __kmpc_omp_task_begin_if0 is not hooked! Exiting!!\n");
			exit (0);
		}
	}
	else
	{
		fprintf (stderr, PACKAGE_NAME": THREAD %d did not find task substitution (%s:%d)\n", THREADID,  __FILE__, __LINE__);
		exit (0);
	}
}

void __kmpc_omp_task_complete_if0 (void *p1, int p2, void *p3)
{
#if defined(DEBUG)
	fprintf (stderr, PACKAGE_NAME": THREAD %d: __kmpc_omp_task_complete_if0_real is at %p\n", THREADID, __kmpc_omp_task_complete_if0_real);
	fprintf (stderr, PACKAGE_NAME": THREAD %d: __kmpc_omp_task_complete_if0 params %p %d %p\n", THREADID, p1, p2, p3);
#endif

	if (__kmpc_omp_task_complete_if0_real != NULL)
	{
		__kmpc_omp_task_complete_if0_real (p1, p2, p3);
		Extrae_OpenMP_TaskUF_Exit ();
	}
	else
	{
		fprintf (stderr, PACKAGE_NAME": __kmpc_omp_task_complete_if0 is not hooked! Exiting!!\n");
		exit (0);
	}
}

int __kmpc_omp_taskwait (void *p1, int p2)
{
	int res;

#if defined(DEBUG)
	fprintf (stderr, PACKAGE_NAME": THREAD %d: __kmpc_omp_taskwait_real is at %p\n", THREADID, __kmpc_omp_taskwait_real);
	fprintf (stderr, PACKAGE_NAME": THREAD %d: __kmpc_omp_taskwait params %p %d\n", THREADID, p1, p2);
#endif

	if (__kmpc_omp_taskwait_real != NULL)
	{
		Extrae_OpenMP_Taskwait_Entry();
		res = __kmpc_omp_taskwait_real (p1, p2);
		Extrae_OpenMP_Taskwait_Exit();
	}
	else
	{
		fprintf (stderr, PACKAGE_NAME": __kmpc_omp_taskwait is not hooked! Exiting!!\n");
		exit (0);
	}
	return res;
}
#endif /* !defined(DYNINST_MODULE) */
