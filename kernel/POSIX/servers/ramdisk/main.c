/*

B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001-2002, Tomohide Naniwa

*/

#include "ramdisk.h"

#define START_FROM_INIT

/*********************************************************************
 *	 ����ѿ��������
 *
 */

/*********************************************************************
 *	 �ɽ��ѿ��������
 *
 */
static W mydevid;		/* ��ʬ���Ȥ�id */
static ID recvport;		/* �׵�����Ĥ��ѥݡ��� */
static char *ramdisk;
static W rd_size;

/*
 *	�ɽ�ؿ��������
 */
static void main_loop(void);
static void init_rd_driver(W size);
static void init_rd(W size);
static void doit(DDEV_REQ * packet);

#ifdef START_FROM_INIT
/* ���ߡ��� main �ؿ� */
main()
{
}

#endif
/*
 * �ǥХ����ɥ饤�Ф� main �ؿ�
 *
 * ���δؿ��ϡ��ǥХ����ɥ饤��Ω���夲���˰������¹Ԥ��롣
 *
 */

start(int argc, char *argv[])
{
    W size;
    extern int atoi(char *);

    if (argc < 2) {
	size = RD_SIZE;
    } else {
	size = atoi(argv[1]);
	if (size < 0)
	    size = RD_SIZE;
	else
	    size *= 1024;
    }
    /* 
     * �׵�����ѤΥݡ��Ȥκ����ȥ��ǥ������ν����
     */
    init_rd_driver(size);

    /*
     * Ω���夲��å�����
     */
    dbg_printf("ram disk driver start (size = %d KB)\n", size / 1024);
    dbg_printf("  receive port is %d\n", recvport);

    /*
     * �ɥ饤�Ф��������롣
     */
    main_loop();
}

static void main_loop()
{
    DDEV_REQ req;
    extern ER sys_errno;
    UW rsize;

    /*
     * �׵���� - �����Υ롼��
     */
    for (;;) {
	/* �׵�μ��� */
	get_req(recvport, &req, &rsize);
	switch (sys_errno) {
	case E_OK:
	    /* ���辰���� */
	    doit(&req);
	    break;

	default:
	    /* Unknown error */
	    dbg_printf("rd: get_req() Unknown error(error = %d)\n",
		       sys_errno);
	    dbg_printf("RAM DISK driver is halt.\n");
	    slp_tsk();
	    break;
	}
    }
    /* �����ιԤˤϡ���ʤ� */
}

/*
 *
 */
/************************************************************************
 *
 *
 */
static void doit(DDEV_REQ * packet)
{
    switch (packet->header.msgtyp) {
    case DEV_OPN:
	/* �ǥХ����Υ����ץ� */
	open_rd(packet->header.mbfid, &(packet->body.opn_req));
	break;

    case DEV_CLS:
	/* �ǥХ����Υ����� */
	close_rd(packet->header.mbfid, &(packet->body.cls_req));
	break;

    case DEV_REA:
	read_rd(packet->header.mbfid, &(packet->body.rea_req));
	break;

    case DEV_WRI:
	write_rd(packet->header.mbfid, &(packet->body.wri_req));
	break;

    case DEV_CTL:
	control_rd(packet->header.mbfid, &(packet->body.ctl_req));
	break;
    }
}

/*
 * �����
 *
 * o �ե�����ơ��֥� (file_table) �ν����
 * o �׵�����Ĥ��ѤΥ�å������Хåե� ID ��ݡ��ȥޥ͡��������Ͽ
 */
static void init_rd_driver(W size)
{
    int i;
    ID root_dev;
    ER error;

    /*
     * �׵�����Ĥ��ѤΥݡ��Ȥ��������롣
     */
#ifdef notdef
    recvport = get_port(sizeof(DDEV_RES), sizeof(DDEV_RES));
#else
    recvport = get_port(0, sizeof(DDEV_RES));
#endif
    if (recvport <= 0) {
	dbg_printf("RD: cannot make receive porrt.\n");
	slp_tsk();
	/* ��å������Хåե������˼��� */
    }

    error = regist_port(RD_DRIVER, recvport);
    if (error != E_OK) {
	/* error */
    }

    init_rd(size);
}

