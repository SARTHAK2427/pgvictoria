/*
 * Copyright (C) 2026 The pgvictoria community
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list
 * of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this
 * list of conditions and the following disclaimer in the documentation and/or other
 * materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may
 * be used to endorse or promote promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <report.h>
#include <html_report.h>
#include <markdown.h>
#include <security.h>
#include <message.h>
#include <postgresql.h>
#include <logging.h>
#include <network.h>
#include <json.h>
#include <value.h>
#include <utils.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <err.h>
#include <ctype.h>

/* libxml2 */
#include <libxml/HTMLtree.h>
#include <libxml/tree.h>

static uintptr_t
pgvictoria_json_get_typed_case_insensitive(struct json* baseline, char* key, enum value_type* type, char** matched_key)
{
   uintptr_t val = pgvictoria_json_get_typed(baseline, key, type);
   if (val)
   {
      if (matched_key)
      {
         *matched_key = key;
      }
      return val;
   }

   /* Try lowercase lookup to avoid iterator scan */
   char lower_key[128];
   size_t key_len = strlen(key);
   if (key_len < sizeof(lower_key))
   {
      for (size_t i = 0; i < key_len; i++)
      {
         lower_key[i] = tolower((unsigned char)key[i]);
      }
      lower_key[key_len] = '\0';

      if (strcmp(lower_key, key) != 0)
      {
         val = pgvictoria_json_get_typed(baseline, lower_key, type);
         if (val)
         {
            if (matched_key)
            {
               *matched_key = NULL;
            }
            return val;
         }
      }
   }
   return 0;
}



static int
detect_pg_version(void)
{
   FILE* fp = popen("pg_config --version", "r");
   int ver = pgvictoria_get_max_supported_version(); /* default fallback to latest */
   if (fp)
   {
      char buf[128];
      bool success = false;
      if (fgets(buf, sizeof(buf), fp))
      {
         if (pgvictoria_starts_with(buf, "PostgreSQL "))
         {
            int parsed_ver = pgvictoria_atoi(buf + 11);
            if (pgvictoria_is_version_supported(parsed_ver))
            {
               ver = parsed_ver;
               success = true;
            }
         }
      }
      int status = pclose(fp);
      if (status != 0 || !success)
      {
         ver = pgvictoria_get_max_supported_version();
      }
   }
   return ver;
}

static void
trim_and_extract_key_value(char* line, char* key, char* value)
{
   char* p = line;
   char* start_key;
   char* start_val;
   char* end_val;
   int key_len;
   int val_len;

   /* Skip leading whitespace */
   while (*p == ' ' || *p == '\t')
   {
      p++;
   }

   /* Skip comments or empty lines */
   if (*p == '#' || *p == '\0' || *p == '\r' || *p == '\n')
   {
      return;
   }

   start_key = p;
   /* Find the end of key (delimited by space, tab, or '=') */
   while (*p && *p != ' ' && *p != '\t' && *p != '=' && *p != '\r' && *p != '\n' && *p != '#')
   {
      p++;
   }

   key_len = p - start_key;
   if (key_len == 0 || key_len >= 128)
   {
      return;
   }
   memcpy(key, start_key, key_len);
   key[key_len] = '\0';

   /* Skip whitespace to find divider or value */
   while (*p == ' ' || *p == '\t' || *p == '=')
   {
      p++;
   }

   /* If there is a comment at the end of the line, or it's empty, skip */
   if (*p == '\0' || *p == '\r' || *p == '\n' || *p == '#')
   {
      return;
   }

   /* Parse value, handling quotes */
   if (*p == '\'' || *p == '\"')
   {
      char quote_char = *p;
      start_val = p + 1;
      p++;
      while (*p && *p != quote_char && *p != '\r' && *p != '\n')
      {
         p++;
      }
      end_val = p;
   }
   else
   {
      start_val = p;
      while (*p && *p != ' ' && *p != '\t' && *p != '\r' && *p != '\n' && *p != '#')
      {
         p++;
      }
      end_val = p;
   }

   val_len = end_val - start_val;
   if (val_len <= 0 || val_len >= 1024)
   {
      return;
   }
   memcpy(value, start_val, val_len);
   value[val_len] = '\0';
}

