/* -*- C -*- */
/*
 * COPYRIGHT 2012 XYRATEX TECHNOLOGY LIMITED
 *
 * THIS DRAWING/DOCUMENT, ITS SPECIFICATIONS, AND THE DATA CONTAINED
 * HEREIN, ARE THE EXCLUSIVE PROPERTY OF XYRATEX TECHNOLOGY
 * LIMITED, ISSUED IN STRICT CONFIDENCE AND SHALL NOT, WITHOUT
 * THE PRIOR WRITTEN PERMISSION OF XYRATEX TECHNOLOGY LIMITED,
 * BE REPRODUCED, COPIED, OR DISCLOSED TO A THIRD PARTY, OR
 * USED FOR ANY PURPOSE WHATSOEVER, OR STORED IN A RETRIEVAL SYSTEM
 * EXCEPT AS ALLOWED BY THE TERMS OF XYRATEX LICENSES AND AGREEMENTS.
 *
 * YOU SHOULD HAVE RECEIVED A COPY OF XYRATEX'S LICENSE ALONG WITH
 * THIS RELEASE. IF NOT PLEASE CONTACT A XYRATEX REPRESENTATIVE
 * http://www.xyratex.com/contact
 *
 * Original author: Rajesh Bhalerao <Rajesh_Bhalerao@xyratex.com>,
 *                  Nikita Danilov <Nikita_Danilov@xyratex.com>
 * Original creation date: 02/18/2011
 */

#pragma once

#ifndef __MERO_LIB_PROCESSOR_H__
#define __MERO_LIB_PROCESSOR_H__

#include "lib/bitmap.h"
#include "lib/types.h"

/**
   @defgroup processor Processor Information

   Interfaces to learn the number and characteristics of "processors"
   for a given system/node.

   @section Definitions

   @subsection Processor
   It's a logical processor. This may mean HT or core inside a physical
   CPU package. It depends on a given OS kernel. HT is the finest
   granularity processing unit at hardware level.

   @subsection Possible-Processors
   Maximum number of processors that can be attached to this OS.

   @subsection Available-Processors
   The number of processors that are currently configured or available under
   this OS. Please note that all the processors may not be enabled or used
   by OS.

   @subsection Online-Processors
   The number of processors that are currently enabled/under use/online under
   this OS.

   @{
 */

#define M0_PROCESSORS_INVALID_ID	((uint32_t)-1)

/** A processor number/identifier. */
typedef uint32_t m0_processor_nr_t;

/**
   Initialize processors interface. This will allow the interface
   to cache/populate the data, if necessary. The data is cached for
   user mode. The data may not be cached for kernel mode as kernel already
   has the data.

   The calling function should not assume hot-plug CPU facility.
   If the underlying OS supports the hot-plug CPU facility, the calling
   program will have to re-initalize the interface (at least in user-mode)
   after registering for platform specific CPU change notification.

   To re-initalize the interface, m0_processors_fini() must be called first,
   before initializing it again.

   @post Interface initialized.

   Concurrency: The interface should not be initialized twice or simultaneously.
                It's not MT-safe and can be called only once. It can be
                called again after calling m0_processors_fini().
 */
M0_INTERNAL int m0_processors_init(void);

/**
   Close the processors interface. This function will destroy any cached data.
   After calling this interface no meaningful data should be assumed.

   Concurrency: Not MT-safe. Assumes no threads are using processor interface.
 */
M0_INTERNAL void m0_processors_fini(void);

/**
   Maximum processors this system can handle.
 */
M0_INTERNAL m0_processor_nr_t m0_processor_nr_max(void);

/**
   Return the bitmap of possible processors.

   @pre map->b_nr >= m0_processor_nr_max()
   @pre m0_processors_init() must be called before calling this function.
   @pre The calling function must allocate memory for 'map' and initialize it.
   @note This function does not take any locks.
 */
M0_INTERNAL void m0_processors_possible(struct m0_bitmap *map);

/**
   Return the bitmap of available processors.

   @pre map->b_nr >= m0_processor_nr_max()
   @pre m0_processors_init() must be called before calling this function.
   @pre The calling function must allocate memory for 'map' and initialize it.
   @note This function does not take any locks.
 */
