/*	$Id: html.h,v 1.63 2011/07/12 16:03:19 kristaps Exp $ */
/*
 * Copyright (c) 2009, 2010, 2011 Kristaps Dzonsons <kristaps@bsd.lv>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#ifndef HTML_H
#define HTML_H

#define	LIBHTML_VER	"0.3.3"

__BEGIN_DECLS

/* Used for bit-masks: NOT for indexing! */
enum	hmode {
	HTML_4_01 = 1,	/* HTML-4.01 strict */
	XHTML_1_0 = 2	/* XHTML-1.0 strict */
};

#define	HMODE_SGML	1 /* bit-mask for SGML modes */
#define	HMODE_XML	2 /* bit-mask for XML modes */
#define	HMODE_ALL	3 /* bit-mask for all modes */

enum	hproct {
	HPROC_XML,	/* <?xml?> pic */
	HPROC__MAX
};

enum	hnodet {
	HNODE_COMMENT,	/* comment nodes */
	HNODE_DECL,	/* type declaration */
	HNODE_ELEM,	/* element ("tag") node  */
	HNODE_PROC,	/* processing instruction */
	HNODE_ROOT,	/* root of tree */
	HNODE_TEXT	/* text node */
};

enum	hdeclt {
	HDECL_DOCTYPE,	/* <!DOCTYPE> */
	HDECL__MAX
};

