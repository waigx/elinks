/* ECMAScript scripting hooks */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "elinks.h"

#include "cache/cache.h"
#include "ecmascript/spidermonkey/util.h"
#include "protocol/uri.h"
#include "main/event.h"
#include "main/module.h"
#include "scripting/smjs/cache_object.h"
#include "scripting/smjs/core.h"
#include "scripting/smjs/elinks_object.h"
#include "scripting/smjs/hooks.h"
#include "session/session.h"


static enum evhook_status
script_hook_goto_url(va_list ap, void *data)
{
	unsigned char **url = va_arg(ap, unsigned char **);
	struct session *ses = va_arg(ap, struct session *);
	enum evhook_status ret = EVENT_HOOK_STATUS_NEXT;
	jsval args[1], rval;

	if (*url == NULL) return EVENT_HOOK_STATUS_NEXT;

	smjs_ses = ses;

	args[0] = STRING_TO_JSVAL(JS_NewStringCopyZ(smjs_ctx, *url));

	if (JS_TRUE == smjs_invoke_elinks_object_method("goto_url_hook",
	                                                args, 1, &rval)) {
		if (JSVAL_IS_BOOLEAN(rval)) {
			if (JS_FALSE == JSVAL_TO_BOOLEAN(rval))
				ret = EVENT_HOOK_STATUS_LAST;
		} else {
			JSString *jsstr = JS_ValueToString(smjs_ctx, rval);
			unsigned char *str = JS_GetStringBytes(jsstr);

			mem_free_set(url, stracpy(str));
		}
	}

	smjs_ses = NULL;

	return ret;
}

static enum evhook_status
script_hook_follow_url(va_list ap, void *data)
{
	unsigned char **url = va_arg(ap, unsigned char **);
	struct session *ses = va_arg(ap, struct session *);
	enum evhook_status ret = EVENT_HOOK_STATUS_NEXT;
	jsval args[1], rval;

	if (*url == NULL) return EVENT_HOOK_STATUS_NEXT;

	smjs_ses = ses;

	args[0] = STRING_TO_JSVAL(JS_NewStringCopyZ(smjs_ctx, *url));

	if (JS_TRUE == smjs_invoke_elinks_object_method("follow_url_hook",
	                                                args, 1, &rval)) {
		if (JSVAL_IS_BOOLEAN(rval)) {
			if (JS_FALSE == JSVAL_TO_BOOLEAN(rval))
				ret = EVENT_HOOK_STATUS_LAST;
		} else {
			JSString *jsstr = JS_ValueToString(smjs_ctx, rval);
			unsigned char *str = JS_GetStringBytes(jsstr);

			mem_free_set(url, stracpy(str));
		}
	}

	smjs_ses = NULL;

	return ret;
}

static enum evhook_status
script_hook_pre_format_html(va_list ap, void *data)
{
	struct session *ses = va_arg(ap, struct session *);
	struct cache_entry *cached = va_arg(ap, struct cache_entry *);
	enum evhook_status ret = EVENT_HOOK_STATUS_NEXT;
	JSObject *cache_entry_object;
	jsval args[1], rval;

	evhook_use_params(ses && cached);

	if (!smjs_ctx || !cached->length) goto end;

	smjs_ses = ses;

	cache_entry_object = smjs_get_cache_entry_object(cached);
	if (!cache_entry_object) goto end;

	args[0] = OBJECT_TO_JSVAL(cache_entry_object);

	if (JS_TRUE == smjs_invoke_elinks_object_method("preformat_html",
	                                                args, 1, &rval))
		if (JS_FALSE == JSVAL_TO_BOOLEAN(rval))
			ret = EVENT_HOOK_STATUS_LAST;

end:
	smjs_ses = NULL;
	return ret;
}

struct event_hook_info smjs_scripting_hooks[] = {
	{ "goto-url", 0, script_hook_goto_url, NULL },
	{ "follow-url", 0, script_hook_follow_url, NULL },
	{ "pre-format-html", 0, script_hook_pre_format_html, NULL },

	NULL_EVENT_HOOK_INFO,
};