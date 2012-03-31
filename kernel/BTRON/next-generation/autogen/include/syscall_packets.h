#ifndef __BTRON_SYSCALL_PACKETS_H__
#define __BTRON_SYSCALL_PACKETS_H__

/* gdsp_ptr */
struct bsc_gdsp_ptr {
  WORD req;
};

/* gget_ptr */
struct bsc_gget_ptr {
  PTRSTS *sts;
  PTRIMAGE *img;
};

/* gini_ptr */
struct bsc_gini_ptr {
  void;
};

/* gmot_ptr */
struct bsc_gmot_ptr {
  WORD sts;
};

/* gmov_ptr */
struct bsc_gmov_ptr {
  POINT pos;
};

/* gset_ptr */
struct bsc_gset_ptr {
  WORD style;
  PTRIMAGE *img;
  COLORVAL fgcol;
  COLORVAL bgcol;
};

/* gcls_env */
struct bsc_gcls_env {
  WORD gid;
};

/* gcnv_abs */
struct bsc_gcnv_abs {
  WORD gid;
  PPTR pp;
};

/* gcnv_col */
struct bsc_gcnv_col {
  WORD gid;
  COLORVAL cv;
  LPTR pixv;
};

/* gcom_bmp */
struct bsc_gcom_bmp {
  WORD gid;
  RPTR dr;
  C_BMPTR cbmp;
  LPTR stat;
};

/* gcop_bmp */
struct bsc_gcop_bmp {
  WORD srcid;
  RPTR sr;
  WORD dstid;
  RPTR dr;
  GSCAN *mask;
  WORD mode;
};

/* gcre_pat */
struct bsc_gcre_pat {
  WORD gid;
  PATPTR def;
  BPTR loc;
  BPTR *pat;
  LPTR size;
};

/* gdra_arc */
struct bsc_gdra_arc {
  WORD gid;
  RECT r;
  POINT sp;
  POINT ep;
  UWORD attr;
  PATPTR pat;
  WORD angle;
  WORD mode;
};

/* gdra_lin */
struct bsc_gdra_lin {
  WORD gid;
  POINT p1;
  POINT p2;
  UWORD attr;
  PATPTR pat;
  WORD mode;
};

/* gdra_pln */
struct bsc_gdra_pln {
  WORD gid;
  POLYGON *p;
  UWORD attr;
  PATPTR pat;
  WORD mode;
};

/* gdra_pnt */
struct bsc_gdra_pnt {
  WORD gid;
  POINT p;
  LONG val;
  WORD mode;
};

/* gdra_spl */
struct bsc_gdra_spl {
  WORD gid;
  WORD np;
  PPTR pt;
  UWORD attr;
  PATPTR pat;
  WORD mode;
};

/* gfil_cho */
struct bsc_gfil_cho {
  WORD gid;
  RECT r;
  POINT sp;
  POINT ep;
  PATPTR pat;
  WORD angle;
  WORD mode;
};

/* gfil_ovl */
struct bsc_gfil_ovl {
  WORD gid;
  RECT r;
  PATPTR pat;
  WORD angle;
  WORD mode;
};

/* gfil_pnt */
struct bsc_gfil_pnt {
  WORD gid;
  POINT p;
  PATPTR pat;
  WORD mode;
};

/* gfil_pol */
struct bsc_gfil_pol {
  WORD gid;
  POLYGON *p;
  PATPTR pat;
  WORD mode;
};

/* gfil_rec */
struct bsc_gfil_rec {
  WORD gid;
  RECT r;
  PATPTR pat;
  WORD angle;
  WORD mode;
};

/* gfil_rgn */
struct bsc_gfil_rgn {
  WORD gid;
  LONG val;
  POINT p;
  PATPTR pat;
  WORD mode;
};

/* gfil_rrc */
struct bsc_gfil_rrc {
  WORD gid;
  RECT r;
  WORD rh;
  WORD rv;
  PATPTR pat;
  WORD angle;
  WORD mode;
};

/* gfil_scn */
struct bsc_gfil_scn {
  WORD gid;
  GSCAN *scan;
  PATPTR pat;
  WORD mode;
};

/* gfil_sec */
struct bsc_gfil_sec {
  WORD gid;
  RECT r;
  POINT sp;
  POINT ep;
  PATPTR pat;
  WORD angle;
  WORD mode;
};

