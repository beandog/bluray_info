bin_PROGRAMS = bluray_info bluray_copy bluray_backup
man_MANS = bluray_info.1 bluray_copy.1

bluray_info_SOURCES = bluray_info.c bluray_open.c bluray_chapter.c bluray_video.c bluray_audio.c bluray_pgs.c bluray_time.c
bluray_info_CFLAGS = $(LIBBLURAY_CFLAGS)
bluray_info_LDADD = $(LIBBLURAY_LIBS) -lm

bluray_copy_SOURCES = bluray_copy.c bluray_open.c bluray_time.c
bluray_copy_CFLAGS = $(LIBBLURAY_CFLAGS)
bluray_copy_LDADD = $(LIBBLURAY_LIBS) -lm

bluray_backup_SOURCES = bluray_backup.c bluray_open.c bluray_time.c
bluray_backup_CFLAGS = $(LIBBLURAY_CFLAGS) $(UUID_CFLAGS)
bluray_backup_LDADD = $(LIBBLURAY_LIBS) $(UUID_LIBS) -lm

if BLURAY_PLAYER
bin_PROGRAMS += bluray_player
man_MANS += bluray_player.1
bluray_player_SOURCES = bluray_player.c bluray_open.c bluray_video.c bluray_audio.c bluray_pgs.c bluray_time.c
bluray_player_CFLAGS = $(LIBBLURAY_CFLAGS) $(MPV_CFLAGS)
bluray_player_LDADD = $(LIBBLURAY_LIBS) $(MPV_LIBS) -lm
endif
