---
layout: page
title: fi_hook(7)
tagline: Libfabric Programmer's Manual
---
{% include JB/setup %}

# NAME

fi_hook \- The Hook Fabric Provider Utility

# OVERVIEW

The hooking provider is a utility function that can intercept calls to any
provider.  The hook provider is always available, but has zero impact on
calls unless enabled.  It is useful for providing performance data on
selected calls or debugging information.

# SUPPORTED FEATURES

Hooking support is enabled through the FI_HOOK environment variable.  To
enable hooking, FI_HOOK must be set to the name of one or more of the
available hooking providers.  When multiple hooks are specified, the
names must be separated by a semi-colon.  To obtain a list of hooking
providers available on the current system, one can use the fi_info
utility with the '--env' command line option.  Hooking providers are
usually identified by 'hook' appearing in the provider name.

Known hooking providers include the following:

*ofi_hook_perf*
: This hooks 'fast path' data operation calls.  Performance data is
  captured on call entrance and exit, in order to provide an average of
  how long each call takes to complete.  See the PERFORMANCE HOOKS section
  for available performance data.

*ofi_hook_trace*
: This hooks most of API calls for fabric communications. The APIs and their
  runtime parameters are logged to provide detail trace information for
  debugging. See the TRACE HOOKS section for the APIs enabled for trace.

*ofi_hook_profile*
: This hooks data operation calls, cq operation calls and mr registration
  calls. The API calls and the amount of data being operated are accumulated,
  to provide a view of APIs' usages and a histogram of data size operated
  in a workload execution. See the PROFILE HOOKS section for the report in
  the detail.

*ofi_hook_monitor*
: Similar to *ofi_hook_profile*, this hooks data operation calls, cq operation
  calls and mr registration calls. The API calls and the amount of data being
  operated are accumulated and made available for export via an external sampler 
  through a shared communication file. See the MONITOR HOOKS section for more details.

# PERFORMANCE HOOKS

The hook provider allows capturing inline performance data by accessing the
CPU Performance Management Unit (PMU).  PMU data is only available on Linux
systems.  Additionally, access to PMU data may be restricted to privileged
(super-user) applications.

Performance data is captured for critical data transfer calls:
fi_msg, fi_rma, fi_tagged, fi_cq, and fi_cntr.  Captured data is displayed
as logged data using the FI_LOG_LEVEL trace level.  Performance data is
logged when the associated fabric is destroyed.

The environment variable FI_PERF_CNTR is used to identify which performance
counter is tracked.  The following counters are available:

*cpu_cycles*
: Counts the number of CPU cycles each function takes to complete.

*cpu_instr*
: Counts the number of CPU instructions each function takes to complete.
  This is the default performance counter if none is specified.

# TRACE HOOKS

This hook provider allows tracing each API call and its runtime parameters.
It is enabled by setting FI_HOOK to "trace".

The trace data include the provider's name, API function, and input/output
parameter values. The APIs enabled for tracing include the following:

*data operation calls*
: This include fi_msg, fi_rma, fi_tagged all data operations. The trace data
  contains the data buffer, data size being operated, data, tags, and flags
  when applicable.

*cq operation calls*
: This includes fi_cq_read, fi_cq_sread, fi_cq_strerror and all cq operations.
  The trace data contains the cq entries based on cq format.

*cm operation calls*
: This includes fi_getname, fi_setname, fi_getpeer, fi_connect and all cm
  operations. The trace data contains the target address.

*resource creation calls*
: This includes fi_av_open, fi_cq_open, fi_endpoing, fi_cntr_open and fi_mr
  operations. The trace data contains the corresponding attributes used for
  resource creation.

The trace data is logged after API is invoked using the FI_LOG_LEVEL trace
level

# PROFILE HOOKS

This hook provider allows capturing data operation calls and the amount of
data being operated. It is enabled by setting FI_HOOK to "profile".

The provider counts the API invoked and accumulates the data size each API
call operates. For data and cq operations, instead of accumulating all data
together, it breaks down the data size into size buckets and accumulates
the amount of data in the corresponding bucket based on the size of the data
operated. For mr registration operations, it breaks down memory registered
per HMEM iface. At the end when the associated fabric is destroyed, the
provider generates a profile report.

