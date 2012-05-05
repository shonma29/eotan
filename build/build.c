/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001-2003, Tomohide Naniwa

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <elf.h>
#include <itron_module.h>	/* module_info 構造体の参照のため */

#define	MAX_LINE	1000
#define BLOCKSIZE	1024

#ifdef DEBUG
#define DPRINTF(x)	printf x;
#else
#define DPRINTF(x)
#endif

#define ERR_OK 0
#define ERR_PARM (-1)
#define ERR_MEMORY (-2)
#define ERR_FILE (-3)
#define ERR_FORMAT (-4)

struct config_entry {
    char *fname;
    int offset;
    struct module_info mod_info;
};

typedef struct {
	int nmodule;
	struct config_entry entry[MAX_MODULE];
} ModuleTable;

static void usage(const char *cmd);
static void pass0(ModuleTable *modules, const char *config);
static void pass2(ModuleTable *modules, const char *outputfile);


static void usage(const char *cmd)
{
    fprintf(stderr, "Usage: %s configuration-file output-file\n", cmd);
}


int main(const int argc, const char **argv)
{
    int i;
    ModuleTable modules;

    if (argc != 3) {
	usage(argv[0]);
	exit(ERR_PARM);
    }

    /* pass 0 -   configuration file の読み込み 
     *            module_table[] を埋める
     */
    pass0(&modules, argv[1]);
    if (modules.nmodule <= 0) {
	fprintf(stderr, "No module.\n");
	exit(ERR_FORMAT);
    }

    /* pass 2 -   最終的な OS ファイルを出力する。
     */
    pass2(&modules, argv[2]);

    fprintf(stderr, "modinfo: \n");
    for (i = 0; i < modules.nmodule; i++) {
	fprintf(stderr,
		"%02d: %-20.20s type = %d, vaddr = 0x%08x, paddr = 0x%08x, file size = %6d\n",
		i, modules.entry[i].mod_info.name,
		modules.entry[i].mod_info.type,
		modules.entry[i].mod_info.vaddr,
		modules.entry[i].mod_info.paddr,
		modules.entry[i].mod_info.length);
    }

    exit(ERR_OK);
}

/* pass0 - configuration file の読み取り
 *
 *	   configuration file のフォーマットは次のようになっている。
 *	   (各エントリは空白または tab で区切っている)
 *
 *		<モジュールファイル>	<仮想アドレス>	<物理アドレス>	<種別>	<モジュール名>
 *
 */
static void pass0(ModuleTable *modules, const char *config)
{
    FILE *fp;
    char line[MAX_LINE], *p;
    int i, mod_index;
    char *path, *vaddr, *paddr, *type, *mod_name;

    fp = fopen(config, "r");
    if (!fp) {
	perror("fopen");
	exit(ERR_FILE);
    }

    for (i = 0, mod_index = 0; fgets(line, MAX_LINE, fp); i++) {
	struct config_entry *mod;
        struct stat buf;

	if ((line[0] == '#') || (line[0] == '\n')) {
	    continue;
	}

	for (p = line; *p; p++) {
	    if (*p != ' ') {
		break;
	    }
	}
	if (!*p || (*p == '\n')) {
	    /* 空白行だった */
	    continue;
	}

	/* モジュールファイル名の読み取り */
	path = strtok(line, " \t\n");
	vaddr = strtok(NULL, " \t\n");
	paddr = strtok(NULL, " \t\n");
	type = strtok(NULL, " \t\n");
	mod_name = strtok(NULL, " \t\n");

	if (strlen(mod_name) > MAX_MODULE_NAME) {
	    fprintf(stderr, "module name too long in %d\n", i);
	    exit(ERR_FORMAT);
	}

	DPRINTF(
		("mod[%d]: %-30.30s, vaddr = %10.10s, paddr = %10.10s, type = %-8.8s, module = %s\n",
		 mod_index, path, vaddr, paddr, type, mod_name));

	if (!path || !vaddr || !paddr
	    || !type || !mod_name) {
	    fprintf(stderr, "syntax error in %d\n", i);
	    exit(ERR_FORMAT);
	}

	/* module_table を埋める */
	mod = &(modules->entry[mod_index]);
	mod->fname = strdup(path);
	sscanf(vaddr, "0x%x", &(mod->mod_info.vaddr));
	if (vaddr <= 0) {
	    fprintf(stderr, "syntax error in %d\n", i);
	    exit(ERR_FORMAT);
	}

	sscanf(paddr, "0x%x", &(mod->mod_info.paddr));
	if (paddr <= 0) {
	    fprintf(stderr, "syntax error in %d\n", i);
	    exit(ERR_FORMAT);
	}

	if (!strcmp(type, "kernel")) {
	    mod->mod_info.type = kernel;
	} else if (!strcmp(type, "user")) {
	    mod->mod_info.type = user;
	} else if (!strcmp(type, "lowlib")) {
	    mod->mod_info.type = lowlib;
	} else if (!strcmp(type, "server")) {
	    mod->mod_info.type = server;
	} else if (!strcmp(type, "driver")) {
	    mod->mod_info.type = driver;
	} else {
	    fprintf(stderr, "Unknown module type: %s in %d\n", type, i);
	    exit(ERR_FORMAT);
	}
	strncpy(mod->mod_info.name, mod_name,
		MAX_MODULE_NAME);

	/* get file size */
	if (stat(mod->fname, &buf)) {
	    perror("stat");
	    exit(ERR_FILE);
	}

	mod->mod_info.length = buf.st_size;

	DPRINTF(
		("mod[%d]: %-30.30s, vaddr = 0x%08.08x, paddr = 0x%08.08x, type = %d, module = %s\n",
		 mod_index, mod->fname,
		 mod->mod_info.vaddr,
		 mod->mod_info.paddr,
		 mod->mod_info.type,
		 mod->mod_info.name));

	mod_index++;
    }

    modules->nmodule = mod_index;
}

/* pass2 - OS ファイルの出力
 */
static void pass2(ModuleTable *modules, const char *outputfile)
{
    int i;
    FILE *fp, *outfp;
    struct boot_header boot;

    outfp = fopen(outputfile, "w");
    if (!outfp) {
	perror("fopen");
	exit(ERR_FILE);
    }

    bzero(&boot, sizeof(boot));
    boot.cookie = 1;
    boot.count = modules->nmodule;
    boot.machine.rootfs = 0xffffffff;

    /* ヘッダの出力 */
    if (fwrite(&boot, sizeof(boot), 1, outfp) != 1) {
	perror("fwrite");
	fclose(outfp);
    }

    for (i = 0; i < modules->nmodule; i++) {
	if (fwrite
	    (&(modules->entry[i].mod_info), sizeof(struct module_info), 1,
	     outfp) != 1) {
	    perror("fwrite");
	    fclose(outfp);
	    exit(ERR_FILE);
	}
    }

    fseek(outfp, BLOCKSIZE, SEEK_SET);

    /* モジュールひとつひとつの出力 */
    for (i = 0; i < modules->nmodule; i++) {
	int len;

	fp = fopen(modules->entry[i].fname, "r");
	if (!fp) {
	    perror("fopen");
	    fclose(outfp);
	    exit(ERR_FILE);
	}

	/* append ELF */
	for (len = modules->entry[i].mod_info.length; len > 0; len--) {
		fputc(fgetc(fp), outfp);
	}

	fclose(fp);
    }

    fclose(outfp);
}
