AM_CFLAGS = $(GLOBAL_CFLAGS) $(libfragmentzip_CFLAGS) $(zlib_CFLAGS) $(libzip_CFLAGS) $(libirecovery_CFLAGS) $(libimobiledevice_CFLAGS) $(libplist_CFLAGS) $(openssl_CFLAGS) $(libcurl_CFLAGS) $(PTHREAD_CFLAGS)
AM_LDFLAGS = $(libfragmentzip_LIBS) $(zlib_LIBS) $(libzip_LIBS) $(libirecovery_LIBS) $(libimobiledevice_LIBS) $(libplist_LIBS) $(openssl_LIBS) $(libcurl_LIBS) $(PTHREAD_CLIBS)

noinst_PROGRAMS = n1ghtshade
n1ghtshade_CFLAGS = -I../../belladonna/src $(AM_CFLAGS)
n1ghtshade_LDADD = ../../belladonna/src/libbelladonna.a $(AM_LDFLAGS)
n1ghtshade_SOURCES = n1ghtshade.c