static void
pgvictoria_report_print_diff(char* key, char* val, struct json* baseline)
{
   enum value_type type;
   char* matched_key = NULL;
   uintptr_t baseline_val_ptr = pgvictoria_json_get_typed_case_insensitive(baseline, key, &type, &matched_key);
   const char* disp_key = matched_key ? matched_key : key;
   const char* def_val = "-";
   char* default_val_str = NULL;
   bool is_default = false;

   if (baseline_val_ptr)
   {
      struct value* v = NULL;
      if (!pgvictoria_value_create(type, baseline_val_ptr, &v))
      {
         if (v)
         {
            default_val_str = pgvictoria_value_to_string(v, FORMAT_TEXT, NULL, 0);
            if (default_val_str)
            {
               def_val = default_val_str;
               if (strcmp(default_val_str, val) == 0)
               {
                  is_default = true;
               }
            }
            pgvictoria_value_destroy(v);
         }
      }
   }

   if (!is_default)
   {
      printf("%-40s | %-20s | %-20s\n", disp_key, def_val, val);
   }

   if (default_val_str)
   {
      free(default_val_str);
   }
}

int
pgvictoria_report_online(int server)
{
   struct main_configuration* config = (struct main_configuration*)shmem;
   struct server* srv;
   SSL* ssl = NULL;
   int fd = -1;
   struct message* msg = NULL;
   struct query_response* version_response = NULL;
   struct query_response* all_response = NULL;
   int version = 0;
   struct json* baseline = NULL;
   int ret = 1;

   if (server < 0 || server >= config->common.number_of_servers)
   {
      warnx("Invalid server index");
      return 1;
   }

   srv = &config->common.servers[server];

   char* password_str = "";
   for (int i = 0; i < config->common.number_of_users; i++)
   {
      if (strcmp(config->common.users[i].username, srv->username) == 0)
      {
         password_str = config->common.users[i].password;
         break;
      }
   }
   if (password_str == NULL || *password_str == '\0')
   {
      if (config->common.number_of_users > 0)
      {
         password_str = config->common.users[0].password;
      }
   }

   if (pgvictoria_server_authenticate(server, "postgres", srv->username, password_str, false, &ssl, &fd) != AUTH_SUCCESS)
   {
      warnx("Failed to authenticate to server");
      goto error;
   }

   if (pgvictoria_create_query_message("SHOW server_version_num;", &msg) != MESSAGE_STATUS_OK)
   {
      goto error;
   }

   if (pgvictoria_query_execute(ssl, fd, msg, &version_response))
   {
      goto error;
   }

   pgvictoria_free_message(msg);
   msg = NULL;

   if (version_response && version_response->tuples && version_response->tuples->data[0])
   {
      char* ver_str = version_response->tuples->data[0];
      if (pgvictoria_is_number(ver_str, 10))
      {
         version = pgvictoria_atoi(ver_str) / 10000;
      }
   }

   baseline = pgvictoria_get_baseline(version);
   if (!baseline)
   {
      warnx("No baseline available for PostgreSQL version %d", version);
      goto error;
   }

   if (pgvictoria_create_query_message("SHOW ALL;", &msg) != MESSAGE_STATUS_OK)
   {
      goto error;
   }

   if (pgvictoria_query_execute(ssl, fd, msg, &all_response))
   {
      goto error;
   }

   printf("\nPostgreSQL %d Configuration Report (Online Mode)\n", version);
   char* os_name = NULL;
   int k_major = 0, k_minor = 0, k_patch = 0;
   if (pgvictoria_os_kernel_version(&os_name, &k_major, &k_minor, &k_patch) == 0)
   {
      printf("System: %s %d.%d.%d\n", os_name, k_major, k_minor, k_patch);
      free(os_name);
   }
   printf("=================================================================================\n");
   printf("%-40s | %-20s | %-20s\n", "Key", "Default", "Current");
   printf("---------------------------------------------------------------------------------\n");

   struct tuple* curr = all_response->tuples;
   while (curr)
   {
      pgvictoria_report_print_diff(curr->data[0], curr->data[1], baseline);
      curr = curr->next;
   }
   printf("=================================================================================\n");

   ret = 0;

error:
   if (msg)
   {
      pgvictoria_free_message(msg);
      msg = NULL;
   }
   if (version_response)
   {
      pgvictoria_free_query_response(version_response);
   }
   if (all_response)
   {
      pgvictoria_free_query_response(all_response);
   }
   if (baseline)
   {
      pgvictoria_json_destroy(baseline);
      baseline = NULL;
   }
   if (ssl)
   {
      pgvictoria_close_ssl(ssl);
   }
   if (fd != -1)
   {
      pgvictoria_disconnect(fd);
   }

   return ret;
}