/* gfra_cho */
struct bsc_gfra_cho {
  WORD gid;
  RECT r;
  POINT sp;
  POINT ep;
  UWORD attr;
  PATPTR pat;
  WORD angle;
  WORD mode;
};

/* gfra_ovl */
struct bsc_gfra_ovl {
  WORD gid;
  RECT r;
  UWORD attr;
  PATPTR pat;
  WORD angle;
  WORD mode;
};

/* gfra_pol */
struct bsc_gfra_pol {
  WORD gid;
  POLYGON *p;
  UWORD attr;
  PATPTR pat;
  WORD mode;
};

/* gfra_rec */
struct bsc_gfra_rec {
  WORD gid;
  RECT r;
  UWORD attr;
  PATPTR pat;
  WORD angle;
  WORD mode;
};

/* gfra_rrc */
struct bsc_gfra_rrc {
  WORD gid;
  RECT r;
  WORD rh;
  WORD rv;
  UWORD attr;
  PATPTR pat;
  WORD angle;
  WORD mode;
};

/* gfra_sec */
struct bsc_gfra_sec {
  WORD gid;
  RECT r;
  POINT sp;
  POINT ep;
  UWORD attr;
  PATPTR pat;
  WORD angle;
  WORD mode;
};

/* gget_bmp */
struct bsc_gget_bmp {
  WORD gid;
  BMPTR bmap;
};

/* gget_bnd */
struct bsc_gget_bnd {
  WORD gid;
  RPTR rp;
};

/* gget_col */
struct bsc_gget_col {
  WORD gid;
  WORD p;
  COLORVAL *cv;
  WORD cnt;
};

/* gget_dev */
struct bsc_gget_dev {
  WORD gid;
  TPTR dev;
};

/* gget_for */
struct bsc_gget_for {
  WORD gid;
  RLPTR rlp;
};

/* gget_fra */
struct bsc_gget_fra {
  WORD gid;
  RPTR rp;
};

/* gget_msk */
struct bsc_gget_msk {
  WORD gid;
  LPTR pixmask;
};

/* gget_spc */
struct bsc_gget_spc {
  TPTR dev;
  DEV_SPEC *buf;
};

/* gget_vis */
struct bsc_gget_vis {
  WORD gid;
  RPTR rp;
};

/* gini_env */
struct bsc_gini_env {
  WORD gid;
};

/* gloc_env */
struct bsc_gloc_env {
  WORD gid;
  WORD lock;
};

/* gmov_bnd */
struct bsc_gmov_bnd {
  WORD gid;
  WORD dh;
  WORD dv;
};

/* gmov_cor */
struct bsc_gmov_cor {
  WORD gid;
  WORD dh;
  WORD dv;
};

/* gnew_env */
struct bsc_gnew_env {
  WORD gid;
};

/* gopn_dev */
struct bsc_gopn_dev {
  TPTR dev;
  BPTR param;
};

/* gopn_mem */
struct bsc_gopn_mem {
  TPTR dev;
  BMPTR bmap;
  BPTR param;
};

/* gpic_arc */
struct bsc_gpic_arc {
  POINT pt;
  RECT r;
  POINT sp;
  POINT ep;
  WORD angle;
  WORD width;
};

/* gpic_cho */
struct bsc_gpic_cho {
  POINT pt;
  RECT r;
  POINT sp;
  POINT ep;
  WORD angle;
  WORD width;
};

/* gpic_lin */
struct bsc_gpic_lin {
  POINT pt;
  POINT p0;
  POINT p1;
  WORD width;
};

/* gpic_ovl */
struct bsc_gpic_ovl {
  POINT pt;
  RECT r;
  WORD angle;
  WORD width;
};

/* gpic_pln */
struct bsc_gpic_pln {
  POINT pt;
  POLYGON *poly;
  WORD width;
};

/* gpic_pol */
struct bsc_gpic_pol {
  POINT pt;
  POLYGON *poly;
  WORD width;
};

/* gpic_rec */
struct bsc_gpic_rec {
  POINT pt;
  RECT r;
  WORD angle;
  WORD width;
};

/* gpic_rrc */
struct bsc_gpic_rrc {
  POINT pt;
  RECT r;
  WORD rh;
  WORD rv;
  WORD angle;
  WORD width;
};

/* gpic_scn */
struct bsc_gpic_scn {
  POINT pt;
  GSCAN *scan;
};

