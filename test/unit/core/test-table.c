/* -*- c-basic-offset: 2; coding: utf-8 -*- */
/*
  Copyright (C) 2009-2010  Kouhei Sutou <kou@clear-code.com>

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

#include <groonga.h>

#include <gcutter.h>
#include <glib/gstdio.h>

#include "../lib/grn-assertions.h"

void test_array_set_data(void);
void data_temporary_table_no_path(void);
void test_temporary_table_no_path(gpointer data);
void data_temporary_table_default_tokenizer(void);
void test_temporary_table_default_tokenizer(gpointer data);
void data_temporary_table_add(void);
void test_temporary_table_add(gpointer data);
void data_array_sort(void);
void test_array_sort(gpointer data);
void test_nonexistent_column(void);
void data_create_with_invalid_name(void);
void test_create_with_invalid_name(gpointer data);
void test_array_truncate(void);

static gchar *tmp_directory;

static grn_logger_info *logger;
static grn_ctx *context;
static grn_obj *database;

void
cut_startup(void)
{
  tmp_directory = g_build_filename(grn_test_get_tmp_dir(),
                                   "test-database",
                                   NULL);
}

void
cut_shutdown(void)
{
  g_free(tmp_directory);
}

static void
remove_tmp_directory(void)
{
  cut_remove_path(tmp_directory, NULL);
}

void
cut_setup(void)
{
  remove_tmp_directory();
  g_mkdir_with_parents(tmp_directory, 0700);

  context = NULL;
  logger = setup_grn_logger();

  context = g_new0(grn_ctx, 1);
  grn_ctx_init(context, 0);

  database = grn_db_create(context,
                           cut_build_path(tmp_directory, "table.db", NULL),
                           NULL);
}

void
cut_teardown(void)
{
  if (context) {
    grn_ctx_fin(context);
    g_free(context);
  }
  teardown_grn_logger(logger);
  cut_remove_path(tmp_directory, NULL);
}

void
test_array_set_data(void)
{
  grn_obj *table;
  grn_id record_id;
  gchar value[] = "sample value";
  grn_obj *record_value;
  grn_obj *retrieved_record_value;
  gchar *value_type_name = "value_type";
  grn_obj *value_type;

  value_type = grn_type_create(context,
                               value_type_name, strlen(value_type_name),
                               0, sizeof(value));
  table = grn_table_create(context, NULL, 0, NULL,
                           GRN_OBJ_TABLE_NO_KEY,
                           NULL, value_type);
  record_id = grn_table_add(context, table, NULL, 0, NULL);

  record_value = grn_obj_open(context, GRN_BULK, 0, 0);
  grn_bulk_write(context, record_value, value, sizeof(value));
  grn_test_assert(grn_obj_set_value(context, table, record_id,
                                    record_value, GRN_OBJ_SET));

  retrieved_record_value = grn_obj_get_value(context, table, record_id, NULL);
  cut_assert_equal_string(value, GRN_BULK_HEAD(retrieved_record_value));
}

void
data_temporary_table_no_path(void)
{
#define ADD_DATA(label, flags)                                          \
  cut_add_data(label, GINT_TO_POINTER(flags), NULL, NULL)

  ADD_DATA("no-key", GRN_OBJ_TABLE_NO_KEY);
  ADD_DATA("hash", GRN_OBJ_TABLE_HASH_KEY);
  ADD_DATA("patricia trie", GRN_OBJ_TABLE_PAT_KEY);

#undef ADD_DATA
}

void
test_temporary_table_no_path(gpointer data)
{
  grn_obj *table;
  grn_obj_flags flags = GPOINTER_TO_INT(data);

  table = grn_table_create(context, NULL, 0, NULL,
                           flags,
                           NULL, NULL);
  cut_assert_equal_string(NULL, grn_obj_path(context, table));
}

void
data_temporary_table_default_tokenizer(void)
{
#define ADD_DATA(label, flags)                                          \
  cut_add_data(label, GINT_TO_POINTER(flags), NULL, NULL)

  ADD_DATA("hash", GRN_OBJ_TABLE_HASH_KEY);
  ADD_DATA("patricia trie", GRN_OBJ_TABLE_PAT_KEY);

#undef ADD_DATA
}

void
test_temporary_table_default_tokenizer(gpointer data)
{
  grn_obj *table;
  grn_obj_flags flags = GPOINTER_TO_INT(data);
  grn_obj *tokenizer = NULL;
  char name[1024];
  int name_size;

  table = grn_table_create(context, NULL, 0, NULL,
                           flags,
                           NULL, NULL);
  grn_obj_set_info(context, table, GRN_INFO_DEFAULT_TOKENIZER,
                   get_object("TokenTrigram"));
  tokenizer = grn_obj_get_info(context, table, GRN_INFO_DEFAULT_TOKENIZER, NULL);
  name_size = grn_obj_name(context, tokenizer, name, sizeof(name));
  name[name_size] = '\0';
  cut_assert_equal_string("TokenTrigram", name);
}

void
data_temporary_table_add(void)
{
#define ADD_DATA(label, flags)                                          \
  cut_add_data(label, GINT_TO_POINTER(flags), NULL, NULL)

  ADD_DATA("no-key", GRN_OBJ_TABLE_NO_KEY);
  ADD_DATA("hash", GRN_OBJ_TABLE_HASH_KEY);
  ADD_DATA("patricia trie", GRN_OBJ_TABLE_PAT_KEY);

#undef ADD_DATA
}

void
test_temporary_table_add(gpointer data)
{
  grn_obj *table;
  grn_obj_flags flags = GPOINTER_TO_INT(data);
  gchar key[] = "key";

  if ((flags & GRN_OBJ_TABLE_TYPE_MASK) == GRN_OBJ_TABLE_NO_KEY) {
    table = grn_table_create(context, NULL, 0, NULL,
                             flags,
                             NULL,
                             NULL);
    grn_table_add(context, table, NULL, 0, NULL);
  } else {
    table = grn_table_create(context, NULL, 0, NULL,
                             flags,
                             get_object("ShortText"),
                             NULL);
    grn_table_add(context, table, key, strlen(key), NULL);
  }

  cut_assert_equal_int(1, grn_table_size(context, table));
}

static GList *make_glist_from_array(const gint *value, guint length)
{
  GList *list = NULL;
  guint i;
  for (i = 0; i < length; ++i)
    list = g_list_prepend(list, GINT_TO_POINTER(value[i]));

  return g_list_reverse(list);
}

void
data_array_sort(void)
{
#define ADD_DATUM(label, offset, limit, expected_values, n_expected_values)  \
  gcut_add_datum(label,                                                      \
                 "offset", G_TYPE_INT, offset,                               \
                 "limit", G_TYPE_INT, limit,                                 \
                 "expected_values", G_TYPE_POINTER,                          \
                   make_glist_from_array(expected_values, n_expected_values),\
                   g_list_free,                                              \
                 NULL)

  const gint32 sorted_values[] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19
  };

  ADD_DATUM("no offset, no limit", 0, -1, &sorted_values[0], 20);
  ADD_DATUM("offset", 10, -1, &sorted_values[10], 10);
  ADD_DATUM("limit", 0, 10, &sorted_values[0], 10);
  ADD_DATUM("offset, limit", 5, 10, &sorted_values[5], 10);

#undef ADD_DATUM
}

void
test_array_sort(gpointer data)
{
  const gint32 values[] = {
    5, 6, 18, 9, 0, 4, 13, 12, 8, 14, 19, 11, 7, 3, 1, 10, 15, 2, 17, 16
  };
  const int n_values = sizeof(values) / sizeof(values[0]);
  const gchar table_name[] = "Store";
  const gchar column_name[] = "sample_column";
  const int n_keys = 1;
  grn_table_sort_key keys[n_keys];

  grn_obj *table, *column, *result;
  grn_table_cursor *cursor;
  int n_results;
  guint i;

  guint n_expected_values;
  GList *expected_values, *sorted_values = NULL;

  table = grn_table_create(context, table_name, strlen(table_name),
                           NULL,
                           GRN_OBJ_TABLE_NO_KEY | GRN_OBJ_PERSISTENT,
                           NULL,
                           NULL);
  column = grn_column_create(context,
                             table,
                             column_name,
                             strlen(column_name),
                             NULL, 0,
                             get_object("Int32"));

  keys[0].key = column;
  keys[0].flags = GRN_TABLE_SORT_ASC;

  for(i = 0; i < n_values; ++i) {
    grn_obj record_value;
    grn_id record_id;
    record_id = grn_table_add(context, table, NULL, 0, NULL);

    GRN_INT32_INIT(&record_value, 0);
    GRN_INT32_SET(context, &record_value, values[i]);
    grn_test_assert(grn_obj_set_value(context, column, record_id,
                                      &record_value, GRN_OBJ_SET));
    GRN_OBJ_FIN(context, &record_value);
  }
  cut_assert_equal_int(n_values, grn_table_size(context, table));

  result = grn_table_create(context, NULL, 0, NULL, GRN_TABLE_NO_KEY,
                            NULL, table);
  n_results = grn_table_sort(context, table,
                             gcut_data_get_int(data, "offset"),
                             gcut_data_get_int(data, "limit"),
                             result, keys, n_keys);
  expected_values = (GList *)gcut_data_get_pointer(data, "expected_values");
  n_expected_values = g_list_length(expected_values);
  cut_assert_equal_int(n_expected_values, n_results);
  cut_assert_equal_int(n_expected_values, grn_table_size(context, result));

  cursor = grn_table_cursor_open(context, result, NULL, 0, NULL, 0,
                                 0, -1, GRN_CURSOR_ASCENDING);
  while (grn_table_cursor_next(context, cursor) != GRN_ID_NIL) {
    void *value;
    grn_id *id;
    grn_obj record_value;

    grn_table_cursor_get_value(context, cursor, &value);
    id = value;

    GRN_INT32_INIT(&record_value, 0);
    grn_obj_get_value(context, column, *id, &record_value);
    sorted_values = g_list_append(sorted_values,
                                  GINT_TO_POINTER(GRN_INT32_VALUE(&record_value)));
    GRN_OBJ_FIN(context, &record_value);
  }
  gcut_take_list(sorted_values, NULL);
  gcut_assert_equal_list_int(expected_values, sorted_values);

  grn_table_cursor_close(context, cursor);
  grn_obj_close(context, result);
}

void
test_nonexistent_column(void)
{
  grn_obj *table;
  char table_name[] = "users";
  char nonexistent_column_name[] = "nonexistent";

  table = grn_table_create(context, table_name, strlen(table_name),
                           NULL,
                           GRN_OBJ_TABLE_NO_KEY,
                           NULL, NULL);
  grn_test_assert_null(context,
                       grn_obj_column(context, table,
                                      nonexistent_column_name,
                                      strlen(nonexistent_column_name)));
}

void
data_create_with_invalid_name(void)
{
#define ADD_DATUM(label, name)                  \
  gcut_add_datum(label,                         \
                 "name", G_TYPE_STRING, name,   \
                 NULL)

  ADD_DATUM("start with '_'", "_users");
  ADD_DATUM("start with number", "0table");
  ADD_DATUM("include '-'", "table-name");
  ADD_DATUM("include a space", "table name");
  ADD_DATUM("space", " ");

#define ADD_DATUM_INVALID_CHARACTER(name)               \
  ADD_DATUM(name, name)

  ADD_DATUM_INVALID_CHARACTER("!");
  ADD_DATUM_INVALID_CHARACTER("\"");
  ADD_DATUM_INVALID_CHARACTER("#");
  ADD_DATUM_INVALID_CHARACTER("$");
  ADD_DATUM_INVALID_CHARACTER("%");
  ADD_DATUM_INVALID_CHARACTER("&");
  ADD_DATUM_INVALID_CHARACTER("'");
  ADD_DATUM_INVALID_CHARACTER("(");
  ADD_DATUM_INVALID_CHARACTER(")");
  ADD_DATUM_INVALID_CHARACTER("*");
  ADD_DATUM_INVALID_CHARACTER("+");
  ADD_DATUM_INVALID_CHARACTER(",");
  ADD_DATUM_INVALID_CHARACTER("-");
  ADD_DATUM_INVALID_CHARACTER(".");
  ADD_DATUM_INVALID_CHARACTER("/");
  ADD_DATUM_INVALID_CHARACTER(":");
  ADD_DATUM_INVALID_CHARACTER(";");
  ADD_DATUM_INVALID_CHARACTER("<");
  ADD_DATUM_INVALID_CHARACTER("=");
  ADD_DATUM_INVALID_CHARACTER(">");
  ADD_DATUM_INVALID_CHARACTER("?");
  ADD_DATUM_INVALID_CHARACTER("@");
  ADD_DATUM_INVALID_CHARACTER("[");
  ADD_DATUM_INVALID_CHARACTER("\\");
  ADD_DATUM_INVALID_CHARACTER("]");
  ADD_DATUM_INVALID_CHARACTER("^");
  ADD_DATUM_INVALID_CHARACTER("_");
  ADD_DATUM_INVALID_CHARACTER("`");
  ADD_DATUM_INVALID_CHARACTER("{");
  ADD_DATUM_INVALID_CHARACTER("|");
  ADD_DATUM_INVALID_CHARACTER("}");
  ADD_DATUM_INVALID_CHARACTER("~");

#undef ADD_DATUM_INVALID_CHARACTER

#undef ADD_DATUM
}

void
test_create_with_invalid_name(gpointer data)
{
  grn_obj *table;
  const gchar *table_name;

  table_name = gcut_data_get_string(data, "name");
  table = grn_table_create(context, table_name, strlen(table_name),
                           NULL,
                           GRN_OBJ_TABLE_NO_KEY,
                           NULL, NULL);
  grn_test_assert_error(GRN_INVALID_ARGUMENT,
                        "name can't start with '_' and "
                        "0-9, and contains only 0-9, A-Z, a-z, or _",
                        context);
}

void
test_array_truncate(void)
{
  grn_obj *table;
  gchar value[] = "sample value";
  gchar *value_type_name = "value_type";
  grn_obj *value_type;

  cut_omit("grn_table_truncate() is not implemented.");

  value_type = grn_type_create(context,
                               value_type_name, strlen(value_type_name),
                               0, sizeof(value));
  table = grn_table_create(context, NULL, 0, NULL,
                           GRN_OBJ_TABLE_NO_KEY,
                           NULL, value_type);
  grn_test_assert_not_nil(grn_table_add(context, table, NULL, 0, NULL));

  cut_assert_equal_uint(1, grn_table_size(context, table));
  grn_test_assert(grn_table_truncate(context, table));
  cut_assert_equal_uint(0, grn_table_size(context, table));
}
