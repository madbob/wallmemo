NULL =

INCLUDES = \
	-DPREFIX=\""$(prefix)"\"				\
	-DLIBDIR=\""$(libdir)"\"				\
	-DDATADIR=\""$(datadir)"\"				\
	-DGNOMELOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\"	\
	$(NULL)

common_cflags = \
	-I$(top_srcdir) \
	$(WALLMEMO_CFLAGS) \
	$(NULL)

common_ldadd = \
	$(WALLMEMO_LIBS) \
	$(NULL)

bin_PROGRAMS = wallmemo

wallmemo_SOURCES = \
	main.c

wallmemo_CFLAGS = $(common_cflags)
wallmemo_LDADD = $(common_ldadd)