/* gpic_sec */
struct bsc_gpic_sec {
  POINT pt;
  RECT r;
  POINT sp;
  POINT ep;
  WORD angle;
  WORD width;
};

/* gpic_spl */
struct bsc_gpic_spl {
  POINT p;
  WORD np;
  PPTR pt;
  WORD width;
};

/* grot_bmp */
struct bsc_grot_bmp {
  WORD srcid;
  WORD dstid;
  RPTR rp;
  PPTR dp;
  WORD angle;
  GSCAN *mask;
  WORD mode;
};

/* grst_bmp */
struct bsc_grst_bmp {
  WORD gid;
  RPTR gr;
  BMPTR bmap;
  RPTR br;
  GSCAN *mask;
  WORD mode;
};

/* gsav_bmp */
struct bsc_gsav_bmp {
  WORD gid;
  RPTR gr;
  BMPTR bmap;
  RPTR br;
  GSCAN *mask;
  WORD mode;
};

/* gscr_rec */
struct bsc_gscr_rec {
  WORD gid;
  RECT r;
  WORD dh;
  dv;
  RLPTR rlp;
  PATPTR pat;
};

/* gset_col */
struct bsc_gset_col {
  WORD gid;
  WORD p;
  COLORVAL *cv;
  WORD cnt;
};

/* gset_for */
struct bsc_gset_for {
  WORD gid;
  RLPTR rlp;
};

/* gset_fra */
struct bsc_gset_fra {
  WORD gid;
  RECT r;
};

/* gset_lmk */
struct bsc_gset_lmk {
  WORD gid;
  WORD nbytes;
  BPTR mask;
};

/* gset_msk */
struct bsc_gset_msk {
  WORD gid;
  LONG pixmask;
};

/* gset_vis */
struct bsc_gset_vis {
  WORD gid;
  RECT r;
};

/* gsiz_cbm */
struct bsc_gsiz_cbm {
  WORD gid;
  RPTR rp;
  WORD compac;
  LPTR size;
};

/* gsiz_pat */
struct bsc_gsiz_pat {
  WORD gid;
  WORD kind;
  WORD hsize;
  WORD vsize;
  LPTR size;
};

/* gtst_pnt */
struct bsc_gtst_pnt {
  WORD gid;
  POINT p;
  LPTR val;
};

/* gunc_bmp */
struct bsc_gunc_bmp {
  WORD gid;
  RPTR dr;
  C_BMPTR cbmp;
  RPTR sr;
  WORD mode;
};

/* gcop_chr */
struct bsc_gcop_chr {
  WORD gid;
  POINT asize;
  POINT isize;
  BPTR img;
  WORD mode;
};

/* gdra_chp */
struct bsc_gdra_chp {
  WORD gid;
  WORD h;
  WORD v;
  TCODE ch;
  WORD mode;
};

/* gdra_chr */
struct bsc_gdra_chr {
  WORD gid;
  TCODE ch;
  WORD mode;
};

/* gdra_stp */
struct bsc_gdra_stp {
  WORD gid;
  WORD h;
  WORD v;
  TPTR str;
  WORD len;
  WORD mode;
};

/* gdra_str */
struct bsc_gdra_str {
  WORD gid;
  TPTR str;
  WORD len;
  WORD mode;
};

/* gget_chc */
struct bsc_gget_chc {
  WORD gid;
  COLORVAL *chfgc;
  COLORVAL *chbgc;
};

/* gget_chh */
struct bsc_gget_chh {
  WORD gid;
  TCODE ch;
};

/* gget_chp */
struct bsc_gget_chp {
  WORD gid;
  WPTR h;
  WPTR v;
};

/* gget_chw */
struct bsc_gget_chw {
  WORD gid;
  TCODE ch;
};

/* gget_fnt */
struct bsc_gget_fnt {
  WORD gid;
  FONTSPEC *fnt;
  FONTINF *inf;
};

/* gget_sth */
struct bsc_gget_sth {
  WORD gid;
  TPTR str;
  WORD len;
  CHGAP *cap;
  WPTR pos;
};

/* gget_stw */
struct bsc_gget_stw {
  WORD gid;
  TPTR str;
  WORD len;
  CHGAP *gap;
  WPTR pos;
};

/* gset_chc */
struct bsc_gset_chc {
  WORD gid;
  COLORVAL chfgc;
  COLORVAL chbgc;
};

/* gset_chp */
struct bsc_gset_chp {
  WORD gid;
  WORD h;
  WORD v;
  WORD abs;
};