enum	hcreft {
	HCREF_AElig,
	HCREF_Aacute,
	HCREF_Acirc,
	HCREF_Agrave,
	HCREF_Alpha,
	HCREF_Aring,
	HCREF_Atilde,
	HCREF_Auml,
	HCREF_Beta,
	HCREF_Ccedil,
	HCREF_Chi,
	HCREF_Dagger,
	HCREF_Delta,
	HCREF_ETH,
	HCREF_Eacute,
	HCREF_Ecirc,
	HCREF_Egrave,
	HCREF_Epsilon,
	HCREF_Eta,
	HCREF_Euml,
	HCREF_Gamma,
	HCREF_Iacute,
	HCREF_Icirc,
	HCREF_Igrave,
	HCREF_Iota,
	HCREF_Iuml,
	HCREF_Kappa,
	HCREF_Lambda,
	HCREF_Mu,
	HCREF_Ntilde,
	HCREF_Nu,
	HCREF_OElig,
	HCREF_Oacute,
	HCREF_Ocirc,
	HCREF_Ograve,
	HCREF_Omega,
	HCREF_Omicron,
	HCREF_Oslash,
	HCREF_Otilde,
	HCREF_Ouml,
	HCREF_Phi,
	HCREF_Pi,
	HCREF_Prime,
	HCREF_Psi,
	HCREF_Rho,
	HCREF_Scaron,
	HCREF_Sigma,
	HCREF_THORN,
	HCREF_Tau,
	HCREF_Theta,
	HCREF_Uacute,
	HCREF_Ucirc,
	HCREF_Ugrave,
	HCREF_Upsilon,
	HCREF_Uuml,
	HCREF_Xi,
	HCREF_Yacute,
	HCREF_Yuml,
	HCREF_Zeta,
	HCREF_aacute,
	HCREF_acirc,
	HCREF_acute,
	HCREF_aelig,
	HCREF_agrave,
	HCREF_alefsym,
	HCREF_alpha,
	HCREF_amp,
	HCREF_and,
	HCREF_ang,
	HCREF_aring,
	HCREF_asymp,
	HCREF_atilde,
	HCREF_auml,
	HCREF_bdquo,
	HCREF_beta,
	HCREF_brvbar,
	HCREF_bull,
	HCREF_cap,
	HCREF_ccedil,
	HCREF_cedil,
	HCREF_cent,
	HCREF_chi,
	HCREF_circ,
	HCREF_clubs,
	HCREF_cong,
	HCREF_copy,
	HCREF_crarr,
	HCREF_cup,
	HCREF_curren,
	HCREF_dArr,
	HCREF_dagger,
	HCREF_darr,
	HCREF_deg,
	HCREF_delta,
	HCREF_diams,
	HCREF_divide,
	HCREF_eacute,
	HCREF_ecirc,
	HCREF_egrave,
	HCREF_empty,
	HCREF_emsp,
	HCREF_ensp,
	HCREF_epsilon,
	HCREF_equiv,
	HCREF_eta,
	HCREF_eth,
	HCREF_euml,
	HCREF_euro,
	HCREF_exist,
	HCREF_fnof,
	HCREF_forall,
	HCREF_frac12,
	HCREF_frac14,
	HCREF_frac34,
	HCREF_frasl,
	HCREF_gamma,
	HCREF_ge,
	HCREF_gt,
	HCREF_hArr,
	HCREF_harr,
	HCREF_hearts,
	HCREF_hellip,
	HCREF_iacute,
	HCREF_icirc,
	HCREF_iexcl,
	HCREF_igrave,
	HCREF_image,
	HCREF_infin,
	HCREF_int,
	HCREF_iota,
	HCREF_iquest,
	HCREF_isin,
	HCREF_iuml,
	HCREF_kappa,
	HCREF_lArr,
	HCREF_lambda,
	HCREF_lang,
	HCREF_laquo,
	HCREF_larr,
	HCREF_lceil,
	HCREF_ldquo,
	HCREF_le,
	HCREF_lfloor,
	HCREF_lowast,
	HCREF_loz,
	HCREF_lrm,
	HCREF_lsaquo,
	HCREF_lsquo,
	HCREF_lt,
	HCREF_macr,
	HCREF_mdash,
	HCREF_micro,
	HCREF_middot,
	HCREF_minus,
	HCREF_mu,
	HCREF_nabla,
	HCREF_nbsp,
	HCREF_ndash,
	HCREF_ne,
	HCREF_ni,
	HCREF_not,
	HCREF_notin,
	HCREF_nsub,
	HCREF_ntilde,
	HCREF_nu,
	HCREF_oacute,
	HCREF_ocirc,
	HCREF_oelig,
	HCREF_ograve,
	HCREF_oline,
	HCREF_omega,
	HCREF_omicron,
	HCREF_oplus,
	HCREF_or,
	HCREF_ordf,
	HCREF_ordm,
	HCREF_oslash,
	HCREF_otilde,
	HCREF_otimes,
	HCREF_ouml,
	HCREF_para,
	HCREF_part,
	HCREF_permil,
	HCREF_perp,
	HCREF_phi,
	HCREF_pi,
	HCREF_piv,
	HCREF_plusmn,
	HCREF_pound,
	HCREF_prime,
	HCREF_prod,
	HCREF_prop,
	HCREF_psi,
	HCREF_quot,
	HCREF_rArr,
	HCREF_radic,
	HCREF_rang,
	HCREF_raquo,
	HCREF_rarr,
	HCREF_rceil,
	HCREF_rdquo,
	HCREF_real,
	HCREF_reg,
	HCREF_rfloor,
	HCREF_rho,
	HCREF_rlm,
	HCREF_rsaquo,
	HCREF_rsquo,
	HCREF_sbquo,
	HCREF_scaron,
	HCREF_sdot,
	HCREF_sect,
	HCREF_shy,
	HCREF_sigma,
	HCREF_sigmaf,
	HCREF_sim,
	HCREF_spades,
	HCREF_sub,
	HCREF_sube,
	HCREF_sum,
	HCREF_sup,
	HCREF_sup1,
	HCREF_sup2,
	HCREF_sup3,
	HCREF_supe,
	HCREF_szlig,
	HCREF_tau,
	HCREF_there4,
	HCREF_theta,
	HCREF_thetasym,
	HCREF_thinsp,
	HCREF_thorn,
	HCREF_tilde,
	HCREF_times,
	HCREF_trade,
	HCREF_uArr,
	HCREF_uacute,
	HCREF_uarr,
	HCREF_ucirc,
	HCREF_ugrave,
	HCREF_uml,
	HCREF_upsih,
	HCREF_upsilon,
	HCREF_uuml,
	HCREF_weierp,
	HCREF_xi,
	HCREF_yacute,
	HCREF_yen,
	HCREF_yuml,
	HCREF_zeta,
	HCREF_zwj,
	HCREF_zwnj,
	HCREF__MAX
};

