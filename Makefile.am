bin_PROGRAMS = bluray_info bluray_copy
man_MANS = bluray_info.1

bluray_info_SOURCES = bluray_info.c bluray_device.c bluray_video.c bluray_audio.c bluray_pgs.c bluray_time.c
bluray_info_LDADD = $(LIBBLURAY_LIBS)

bluray_copy_SOURCES = bluray_copy.c bluray_time.c bluray_device.c
bluray_copy_LDADD = $(LIBBLURAY_LIBS)
