/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org/>
*/
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "cunit.h"


char *testfile() {
	int fd;
	struct stat st;
	struct stat st2;
	char buf[1];
	char buf2[1];

	fd = open("/init.fm", O_RDWR);
	assert_ne("open[0]", -1, fd);

	assert_eq("fstat", 0, fstat(fd, &st));

	printf("fstat: dev=%x\n", st.st_dev);
	printf("fstat: ino=%d\n", st.st_ino);
	printf("fstat: mode=%x\n", st.st_mode);
	printf("fstat: nlink=%d\n", st.st_nlink);
	printf("fstat: uid=%d\n", st.st_uid);
	printf("fstat: gid=%d\n", st.st_gid);
	printf("fstat: rdev=%x\n", st.st_rdev);
	printf("fstat: size=%d\n", st.st_size);
	printf("fstat: blksize=%d\n", st.st_blksize);
	printf("fstat: blocks=%d\n", st.st_blocks);
	printf("fstat: atime=%d\n", st.st_atime);
	printf("fstat: mtime=%d\n", st.st_mtime);
	printf("fstat: ctime=%d\n", st.st_ctime);

	assert_eq("read[0]", 1, read(fd, buf, 1));
	assert_eq("read[0] buf[0]", 0x23, buf[0]);

	assert_eq("read[1]", 1, read(fd, buf, 1));
	assert_eq("read[1] buf[0]", 0x20, buf[0]);

	assert_eq("lseek[1]", 1, lseek(fd, 1, SEEK_SET));
	assert_eq("read[1-2]", 1, read(fd, buf2, 1));
	assert_eq("read[1-2] buf[0]", buf[0], buf2[0]);
	assert_eq("read[1-2] cmp", buf[0], buf2[0]);

	assert_eq("lseek[1]", 1, lseek(fd, 1, SEEK_SET));
	buf[0] = 0x41;
	assert_eq("write[1]", 1, write(fd, buf, 1));

	assert_eq("close[0]", 0, close(fd));

	assert_eq("chown[0]", 0, chown("/init.fm", 147, 258));
	assert_eq("chmod[0]", 0, chmod("/init.fm", S_IXGRP | S_IROTH | S_IWOTH));

	fd = open("/init.fm", O_RDONLY);
	assert_ne("open[1]", -1, fd);

	assert_eq("fstat[1]", 0, fstat(fd, &st2));
	assert_eq("chown[0-1]", 147, st2.st_uid);
	assert_eq("chown[0-2]", 258, st2.st_gid);

	printf("fstat: mode=%x\n", st2.st_mode);
	assert_eq("chmod[0-1]",
			S_IFREG | S_IXGRP | S_IROTH | S_IWOTH,
			st2.st_mode);

	assert_eq("read[0]", 1, read(fd, buf, 1));
	assert_eq("read[0] buf[0]", 0x23, buf[0]);

	assert_eq("read[1]", read(fd, buf, 1), 1);
	assert_eq("read[1] buf[0]", 0x41, buf[0]);

	assert_eq("close[0]", 0, close(fd));

	return NULL;
}

char *testdir() {
	int fd;
	struct stat st;
	char buf[1024];

	assert_eq("getcwd[0]", 0, strcmp("/", getcwd(buf, sizeof(buf))));

	assert_eq("chdir", 0, chdir("system"));
	assert_eq("getcwd[1]", 0, strcmp("/system", getcwd(buf, sizeof(buf))));

	assert_eq("mkdir[0]", 0,
			mkdir("hoge", S_IRGRP | S_IROTH | S_IWOTH));
	fd = open("hoge", O_RDONLY);
	assert_ne("mkdir-open[0]", -1, fd);
	assert_eq("mkdir-fstat[0]", 0, fstat(fd, &st));
	assert_eq("mkdir-fstat[0-1]",
			S_IFDIR | S_IRGRP | S_IROTH | S_IWOTH,
			st.st_mode);
	assert_eq("mkdir-close[0]", 0, close(fd));

	return NULL;
}

char *testmm() {
	struct utsname name;
	int pid;
	time_t t;

	pid = fork();
	if (pid == 0) {
		printf("child's pid = %d, ppid = %d\n",
				getpid(), getppid());
		_exit(0);
	}
	else {
		printf("parent's pid = %d, ppid = %d\n",
				getpid(), getppid());
		waitpid(-1, &pid, 0);
	}

	assert_eq("uname", 0, uname(&name));
	printf("uname = %s\n", name.sysname);
	printf("uname = %s\n", name.nodename);
	printf("uname = %s\n", name.release);
	printf("uname = %s\n", name.version);
	printf("uname = %s\n", name.machine);

	printf("time = %d\n", time(&t));
	printf("time = %d\n", t);

	printf("uid = %d\n", getuid());
	printf("euid = %d\n", geteuid());
	printf("gid = %d\n", getgid());
	printf("egid = %d\n", getegid());
	setuid(682);
	setgid(345);
	printf("uid = %d\n", getuid());
	printf("euid = %d\n", geteuid());
	printf("gid = %d\n", getgid());
	printf("egid = %d\n", getegid());

	return NULL;
}

int main(int argc, char **argv)
{
	test(testfile);
	test(testdir);
	test(testmm);

	return 0;
}