enum	helemt {
	HELEM_A,
	HELEM_ABBR,
	HELEM_ACRONYM,
	HELEM_ADDRESS,
	HELEM_APPLET,
	HELEM_AREA,
	HELEM_B,
	HELEM_BASE,
	HELEM_BASEFONT,
	HELEM_BDO,
	HELEM_BIG,
	HELEM_BLOCKQUOTE,
	HELEM_BODY,
	HELEM_BR,
	HELEM_BUTTON,
	HELEM_CAPTION,
	HELEM_CENTER,
	HELEM_CITE,
	HELEM_CODE,
	HELEM_COL,
	HELEM_COLGROUP,
	HELEM_DD,
	HELEM_DEL,
	HELEM_DFN,
	HELEM_DIR,
	HELEM_DIV,
	HELEM_DL,
	HELEM_DT,
	HELEM_EM,
	HELEM_FIELDSET,
	HELEM_FONT,
	HELEM_FORM,
	HELEM_FRAME,
	HELEM_FRAMESET,
	HELEM_H1,
	HELEM_H2,
	HELEM_H3,
	HELEM_H4,
	HELEM_H5,
	HELEM_H6,
	HELEM_HEAD,
	HELEM_HR,
	HELEM_HTML,
	HELEM_I,
	HELEM_IFRAME,
	HELEM_IMG,
	HELEM_INPUT,
	HELEM_INS,
	HELEM_ISINDEX,
	HELEM_KBD,
	HELEM_LABEL,
	HELEM_LEGEND,
	HELEM_LI,
	HELEM_LINK,
	HELEM_MAP,
	HELEM_MENU,
	HELEM_META,
	HELEM_NOFRAMES,
	HELEM_NOSCRIPT,
	HELEM_OBJECT,
	HELEM_OL,
	HELEM_OPTGROUP,
	HELEM_OPTION,
	HELEM_P,
	HELEM_PARAM,
	HELEM_PRE,
	HELEM_Q,
	HELEM_S,
	HELEM_SAMP,
	HELEM_SCRIPT,
	HELEM_SELECT,
	HELEM_SMALL,
	HELEM_SPAN,
	HELEM_STRIKE,
	HELEM_STRONG,
	HELEM_STYLE,
	HELEM_SUB,
	HELEM_SUP,
	HELEM_TABLE,
	HELEM_TBODY,
	HELEM_TD,
	HELEM_TEXTAREA,
	HELEM_TFOOT,
	HELEM_TH,
	HELEM_THEAD,
	HELEM_TITLE,
	HELEM_TR,
	HELEM_TT,
	HELEM_U,
	HELEM_UL,
	HELEM_VAR,
	HELEM__MAX
};