/* gset_fnt */
struct bsc_gset_fnt {
  WORD gid;
  FONTSPEC *fnt;
};

/* chg_dmd */
struct bsc_chg_dmd {
  TPTR dev;
  UWORD mode;
};

/* cls_dev */
struct bsc_cls_dev {
  WORD dd;
  WORD eject;
  WPTR error;
};

/* ctl_dev */
struct bsc_ctl_dev {
  WORD dd;
  UWORD cmd;
  BPTR param;
  WPTR error;
};

/* dev_sts */
struct bsc_dev_sts {
  TPTR dev;
  DEV_STATE *buf;
};

/* get_dev */
struct bsc_get_dev {
  TPTR dev;
  WORD num;
};

/* lst_dev */
struct bsc_lst_dev {
  DEV_INFO *dev;
  WORD ndev;
};

/* opn_dev */
struct bsc_opn_dev {
  TPTR dev;
  UWORD o_mode;
  WPTR error;
};

/* rea_dev */
struct bsc_rea_dev {
  WORD dd;
  LONG start;
  BPTR buf;
  LONG size;
  LPTR a_size;
  WPTR error;
};

/* wri_dev */
struct bsc_wri_dev {
  WORD dd;
  LONG start;
  BPTR buf;
  LONG size;
  LPTR a_size;
  WPTR error;
};

/* chg_emk */
struct bsc_chg_emk {
  WORD mask;
};

/* chg_pda */
struct bsc_chg_pda {
  WORD attr;
};

/* clr_evt */
struct bsc_clr_evt {
  WORD t_mask;
  WORD lask_mask;
};

/* get_etm */
struct bsc_get_etm {
  ULPTR time;
};

/* get_evt */
struct bsc_get_evt {
  WORD t_mask;
  EVENT *evt;
  UWORD opt;
};

/* get_kid */
struct bsc_get_kid {
  KBD_ID *id;
};

/* get_kmp */
struct bsc_get_kmp {
  KEYMAP keymap;
};

/* get_krm */
struct bsc_get_krm {
  KEPMAP keymap;
};

/* get_krp */
struct bsc_get_krp {
  WPTR offset;
  WPTR interval;
};

/* get_ktb */
struct bsc_get_ktb {
  KEYTAB *keytab;
};

/* get_pdp */
struct bsc_get_pdp {
  POINT *pos;
};

/* put_evt */
struct bsc_put_evt {
  EVENT *evt;
  UWORD opt;
};

/* set_krm */
struct bsc_set_krm {
  KEYMAP keymap;
};

/* set_krp */
struct bsc_set_krp {
  WORD offset;
  WORD interval;
};

/* set_ktb */
struct bsc_set_ktb {
  KEYTAB *keytab;
};

/* set_pdp */
struct bsc_set_pdp {
  POINT pos;
};

/* sig_buz */
struct bsc_sig_buz {
  UWORD type;
};

/* apd_rec */
struct bsc_apd_rec {
  WORD fd;
  BPTR buf;
  LONG size;
  WORD type;
  UWORD subtype;
  UWORD units;
};

/* att_fls */
struct bsc_att_fls {
  TPTR dev;
  TPTR name;
  LINKPTR lnk;
  WORD r_only;
};

/* chg_fat */
struct bsc_chg_fat {
  LINKPTR lnk;
  WORD attr;
};

/* chg_fls */
struct bsc_chg_fls {
  TPTR dev;
  TPTR fs_name;
  TPTR fs_locate;
};

/* chg_fmd */
struct bsc_chg_fmd {
  LINKPTR lnk;
  A_MODE *mode;
};

/* chg_fnm */
struct bsc_chg_fnm {
  LINKPTR lnk;
  TPTR name;
};

/* chg_ftm */
struct bsc_chg_ftm {
  LINKPTR lnk;
  F_TIME *times;
};

/* chg_wrk */
struct bsc_chg_wrk {
  LINKPTR lnk;
};

/* chk_fil */
struct bsc_chk_fil {
  LINKPTR lnk;
  UWORD mode;
  TPTR pwd;
};

/* cls_fil */
struct bsc_cls_fil {
  WORD fd;
};

/* cre_fil */
struct bsc_cre_fil {
  LINKPTR lnk;
  TPTR name;
  A_MODE *mode;
  UWORD atype;
  WORD opt;
};

