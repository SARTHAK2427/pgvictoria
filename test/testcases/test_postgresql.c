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

#include <mctf.h>
#include <tscommon.h>
#include <postgresql.h>
#include <json.h>
#include <utils.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>

MCTF_TEST_SETUP(postgresql)
{
   pgvictoria_test_setup();
}

MCTF_TEST_TEARDOWN(postgresql)
{
   pgvictoria_test_teardown();
}

MCTF_TEST(test_postgresql_supported_versions)
{
   MCTF_ASSERT(pgvictoria_is_version_supported(14), cleanup);
   MCTF_ASSERT(pgvictoria_is_version_supported(15), cleanup);
   MCTF_ASSERT(pgvictoria_is_version_supported(16), cleanup);
   MCTF_ASSERT(pgvictoria_is_version_supported(17), cleanup);
   MCTF_ASSERT(pgvictoria_is_version_supported(18), cleanup);
   MCTF_ASSERT(pgvictoria_is_version_supported(19), cleanup);

   MCTF_ASSERT(!pgvictoria_is_version_supported(13), cleanup);
   MCTF_ASSERT(!pgvictoria_is_version_supported(20), cleanup);
   MCTF_ASSERT(!pgvictoria_is_version_supported(0), cleanup);
   MCTF_ASSERT(!pgvictoria_is_version_supported(-5), cleanup);

cleanup:
   MCTF_FINISH();
}

MCTF_TEST(test_postgresql_static_baselines)
{
   struct json* baseline = NULL;

   baseline = pgvictoria_get_baseline(17);
   MCTF_ASSERT_PTR_NONNULL(baseline, cleanup);
   pgvictoria_json_destroy(baseline);
   baseline = NULL;

   baseline = pgvictoria_get_baseline(14);
   MCTF_ASSERT_PTR_NONNULL(baseline, cleanup);
   pgvictoria_json_destroy(baseline);
   baseline = NULL;

   baseline = pgvictoria_get_baseline(13);
   MCTF_ASSERT_PTR_NULL(baseline, cleanup);

cleanup:
   if (baseline != NULL)
   {
      pgvictoria_json_destroy(baseline);
   }
   MCTF_FINISH();
}

MCTF_TEST(test_postgresql_version_bounds)
{
   int min_ver = pgvictoria_get_min_supported_version();
   int max_ver = pgvictoria_get_max_supported_version();

   MCTF_ASSERT_INT_EQ(min_ver, 14, cleanup);
   MCTF_ASSERT(max_ver >= 19, cleanup);

cleanup:
   MCTF_FINISH();
}

MCTF_TEST(test_postgresql_dynamic_baselines)
{
   char temp_dir[2048];
   char file_path[2100];
   FILE* f = NULL;
   struct json* baseline = NULL;

   /* Setup temporary directory for dynamic baselines under base log path */
   snprintf(temp_dir, sizeof(temp_dir), "%s/baselines_test", TEST_BASE_DIR);
   pgvictoria_mkdir(temp_dir);

   /* Create dummy pg25.json baseline configuration */
   snprintf(file_path, sizeof(file_path), "%s/pg25.json", temp_dir);
   f = fopen(file_path, "w");
   MCTF_ASSERT_PTR_NONNULL(f, cleanup);
   fprintf(f, "{\"version\": 25, \"comment\": \"mock baseline\"}");
   fclose(f);
   f = NULL;

   /* Test that version 25 is not supported initially */
   MCTF_ASSERT(!pgvictoria_is_version_supported(25), cleanup);

   /* Set environment variable to redirect baseline scanner to our temp dir */
   setenv("PGVICTORIA_BASELINES_DIR", temp_dir, 1);

   /* Test version 25 is supported now */
   MCTF_ASSERT(pgvictoria_is_version_supported(25), cleanup);
   MCTF_ASSERT_INT_EQ(pgvictoria_get_max_supported_version(), 25, cleanup);

   /* Retrieve and check JSON baseline */
   baseline = pgvictoria_get_baseline(25);
   MCTF_ASSERT_PTR_NONNULL(baseline, cleanup);

cleanup:
   if (f != NULL)
   {
      fclose(f);
   }
   if (baseline != NULL)
   {
      pgvictoria_json_destroy(baseline);
   }
   unsetenv("PGVICTORIA_BASELINES_DIR");
   unlink(file_path);
   rmdir(temp_dir);
   MCTF_FINISH();
}
