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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include "cunit.h"

extern void *malloc(size_t);
extern void free(void *);


char *testfile(void)
{
	struct stat st0;
	assert_eq("stat", 0, stat("/motd", &st0));

	int fd = open("/motd", O_RDWR);
	assert_ne("open[0]", -1, fd);

	struct stat st;
	assert_eq("fstat", 0, fstat(fd, &st));

	printf("fstat: dev=%x : %x\n", st0.st_dev, st.st_dev);
	printf("fstat: ino=%d : %d\n", st0.st_ino, st.st_ino);
	printf("fstat: mode=%x : %x\n", st0.st_mode, st.st_mode);
	printf("fstat: nlink=%d : %d\n", st0.st_nlink, st.st_nlink);
	printf("fstat: uid=%d : %d\n", st0.st_uid, st.st_uid);
	printf("fstat: gid=%d : %d\n", st0.st_gid, st.st_gid);
	printf("fstat: rdev=%x : %x\n", st0.st_rdev, st.st_rdev);
	printf("fstat: size=%d : %d\n", (int) st0.st_size, (int) st.st_size);
	printf("fstat: blksize=%d : %d\n", st0.st_blksize, st.st_blksize);
	printf("fstat: blocks=%d : %d\n", st0.st_blocks, st.st_blocks);

	printf("fstat: atime=%d, %d : %d, %d\n",
			(int) ((long long int) st0.st_atime >> 32),
			(int) ((long long int) st0.st_atime & 0xffffffff),
			(int) ((long long int) st.st_atime >> 32),
			(int) ((long long int) st.st_atime & 0xffffffff));
	printf("fstat: mtime=%d, %d : %d, %d\n",
			(int) ((long long int) st0.st_mtime >> 32),
			(int) ((long long int) st0.st_mtime & 0xffffffff),
			(int) ((long long int) st.st_mtime >> 32),
			(int) ((long long int) st.st_mtime & 0xffffffff));
	printf("fstat: ctime=%d, %d : %d, %d\n",
			(int) ((long long int) st0.st_ctime >> 32),
			(int) ((long long int) st0.st_ctime & 0xffffffff),
			(int) ((long long int) st.st_ctime >> 32),
			(int) ((long long int) st.st_ctime & 0xffffffff));

	char buf[1];
	assert_eq("read[0]", 1, read(fd, buf, 1));
	assert_eq("read[0] buf[0]", 'k', buf[0]);

	assert_eq("read[1]", 1, read(fd, buf, 1));
	assert_eq("read[1] buf[0]", 'e', buf[0]);

	off_t offset = lseek(fd, 1, SEEK_SET);
	assert_t("lseek[1]", offset == 1LL);

	char buf2[1];
	assert_eq("read[1-2]", 1, read(fd, buf2, 1));
	assert_eq("read[1-2] buf[0]", buf[0], buf2[0]);
	assert_eq("read[1-2] cmp", buf[0], buf2[0]);

	assert_eq("lseek[1]", 1, lseek(fd, 1, SEEK_SET));
	buf[0] = '@';
	assert_eq("write[1]", 1, write(fd, buf, 1));

	assert_eq("close[0]", 0, close(fd));

	assert_eq("chmod[0]", 0, chmod("/motd", S_IRGRP | S_IXGRP | S_IWOTH));

	fd = open("/motd", O_RDONLY);
	assert_ne("open[1]", -1, fd);

	struct stat st2;
	assert_eq("fstat[1]", 0, fstat(fd, &st2));

	printf("fstat: mode=%x\n", st2.st_mode);
	assert_eq("chmod[0-1]",
			S_IFREG | S_IRGRP | S_IXGRP | S_IWOTH,
			st2.st_mode);

	assert_eq("read[2-0]", 1, read(fd, buf, 1));
	assert_eq("read[2-0] buf[0]", 'k', buf[0]);

	assert_eq("read[2-1]", read(fd, buf, 1), 1);
	assert_eq("read[2-1] buf[0]", '@', buf[0]);

	assert_eq("close[1]", 0, close(fd));

	assert_eq("access[0]", 0,
			access("/motd", R_OK | W_OK | X_OK));

	return NULL;
}

char *testdup(void)
{
	int fd = dup2(1, 2);
	assert_eq("dup2[0]", 2, fd);

	char buf[1];
	buf[0] = 'b';
	write(fd, buf, 1);
	assert_eq("dup2-close[0]", 0, close(fd));

	return NULL;
}