/* cre_lnk */
struct bsc_cre_lnk {
  LINKPTR lnk;
  F_LINK *ref;
  WORD opt;
};

/* del_fil */
struct bsc_del_fil {
  LINKPTR org;
  LINKPTR lnk;
  WORD force;
};

/* del_fls */
struct bsc_del_fls {
  TPTR dev;
  WORD eject;
};

/* del_rec */
struct bsc_del_rec {
  WORD fd;
};

/* fil_sts */
struct bsc_fil_sts {
  LINKPTR lnk;
  TPTR name;
  F_STATE *stat;
  F_LOCATE *locat;
};

/* fls_sts */
struct bsc_fls_sts {
  TPTR dev;
  FS_STATE *buff;
};

/* fnd_lnk */
struct bsc_fnd_lnk {
  WORD fd;
  WORD mode;
  LINKPTR lnk;
  UWORD subtype;
  LPTR recnum;
};

/* fnd_rec */
struct bsc_fnd_rec {
  WORD fd;
  WORD mode;
  ULONG typemask;
  UWORD subtype;
  LPTR recnum;
};

/* gen_fil */
struct bsc_gen_fil {
  LINKPTR lnk;
  F_LINK *ref;
  WORD opt;
};

/* get_dfm */
struct bsc_get_dfm {
  DA_MODE *mode;
};

/* get_lnk */
struct bsc_get_lnk {
  TPTR path;
  LINKPTR lnk;
  WORD mode;
};

/* get_nlk */
struct bsc_get_nlk {
  LINKPTR lnk;
};

/* ins_rec */
struct bsc_ins_rec {
  WORD fd;
  BPTR buf;
  LONG size;
  WORD type;
  UWORD subtype;
  UWORD units;
};

/* lnk_sts */
struct bsc_lnk_sts {
  LINKPTR lnk;
  F_LINK *stat;
};

/* loc_rec */
struct bsc_loc_rec {
  WORD fd;
  WORD mode;
};

/* lst_fls */
struct bsc_lst_fls {
  F_ATTACH *buff;
  WORD cnt;
};

/* ofl_sts */
struct bsc_ofl_sts {
  WORD fd;
  TPTR name;
  F_STATE *stat;
  F_LOCATE *locat;
};

/* opn_fil */
struct bsc_opn_fil {
  LINKPTR lnk;
  UWORD o_mode;
  TPTR pwd;
};

/* rea_rec */
struct bsc_rea_rec {
  WORD fd;
  WORD offset;
  BPTR buf;
  LONG size;
  LPTR r_size;
  UWPTR subtype;
};

/* see_rec */
struct bsc_see_rec {
  WORD fd;
  LONG offset;
  WORD mode;
  LPTR recnum;
};

/* set_dfm */
struct bsc_set_dfm {
  DA_MODE *mode;
};

/* syn_fls */
struct bsc_syn_fls {
  void;
};

/* syn_lnk */
struct bsc_syn_lnk {
  LINKPTR lnk;
  WORD opt;
};

/* trc_rec */
struct bsc_trc_rec {
  WORD fd;
  LONG size;
};

/* wri_rec */
struct bsc_wri_rec {
  WORD fd;
  LONG offset;
  BPTR buf;
  LONG size;
  LPTR r_size;
  UWPTR subtype;
  UWORD units;
};

/* xch_fil */
struct bsc_xch_fil {
  WORD fd_1;
  WORD fd_2;
};

/* apd_mem */
struct bsc_apd_mem {
  LONG phaddr;
  LONG size;
};

/* cre_mpl */
struct bsc_cre_mpl {
  WORD mpl_id;
  LONG size;
  UWORD attr;
};

/* del_mpl */
struct bsc_del_mpl {
  WORD mpl_id;
};

/* get_lmb */
struct bsc_get_lmb {
  BPTR *mptr;
  LONG size;
  UWORD opt;
};

/* get_ptr */
struct bsc_get_ptr {
  BPTR sptr;
  LONG offset;
  BPTR *mptr;
  LONG size;
};

/* get_sma */
struct bsc_get_sma {
  WORD akey;
};

/* get_smb */
struct bsc_get_smb {
  WORD mpl_id;
  LONG size;
  UWORD opt;
};

/* get_smi */
struct bsc_get_smi {
  WORD akey;
  LPTR info;
};

/* lmb_siz */
struct bsc_lmb_siz {
  BPTR mptr;
  LPTR size;
};