enum	hattrt {
	HATTR_ABBR,
	HATTR_ACCEPT,
	HATTR_ACCEPT_CHARSET,
	HATTR_ACCESSKEY,
	HATTR_ACTION,
	HATTR_ALIGN,
	HATTR_ALINK,
	HATTR_ALT,
	HATTR_ARCHIVE,
	HATTR_AXIS,
	HATTR_BACKGROUND,
	HATTR_BGCOLOR,
	HATTR_BORDER,
	HATTR_CELLPADDING,
	HATTR_CELLSPACING,
	HATTR_CHAR,
	HATTR_CHAROFF,
	HATTR_CHARSET,
	HATTR_CHECKED,
	HATTR_CITE,
	HATTR_CLASS,
	HATTR_CLASSID,
	HATTR_CLEAR,
	HATTR_CODE,
	HATTR_CODEBASE,
	HATTR_CODETYPE,
	HATTR_COLOR,
	HATTR_COLS,
	HATTR_COLSPAN,
	HATTR_COMPACT,
	HATTR_CONTENT,
	HATTR_COORDS,
	HATTR_DATA,
	HATTR_DATETIME,
	HATTR_DECLARE,
	HATTR_DEFER,
	HATTR_DIR,
	HATTR_DISABLED,
	HATTR_ENCODING,
	HATTR_ENCTYPE,
	HATTR_FACE,
	HATTR_FOR,
	HATTR_FRAME,
	HATTR_FRAMEBORDER,
	HATTR_HEADERS,
	HATTR_HEIGHT,
	HATTR_HREF,
	HATTR_HREFLANG,
	HATTR_HSPACE,
	HATTR_HTTP_EQUIV,
	HATTR_ID,
	HATTR_ISMAP,
	HATTR_LABEL,
	HATTR_LANG,
	HATTR_LANGUAGE,
	HATTR_LINK,
	HATTR_LONGDESC,
	HATTR_MARGINHEIGHT,
	HATTR_MARGINWIDTH,
	HATTR_MAX,
	HATTR_MAXLENGTH,
	HATTR_MEDIA,
	HATTR_METHOD,
	HATTR_MULTIPLE,
	HATTR_NAME,
	HATTR_NOHREF,
	HATTR_NORESIZE,
	HATTR_NOSHADE,
	HATTR_NOWRAP,
	HATTR_OBJECT,
	HATTR_ONBLUR,
	HATTR_ONCHANGE,
	HATTR_ONCLICK,
	HATTR_ONDBLCLICK,
	HATTR_ONFOCUS,
	HATTR_ONKEYDOWN,
	HATTR_ONKEYPRESS,
	HATTR_ONKEYUP,
	HATTR_ONLOAD,
	HATTR_ONMOUSEDOWN,
	HATTR_ONMOUSEMOVE,
	HATTR_ONMOUSEOUT,
	HATTR_ONMOUSEOVER,
	HATTR_ONMOUSEUP,
	HATTR_ONRESET,
	HATTR_ONSELECT,
	HATTR_ONSUBMIT,
	HATTR_ONUNLOAD,
	HATTR_PROFILE,
	HATTR_PROMPT,
	HATTR_READONLY,
	HATTR_REL,
	HATTR_REV,
	HATTR_ROWS,
	HATTR_ROWSPAN,
	HATTR_RULES,
	HATTR_SCHEME,
	HATTR_SCOPE,
	HATTR_SCROLLING,
	HATTR_SELECTED,
	HATTR_SHAPE,
	HATTR_SIZE,
	HATTR_SPAN,
	HATTR_SRC,
	HATTR_STANDBY,
	HATTR_START,
	HATTR_STYLE,
	HATTR_SUMMARY,
	HATTR_TABINDEX,
	HATTR_TARGET,
	HATTR_TEXT,
	HATTR_TITLE,
	HATTR_TYPE,
	HATTR_USEMAP,
	HATTR_VALIGN,
	HATTR_VALUE,
	HATTR_VALUETYPE,
	HATTR_VERSION,
	HATTR_VLINK,
	HATTR_VSPACE,
	HATTR_WIDTH,
	HATTR_XMLNS,
	HATTR__MAX
};

struct	hattr;
struct	hident;
struct	hnode;
struct	hparse;
struct	hvalid;
struct	hwrite;

struct	hcid {
	int		 id;
	const char	*name;
};

