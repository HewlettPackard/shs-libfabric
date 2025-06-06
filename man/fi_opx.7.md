---
layout: page
title: fi_opx(7)
tagline: Libfabric Programmer's Manual
---
{%include JB/setup %}

# NAME

fi_opx \- The Omni-Path Express Fabric Provider

# OVERVIEW

The *opx* provider is a native libfabric provider suitable for
use with Omni-Path fabrics.  OPX features great scalability and
performance when running libfabric-enabled message layers.
OPX requires 3 additonal external development libraries to build:
libuuid, libnuma, and the Linux kernel headers.


# SUPPORTED FEATURES

The OPX provider supports most features defined for the libfabric API.

Key features include:

Endpoint types
: The Omni-Path HFI hardware is connectionless and reliable.
  The OPX provider only supports the *FI_EP_RDM* endpoint type.

Capabilities
: Supported capabilities include *FI_MSG*, *FI_RMA, *FI_TAGGED*, *FI_ATOMIC*,
  *FI_SOURCE*, *FI_SEND*, *FI_RECV*, *FI_MULTI_RECV*, *FI_DIRECTED_RECV*,
  *FI_SOURCE*.

  Notes on *FI_DIRECTED_RECV* capability: The immediate data which is sent
  within the "senddata" call to support *FI_DIRECTED_RECV* for OPX
  must be exactly 4 bytes, which OPX uses to completely identify the
  source address to an exascale\-level number of ranks for tag matching on
  the recv and can be managed within the MU packet.
  Therefore the domain attribute "cq_data_size" is set to 4 which is the OFI
  standard minimum.

Modes
: Two modes are defined: *FI_CONTEXT2* and *FI_ASYNC_IOV*.
  The OPX provider requires *FI_CONTEXT2*.

Additional features
: Supported additional features include *FABRIC_DIRECT* and *counters*.

Progress
: *FI_PROGRESS_MANUAL* and *FI_PROGRESS_AUTO* are supported, for best performance, use
  *FI_PROGRESS_MANUAL* when possible. *FI_PROGRESS_AUTO* will spawn 1 thread per CQ.

Address vector
: *FI_AV_MAP* and *FI_AV_TABLE* are both supported. *FI_AV_MAP* is default.

Memory registration modes
: Only *FI_MR_SCALABLE* is supported.

# UNSUPPORTED FEATURES

Endpoint types
: Unsupported endpoint types include *FI_EP_DGRAM* and *FI_EP_MSG*.

Capabilities
: The OPX provider does not support *FI_RMA_EVENT* and *FI_TRIGGER*
  capabilities.

# LIMITATIONS

OPX supports the following MPI versions:

Intel MPI from Parallel Studio 2020, update 4.
Intel MPI from OneAPI 2021, update 3.
Open MPI 4.1.2a1 (Older version of Open MPI will not work).
MPICH 3.4.2 and later.

Usage:

If using with OpenMPI 4.1.x, disable UCX and openib transports.
OPX is not compatible with Open MPI 4.1.x PML/BTL.

# CONFIGURATION OPTIONS

*OPX_AV*
: OPX supports the option of setting the AV mode to use in a build.
  3 settings are supported:
  - table
  - map
  - runtime

  Using table or map will only allow OPX to use FI_AV_TABLE or FI_AV_MAP.
  Using runtime will allow OPX to use either AV mode depending on what the
  application requests. Specifying map or table however may lead to a slight
  performance improvement depending on the application.

  To change OPX_AV, add OPX_AV=table, OPX_AV=map, or OPX_AV=runtime to the
  configure command. For example, to create a new build with OPX_AV=table:\
  OPX_AV=table ./configure\
  make install\
\
  There is no way to change OPX_AV after it is set. If OPX_AV is not set in
  the configure, the default value is runtime.

# RUNTIME PARAMETERS

*FI_OPX_UUID*
: OPX requires a unique ID for each job. In order for all processes in a
  job to communicate with each other, they require to use the same UUID.
  This variable can be set with FI_OPX_UUID=${RANDOM}
  The default UUID is 00112233445566778899aabbccddeeff.

*FI_OPX_FORCE_CPUAFFINITY*
: Boolean (1/0, on/off, true/false, yes/no). Causes the thread to bind
  itself to the cpu core it is running on. Defaults to "No"

