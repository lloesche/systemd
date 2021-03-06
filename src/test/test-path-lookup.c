/***
  This file is part of systemd.

  Copyright 2014 Zbigniew Jędrzejewski-Szmek

  systemd is free software; you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation; either version 2.1 of the License, or
  (at your option) any later version.

  systemd is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with systemd; If not, see <http://www.gnu.org/licenses/>.
***/

#include <stdlib.h>
#include <sys/stat.h>

#include "log.h"
#include "path-lookup.h"
#include "rm-rf.h"
#include "string-util.h"
#include "strv.h"

static void test_paths(UnitFileScope scope) {
        char template[] = "/tmp/test-path-lookup.XXXXXXX";

        _cleanup_lookup_paths_free_ LookupPaths lp_without_env = {};
        _cleanup_lookup_paths_free_ LookupPaths lp_with_env = {};
        char *systemd_unit_path;

        assert_se(mkdtemp(template));

        assert_se(unsetenv("SYSTEMD_UNIT_PATH") == 0);
        assert_se(lookup_paths_init(&lp_without_env, scope, 0, NULL) >= 0);
        assert_se(!strv_isempty(lp_without_env.search_path));
        assert_se(lookup_paths_reduce(&lp_without_env) >= 0);

        systemd_unit_path = strjoina(template, "/systemd-unit-path");
        assert_se(setenv("SYSTEMD_UNIT_PATH", systemd_unit_path, 1) == 0);
        assert_se(lookup_paths_init(&lp_with_env, scope, 0, NULL) == 0);
        assert_se(strv_length(lp_with_env.search_path) == 1);
        assert_se(streq(lp_with_env.search_path[0], systemd_unit_path));
        assert_se(lookup_paths_reduce(&lp_with_env) >= 0);
        assert_se(strv_length(lp_with_env.search_path) == 0);

        assert_se(rm_rf(template, REMOVE_ROOT|REMOVE_PHYSICAL) >= 0);
}

static void print_generator_binary_paths(UnitFileScope scope) {
        _cleanup_strv_free_ char **paths;
        char **dir;

        log_info("Generators dirs (%s):", scope == UNIT_FILE_SYSTEM ? "system" : "user");

        paths = generator_binary_paths(scope);
        STRV_FOREACH(dir, paths)
                log_info("        %s", *dir);
}

int main(int argc, char **argv) {
        log_set_max_level(LOG_DEBUG);
        log_parse_environment();
        log_open();

        test_paths(UNIT_FILE_SYSTEM);
        test_paths(UNIT_FILE_USER);
        test_paths(UNIT_FILE_GLOBAL);

        print_generator_binary_paths(UNIT_FILE_SYSTEM);
        print_generator_binary_paths(UNIT_FILE_USER);

        return EXIT_SUCCESS;
}