struct	ioctx {
	/*
	 * Open ioctx.  May be NULL.  After invocation, ioctx is assumed
	 * to be ready for any and all callback functions.  Return 0
	 * fail, 1 success.
	 */
	int	(*open)(void *);
	/*
	 * Get character.  Note that rew function may ask that this be
	 * pushed back in.  See rew callback for details.  Return -1
	 * fail, 0 eof, 1 success.
	 */
	int	(*getchar)(char *, void *);
	/*
	 * Rewind by one character.  Returnn 0 fail, 1 success.  This is
	 * not called until after getchar has been invoked and is
	 * guaranteed to only be called once at a time, i.e., at most
	 * once for each invocation of getchar.
	 */
	int	(*rew)(void *);
	/*
	 * Close ioctx.  After invocation, no more callbacks shall be
	 * used.  May be NULL.  Return 0 fail, 1 success.
	 */
	int	(*close)(void *);
	/*
	 * System error.  May be NULL.
	 */
	void	(*error)(const char *, void *);
	/*
	 * Warnings at current position.  May be NULL.  If returning 0,
	 * the system will consider it an error.
	 */
	int	(*warn)(const char *, int, int, void *);
	/*
	 * Syntax error at current position.  May be NULL.
	 */
	void	(*syntax)(const char *, int, int, void *);
	/*
	 * Transcode a character into a valid HTML-UTF-8 string as
	 * defined by UTR-20.  May be NULL.  This can be used to parse
	 * and encode arbitrary character sets.  Return 0 fail, 1
	 * success.  If return is success and char-ptr result is
	 * non-NULL, it will be included in the character stream (it's
	 * legal to transcode a zero-length representation).  If the
	 * char-ptr result is NULL, the original character will be
	 * written to the character stream.
	 */
	int	(*xcode)(char, char **, size_t *, void *);
	/*
	 * Private ioctx data.  May be NULL.
	 */
	void	 *arg;
};

struct	ioout {
	/* Open ioout.  May be NULL.  Return 0 fail, 1 success. */
	int	(*open)(void *);
	/* Close ioout.  May be NULL.  Return 0 fail, 1 success. */
	int	(*close)(void *);
	/* Write nil-terminated string. Return 0 fail, 1 success. */
	int	(*puts)(const char *, void *);
	/* Write single character. Return 0 fail, 1 success. */
	int	(*putchar)(char, void *);
	/* Private ioout data.  May be NULL.  */
	void	 *arg;
};

struct	iovalid {
	/* Attribute is valid. Return 0 to trigger fail, 1 continue. */
	int	(*valid_attr)(struct hattr *, void *);
	/* Attribute is invalid. */
	void	(*valid_battr)(struct hattr *, const char *, void *);
	/* Identifier is valid. Return 0 to trigger fail, 1 continue. */
	int	(*valid_ident)(struct hident *, void *);
	/* Node is valid. Return 0 to trigger fail, 1 continue. */
	int	(*valid_node)(struct hnode *, void *);
	/* Node is invalid. */
	void	(*valid_bnode)(struct hnode *, const char *, void *);
	/* Private iovalid data.  May be NULL. */
	void	 *arg;
};

struct	iofd {
	int		  fd; /* underlying file descriptor */
	char		  last; /* last-known char (for FD_REW) */
	int		  flags;
#define	FD_REW		  1 /* whether we just rewound */
	const char	 *file; /* underlying filename */
	void		(*perr)(const char *, void *); /* errors */
	void		 *arg; /* passed to perr */
};

/*
 * These are ALL documented in html.3.
 */

int		  iofd_close(void *);
int		  iofd_getchar(char *, void *);
int		  iofd_open(void *);
int		  iofd_rew(void *);
int		  iostdout_putchar(char, void *);
int		  iostdout_puts(const char *, void *);

struct	hattr	 *hattr_alloc_chars(enum hattrt, const char *, int, int, int);
struct	hattr	 *hattr_alloc_text(enum hattrt, const char *, int, int, int);
struct	hattr	 *hattr_clone(struct hattr *);
int		  hattr_column(struct hattr *);
void		  hattr_delete(struct hattr *);
const	char	 *hattr_enumname(enum hattrt);
enum	hattrt	  hattr_find(const char *);
int		  hattr_line(struct hattr *);
int		  hattr_literal(struct hattr *);
const	char	 *hattr_name(enum hattrt);
struct	hnode	 *hattr_parent(struct hattr *);
struct	hattr	 *hattr_sibling(struct hattr *);
enum	hattrt	  hattr_type(struct hattr *);
const	char	 *hattr_value(struct hattr *);
struct	hcache	 *hcache_alloc(struct hnode *,
			const struct hcid *, int, size_t);