char *testdir(void)
{
	char buf[1024];
	assert_eq("getcwd[0]", 0, strcmp("/", getcwd(buf, sizeof(buf))));

	assert_eq("chdir", 0, chdir("bin"));
	assert_eq("getcwd[1]", 0, strcmp("/bin", getcwd(buf, sizeof(buf))));

	assert_eq("mkdir[0]", 0,
			mkdir("hoge", S_IRGRP | S_IROTH | S_IWOTH));

	int fd = open("hoge", O_RDONLY);
	struct stat st;
	assert_ne("mkdir-open[0]", -1, fd);
	assert_eq("mkdir-fstat[0]", 0, fstat(fd, &st));
	assert_eq("mkdir-fstat[0-1]",
			S_IFDIR | S_IRGRP | S_IROTH,
			st.st_mode);
	assert_eq("mkdir-close[0]", 0, close(fd));

	/* umask */
	assert_eq("chmod", 0, chmod("hoge", 0705));
	fd = open("hoge/mtestfile", O_CREAT | O_WRONLY, 0666);
	assert_ne("umask[0-1]", -1, fd);
	assert_eq("umask[0-2]", 0, fstat(fd, &st));
	printf("fstat: mode=%x\n", st.st_mode);
	assert_eq("umask[0-3]",
			S_IFREG | S_IRUSR | S_IWUSR | S_IROTH,
			st.st_mode);
	assert_eq("umask[0-4]", 0, close(fd));

	assert_eq("mkdir[1]", 0,
			mkdir("hoge/mtestdir", 0777));
	fd = open("hoge/mtestdir", O_RDONLY);
	assert_ne("mkdir-open[1]", -1, fd);
	assert_eq("mkdir-fstat[1]", 0, fstat(fd, &st));
	assert_eq("mkdir-fstat[1-1]",
			S_IFDIR | S_IRUSR | S_IWUSR | S_IXUSR | S_IROTH | S_IXOTH,
			st.st_mode);
	assert_eq("mkdir-close[1]", 0, close(fd));

	return NULL;
}

char *testmm(void)
{
	int pid = fork();
	if (pid == 0) {
		printf("child's pid = %d, ppid = %d\n",
				getpid(), getppid());
		exit(EXIT_SUCCESS);
	} else {
		sleep(1);
		printf("parent's pid = %d, ppid = %d\n",
				getpid(), getppid());
		wait(&pid);
	}

	time_t t1;
	time_t t2 = time(&t1);
	struct timespec ts;
	printf("time = %d, %d\n",
			(int) ((long long int) t1 >> 32),
			(int) ((long long int) t1 & 0xffffffff));
	printf("time = %d, %d\n",
			(int) ((long long int) t2 >> 32),
			(int) ((long long int) t2 & 0xffffffff));
	assert_eq("clock_gettime[0]", 0, clock_gettime(CLOCK_REALTIME, &ts));
	printf("time = %d, %d, %x\n",
			(int) ((long long int) (ts.tv_sec) >> 32),
			(int) ((long long int) (ts.tv_sec) & 0xffffffff),
			(int) (ts.tv_nsec));
	assert_eq("clock_gettime[1]", -1, clock_gettime(CLOCK_MONOTONIC, &ts));

	sleep(2);
	clock_gettime(CLOCK_REALTIME, &ts);
	printf("time = %d, %d, %x\n",
			(int) ((long long int) (ts.tv_sec) >> 32),
			(int) ((long long int) (ts.tv_sec) & 0xffffffff),
			(int) (ts.tv_nsec));

	printf("uid = %d\n", getuid());
	printf("gid = %d\n", getgid());

	char *buf = malloc(1024 * 1024);
	assert_ne("brk", 0, buf);
	for (int i = 0; i < 1024 * 1024; i++)
		buf[i] = '0';

	free(buf);
	return NULL;
}

char *testothers(void)
{
	for (int i = 0; i < 5; i++)
		printf("%d\n", rand());

	srand(1);
	for (int i = 0; i < 5; i++)
		printf("%d\n", rand());

	return NULL;
}

int main(int argc, char **argv)
{
	test(testfile);
	test(testdup);
	test(testdir);
	test(testmm);
	test(testothers);

	return 0;
}