M0_INTERNAL void m0_processors_available(struct m0_bitmap *map);

/**
   Return the bitmap of online processors.

   @pre map->b_nr >= m0_processor_nr_max()
   @pre m0_processors_init() must be called before calling this function.
   @pre The calling function must allocate memory for 'map' and initialize it.
   @note This function does not take any locks.
 */
M0_INTERNAL void m0_processors_online(struct m0_bitmap *map);

/**
   Return the id of the processor on which the calling thread is running.

   @return logical processor id (as supplied by the system) on which the
           calling thread is running, if the call is unsupported.
           It will return M0_PROCESSORS_INVALID_ID, if this call is not
           supported.
 */
M0_INTERNAL m0_processor_nr_t m0_processor_id_get(void);

/**
   Description of a processor in the system.
   Please note that L1 and L2 cache ids may have to bee generated by the
   program.

   Example : Id generation on Linux (user-mode)
   @verbatim
   +---------------+-----------------------------------------------------------+
   | Name          |    Identifier Description                                 |
   +---------------+-----------------------------------------------------------+
   | pd_numa_node  | NumaNode Id as supplied by the system                     |
   +---------------+-----------------------------------------------------------+
   | pd_id         | Logical processor id supplied by the system               |
   +---------------+-----------------------------------------------------------+
   | pd_l1         | 1. If L1 is not shared with any other processor, then     |
   |               |    it's same as pd_id.                                    |
   |               | 2. If L1 is shared,                                       |
   |               |    Physical Package Id (16-31) |  Core Id (0-15)          |
   +---------------+-----------------------------------------------------------+
   | pd_l2         | 1. If L2 is not shared with any other processor, then     |
   |               |    it's same as pd_id.                                    |
   |               | 3. If L2 is shared and L3 is present, its                 |
   |               |    Physical Package Id (16-31) | Core Id (0-15)           |
   |               | 3. If L2 is shared and L3 is not present, its             |
   |               |    Physical Package Id                                    |
   +---------------+-----------------------------------------------------------+
   | pd_pipeline   | Same as pd_id                                             |
   +---------------+-----------------------------------------------------------+
   @endverbatim
 */
struct m0_processor_descr {
	/** Processor identifier. */
	m0_processor_nr_t pd_id;
	/** All processors in the same numa node share this */
	uint32_t          pd_numa_node;
	/** Id for L1 cache. If multiple processors share L1 cache, all of them
	    will have same L1 cache id */
	uint32_t          pd_l1;
	/** Id for L2 cache. If multiple processors share L2 cache, all of them
	    will have same L2 cache id */
	uint32_t          pd_l2;
	/** L1 cache size (in bytes) for this processor */
	size_t            pd_l1_sz;
	/** L2 cache size (in bytes) for this processor */
	size_t            pd_l2_sz;
	/** All processors sharing the same pipeline have the same value of
	    this. */
	uint32_t          pd_pipeline;
};

/**
   Obtain information on the processor with a given id.
   @param id -> id of the processor for which information is requested.
   @param pd -> processor descripto structure. Memory for this should be
                allocated by the calling function. Interface does not allocate
                memory.

   @retval 0 if a matching processor is found
   @retval -EINVAL if id does not match with any of the processors or NULL
                   memory pointer for 'pd' is passed.

   @pre  Memory must be allocated for pd. Interface donot allocated memory.
   @pre m0_processors_init() must be called before calling this function.
   @post d->pd_id == id or none

   Concurrency: This is read only data. Interface by itself does not do
                any locking. When used in kernel-mode, the interface may
                call some functions that may use some kind of locks.
 */
M0_INTERNAL int m0_processor_describe(m0_processor_nr_t id,
				      struct m0_processor_descr *pd);

/** @} end of processor group */

/* __MERO_LIB_PROCESSOR_H__ */
#endif

/*
 *  Local variables:
 *  c-indentation-style: "K&R"
 *  c-basic-offset: 8
 *  tab-width: 8
 *  fill-column: 80
 *  scroll-step: 1
 *  End:
 */