static int
detect_pg_version_from_file(const char* filename)
{
   FILE* file = fopen(filename, "r");
   if (!file)
   {
      return 0;
   }
   char line[256];
   int version = 0;
   while (fgets(line, sizeof(line), file))
   {
      if (line[0] == '#')
      {
         char* p = strstr(line, "PostgreSQL ");
         if (p)
         {
            int v = pgvictoria_atoi(p + 11);
            if (pgvictoria_is_version_supported(v))
            {
               version = v;
               break;
            }
         }
         p = strstr(line, "version ");
         if (p)
         {
            int v = pgvictoria_atoi(p + 8);
            if (pgvictoria_is_version_supported(v))
            {
               version = v;
               break;
            }
         }
      }
   }
   fclose(file);
   return version;
}

int
pgvictoria_report_file(char* filename, char* output_html_path, int override_version)
{
   int version = 0;
   struct json* baseline = NULL;
   FILE* file = NULL;
   char* resolved_filename = NULL;
   char* resolved_output = NULL;
   int ret = 0;

   if (filename == NULL || strlen(filename) == 0 || strlen(filename) >= MAX_PATH)
   {
      warnx("pgvictoria-cli: Invalid or excessively long configuration filename");
      return 1;
   }

   if (pgvictoria_resolve_path(filename, &resolved_filename) != 0 || resolved_filename == NULL)
   {
      resolved_filename = strdup(filename);
   }

   if (pgvictoria_is_directory(resolved_filename))
   {
      warnx("pgvictoria-cli: %s is a directory, not a file", resolved_filename);
      free(resolved_filename);
      return 1;
   }

   if (!pgvictoria_is_file(resolved_filename))
   {
      warnx("pgvictoria-cli: %s is not a regular file", resolved_filename);
      free(resolved_filename);
      return 1;
   }

   if (pgvictoria_is_binary_file(resolved_filename))
   {
      warnx("pgvictoria-cli: Configuration file %s appears to be a binary file, rejecting", resolved_filename);
      free(resolved_filename);
      return 1;
   }

   if (output_html_path != NULL)
   {
      if (strlen(output_html_path) == 0 || strlen(output_html_path) >= MAX_PATH)
      {
         warnx("pgvictoria-cli: Invalid or excessively long output path");
         free(resolved_filename);
         return 1;
      }

      if (pgvictoria_resolve_path(output_html_path, &resolved_output) != 0 || resolved_output == NULL)
      {
         resolved_output = strdup(output_html_path);
      }
   }

   if (pgvictoria_is_version_supported(override_version))
   {
      version = override_version;
   }
   else
   {
      /* Fallback to file comment detection */
      version = detect_pg_version_from_file(resolved_filename);

      /* Fallback to local system check if file comment check failed */
      if (!pgvictoria_is_version_supported(version))
      {
         version = detect_pg_version();
      }
   }

   baseline = pgvictoria_get_baseline(version);
   if (!baseline)
   {
      warnx("No baseline available for PostgreSQL version %d", version);
      free(resolved_filename);
      if (resolved_output)
      {
         free(resolved_output);
      }
      return 1;
   }

   file = fopen(resolved_filename, "r");
   if (!file)
   {
      warn("pgvictoria-cli: Cannot open configuration file %s", resolved_filename);
      pgvictoria_json_destroy(baseline);
      free(resolved_filename);
      if (resolved_output)
      {
         free(resolved_output);
      }
      return 1;
   }

   /* Parse file and build comparison list */
   struct pgvictoria_diff_item* head = NULL;
   struct pgvictoria_diff_item* tail = NULL;

   char line[1024];
   char key[128];
   char value[1024];

   while (fgets(line, sizeof(line), file))
   {
      memset(key, 0, sizeof(key));
      memset(value, 0, sizeof(value));
      trim_and_extract_key_value(line, key, value);

      if (strlen(key) > 0 && strlen(value) > 0)
      {
         enum value_type type;
         char* matched_key = NULL;
         uintptr_t baseline_val_ptr = pgvictoria_json_get_typed_case_insensitive(baseline, key, &type, &matched_key);

         const char* disp_key = matched_key ? matched_key : key;
         const char* def_val = "-";
         const char* status_text = "Custom";
         char* default_val_str = NULL;

         if (baseline_val_ptr)
         {
            struct value* v = NULL;
            if (!pgvictoria_value_create(type, baseline_val_ptr, &v))
            {
               if (v)
               {
                  default_val_str = pgvictoria_value_to_string(v, FORMAT_TEXT, NULL, 0);
                  if (default_val_str)
                  {
                     def_val = default_val_str;
                     if (strcmp(default_val_str, value) == 0)
                     {
                        status_text = "Default";
                     }
                     else
                     {
                        status_text = "Modified";
                     }
                  }
                  pgvictoria_value_destroy(v);
               }
            }
         }

         /* Create a diff item */
         struct pgvictoria_diff_item* item = malloc(sizeof(struct pgvictoria_diff_item));
         if (item)
         {
            snprintf(item->key, sizeof(item->key), "%s", disp_key);
            snprintf(item->baseline_val, sizeof(item->baseline_val), "%s", def_val);
            snprintf(item->current_val, sizeof(item->current_val), "%s", value);
            snprintf(item->status, sizeof(item->status), "%s", status_text);
            item->next = NULL;

            if (!head)
            {
               head = item;
               tail = item;
            }
            else
            {
               tail->next = item;
               tail = item;
            }
         }

         if (default_val_str)
         {
            free(default_val_str);
         }
      }
   }

   fclose(file);
   pgvictoria_json_destroy(baseline);

   if (resolved_output != NULL)
   {
      if (pgvictoria_ends_with(resolved_output, ".md") || pgvictoria_ends_with(resolved_output, ".markdown"))
      {
         ret = pgvictoria_generate_markdown_report(resolved_filename, resolved_output, version, head);
      }
      else
      {
         ret = pgvictoria_generate_html_report(resolved_filename, resolved_output, version, head);
      }
   }
   else
   {
      /* Output plain text directly to console */
      printf("\nPostgreSQL %d Configuration Report (File Mode: %s)\n", version, resolved_filename);
      char* os_name = NULL;
      int k_major = 0, k_minor = 0, k_patch = 0;
      if (pgvictoria_os_kernel_version(&os_name, &k_major, &k_minor, &k_patch) == 0)
      {
         printf("System: %s %d.%d.%d\n", os_name, k_major, k_minor, k_patch);
         free(os_name);
      }
      printf("=================================================================================\n");
      printf("%-40s | %-20s | %-20s\n", "Key", "Default", "Current");
      printf("---------------------------------------------------------------------------------\n");

      struct pgvictoria_diff_item* curr = head;
      while (curr)
      {
         if (strcmp(curr->status, "Default") != 0)
         {
            printf("%-40s | %-20s | %-20s\n", curr->key, curr->baseline_val, curr->current_val);
         }
         curr = curr->next;
      }
      printf("=================================================================================\n");
   }

   /* Cleanup comparison list */
   struct pgvictoria_diff_item* curr = head;
   while (curr)
   {
      struct pgvictoria_diff_item* next = curr->next;
      free(curr);
      curr = next;
   }

   free(resolved_filename);
   if (resolved_output)
   {
      free(resolved_output);
   }

   return ret;
}