The report contains the usage of data operation APIs, the amount of data
received in each CQ format and the amount of memory registered for rma
operations if any exist. In addition, the data operation APIs are grouped
into 4 groups based on the nature of the operations, message send (fi_sendXXX,
fi_tsendXXX), message receive (fi_recvXXX, fi_trecvXXX), rma read (fi_readXXX)
and rma write (fi_writeXXX) to present the percentage usage of each API.

The report is in a table format which has APIs invoked in rows, and the columns
contain the following fields:

*API*
: The API calls are invoked.

*Size*
: Data size bucket that at least one API call operates data in that size bucket.
  The pre-defined size buckets (in Byte) are [0-64] [64-512] [512-1K] [1K-4K]
  [4K-64K] [64K-256K] [256K-1M] [1M-4M] [4M-UP].

*Count*
: Count of the API calls.

*Amount*
: Amount of data the API operated.

*% Count*
: Percentage of the API calls over the total API calls in the same data operation
  group.

*% Amount*
: Percentage of the amount of data from the API over the total amount
  of data operated in the same data operation group.

The report is logged using the FI_LOG_LEVEL trace level.

# MONITOR HOOKS

This hook provider builds on the "profile" hook provider and provides continuous readout
capabilities useful for monitoring libfabric applications. It is enabled by setting
FI_HOOK to "monitor".

Similar to the "profile" hook provider, this provider counts the number of invoked API calls
and accumulates the amount of data handled by each API call. Refer to the documentation
on the "profile" hook for more information.

Data export is facilitated using a communication file on the filesystem, which is created for
each hooked libfabric provider. The monitor hook expects to be run on a tmpfs.
If available and unless otherwise specified, files will be created under the tmpfs `/dev/shm`.

A sampler can periodically read this file to extract the gathered data.
Every N intercepted API calls or "ticks", the provider will check
whether data export has been requested by the sampler. If so, the currently gathered
counter data is copied to the file and the sampler informed about the new data. The provider-local
counter data is then cleared. 
Each sample contains the counter delta to the previous sample.

Communication files will be created at path `$FI_OFI_HOOK_MONITOR_BASEPATH/<uid>/<hostname>` and 
will have the name: `<ppid>_<pid>_<sequential id>_<job id>_<provider name>`.
`ppid` and `pid` are taken from the perspective of the monitored application.
In a batched environment running SLURM, `job id` is set to the SLURM job ID, otherwise it is set to 0.

See [`fi_mon_sampler`(1)](fi_mon_sampler.1.html) for documentation on how to use the monitor provider sampler.

## CONFIGURATION

The "monitor" hook provider exposes several runtime options via environment variables:

*FI_OFI_HOOK_MONITOR_BASEPATH*
:   String to basepath for communication files. (default: /dev/shm/ofi)

*FI_OFI_HOOK_MONITOR_DIR_MODE*
:   POSIX mode/permission for directories in basepath. (default: 01700)

*FI_OFI_HOOK_MONITOR_FILE_MODE*
:   POSIX mode/permission for communication files. (default: 0600)

*FI_OFI_HOOK_MONITOR_TICK_MAX*
:   Number of API calls before communication files are checked for data request.
    (default: 1024)

*FI_OFI_HOOK_MONITOR_LINGER*
:   Whether communication files should linger after termination. (default: 0)
    This is useful to allow the sampler to read the last counter data even if the libfabric
    application has already terminated.
    Note: Using this option without a sampler results in files cluttering
    FI_OFI_HOOK_MONITOR_BASEPATH. Make sure to either run a sampler or clean
    these files manually.

# LIMITATIONS

Hooking functionality is not available for providers built using the
FI_FABRIC_DIRECT feature.  That is, directly linking to a provider prevents
hooking.

The hooking provider does not work with triggered operations.  Application
that use FI_TRIGGER operations that attempt to hook calls will likely crash.

# SEE ALSO

[`fabric`(7)](fabric.7.html),
[`fi_provider`(7)](fi_provider.7.html)
