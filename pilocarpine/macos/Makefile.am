AM_OBJCFLAGS = $(GLOBAL_CFLAGS) $(libfragmentzip_CFLAGS) $(zlib_CFLAGS) $(libzip_CFLAGS) $(libirecovery_CFLAGS) $(libimobiledevice_CFLAGS) $(libplist_CFLAGS) $(openssl_CFLAGS) $(libcurl_CFLAGS) $(PTHREAD_CFLAGS)
AM_LDFLAGS = $(libfragmentzip_LIBS) $(zlib_LIBS) $(libzip_LIBS) $(libirecovery_LIBS) $(libimobiledevice_LIBS) $(libplist_LIBS) $(openssl_LIBS) $(libcurl_LIBS) $(PTHREAD_CLIBS)

noinst_PROGRAMS = n1ghtshade
n1ghtshade_OBJCFLAGS = -I../../belladonna/src $(AM_OBJCFLAGS)
n1ghtshade_LDFLAGS = -rpath @loader_path/../Frameworks -framework Cocoa -Ln1ghtshade.app/Contents/Frameworks
n1ghtshade_LDADD = ../../belladonna/src/libbelladonna.a $(AM_LDFLAGS)
n1ghtshade_SOURCES = main.m common.m dfu_enter_view.m other_options_view.m tasks_view.m main_view.m callback.m

all-local:
	@mv n1ghtshade n1ghtshade.app/Contents/MacOS