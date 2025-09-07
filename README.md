# bluray_info

The bluray_info project is a set of utilities for accessing Blu-rays:

* `bluray_info` - display information about a Blu-ray in multiple formats
* `bluray_copy` - copy a Blu-rays track to a file or stdout
* `bluray_player` - a small Blu-ray player using libmpv as backend
* `bluray_dvd_rip` - a shell script to rip Blu-rays using ffmpeg-7.0 and above

# Requirements:

* libbluray >= 1.2.0
* libaacs for decryption
* libmpv for `bluray_player`
* ffmpeg-7.0 or greater for `bluray_rip`

# Installation

```
cmake .
make
sudo make install
```
Building ``bluray_player`` requires ``mpv`` to be installed with libmpv support first.

# Documentation

Run ``--help`` for each program to see its options.

There are man pages for each program as well.

# Disc access

Decrypting Blu-ray discs is done through libaacs. Even with libaacs, you
will need a KEYDB.cfg file to bypass the encryption, and can be found
various places online. The best location is http://fvonline-db.bplaced.net/

libaacs by default will look for KEYDB.cfg in ~/.config/aacs/ but with these
utilities, you can specify the path directly if you'd like.

# Input source

The input source can be either a device name, a single file, or a directory. The
program will use the default Blu-ray drive based on your OS if no path is given.

  $ bluray_info
  $ bluray_info /dev/sr0
  $ bluray_info ~/Media/BD.ADVENTURE.iso
  $ bluray_info ~/Media/BD.ADVENTURE_BACKUP/

If you're going to do a lot of reads on a disc drive, I'd recommend mounting it
so the access can be cached -- this is especially helpful when using / testing
bluray_copy a lot to get chapters, playlists, etc.

```
sudo mount /dev/sr0 -o ro -t udf /mnt
```

# Playlists

bluray_info uses libbluray to determine which playlist is the main one, and
doesn't calculate it itself.

libbluray has two ways to display all playlists: there are "relevant" ones
where the library filters out any duplicates. However, the list of duplicates
can change across locations, filesystems, systems, etc. For that reason,
the bluray utilities will enable displaying all titles, not just the relvant ones.
If you don't want this, use --no-duplicates option

# Support

Please file any issues at https://github.com/beandog/bluray_info/issues or email me at ``steve.dibb@gmail.com``

# Copyright

Licensed under GPL-2. See https://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
