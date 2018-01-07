#include <unistd.h>
#include <commands.h>
#include <apr_uri.h>
#include <apr_fnmatch.h>
#include <dbg.h>
#include <shell.h>
#include <db.h>

static int Command_fetchAndUntar(
	apr_pool_t *pool, const char *urlScheme, const char *url,
	const char *targetSrcDir, const char *targetBuildDir);

int Command_fetch(apr_pool_t *p, const char *url)
{
	int rc;
	apr_uri_t info;
	rc = apr_uri_parse(p, url, &info);
	check(rc == APR_SUCCESS, "Failed to parse URL: %s", url);

	if (apr_fnmatch(GIT_PAT, info.path, 0) == APR_SUCCESS)
	{
		rc = Shell_exec(GIT_SH, "URL", url, NULL);
		check(rc == 0, "Failed to clone %s to %s", url, BUILD_DIR);
	}
	else if (apr_fnmatch(TAR_GZ_PAT, info.path, 0) == APR_SUCCESS)
	{
		rc = Command_fetchAndUntar(
			p, info.scheme, url, TAR_GZ_SRC, BUILD_DIR);
		check(rc == 0, "Failed fetch and untar");
	}
	else if (apr_fnmatch(TAR_BZ2_PAT, info.path, 0) == APR_SUCCESS)
	{
		rc = Command_fetchAndUntar(
			p, info.scheme, url, TAR_BZ2_SRC, BUILD_DIR);
		check(rc == 0, "Failed fetch and untar");
	}
	else if (apr_fnmatch(DEPEND_PAT, info.path, 0) == APR_SUCCESS)
	{
		rc = Shell_exec(CURL_SH, "TARGET", DEPENDS_PATH, "URL", url, NULL);
		check(rc == 0, "Failed to fetch DEPENDS file %s", url);

		// TODO: Invoke Command_depends
	}
	else
	{
		sentinel("Don't know how to handle this type of file %s", url);
	}

error: // fallthrough
	return rc;
}

int Command_install(
	apr_pool_t *p, const char *url, const char *configure_opts,
	const char *make_opts, const char *install_opts)
{
	return -1;
}

int Command_depends(apr_pool_t *p, const char *path)
{
	return -1;
}

int Command_build(
	apr_pool_t *p, const char *url, const char *configure_opts,
	const char *make_opts, const char *install_opts)
{
	int rc = 0;

	rc = access(BUILD_DIR, X_OK | W_OK | R_OK);
	check(rc == 0, "Cannot access build directory: %s", BUILD_DIR);

	if (access(CONFIG_SCRIPT, X_OK | W_OK | R_OK) == 0)
	{
		rc = Shell_exec(CONFIGURE_SH, "OPTS", configure_opts, NULL);
		check(rc == 0, "Failed configure step");
	}

	rc = Shell_exec(MAKE_SH, "OPTS", make_opts, NULL);
	check(rc == 0, "Failed make step");

	//install_opts = install_opts ? install_opts : "install";
	//rc = Shell_exec(INSTALL_SH, "OPTS", install_opts, NULL);
	//check(rc == 0, "Failed install step");

	rc = Shell_exec(CLEANUP_SH, NULL);
	check(rc == 0, "Failed cleanup step");

	rc = DB_update(url);
	check(rc == 0, "Failed to add package to database");

error: // fallthrough
	return rc;
}

static int Command_fetchAndUntar(
	apr_pool_t *pool, const char *urlScheme, const char *url,
	const char *targetSrcDir, const char *targetBuildDir)
{
	int rc = 0;
	Shell tarCommand =
		apr_fnmatch(TAR_GZ_PAT, url, 0) == APR_SUCCESS ?
			TAR_GZ_SH : TAR_BZ2_SH;

	if (urlScheme)
	{
		rc = Shell_exec(CURL_SH, "TARGET", targetSrcDir, "URL", url, NULL);
		check(rc == 0, "Failed to curl %s into %s", url, targetSrcDir);
	}

	rc = apr_dir_make_recursive(targetBuildDir,
		APR_FPROT_GREAD | APR_FPROT_GWRITE | APR_FPROT_GEXECUTE |
		APR_FPROT_UREAD | APR_FPROT_UWRITE | APR_FPROT_UEXECUTE,
		pool);
	check(rc == APR_SUCCESS,
		"Failed to create build directory: %s", targetBuildDir);

	rc = Shell_exec(tarCommand, "FILE", targetSrcDir, NULL);
	check(rc == 0, "Failed to untar %s", targetSrcDir);

error: // fallthrough
	return rc;
}

