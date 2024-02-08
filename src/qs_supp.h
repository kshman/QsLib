#pragma once
#define __QS_SUPP__

// 인수 검사 매크로
#define VAR_CHK_NAME							"QS"
#ifndef QS_SUPPRESS_VAR_CHK
#define VAR_CHK_RET(mesg,ret)					QN_STMT_BEGIN{qn_mesgf(true, VAR_CHK_NAME, "%s: %s", __FUNCTION__, mesg); return ret;}QN_STMT_END
#define VAR_CHK_GOTO(mesg,label)				QN_STMT_BEGIN{qn_mesgf(true, VAR_CHK_NAME, "%s: %s", __FUNCTION__, mesg); goto label;}QN_STMT_END
#define VAR_CHK_IF_COND(cond,mesg,ret)			QN_STMT_BEGIN{if ((cond)) { qn_mesgf(true, VAR_CHK_NAME, "%s: %s", __FUNCTION__, mesg); return ret; }}QN_STMT_END
#define VAR_CHK_IF_NULL(item,ret)				QN_STMT_BEGIN{if ((item)==NULL) { qn_mesgf(true, VAR_CHK_NAME, "%s: '%s' is null", __FUNCTION__, #item); return ret; }}QN_STMT_END
#define VAR_CHK_IF_ZERO(item,ret)				QN_STMT_BEGIN{if ((size_t)(item) == 0) { qn_mesgf(true, VAR_CHK_NAME, "%s: '%s' is zero", #item); return ret; }}QN_STMT_END
#define VAR_CHK_IF_NEQ(item,value,ret)			QN_STMT_BEGIN{if ((item)==(value)) { qn_mesgf(true, VAR_CHK_NAME, "%s: '%s' must not %s", __FUNCTION__, #item, #value); return ret; }}QN_STMT_END
#define VAR_CHK_IF_NEED(item,value,ret)			QN_STMT_BEGIN{if ((item)!=(value)) { qn_mesgf(true, VAR_CHK_NAME, "%s: '%s' is not %s", __FUNCTION__, #item, #value); return ret; }}QN_STMT_END
#define VAR_CHK_IF_MAX(item,vmax,ret)			QN_STMT_BEGIN{if ((size_t)(item) >= (vmax)) { qn_mesgf(true, VAR_CHK_NAME, "%s: invalid '%s' value: %d", __FUNCTION__, #item, item); return ret; }}QN_STMT_END
#define VAR_CHK_IF_MIN(item,vmin,ret)			QN_STMT_BEGIN{if ((item) <= (vmin)) { qn_mesgf(true, VAR_CHK_NAME, "%s: invalid '%s' value: %d", __FUNCTION__, #item, item); return ret; }}QN_STMT_END
#define VAR_CHK_IF_NULL2(ctn,item,ret)			QN_STMT_BEGIN{if ((ctn)->item == NULL) { qn_mesgf(true, VAR_CHK_NAME, "%s: '%s.%s' is null", __FUNCTION__, #ctn, #item); return ret; }}QN_STMT_END
#define VAR_CHK_IF_ZERO2(ctn,item,ret)			QN_STMT_BEGIN{if (((size_t)(ctn)->item) == 0) { qn_mesgf(true, VAR_CHK_NAME, "%s: '%s.%s' is zero", __FUNCTION__, #ctn, #item); return ret; }}QN_STMT_END
#define VAR_CHK_IF_NEQ2(ctn,item,value,ret)		QN_STMT_BEGIN{if (((ctn)->item) == (value)) { qn_mesgf(true, VAR_CHK_NAME, "%s: '%s.%s' must not %s", __FUNCTION__, #ctn, #item, #value); return ret; }}QN_STMT_END
#define VAR_CHK_IF_NEED2(ctn,item,value,ret)	QN_STMT_BEGIN{if (((ctn)->item) == 0) { qn_mesgf(true, VAR_CHK_NAME, "%s: '%s.%s' is not %s", __FUNCTION__, #ctn, #item, #value); return ret; }}QN_STMT_END
#define VAR_CHK_IF_MAX2(ctn,item,vmax,ret)		QN_STMT_BEGIN{if ((size_t)(ctn)->item >= (vmax)) { qn_mesgf(true, VAR_CHK_NAME, "%s: invalid '%s.%s' value: %d", __FUNCTION__, #ctn, #item, (ctn)->item); return ret; }}QN_STMT_END
#define VAR_CHK_IF_MIN2(ctn,item,vmin,ret)		QN_STMT_BEGIN{if ((ctn)->item <= (vmin)) { qn_mesgf(true, VAR_CHK_NAME, "%s: invalid '%s.%s' value: %d", __FUNCTION__, #ctn, #item, (ctn)->item); return ret; }}QN_STMT_END
#define VAR_CHK_IF_NULL3(ctn,sec,item,ret)		QN_STMT_BEGIN{if ((ctn)->sec.item == NULL) { qn_mesgf(true, VAR_CHK_NAME, "%s: '%s.%s.%s' is null", __FUNCTION__, #ctn, #sec, #item); return ret; }}QN_STMT_END
#define VAR_CHK_IF_ZERO3(ctn,sec,item,ret)		QN_STMT_BEGIN{if ((size_t)((ctn)->sec.item) == 0) { qn_mesgf(true, VAR_CHK_NAME, "%s: '%s.%s.%s' is zero", __FUNCTION__, #ctn, #sec, #item); return ret; }}QN_STMT_END
#define VAR_CHK_IF_MAX3(ctn,sec,item,vmax,ret)	QN_STMT_BEGIN{if ((size_t)(ctn)->sec.item >= (vmax)) { qn_mesgf(true, VAR_CHK_NAME, "%s: invalid '%s.%s.%s' value: %d", __FUNCTION__, #ctn, #sec, #item, (ctn)->sec.item); return ret; }}QN_STMT_END
#define VAR_CHK_IF_MIN3(ctn,sec,item,vmin,ret)	QN_STMT_BEGIN{if ((ctn)->sec.item <= (vmin)) { qn_mesgf(true, VAR_CHK_NAME, "%s: invalid '%s.%s.%s' value: %d", __FUNCTION__, #ctn, #sec, #item, (ctn)->sec.item); return ret; }}QN_STMT_END
#else
#define VAR_CHK_RET(mesg,ret)					QN_STMT_BEGIN{return ret;}QN_STMT_END
#define VAR_CHK_GOTO(mesg,label)				QN_STMT_BEGIN{goto label;}QN_STMT_END
#define VAR_CHK_IF_COND(cond,mesg,ret)			QN_STMT_BEGIN{if ((cond)) return ret; }QN_STMT_END
#define VAR_CHK_IF_NULL(item,ret)				QN_STMT_BEGIN{if ((item)==NULL) return ret; }QN_STMT_END
#define VAR_CHK_IF_ZERO(item,ret)				QN_STMT_BEGIN{if ((size_t)(item)==0) return ret; }QN_STMT_END
#define VAR_CHK_IF_NEQ(item,value,ret)			QN_STMT_BEGIN{if ((item)==(value)) return ret; }QN_STMT_END
#define VAR_CHK_IF_NEED(item,value,ret)			QN_STMT_BEGIN{if ((item)!=(value)) return ret; }QN_STMT_END
#define VAR_CHK_IF_MAX(item,vmax,ret)			QN_STMT_BEGIN{if ((size_t)(item) >= (vmax)) return ret; }QN_STMT_END
#define VAR_CHK_IF_MIN(item,vmin,ret)			QN_STMT_BEGIN{if ((item) <= (vmin)) return ret; }QN_STMT_END
#define VAR_CHK_IF_NULL2(ctn,item,ret)			QN_STMT_BEGIN{if ((ctn)->item == NULL) return ret; }QN_STMT_END
#define VAR_CHK_IF_ZERO2(ctn,item,ret)			QN_STMT_BEGIN{if (((size_t)(ctn)->item) == 0) return ret; }QN_STMT_END
#define VAR_CHK_IF_NEQ2(ctn,item,value,ret)		QN_STMT_BEGIN{if (((ctn)->item) == (value)) return ret; }QN_STMT_END
#define VAR_CHK_IF_NEED2(ctn,item,value,ret)	QN_STMT_BEGIN{if (((ctn)->item) == 0) return ret; }QN_STMT_END
#define VAR_CHK_IF_MAX2(ctn,item,vmax,ret)		QN_STMT_BEGIN{if ((size_t)(ctn)->item >= (vmax)) return ret; }QN_STMT_END
#define VAR_CHK_IF_MIN2(ctn,item,vmin,ret)		QN_STMT_BEGIN{if ((ctn)->item <= (vmin)) return ret; }QN_STMT_END
#define VAR_CHK_IF_NULL3(ctn,sec,item,ret)		QN_STMT_BEGIN{if ((ctn)->sec.item == NULL) return ret; }QN_STMT_END
#define VAR_CHK_IF_ZERO3(ctn,sec,item,ret)		QN_STMT_BEGIN{if ((size_t)((ctn)->sec.item) == 0) return ret; }QN_STMT_END
#define VAR_CHK_IF_MAX3(ctn,sec,item,vmax,ret)	QN_STMT_BEGIN{if ((size_t)(ctn)->sec.item >= (vmax)) return ret; }QN_STMT_END
#define VAR_CHK_IF_MIN3(ctn,sec,item,vmin,ret)	QN_STMT_BEGIN{if ((ctn)->sec.item <= (vmin)) return ret; }QN_STMT_END
#endif