struct	hcache	 *hcache_clone(struct hcache *);
struct	hnode	 *hcache_get(struct hcache *, int);
void		  hcache_delete(struct hcache *);
struct	hnode	 *hcache_root(struct hcache *);
int		  hcache_verify(struct hcache *);
enum	hcreft	  hcref_find(const char *);
const	char	 *hcref_name(enum hcreft);
const	char	 *hdecl_enumname(enum hdeclt);
enum	hdeclt	  hdecl_find(const char *);
const	char	 *hdecl_name(enum hdeclt);
struct	hident	 *hident_alloc(const char *, int, int, int);
struct	hident	 *hident_clone(struct hident *);
int		  hident_column(struct hident *);
void		  hident_delete(struct hident *);
int		  hident_line(struct hident *);
int		  hident_literal(struct hident *);
struct	hident	 *hident_sibling(struct hident *);
const	char	 *hident_value(struct hident *);
const	char	 *helem_enumname(enum helemt);
enum	helemt	  helem_find(const char *);
const	char	 *helem_name(enum helemt);
int		  hnode_addattr(struct hnode *, struct hattr *);
int		  hnode_addchild(struct hnode *, struct hnode *);
int		  hnode_addident(struct hnode *, struct hident *);
struct	hnode	 *hnode_alloc_chars(const char *, int, int);
struct	hnode	 *hnode_alloc_comment(const char *, int, int);
struct	hnode	 *hnode_alloc_decl(enum hdeclt, int, int);
struct	hnode	 *hnode_alloc_elem(enum helemt, int, int);
struct	hnode	 *hnode_alloc_proc(enum hproct, int, int);
struct	hnode	 *hnode_alloc_root(int, int);
struct	hnode	 *hnode_alloc_text(const char *, int, int);
struct	hattr	 *hnode_attr(struct hnode *);
struct	hnode	 *hnode_child(struct hnode *);
struct	hnode	 *hnode_clone(struct hnode *);
int		  hnode_column(struct hnode *);
const	char	 *hnode_comment(struct hnode *);
void		  hnode_dechild(struct hnode *);
void		  hnode_dechildpart(struct hnode *,
			const struct hnode * const *, int);
enum	hdeclt	  hnode_decl(struct hnode *);
void		  hnode_delete(struct hnode *);
enum	helemt	  hnode_elem(struct hnode *);
struct	hident	 *hnode_ident(struct hnode *);
int		  hnode_line(struct hnode *);
struct	hnode	 *hnode_parent(struct hnode *);
enum	hproct	  hnode_proc(struct hnode *);
int		  hnode_repattr(struct hnode *, struct hattr *);
struct	hnode	 *hnode_sibling(struct hnode *);
const	char	 *hnode_text(struct hnode *);
enum	hnodet	  hnode_type(struct hnode *);
struct	hparse	 *hparse_alloc(enum hmode);
void		  hparse_delete(struct hparse *);
int		  hparse_tree(struct hparse *,
			struct ioctx *, struct hnode **);
const	char	 *hproc_enumname(enum hproct);
enum	hproct	  hproc_find(const char *);
const	char	 *hproc_name(enum hproct);
struct	hwrite	 *hwrite_alloc(enum hmode);
void		  hwrite_delete(struct hwrite *);
void		  hwrite_mode(struct hwrite *, enum hmode);
int		  hwrite_tree(struct hwrite *,
			struct ioout *, struct hnode *);
struct	hvalid	 *hvalid_alloc(enum hmode);
void		  hvalid_delete(struct hvalid *);
int		  hvalid_tree(struct hvalid *,
			struct iovalid *, struct hnode *);

__END_DECLS

#endif /*! HTML_H*/
