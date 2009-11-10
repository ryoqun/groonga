/* -*- c-basic-offset: 2; coding: utf-8 -*- */
/*
  Copyright (C) 2009  Yuto HAYAMIZU <y.hayamizu@gmail.com>

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

#include <soupcutter.h>

#include "../lib/grn-assertions.h"
#include "../lib/grn-test-server.h"

static GrnTestServer *server;

static SoupCutClient *client;

static grn_ctx context;
static grn_obj *database;

void
cut_setup(void)
{
  GError *error = NULL;
  const gchar *db_path;

  server = grn_test_server_new();

  client = soupcut_client_new();
  grn_ctx_init(&context, 0);
  database = NULL;

  grn_test_server_start(server, &error);
  gcut_assert_error(error);

  soupcut_client_set_base(client, grn_test_server_get_http_uri_base(server));
  db_path = grn_test_server_get_database_path(server, &error);
  gcut_assert_error(error);
  database = grn_db_open(&context, db_path);
}

void
cut_teardown(void)
{
  if (client) {
    g_object_unref(client);
  }

  if (database) {
    grn_obj_unlink(&context, database);
  }

  grn_ctx_fin(&context);

  if (server) {
    g_object_unref(server);
  }
}

void
test_get_root(void)
{
  soupcut_client_get(client, "/", NULL);

  soupcut_client_assert_response(client);
  soupcut_client_assert_equal_content_type("application/json", client);
  soupcut_client_assert_equal_body("", client);
}

void
test_get_status(void)
{
  soupcut_client_get(client, "/d/status", NULL);

  soupcut_client_assert_response(client);
  soupcut_client_assert_equal_content_type("application/json", client);
  soupcut_client_assert_match_body("{"
                                   "\"alloc_count\":\\d+,"
                                   "\"starttime\":\\d+,"
                                   "\"uptime\":\\d+"
                                   "}",
                                   client);
}

void
test_get_table_list(void)
{
  grn_obj *users;
  grn_obj_flags flags;
  const gchar *table_name = "users";

  soupcut_client_get(client, "/d/table_list", NULL);

  soupcut_client_assert_response(client);
  soupcut_client_assert_equal_content_type("application/json", client);
  soupcut_client_assert_equal_body(
    "[[\"id\",\"name\",\"path\",\"flags\",\"domain\"]]",
    client);
  flags = GRN_OBJ_PERSISTENT | GRN_OBJ_TABLE_PAT_KEY;
  soupcut_client_get(client,
                     "/d/table_create",
                     "name", table_name,
                     "flags", cut_take_printf("%u", flags),
                     "key_type", "Int8",
                     "value_type", "Object",
                     "default_tokenizer", "",
                     NULL);
  soupcut_client_assert_response(client);
  soupcut_client_assert_equal_body("true", client);

  users = grn_ctx_get(&context, table_name, strlen(table_name));
  grn_test_assert_not_null(&context, users);

  soupcut_client_get(client, "/d/table_list", NULL);
  soupcut_client_assert_equal_content_type("application/json", client);
  soupcut_client_assert_equal_body(
    cut_take_printf("["
                    "[\"id\",\"name\",\"path\",\"flags\",\"domain\"],"
                    "[%u,\"%s\",\"%s\",%u,%u]"
                    "]",
                    grn_obj_id(&context, users),
                    table_name,
                    grn_obj_path(&context, users),
                    flags | GRN_OBJ_KEY_INT,
                    GRN_DB_INT8),
    client);
}

void
test_get_column_list(void)
{
  const gchar *table_name = "users";
  const gchar *column_name = "age";
  const gchar *full_column_name;
  grn_obj *users;
  grn_obj *age;

  full_column_name = cut_take_printf("%s.%s", table_name, column_name);
  users = grn_table_create(&context, table_name, strlen(table_name),
                           NULL, GRN_OBJ_PERSISTENT | GRN_OBJ_TABLE_PAT_KEY,
                           grn_ctx_at(&context, GRN_DB_INT8),
                           grn_ctx_at(&context, GRN_DB_OBJECT));
  grn_test_assert_not_null(&context, users);

  soupcut_client_get(client, "/d/column_list", "table", table_name, NULL);
  soupcut_client_assert_response(client);
  soupcut_client_assert_equal_content_type("application/json", client);
  soupcut_client_assert_equal_body(
    "[[\"id\",\"name\",\"path\",\"type\",\"flags\",\"domain\"]]",
    client);

  soupcut_client_get(client,
                     "/d/column_create",
                     "table", table_name,
                     "name", column_name,
                     "flags", cut_take_printf("%u", GRN_OBJ_COLUMN_SCALAR),
                     "type", "Int8",
                     NULL);
  soupcut_client_assert_response(client);
  soupcut_client_assert_equal_content_type("application/json", client);
  soupcut_client_assert_equal_body("true", client);

  age = grn_ctx_get(&context, full_column_name, strlen(full_column_name));
  grn_test_assert_not_null(&context, age);

  soupcut_client_get(client, "/d/column_list", "table", table_name, NULL);
  soupcut_client_assert_response(client);
  soupcut_client_assert_equal_content_type("application/json", client);
  soupcut_client_assert_equal_body(
    cut_take_printf("["
                    "[\"id\",\"name\",\"path\",\"type\",\"flags\",\"domain\"],"
                    "[%u,\"%s\",\"%s\",\"fix\",%u,%u]"
                    "]",
                    grn_obj_id(&context, age),
                    column_name,
                    grn_obj_path(&context, age),
                    GRN_OBJ_COLUMN_SCALAR | GRN_OBJ_PERSISTENT | GRN_OBJ_KEY_INT,
                    grn_obj_id(&context, users)),
    client);
}

void
test_select(void)
{
  const gchar *table_name = "users";
  const gchar *column_name = "age";
  const gchar *hayamizu_name = "Hayamizu";
  gint hayamizu_age = 22;
  grn_obj *users;
  grn_obj *age;
  grn_id hayamizu_id;
  grn_obj age_value;

  users = grn_table_create(&context, table_name, strlen(table_name),
                           NULL, GRN_OBJ_PERSISTENT | GRN_OBJ_TABLE_PAT_KEY,
                           grn_ctx_at(&context, GRN_DB_SHORT_TEXT),
                           NULL);
  grn_test_assert_not_null(&context, users);
  age = grn_column_create(&context, users, column_name, strlen(column_name),
                          NULL, GRN_OBJ_PERSISTENT | GRN_OBJ_COLUMN_SCALAR,
                          grn_ctx_at(&context, GRN_DB_INT32));
  grn_test_assert_not_null(&context, age);

  hayamizu_id = grn_table_add(&context, users,
                              hayamizu_name, strlen(hayamizu_name),
                              NULL);
  grn_test_assert_not_nil(hayamizu_id);
  cut_assert_equal_uint(1, grn_table_size(&context, users));

  GRN_INT32_INIT(&age_value, 0);
  GRN_INT32_SET(&context, &age_value, 22);
  grn_obj_set_value(&context, age, hayamizu_id, &age_value, GRN_OBJ_SET);
  grn_obj_unlink(&context, &age_value);

  soupcut_client_get(client,
                     "/d/select",
                     "table", table_name,
                     "query", cut_take_printf("%s:%d",
                                              column_name, hayamizu_age),
                     NULL);
  soupcut_client_assert_response(client);
  soupcut_client_assert_equal_content_type("application/json", client);
  soupcut_client_assert_equal_body(
    cut_take_printf("[[%d],"
                    "[[1],"
                    "[\"_id\",\"_key\",\"%s\"],"
                    "[%u,\"%s\",%d]"
                    "]]",
                    GRN_SUCCESS,
                    column_name, hayamizu_id, hayamizu_name, hayamizu_age),
    client);
}
