/**************************************************************************/
/*!
    @file     fifo.h
    @author   hathach (tinyusb.org)

    @section LICENSE

    Software License Agreement (BSD License)

    Copyright (c) 2018, hathach (tinyusb.org)
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    1. Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    3. Neither the name of the copyright holders nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    INCLUDING NEGLIGENCE OR OTHERWISE ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	  This file is part of the tinyusb stack.
*/
/**************************************************************************/

/** \ingroup Group_Common
 * \defgroup group_fifo fifo
 *  @{ */

#ifndef _TUSB_FIFO_H_
#define _TUSB_FIFO_H_

#define CFG_FIFO_MUTEX      0

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#ifdef __cplusplus
 extern "C" {
#endif

#if CFG_FIFO_MUTEX

#include "osal/osal.h"

#if CFG_TUSB_OS == OPT_OS_NONE
// Since all fifo read/write is done in thread mode, there should be
// no conflict except for osal queue which will be address seperatedly.
// Therefore there may be no need for mutex with internal use of fifo

#define _ff_mutex_def(mutex)

#else
#define fifo_mutex_t          struct os_mutex

#define fifo_mutex_lock(m)    os_mutex_pend(m, OS_TIMEOUT_NEVER)
#define fifo_mutex_unlock(m)  os_mutex_release(m)

/* Internal use only */
#define _mutex_declare(m)     .mutex = m

#endif

#else

#define _mutex_declare(m)

#endif


/** \struct fifo_t
 * \brief Simple Circular FIFO
 */
typedef struct
{
           uint8_t* buffer    ; ///< buffer pointer
           uint16_t depth     ; ///< max items
           uint16_t item_size ; ///< size of each item

  volatile uint16_t count     ; ///< number of items in queue
  volatile uint16_t wr_idx    ; ///< write pointer
  volatile uint16_t rd_idx    ; ///< read pointer

           bool overwritable  ;

#if CFG_FIFO_MUTEX
  fifo_mutex_t * const mutex;
#endif

} fifo_t;

#define FIFO_DEF(name, ff_depth, type, is_overwritable) /*, irq_mutex)*/ \
  uint8_t name##_buffer[ff_depth*sizeof(type)];\
  fifo_t name = {\
      .buffer       = name##_buffer,\
      .depth        = ff_depth,\
      .item_size    = sizeof(type),\
      .overwritable = is_overwritable,\
      /*.irq          = irq_mutex*/\
      _mutex_declare(_mutex)\
  }

void fifo_clear(fifo_t *f);
void fifo_config(fifo_t *f, void* buffer, uint16_t depth, uint16_t item_size, bool overwritable);

bool     fifo_write   (fifo_t* f, void const * p_data);
uint16_t fifo_write_n (fifo_t* f, void const * p_data, uint16_t count);

bool     fifo_read    (fifo_t* f, void * p_buffer);
uint16_t fifo_read_n  (fifo_t* f, void * p_buffer, uint16_t count);

bool     fifo_peek_at (fifo_t* f, uint16_t position, void * p_buffer);

static inline bool fifo_peek(fifo_t* f, void * p_buffer)
{
  return fifo_peek_at(f, 0, p_buffer);
}

static inline bool fifo_empty(fifo_t* f)
{
  return (f->count == 0);
}

static inline bool fifo_full(fifo_t* f)
{
  return (f->count == f->depth);
}

static inline uint16_t fifo_count(fifo_t* f)
{
  return f->count;
}

static inline uint16_t fifo_remaining(fifo_t* f)
{
  return f->depth - f->count;
}

static inline uint16_t fifo_depth(fifo_t* f)
{
  return f->depth;
}

#ifdef __cplusplus
 }
#endif

#endif /* _TUSB_FIFO_H_ */
