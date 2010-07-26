/* -*- c-basic-offset: 2 -*- */
/* Copyright(C) 2009 Brazil

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License version 2.1 as published by the Free Software Foundation.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#ifndef GRN_PAT_H
#define GRN_PAT_H

#ifndef GROONGA_IN_H
#include "groonga_in.h"
#endif /* GROONGA_IN_H */

#include "db.h"
#include "hash.h"

#ifdef __cplusplus
extern "C" {
#endif

#define GRN_PAT_MAX_KEY_SIZE                    GRN_TABLE_MAX_KEY_SIZE

struct _grn_pat {
  grn_db_obj obj;
  grn_io *io;
  struct grn_pat_header *header;
  grn_encoding encoding;
  uint32_t key_size;
  uint32_t value_size;
  grn_obj *tokenizer;
};

#define GRN_PAT_NDELINFOS 0x100

typedef struct {
  grn_id d;
  grn_id ld;
  uint32_t stat;
  uint32_t shared;
} grn_pat_delinfo;

struct grn_pat_header {
  uint32_t flags;
  grn_encoding encoding;
  uint32_t key_size;
  uint32_t value_size;
  grn_id tokenizer;
  uint32_t n_entries;
  uint32_t curr_rec;
  int32_t curr_key;
  int32_t curr_del;
  int32_t curr_del2;
  int32_t curr_del3;
  uint32_t n_garbages;
  uint32_t reserved[1005];
  grn_pat_delinfo delinfos[GRN_PAT_NDELINFOS];
  grn_id garbages[GRN_PAT_MAX_KEY_SIZE + 1];
};

struct _grn_pat_cursor_entry {
  grn_id id;
  uint16_t check;
};

typedef struct _grn_pat_cursor_entry grn_pat_cursor_entry;

struct _grn_pat_cursor {
  grn_db_obj obj;
  grn_id curr_rec;
  grn_pat *pat;
  grn_ctx *ctx;
  unsigned int size;
  unsigned int sp;
  grn_id tail;
  unsigned int rest;
  grn_pat_cursor_entry *ss;
  uint8_t curr_key[GRN_TABLE_MAX_KEY_SIZE];
};

grn_id grn_pat_curr_id(grn_ctx *ctx, grn_pat *pat);

/* private */
grn_rc grn_pat_truncate(grn_ctx *ctx, grn_pat *pat);
const char *_grn_pat_key(grn_ctx *ctx, grn_pat *pat, grn_id id, uint32_t *key_size);
grn_id grn_pat_next(grn_ctx *ctx, grn_pat *pat, grn_id id);
const char *grn_pat_get_value_(grn_ctx *ctx, grn_pat *pat, grn_id id, uint32_t *size);
grn_id grn_pat_at(grn_ctx *ctx, grn_pat *pat, grn_id id);
void grn_pat_check(grn_ctx *ctx, grn_pat *pat);
void grn_pat_inspect_nodes(grn_ctx *ctx, grn_pat *pat, grn_obj *buf);
void grn_pat_cursor_inspect(grn_ctx *ctx, grn_pat_cursor *c, grn_obj *buf);

#ifdef __cplusplus
}
#endif

#endif /* GRN_PAT_H */
