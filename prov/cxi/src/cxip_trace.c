/*
 * (C) Copyright 2021-2023 Hewlett Packard Enterprise Development LP
 *
 * This software is available to you under a choice of one of two
 * licenses.  You may choose to be licensed under the terms of the GNU
 * General Public License (GPL) Version 2, available from the file
 * COPYING in the main directory of this source tree, or the
 * BSD license below:
 *
 *     Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/**
 * @brief TRACE function for producing runtime debugging logs
 *
 * The tracing call to place inline within the code is a macro, CXIP_TRACE.
 * This is defined in cxip.h, and is enabled by the flag ENABLE_DEBUG at
 * compile time, which is triggered by having DBG=1 in the compile
 * environment.
 *
 * If ENABLE_DEBUG is false at compile time, CXIP_TRACE is a syntactically
 * robust NOOP which results in no code being emitted, ensuring that these
 * trace calls do not affect performance in production.
 *
 * CXIP_TRACE can be used directly in each code module, but is usually
 * defined behind one or more TRACE* macros that are local to a code file,
 * which allows trace generation to be isolated to certain areas of code.
 *
 * This is structured to allow the trace functions to be replaced in specific
 * test environments, by simply replacing the function pointers with new
 * pointers. They are statically initialized to the functions in this module,
 * and can be overwritten at any point in the test environment to give new
 * behavior. For instance, all of the tracing functions could be replaced
 * using sockets-based telemetry code.
 *
 * - cxip_trace_fn is the function that logs a trace message.
 * - cxip_trace_flush_fn can be used to flush buffered trace messages.
 * - cxip_trace_close_fn can be used to flush and close the output.
 * - cxip_trace_enable_fn is used to enable/disable tracing.
 *
 * Note that ENABLE_DEBUG=1 with tracing disabled will still slow
 * applications slightly, since every CXIP_TRACE macro will generate a
 * test/branch on cxip_trace_enabled. For maximum performance, ENABLE_DEBUG
 * should be set to FALSE, which compiles out all of the TRACE code.
 *
 * Global variables for rank and numranks must generally be supplied by the
 * application, and apply only to the formatted TRACE output, which helps to
 * clearly disambiguate the stream of trace messages.
 *
 * The functions in this module are intended for use in a multinode,
 * multirank test environment, where ranks generally represent independent
 * processes running on different compute nodes.
 * 
 * The normal sequence is to set cxip_trace_rank, cxip_trace_numranks, and
 * cxip_trace_append, then call cxip_trace_enable(true). This will create a
 * file named "traceN", where N is the cxip_trace_rank value. The file is
 * created using the "w" or "a" file flag, depending on cxip_trace_append,
 * and remains open throughout the test run. Output can be suspended by
 * calling cxip_trace_enable(false), and can be resumed by calling
 * cxip_trace_enable(true).
 *
 * cxip_trace_fid is exposed, and can be manipulated using the normal file
 * stream functions. Default buffering is fully buffered output, which can
 * result in delays in the appearance of logging information. Using
 * setlinebuf() will run slower, but will display lines more quickly.
 *
 * cxip_trace_flush() forces all output be flushed AND written to disk, but
 * leaves the file open for more writing.
 *
 * cxip_trace_close() flushes all output and closes the file.
 *
 * Close is also performed automatically if the running process terminates.
 * This is the main reason for the cxip_trace_append flag, which should be
 * set to true if tracing is enabled for multiple Criterion test cases under
 * NETSIM. Each test case is run as a separate process. If cxip_trace_append
 * is false, you will only see the results of the last test performed.
 */
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "cxip.h"

bool cxip_trace_enabled;
bool cxip_trace_append;
int cxip_trace_rank;
int cxip_trace_numranks;
FILE *cxip_trace_fid;

/* Static initialization of default trace functions, can be overridden */
cxip_trace_t cxip_trace_attr cxip_trace_fn = cxip_trace;
cxip_trace_flush_t cxip_trace_flush_fn = cxip_trace_flush;
cxip_trace_close_t cxip_trace_close_fn = cxip_trace_close;
cxip_trace_enable_t cxip_trace_enable_fn = cxip_trace_enable;

void cxip_trace_flush(void)
{
	if (cxip_trace_fid) {
		fflush(cxip_trace_fid);
		fsync(fileno(cxip_trace_fid));
	}
}

void cxip_trace_close(void)
{
	if (cxip_trace_fid) {
		cxip_trace_flush();
		fclose(cxip_trace_fid);
		cxip_trace_fid = NULL;
	}
}

int cxip_trace_attr cxip_trace(const char *fmt, ...)
{
	va_list args;
	char *str;
	int len;

	if (!cxip_trace_enabled)
		return 0;
	va_start(args, fmt);
	len = vasprintf(&str, fmt, args);
	va_end(args);
	if (len >= 0) {
		len = fprintf(cxip_trace_fid, "[%2d|%2d] %s",
			      cxip_trace_rank, cxip_trace_numranks, str);
		free(str);
	}
	return len;
}

bool cxip_trace_enable(bool enable)
{
	bool was_enabled = cxip_trace_enabled;
	char fnam[256], *mode;

	if (!cxip_trace_fn) {
		fprintf(stderr, "cxip_trace_fn not defined\n");
		return false;
	}
	if (!cxip_trace_flush_fn) {
		fprintf(stderr, "cxip_trace_flush_fn not defined\n");
		return false;
	}
	if (!enable && cxip_trace_enabled) {
		fflush(cxip_trace_fid);
		cxip_trace_enabled = false;
	} else if (enable && !cxip_trace_enabled) {
		if (!cxip_trace_fid) {
			sprintf(fnam, "./trace%d", cxip_trace_rank);
			mode = (cxip_trace_append) ? "a" : "w";
			cxip_trace_fid = fopen(fnam, mode);
			if (!cxip_trace_fid) {
				fprintf(stderr, "open(%s) failed: %s\n",
					fnam, strerror(errno));
			}
		}
		if (cxip_trace_fid)
			cxip_trace_enabled = true;
	}
	return was_enabled;
}
