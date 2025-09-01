#ifndef SEQLOCK_H
#define SEQLOCK_H

/*
 * Lightweight single‑writer / multiple‑reader sequence lock for RP2040
 * --------------------------------------------------------------------
 * ‑ The writer increments the sequence counter to an **odd** value, writes the
 *   payload, then increments again to make it **even**.
 * ‑ Readers copy the payload only if the counter is *even* before & after.
 *   If it changed (or was odd) they retry.
 *
 * This avoids disabling IRQs in the writer, so Bluetooth HCI interrupts stay
 * responsive while still giving readers an internally‑consistent snapshot.
 *
 * Header‑only; include this in a shared header for both cores.
 * C11 atomics are used, compatible with C++ as well.
 */

#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>

/* ------------------------------------------------------------------ */
/*  Public macros                                                      */
/* ------------------------------------------------------------------ */

/* Declare a seqlock‑protected structure type.
 *   SEQLOCK_DECL(name, payload_type);
 * This expands to a struct named <name> with fields:
 *   _Atomic uint32_t seq;   // internal sequence counter
 *   payload_type data;      // user payload
 */
#define SEQLOCK_DECL(name, type) \
  typedef struct {               \
    _Atomic uint32_t seq;        \
    type data;                   \
  } name

/* Begin a write section: sequence++ (odd) */
static inline void seqlock_write_begin(_Atomic uint32_t* seq) {
  atomic_fetch_add_explicit(seq, 1u, memory_order_relaxed);
}

/* End a write section: sequence++ (even, release) */
static inline void seqlock_write_end(_Atomic uint32_t* seq) {
  atomic_fetch_add_explicit(seq, 1u, memory_order_release);
}

/* Try to read a consistent snapshot.
 *   dest_ptr  – pointer to copy destination (payload_type *)
 *   src_obj   – seqlock instance (not pointer!)
 * Returns true if *dest_ptr now contains a consistent snapshot.
 * Usage example:
 *   uni_gamepad_t gp;
 *   if (SEQLOCK_TRY_READ(&gp, g_ctrl)) {
 *       // gp is valid
 *   }
 */
#define SEQLOCK_TRY_READ(dest_ptr, src_obj)                              \
  ({                                                                     \
    bool __ok;                                                           \
    uint32_t __v1, __v2;                                                 \
    do {                                                                 \
      __v1 = atomic_load_explicit(&(src_obj).seq, memory_order_acquire); \
      if (__v1 & 1u) {                                                   \
        __ok = false;                                                    \
        break;                                                           \
      }                                                                  \
      *(dest_ptr) = (src_obj).data;                                      \
      __asm volatile("dmb sy" ::: "memory");                             \
      __v2 = atomic_load_explicit(&(src_obj).seq, memory_order_relaxed); \
    } while (__v1 != __v2);                                              \
    __ok = (__v1 == __v2);                                               \
    __ok;                                                                \
  })

/* ------------------------------------------------------------------ */
/*  Example                                                           */
/* ------------------------------------------------------------------ */
/*
#include "seqlock.h"
#include "uni_gamepad.h"

SEQLOCK_DECL(shared_ctrl_t, uni_gamepad_t);

// Shared instance (place in .bss and ensure 32‑byte alignment for RP2040 cores)
shared_ctrl_t g_ctrl __attribute__((aligned(32)));

// Writer (Bluetooth core)
void bt_new_packet(const uni_gamepad_t *src)
{
    seqlock_write_begin(&g_ctrl.seq);
    g_ctrl.data = *src;                    // struct copy
    seqlock_write_end(&g_ctrl.seq);
}

// Reader (USB core)
bool usb_get_snapshot(uni_gamepad_t *dst)
{
    return SEQLOCK_TRY_READ(dst, g_ctrl);
}
*/

#endif /* SEQLOCK_H */