*FI_OPX_RELIABILITY_SERVICE_USEC_MAX*
: Integer. This setting controls how frequently the reliability/replay
  function will issue PING requests to a remote connection. Reducing this
  value may improve performance at the expense of increased traffic on the
  OPX fabric.
  Default setting is 500.

*FI_OPX_RELIABILITY_SERVICE_MAX_OUTSTANDING_BYTES*
: Integer. This setting controls the maximum number of bytes allowed to be
  in-flight (sent but un-ACK'd by receiver) per reliability flow (one-way
  communication between two endpoints).

  Valid values are in the range of 8192-150,994,944 (8KB-144MB), inclusive.

  Default setting is 7,340,032 (7MB).

*FI_OPX_RELIABILITY_SERVICE_PRE_ACK_RATE*
: Integer. This setting controls how frequently a receiving rank will send ACKs
  for packets it has received without being prompted through a PING request.
  A non-zero value N tells the receiving rank to send an ACK for the
  last N packets every Nth packet. Used in conjunction with an increased
  value for FI_OPX_RELIABILITY_SERVICE_USEC_MAX may improve performance.

  Valid values are 0 (disabled) and powers of 2 in the range of 1-32,768, inclusive.

  Default setting is 64.

*FI_OPX_RELIABILITY_MAX_UNCONGESTED_PINGS*
: Integer. This setting controls how many PING requests the reliability/replay
  function will issue per iteration of FI_OPX_RELIABILITY_SERVICE_USEC_MAX in situations
  with less contending outgoing traffic from the HFI.
  Default setting is 128. Range of valid values is 1-65535.

*FI_OPX_RELIABILITY_MAX_CONGESTED_PINGS*
: Integer. This setting controls how many PING requests the reliability/replay
  function will issue per iteration of FI_OPX_RELIABILITY_SERVICE_USEC_MAX in situations
  with more contending, outgoing traffic from the HFI.
  Default setting is 4. Range of valid values is 1-65535.

*FI_OPX_SELINUX*
: Boolean (1/0, on/off, true/false, yes/no). Set to true if you're running a
  security-enhanced Linux. This enables updating the Jkey used based on system
  settings. Defaults to "No"

*FI_OPX_HFI_SELECT*
: String. Controls how OPX chooses which HFI to use when opening a context.
  Has two forms:
  - `<hfi-unit>` Force OPX provider to use `hfi-unit`.
  - `<selector1>[,<selector2>[,...,<selectorN>]]` Select HFI based on first matching `selector`

  Where `selector` is one of the following forms:
  - `default` to use the default logic
  - `fixed:<hfi-unit>` to fix to one `hfi-unit`
  - `<selector-type>:<hfi-unit>:<selector-data>`

  The above fields have the following meaning:
  - `selector-type` The selector criteria the caller opening the context is evaluated against.
  - `hfi-unit` The HFI to use if the caller matches the selector.
  - `selector-data` Data the caller must match (e.g. NUMA node ID).

  Where `selector-type` is one of the following:
  - `numa` True when caller is local to the NUMA node ID given by `selector-data`.
  - `core` True when caller is local to the CPU core given by `selector-data`.

  And `selector-data` is one of the following:
  - `value` The specific value to match
  - `<range-start>-<range-end>` Matches with any value in that range

  In the second form, when opening a context, OPX uses the `hfi-unit` of the
  first-matching selector. Selectors are evaluated left-to-right. OPX will
  return an error if the caller does not match any selector.

  In either form, it is an error if the specified or selected HFI is not in the
  Active state. In this case, OPX will return an error and execution will not
  continue.

  With this option, it is possible to cause OPX to try to open more contexts on
  an HFI than there are free contexts on that HFI. In this case, one or more of
  the context-opening calls will fail and OPX will return an error.
  For the second form, as which HFI is selected depends on properties of the
  caller, deterministic HFI selection requires deterministic caller properties.
  E.g.  for the `numa` selector, if the caller can migrate between NUMA domains,
  then HFI selection will not be deterministic.

  The logic used will always be the first valid in a selector list. For example, `default` and
  `fixed` will match all callers, so if either are in the beginning of a selector list, you will
  only use `fixed` or `default` regardles of if there are any more selectors.

  Examples:
  - `FI_OPX_HFI_SELECT=0` all callers will open contexts on HFI 0.
  - `FI_OPX_HFI_SELECT=1` all callers will open contexts on HFI 1.
  - `FI_OPX_HFI_SELECT=numa:0:0,numa:1:1,numa:0:2,numa:1:3` callers local to NUMA nodes 0 and 2 will use HFI 0, callers local to NUMA domains 1 and 3 will use HFI 1.
  - `FI_OPX_HFI_SELECT=numa:0:0-3,default` callers local to NUMA nodes 0 thru 3 (including 0 and 3) will use HFI 0, and all else will use default selection logic.
  - `FI_OPX_HFI_SELECT=core:1:0,fixed:0` callers local to CPU core 0 will use HFI 1, and all others will use HFI 0.
  - `FI_OPX_HFI_SELECT=default,core:1:0` all callers will use default HFI selection logic.