/* lmb_sts */
struct bsc_lmb_sts {
  LM_STATE *buff;
};

/* map_mem */
struct bsc_map_mem {
  LONG phaddr;
  BPTR *logaddr;
  LONG size;
  UWORD opt;
};

/* mpl_sts */
struct bsc_mpl_sts {
  WORD mpl_id;
  MP_STATE *buff;
};

/* rea_smb */
struct bsc_rea_smb {
  WORD akey;
  BPTR buff;
  LONG offset;
  LONG size;
  LPTR a_size;
};

/* rel_lmb */
struct bsc_rel_lmb {
  BPTR mptr;
};

/* rel_ptr */
struct bsc_rel_ptr {
  BPTR mptr;
};

/* rel_smb */
struct bsc_rel_smb {
  WORD akey;
};

/* rgt_smb */
struct bsc_rgt_smb {
  WORD akey;
};

/* rsz_lmb */
struct bsc_rsz_lmb {
  BPTR *n_mptr;
  BPTR mptr;
  LONG size;
  UWORD opt;
};

/* set_sma */
struct bsc_set_sma {
  WORD akey;
  UWORD attr;
};

/* set_smi */
struct bsc_set_smi {
  WORD akey;
  UWORD attr;
};

/* smb_adr */
struct bsc_smb_adr {
  WORD akey;
  BPTR *maddr;
};

/* smb_key */
struct bsc_smb_key {
  BPTR maddr;
};

/* smb_sts */
struct bsc_smb_sts {
  WORD akey;
  SM_STATE *buff;
};

/* ump_mem */
struct bsc_ump_mem {
  BPTR logaddr;
};

/* wri_smb */
struct bsc_wri_smb {
  WORD akey;
  BPTR buff;
  LONG offset;
  LONG size;
  LPTR a_size;
};

/* can_tmg */
struct bsc_can_tmg {
  void;
};

/* chg_pri */
struct bsc_chg_pri {
  WORD pid;
  WORD new_pri;
  UWORD opt;
};

/* chg_usr */
struct bsc_chg_usr {
  P_USER *buff;
};

/* clr_msg */
struct bsc_clr_msg {
  LONG t_mask;
  LONG last_mask;
};

/* cre_nam */
struct bsc_cre_nam {
  TPTR name;
  LONG data;
  UWORD opt;
};

/* cre_prc */
struct bsc_cre_prc {
  LINKPTR lnk;
  WORD pri;
  MESSAGE *msg;
};

/* cre_sem */
struct bsc_cre_sem {
  WORD sem_id;
  WORD cnt;
  UWORD opt;
};

/* def_msg */
struct bsc_def_msg {
  LONG t_mask;
  FUNCP msg_hdr;
};

/* del_nam */
struct bsc_del_nam {
  TPTR name;
};

/* del_sem */
struct bsc_del_sem {
  WORD sem_id;
};

/* ext_prc */
struct bsc_ext_prc {
  WORD exit_code;
};

/* get_inf */
struct bsc_get_inf {
  WORD pid;
  P_INFO *buff;
};

/* get_nam */
struct bsc_get_nam {
  TPTR name;
  LPTR data;
};

/* get_usr */
struct bsc_get_usr {
  WORD pid;
  P_USER *buff;
};

/* ini_sem */
struct bsc_ini_sem {
  WORD sem_id;
  WORD cnt;
};

/* loc_prc */
struct bsc_loc_prc {
  WORD onoff;
};

/* prc_sts */
struct bsc_prc_sts {
  WORD pid;
  P_STATE *buff;
  TPTR path;
};

/* rcr_prc */
struct bsc_rcr_prc {
  LINKPTR lnk;
  MESSAGE *msg;
};

/* rcv_msg */
struct bsc_rcv_msg {
  WORD t_mask;
  MESSAGE *msg;
  WORD msgsz;
  UWORD opt;
};

/* req_tmg */
struct bsc_req_tmg {
  LONG time;
  WORD code;
};

/* ret_msg */
struct bsc_ret_msg {
  WORD ret;
};

/* sig_sem */
struct bsc_sig_sem {
  WORD sem_id;
  WORD cnt;
};

/* snd_msg */
struct bsc_snd_msg {
  WORD pid;
  MESSAGE *msg;
  UWORD opt;
};

/* snr_msg */
struct bsc_snr_msg {
  WORD pid;
  MESSAGE *s_msg;
  WORD t_mask;
  MESSAGE *r_msg;
  WORD msgsz;
};

