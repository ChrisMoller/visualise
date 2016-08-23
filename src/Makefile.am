bin_PROGRAMS = vis
AM_YFLAGS = -d -v
AM_LFLAGS = -o$(LEX_OUTPUT_ROOT).c
vis_SOURCES = vis.c \
              vis-parse.y vis-lex.l \
              fcn-decls.h fcn-table.h \
              parse-utilities.c parse-utilities.h \
              magick.c magick.h
DISTCLEANFILES = .deps/*.P



vis_CFLAGS = -std=c99 -Wall -Werror \
              $(GTK_CFLAGS) $(CAIRO_CFLAGS) $(MAGICKWAND_CFLAGS)

vis_LDFLAGS = -lm $(GTK_LIBS) $(CAIRO_LIBS) $(MAGICKWAND_LIBS)

# -leak-check=full
#  algrind --tool=memcheck --leak-check=full --leak-resolution=high -
#                -num-callers=20 --log-file=vgdump ./gfig
#  
#VALGRIND_FLAGS = --gen-suppressions=all --run-libc-freeres=yes \
#    --suppressions=valgrind.supp
#VALGRIND_FLAGS = --gen-suppressions=all --run-libc-freeres=yes

valgrind.log : vis
	valgrind $(VALGRIND_FLAGS) --log-file=$@ ./vis

clean-local: clean-local-check
.PHONY: clean-local-check
clean-local-check:
	rm -rf vis-parse.c vis-parse.h vis-lex.c