*FI_OPX_PORT*
: Integer. HFI1 port number.  If the specified port is not available, a default active port will be selected.
  Special value 0 indicates any available port. Defaults to port 1 on OPA100 and any port on CN5000.

*FI_OPX_DELIVERY_COMPLETION_THRESHOLD*
: Integer. Will be deprecated. Please use FI_OPX_SDMA_BOUNCE_BUF_THRESHOLD.

*FI_OPX_SDMA_BOUNCE_BUF_THRESHOLD*
: Integer. The maximum message length in bytes that will be copied to the SDMA bounce buffer.
  For messages larger than this threshold, the send will not be completed until receiver
  has ACKed. Value must be between 16385 and 2147483646. Defaults to 16385.

*FI_OPX_SDMA_DISABLE*
: Boolean (1/0, on/off, true/false, yes/no). Disables SDMA offload hardware. Default is 0.

*FI_OPX_MAX_PKT_SIZE*
: Integer. Set the maximum packet size which must be less than or equal to the driver's
  MTU (Maximum Transmission Unit) size.  Valid values: 2048, 4096, 8192, 10240.
  Default is set to 10240 for libraries built on CN5000 systems and set to 8192 for
  libraries built on OPA100 systems.

*FI_OPX_SDMA_MIN_PAYLOAD_BYTES*
: Integer. The minimum length in bytes where SDMA will be used.
  For messages smaller than this threshold, the send will be completed using PIO.
  Value must be between 64 and 2147483646. Defaults to 16385.

*FI_OPX_SDMA_MAX_WRITEVS_PER_CYCLE*
: Integer. The maximum number of times writev will be called during a single poll cycle.
  Value must be between 1 and 1024. Defaults to 1.

*FI_OPX_SDMA_MAX_IOVS_PER_WRITEV*
: Integer. The maximum number of IOVs passed to each writev call.
  Value must be between 3 and 128. Defaults to 64.

*FI_OPX_SDMA_MAX_PKTS*
: Integer. The maximum number of packets transmitted per SDMA request when expected receive (TID) is NOT being used.
  Value must be between 1 and 128. Defaults to 32.

*FI_OPX_SDMA_MAX_PKTS_TID*
: Integer. The maximum number of packets transmitted per SDMA request when expected receive (TID) is being used.
  Value must be between 1 and 512. Defaults to 64.

*FI_OPX_TID_MIN_PAYLOAD_BYTES*
: Integer. The minimum length in bytes where TID (Expected Receive) will be used.
  For messages smaller than this threshold, the send will be completed using Eager Receive.
  Value must be between 4096 and 2147483646. Defaults to 4096.

*FI_OPX_RZV_MIN_PAYLOAD_BYTES*
: Integer. The minimum length in bytes where rendezvous will be used.
  For messages smaller than this threshold, the send will first try to be completed using eager or multi-packet eager.
  Value must be between 64 and 65536. Defaults to 16385.

*FI_OPX_MP_EAGER_DISABLE*
: Boolean (1/0, on/off, true/false, yes/no). Disables multi-packet eager. Defaults to 0.

*FI_OPX_TID_DISABLE*
: Boolean (1/0, on/off, true/false, yes/no). Disables using Token ID (TID). Defaults to 0.