/* ter_prc */
struct bsc_ter_prc {
  WORD pid;
  WORD abort_code;
  UWORD opt;
};

/* wai_prc */
struct bsc_wai_prc {
  LONG time;
};

/* wai_sem */
struct bsc_wai_sem {
  WORD sem_id;
};

/* cre_dbg */
struct bsc_cre_dbg {
  LINKPTR lnk;
  WORD pri;
  MESSAGE arg;
  WORD dpid;
};

/* def_cup */
struct bsc_def_cup {
  FUNCP clup_hdr;
};

/* def_exc */
struct bsc_def_exc {
  WORD exc_vec;
  FUNCP exc_hdr;
};

/* def_svc */
struct bsc_def_svc {
  UWORD svc_code;
  FUNCP *svc_hdrs;
  WORD opt;
};

/* get_err */
struct bsc_get_err {
  void;
};

/* get_ver */
struct bsc_get_ver {
  T_VER *version;
};

/* lod_spg */
struct bsc_lod_spg {
  LINKPTR lnk;
  TPTR arg;
  UWORD attr;
  LPTR info;
};

/* prc_trc */
struct bsc_prc_trc {
  TRACE trace;
};

/* ret_exc */
struct bsc_ret_exc {
  WORD ret;
};

/* unl_spg */
struct bsc_unl_spg {
  WORD prog_id;
};

/* get_tim */
struct bsc_get_tim {
  LPTR time;
  TIMEZONE *tz;
};

/* get_tod */
struct bsc_get_tod {
  DATE_TIM *date_tim;
  LPTR time;
  WORD local;
};

/* get_tod */
struct bsc_get_tod {
  DATE_TIM *date_tim;
  LONG time;
  WORD local;
};

/* set_tim */
struct bsc_set_tim {
  LONG time;
  TIMEZONE *tz;
};

/* mchg_atr */
struct bsc_mchg_atr {
  WORD mid;
  WORD selnum;
  UWORD mode;
};

/* mchg_dsp */
struct bsc_mchg_dsp {
  MENUDISP *attr;
  WORD posattr;
};

/* mchg_dtm */
struct bsc_mchg_dtm {
  WORD time;
};

/* mchg_gat */
struct bsc_mchg_gat {
  WORD mid;
  WORD num;
  UWORD mode;
};

/* mcre_gmn */
struct bsc_mcre_gmn {
  GMENU *gm;
};

/* mcre_men */
struct bsc_mcre_men {
  WORD nitem;
  MENUITEM *item;
  MENUDISP *attr;
};

/* mdel_gmn */
struct bsc_mdel_gmn {
  WORD mid;
};

/* mdel_men */
struct bsc_mdel_men {
  WORD mid;
};

/* mfnd_key */
struct bsc_mfnd_key {
  WORD mid;
  TCODE ch;
};

/* mget_itm */
struct bsc_mget_itm {
  WORD mid;
  WORD pnum;
  MENUITEM *item;
};

/* mopn_gmn */
struct bsc_mopn_gmn {
  WORD dnum;
};

/* mopn_men */
struct bsc_mopn_men {
  WORD dnum;
};

/* msel_gmn */
struct bsc_msel_gmn {
  WORD mid;
  POINT pos;
};

/* msel_men */
struct bsc_msel_men {
  WORD mid;
  POINT pos;
};

/* mset_itm */
struct bsc_mset_itm {
  WORD mid;
  WORD pnum;
  MENUITEM *item;
};

/* wchg_dck */
struct bsc_wchg_dck {
  WORD time;
};

/* wchg_dsp */
struct bsc_wchg_dsp {
  WDDISP *atr;
  PATPTR bgpat;
};

/* wchg_ful */
struct bsc_wchg_ful {
  void;
};

/* wchg_wnd */
struct bsc_wchg_wnd {
  WORD wid;
  RPTR r;
  WORD mode;
};

/* wchk_dck */
struct bsc_wchk_dck {
  ULONG first;
};

/* wchk_dsp */
struct bsc_wchk_dsp {
  WORD wid;
};

/* wcls_wnd */
struct bsc_wcls_wnd {
  WORD wid;
  WORD opt;
};

/* wdef_fep */
struct bsc_wdef_fep {
  WORD onoff;
};

/* wend_drg */
struct bsc_wend_drg {
  void;
};

