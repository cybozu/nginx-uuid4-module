/* (C) 2015 Cybozu.  All rights reserved. */

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include "mt19937/mt64.h"


static ngx_int_t ngx_http_uuid4_init(ngx_conf_t *cf);
static char *ngx_http_uuid4(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);


static ngx_command_t  ngx_http_uuid4_commands[] = {

    { ngx_string("uuid4"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
      ngx_http_uuid4,
      NGX_HTTP_LOC_CONF_OFFSET,
      0,
      NULL },

    ngx_null_command
};


static ngx_http_module_t  ngx_http_uuid4_module_ctx = {
    NULL,                       /* preconfiguration */
    ngx_http_uuid4_init,        /* postconfiguration */

    NULL,                       /* create main configuration */
    NULL,                       /* init main configuration */

    NULL,                       /* create server configuration */
    NULL,                       /* merge server configuration */

    NULL,                       /* create location configuration */
    NULL                        /* merge location configuration */
};


ngx_module_t  ngx_http_uuid4_module = {
    NGX_MODULE_V1,
    &ngx_http_uuid4_module_ctx,            /* module context */
    ngx_http_uuid4_commands,               /* module directives */
    NGX_HTTP_MODULE,                       /* module type */
    NULL,                                  /* init master */
    NULL,                                  /* init module */
    NULL,                                  /* init process */
    NULL,                                  /* init thread */
    NULL,                                  /* exit thread */
    NULL,                                  /* exit process */
    NULL,                                  /* exit master */
    NGX_MODULE_V1_PADDING
};


static ngx_int_t
ngx_http_uuid4_variable(ngx_http_request_t *r, ngx_http_variable_value_t *v,
    uintptr_t data)
{
    static const size_t UUID_STR_LENGTH = 36;

    uint64_t upper = (uint64_t)genrand64_int64();
    uint64_t lower = (uint64_t)genrand64_int64();
    upper &= ~((1ULL << 12) | (1ULL << 13) | (1ULL << 15));
    upper |= (1ULL << 14);
    lower &= ~(1ULL << 62);
    lower |= (1ULL << 63);

    v->len = UUID_STR_LENGTH;
    v->data = ngx_palloc(r->pool, UUID_STR_LENGTH);
    if (v->data == NULL) {
        *v = ngx_http_variable_null_value;
        return NGX_OK;
    }
    ngx_snprintf(v->data, UUID_STR_LENGTH, "%08uxL-%04uxL-%04uxL-%04uxL-%012uxL",
        upper >> 32, (upper >> 16) & 0xFFFFULL, upper & 0xFFFFULL,
        lower >> 48, lower & 0xFFFFFFFFFFFFULL);
    v->valid = 1;
    v->no_cacheable = 0;
    v->not_found = 0;

    return NGX_OK;
}


static char *
ngx_http_uuid4(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_str_t           *value;
    ngx_http_variable_t *v;
    ngx_int_t            index;

    value = cf->args->elts;
    if (value[1].data[0] != '$') {
        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                           "invalid variable name \"%V\"", &value[1]);
        return NGX_CONF_ERROR;
    }

    value[1].len--;
    value[1].data++;

    v = ngx_http_add_variable(cf, &value[1], NGX_HTTP_VAR_CHANGEABLE);
    if (v == NULL) {
        return NGX_CONF_ERROR;
    }

    index = ngx_http_get_variable_index(cf, &value[1]);
    if (index == NGX_ERROR) {
        return NGX_CONF_ERROR;
    }

    v->get_handler = ngx_http_uuid4_variable;

    return NGX_CONF_OK;
}


static ngx_int_t
ngx_http_uuid4_init(ngx_conf_t *cf)
{
    static const size_t SEED_LENGTH = 312;
    unsigned long long  seed[SEED_LENGTH];
    size_t              n;

    /* initialize MT */
    FILE* f = fopen("/dev/urandom", "r");
    if (f == NULL) {
        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                           "failed to open /dev/urandom");
        return NGX_ERROR;
    }
    n = fread(seed, sizeof(unsigned long long), SEED_LENGTH, f);
    if (n < SEED_LENGTH) {
        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                           "failed to read /dev/urandom");
        fclose(f);
        return NGX_ERROR;
    }
    fclose(f);
    init_by_array64(seed, SEED_LENGTH);

    return NGX_OK;
}