*FI_OPX_EXPECTED_RECEIVE_ENABLE*
: Deprecated. Use FI_OPX_TID_DISABLE instead.

*FI_OPX_PROG_AFFINITY*
: String. This sets the affinity to be used for any progress threads. Set as a colon-separated
  triplet as `start:end:stride`, where stride controls the interval between selected cores.
  For example, `1:5:2` will have cores 1, 3, and 5 as valid cores for progress threads. By default
  no affinity is set.

*FI_OPX_AUTO_PROGRESS_INTERVAL_USEC*
: Deprecated/ignored. Auto progress threads are now interrupt-driven and only poll when data is available.

*FI_OPX_PKEY*
: Integer. Partition key, a 2 byte positive integer. Default is the Pkey in the index 0 of the
  Pkey table of the unit and port on which context is created.

*FI_OPX_SL*
: Integer. Service Level. This will also determine Service Class and Virtual Lane.  Default is 0

*FI_OPX_GPU_IPC_INTRANODE*
: Boolean (0/1, on/off, true/false, yes/no). This setting controls whether IPC will be used
  to facilitate GPU to GPU intranode copies over PCIe, NVLINK, or xGMI. When this is turned off,
  GPU data will be copied to the host before being copied to another GPU which is slower than using IPC.
  This only has an effect with HMEM enabled builds of OPX.
  Defaults to on.

*FI_OPX_DEV_REG_SEND_THRESHOLD*
: Integer. The individual packet threshold where lengths above do not use a device
  registered copy when sending data from GPU.
  The default threshold is 4096.
  This has no meaning if Libfabric was not configured with GDRCopy or ROCR support.

*FI_OPX_DEV_REG_RECV_THRESHOLD*
: Integer. The individual packet threshold where lengths above do not use a device
  registered copy when receiving data into GPU.
  The default threshold is 8192.
  This has no meaning if Libfabric was not configured with GDRCopy or ROCR support.

*FI_OPX_MIXED_NETWORK*
: Boolean (1/0, on/off, true/false, yes/no). Indicates that the network requires OPA100
  support. Set to 0 if OPA100 support is not needed. Default is 1.

*FI_OPX_ROUTE_CONTROL*
: Integer. Specify the route control for each packet type. The format is
  - `<inject packet type value>:<eager packet type value>:<multi-packet eager packet type value>:<dput packet type value>:<rendezvous control packet value>:<rendezvous data packet value>`.

  Each value can range from 0-7. 0-3 is used for in-order and
  4-7 is used for out-of-order. If Token ID (TID) is enabled
  the out-of-order route controls are disabled.

  Default is `0:0:0:0:0:0 ` on OPA100 and  `4:4:4:4:0:4 ` on CN5000.

*FI_OPX_SHM_ENABLE*
: Boolean (1/0, on/off, true/false, yes/no). Enables shm across all ports and hfi units
  on the node. Setting it to NO disables shm except peers with same lid and same
  hfi1 (loopback).  Defaults to: "YES"

*FI_OPX_LINK_DOWN_WAIT_TIME_MAX_SEC*
: Integer. The maximum time in seconds to wait for a link to come back up. Default is 70 seconds.

*FI_OPX_MMAP_GUARD*
: Boolean (0/1, on/off, true/false, yes/no). Enable guards around OPX/HFI mmaps. When enabled,
this will cause a segfault when mmapped memory is illegally accessed through buffer overruns
or underruns.  Default is false.

*FI_OPX_CONTEXT_SHARING*
: Boolean (1/0, on/off, true/false, yes/no). Enables context sharing in OPX. Defaults to FALSE (1 HFI context per endpoint).

*FI_OPX_ENDPOINTS_PER_HFI_CONTEXT*
: Integer. Specify how many endpoints should share a single HFI context. Valid values are from 2 to 8.
  Default is to determine optimal value based on the number of contexts available on the system and number of processors online.
  Only applicable if context sharing is enabled. Otherwise this value is ignored.

# SEE ALSO

[`fabric`(7)](fabric.7.html),
[`fi_provider`(7)](fi_provider.7.html),
[`fi_getinfo`(7)](fi_getinfo.7.html),