/*
 * init_rd --- RAM DISK �ɥ饤�Фν����
 *
 */
void init_rd(W size)
{
#ifdef START_FROM_INIT
    if (brk(VADDR_HEAP + size) < 0) {
	dbg_printf("RD: cannot allocate memory.\n");
	_exit(-1);
    } else {
	ramdisk = (char *) VADDR_HEAP;
    }
#else
    init_malloc(VADDR_HEAP);	/* Ŭ������ */
    ramdisk = (char *) malloc(size);
    if (ramdisk == NULL) {
	dbg_printf("RD: cannot allocate memory.\n");
	slp_tsk();
    }
#endif
    rd_size = size;
}

/************************************************************************
 * open_rd --- RAM DISK �Υ����ץ�
 *
 * ������	dd	�ɥ饤���ֹ�
 *		o_mode	�����ץ�⡼��
 *		error	���顼�ֹ�
 *
 * ���͡�	
 *
 *
 */
W open_rd(ID caller, DDEV_OPN_REQ * packet)
{
    DDEV_RES res;

    res.body.opn_res.dd = packet->dd;
    res.body.opn_res.size = rd_size;
    res.body.opn_res.errcd = E_OK;
    res.body.opn_res.errinfo = E_OK;
    snd_mbf(caller, sizeof(res), &res);
    return (E_OK);
}

/************************************************************************
 * close_rd --- �ɥ饤�ФΥ�����
 *
 * ������	dd	�ɥ饤���ֹ�
 *		o_mode	�����ץ�⡼��
 *		error	���顼�ֹ�
 *
 * ���͡�	
 *
 */
W close_rd(ID caller, DDEV_CLS_REQ * packet)
{
    DDEV_RES res;

    res.body.cls_res.dd = packet->dd;
    res.body.cls_res.errcd = E_OK;
    res.body.cls_res.errinfo = E_OK;
    snd_mbf(caller, sizeof(res), &res);
    return (E_OK);
}

/*************************************************************************
 * read_rd --- 
 *
 * ������	caller	�ƤӽФ����ؤ��������֤�����Υݡ���
 *		packet	�ɤ߹��ߥǡ����Υѥ�᡼��
 *
 * ���͡�	���顼�ֹ�
 *
 * ������	���δؿ��ϡ��ʲ��ν�����Ԥ���
 *
 */
W read_rd(ID caller, DDEV_REA_REQ * packet)
{
    DDEV_RES res;
    W done_length;

    if (packet->start + packet->size >= rd_size)
	done_length = rd_size - packet->start;
    else
	done_length = packet->size;

    bcopy(&ramdisk[packet->start], res.body.rea_res.dt, done_length);
    res.body.rea_res.dd = packet->dd;
    res.body.rea_res.a_size = done_length;
    res.body.rea_res.errcd = E_OK;
    res.body.rea_res.errinfo = E_OK;
    snd_mbf(caller, sizeof(res), &res);
    return (E_OK);
}


/************************************************************************
 *	write_rd
 */
W write_rd(ID caller, DDEV_WRI_REQ * packet)
{
    DDEV_RES res;
    W done_length;

    if (packet->start + packet->size >= rd_size)
	done_length = rd_size - packet->start;
    else
	done_length = packet->size;

    bcopy(packet->dt, &ramdisk[packet->start], done_length);

    res.body.wri_res.dd = packet->dd;
    res.body.wri_res.a_size = done_length;
    res.body.wri_res.errcd = E_OK;
    res.body.wri_res.errinfo = E_OK;
    snd_mbf(caller, sizeof(res), &res);
    return (E_OK);
}

/************************************************************************
 *	control_rd
 */
W control_rd(ID caller, DDEV_CTL_REQ * packet)
{
    DDEV_RES res;
    ER error = E_OK;
    W drive;

    switch (packet->cmd) {
    default:
	error = E_NOSPT;
	break;
    }
    res.body.ctl_res.dd = packet->dd;
    res.body.ctl_res.errcd = error;
    res.body.ctl_res.errinfo = error;
    snd_mbf(caller, sizeof(res), &res);
    return (error);
}
