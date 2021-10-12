as os.asm
sqz os.o
as gfx.asm
sqz gfx.o
cc -D__CPM86__ -I. util.c
sqz util.o
cc -D__CPM86__ -I. dirent.c
sqz dirent.o
cc -D__CPM86__ -I. conio.c
sqz conio.o
cc -D__CPM86__ -I. dpb.c
sqz dpb.o
cc -D__CPM86__ -I. debug.c
sqz debug.o
lb util.lib util.o conio.o dirent.o dpb.o debug.o os.o gfx.o
cc -D__CPM86__ -I. dump.c
sqz dump.o
ln -o dump.cmd dump.o util.lib -lc86
cc -D__CPM86__ -I. more.c
sqz more.o
ln -o mode.cmd more.o util.lib -lc86
cc -D__CPM86__ -I. mode.c
sqz mode.o
ln -o mode.cmd mode.o util.lib -lc86
cc -D__CPM86__ -I. write.c
sqz write.o
ln -o write.cmd write.o util.lib -lc86
cc -D__CPM86__ -I. ls.c
sqz ls.o
ln -o ls.cmd ls.o util.lib -lc86
cc -D__CPM86__ -I. touch.c
sqz touch.o
ln -o touch.cmd touch.o -lc86
cc -D__CPM86__ -I. rm.c
sqz rm.o
ln -o rm.cmd rm.o util.lib -lc86
cc -D__CPM86__ -I. printenv.c
sqz printenv.o
ln -o printenv.cmd printenv.o util.lib -lc86
cc -D__CPM86__ -I. ciotest.c
sqz ciotest.o
ln -o ciotest.cmd ciotest.o util.lib -lc86
cc -D__CPM86__ -I. ball.c
sqz ball.o
ln -o ball.cmd ball.o util.lib -lc86
cc -D__CPM86__ -I. getch.c
sqz getch.o
ln -o getch.cmd getch.o util.lib -lc86
rasm86 atinit $ pz sz
link86 atinit[$sz]
rasm86 attime $ pz sz
link86 attime[$sz]
rasm86 mem $ pz sz
link86 mem[$sz]
rasm86 ver $ pz sz
link86 ver[$sz]
rasm86 clsansi $ pz sz
link86 clsansi[$sz]
rasm86 cls $ pz sz
link86 cls[$sz]
rasm86 pause $ pz sz
link86 pause[$sz]
rasm86 tod $ pz sz
link86 tod[$sz]
rasm86 reboot $ pz sz
link86 reboot[$sz]