/* wend_dsp */
struct bsc_wend_dsp {
  WORD wid;
};

/* wera_wnd */
struct bsc_wera_wnd {
  WORD wid;
  RPTR r;
};

/* wexe_dmn */
struct bsc_wexe_dmn {
  WORD item;
};

/* wfnd_wnd */
struct bsc_wfnd_wnd {
  PPTR gpos;
  PPTR lpos;
  WPTR wid;
};

/* wget_act */
struct bsc_wget_act {
  WPTR pid;
};

/* wget_bar */
struct bsc_wget_bar {
  WORD wid;
  WPTR rbar;
  WPTR bbar;
  WPTR lbar;
};

/* wget_bgp */
struct bsc_wget_bgp {
  WORD wid;
  PATPTR pat;
  WORD size;
};

/* wget_dat */
struct bsc_wget_dat {
  WORD wid;
  LPTR dat;
};

/* wget_dmn */
struct bsc_wget_dmn {
  TPTR *dmenu;
};

/* wget_drg */
struct bsc_wget_drg {
  PPTR pos;
  WEVENT *evt;
};

/* wget_evt */
struct bsc_wget_evt {
  WEVENT *evt;
  WORD mode;
};

/* wget_gid */
struct bsc_wget_gid {
  WORD wid;
};

/* wget_org */
struct bsc_wget_org {
  WORD wid;
  WPTR parent;
  RPTR org;
};

/* wget_sts */
struct bsc_wget_sts {
  WORD wid;
  WDSTAT *stat;
  WDDISP *atr;
};

/* wget_tit */
struct bsc_wget_tit {
  WORD wid;
  WPTR pict;
  TPTR title;
};

/* wget_wrk */
struct bsc_wget_wrk {
  WORD wid;
  RPTR r;
};

/* wlst_wnd */
struct bsc_wlst_wnd {
  WORD wid;
  WORD size;
  WPTR wids;
};

/* wmov_drg */
struct bsc_wmov_drg {
  WEVENT *evt;
  RPTR new;
};

/* wmov_wnd */
struct bsc_wmov_wnd {
  WORD wid;
  RPTR new;
};

/* wopn_iwd */
struct bsc_wopn_iwd {
  WORD gid;
};

/* wopn_pwd */
struct bsc_wopn_pwd {
  RPTR r;
};

/* wpas_evt */
struct bsc_wpas_evt {
  WEVENT *evt;
};

/* wreq_dsp */
struct bsc_wreq_dsp {
  WORD wid;
};

/* wrsp_evt */
struct bsc_wrsp_evt {
  WEVENT *evt;
  WORD nak;
};

/* wrsz_drg */
struct bsc_wrsz_drg {
  WEVENT *evt;
  RPTR limit;
  RPTR new;
};

/* wrsz_wnd */
struct bsc_wrsz_wnd {
  WORD wid;
  RPTR new;
};

/* wscr_wnd */
struct bsc_wscr_wnd {
  WORD wid;
  RPTR r;
  WORD dh;
  WORD dv;
  WORD mode;
};

/* wset_bgp */
struct bsc_wset_bgp {
  WORD wid;
  PATPTR pat;
};

/* wset_dat */
struct bsc_wset_dat {
  WORD wid;
  LONG dat;
};

/* wset_org */
struct bsc_wset_org {
  WORD wid;
  WORD parent;
  RPTR org;
};

/* wset_tit */
struct bsc_wset_tit {
  WORD wid;
  WORD pict;
  TPTR title;
  WORD mode;
};

/* wset_wrk */
struct bsc_wset_wrk {
  WORD wid;
  RPTR r;
};

/* wsnd_evt */
struct bsc_wsnd_evt {
  WEVENT *evt;
};

/* wsta_drg */
struct bsc_wsta_drg {
  WORD wid;
  WORD lock;
};

/* wsta_dsp */
struct bsc_wsta_dsp {
  WORD wid;
  RPTR r;
  RLPTR rlst;
};

/* wswi_wnd */
struct bsc_wswi_wnd {
  WORD wid;
  WEVENT *evt;
};

/* wugt_evt */
struct bsc_wugt_evt {
  WEVENT *evt;
};

/* wwai_rsp */
struct bsc_wwai_rsp {
  WEVENT *evt;
  WORD cmd;
  UWORD tmout;
};


#endif /*__BTRON_SYSCALL_PACKETS_H__*/
