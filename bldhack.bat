@echo off
cc -D__CPM86__ -D__LEGACY__ -ic: dump.c
sqz dump.o
ln -o dump.cmd dump.o -lc
cc -D__CPM86__ -D__LEGACY__ -ic: more.c
sqz more
ln -o mode.cmd more.o c:m.lib -lc
cc -D__CPM86__ -D__LEGACY__ -ic: mode.c
sqz mode
ln -o mode.cmd mode.o c:m.lib -lc
cc -D__CPM86__ -D__LEGACY__ -ic: write.c
sqz write.o
ln -o write.cmd write.o -lc
cc -D__CPM86__ -D__LEGACY__ -ic: dirent.c
sqz dirent.o
cc -D__CPM86__ -D__LEGACY__ -ic: debug.c
sqz debug.o
cc -D__CPM86__ -D__LEGACY__ -ic: ls.c
sqz ls.o
ln -o ls.cmd ls.o dirent.o debug.o -lc
cc -D__CPM86__ -D__LEGACY__ -ic: rm.c
sqz rm.o
ln -o rm.cmd rm.o dirent.o debug.o -lc
rasm86 ver $ pz sz
link86 ver[$sz]
rasm86 cls $ pz sz
link86 cls[$sz]
rasm86 pause $ pz sz
link86 pause[$sz]
rasm86 tod $ pz sz
link86 tod[$sz]
rasm86 reboot $ pz sz
link86 reboot[$sz]
del rm.o
del dirent.o
del debug.o
del write.o
del ls.o
del mode.o
del more.o
del dump.o
