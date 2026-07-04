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
 * be used to endorse or promote products derived from this software without specific
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
 *
 */

#include <tscommon.h>
#include <shmem.h>
#include <logging.h>
#include <memory.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

char TEST_BASE_DIR[MAX_PATH];

void
pgvictoria_test_environment_create(void)
{
   int ret;
   size_t size = sizeof(struct main_configuration);

   ret = pgvictoria_create_shared_memory(size, HUGEPAGE_OFF, &shmem);
   assert(!ret);

   struct main_configuration* config = (struct main_configuration*)shmem;

   // Set up basic mock configurations for logging
   config->common.log_level = PGVICTORIA_LOGGING_LEVEL_DEBUG5;
   config->common.log_type = PGVICTORIA_LOGGING_TYPE_FILE;

   char* base_dir = getenv("PGVICTORIA_TEST_BASE_DIR");
   if (base_dir != NULL && base_dir[0] != '\0')
   {
      strncpy(TEST_BASE_DIR, base_dir, sizeof(TEST_BASE_DIR) - 1);
      TEST_BASE_DIR[sizeof(TEST_BASE_DIR) - 1] = '\0';
   }
   else
   {
      strcpy(TEST_BASE_DIR, "/tmp/pgvictoria-test/base");
   }

   // Extract parent directory of TEST_BASE_DIR
   char base[MAX_PATH];
   strncpy(base, TEST_BASE_DIR, sizeof(base) - 1);
   base[sizeof(base) - 1] = '\0';
   char* slash = strrchr(base, '/');
   if (slash != NULL)
   {
      *slash = '\0';
   }

   snprintf(config->common.log_path, sizeof(config->common.log_path), "%.100s/log/pgvictoria.log", base);

   ret = pgvictoria_start_logging();
   assert(!ret);
}

void
pgvictoria_test_environment_destroy(void)
{
   pgvictoria_stop_logging();
   pgvictoria_destroy_shared_memory(shmem, sizeof(struct main_configuration));
}

void
pgvictoria_test_setup(void)
{
   pgvictoria_memory_init();
}

void
pgvictoria_test_teardown(void)
{
   pgvictoria_memory_destroy();
}
