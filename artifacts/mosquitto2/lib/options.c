/*
Copyright (c) 2010-2019 Roger Light <roger@atchoo.org>

All rights reserved. This program and the accompanying materials
are made available under the terms of the Eclipse Public License v1.0
and Eclipse Distribution License v1.0 which accompany this distribution.
 
The Eclipse Public License is available at
   http://www.eclipse.org/legal/epl-v10.html
and the Eclipse Distribution License is available at
  http://www.eclipse.org/org/documents/edl-v10.php.
 
Contributors:
   Roger Light - initial implementation and documentation.
*/

#include "config.h"

#ifndef WIN32
#  include <strings.h>
#endif

#include <string.h>


#include "mosquitto.h"
#include "mosquitto_internal.h"
#include "memory_mosq.h"
#include "mqtt_protocol.h"
#include "util_mosq.h"
#include "will_mosq.h"


int mosquitto_will_set(struct mosquitto *mosq, const char *topic, int payloadlen, const void *payload, int qos, bool retain)
{
	return mosquitto_will_set_v5(mosq, topic, payloadlen, payload, qos, retain, NULL);
}


int mosquitto_will_set_v5(struct mosquitto *mosq, const char *topic, int payloadlen, const void *payload, int qos, bool retain, mosquitto_property *properties)
{
	int rc;

	if(!mosq) return MOSQ_ERR_INVAL;

	if(properties){
		rc = mosquitto_property_check_all(CMD_WILL, properties);
		if(rc) return rc;
	}

	return will__set(mosq, topic, payloadlen, payload, qos, retain, properties);
}


int mosquitto_will_clear(struct mosquitto *mosq)
{
	if(!mosq) return MOSQ_ERR_INVAL;
	return will__clear(mosq);
}


int mosquitto_username_pw_set(struct mosquitto *mosq, const char *username, const char *password)
{
	if(!mosq) return MOSQ_ERR_INVAL;

	if(mosq->protocol == mosq_p_mqtt311 || mosq->protocol == mosq_p_mqtt31){
		if(password != NULL && username == NULL){
			return MOSQ_ERR_INVAL;
		}
	}

	mosquitto__free(mosq->username);
	mosq->username = NULL;

	mosquitto__free(mosq->password);
	mosq->password = NULL;

	if(username){
		if(mosquitto_validate_utf8(username, strlen(username))){
			return MOSQ_ERR_MALFORMED_UTF8;
		}
		mosq->username = mosquitto__strdup(username);
		if(!mosq->username) return MOSQ_ERR_NOMEM;
	}

	if(password){
		mosq->password = mosquitto__strdup(password);
		if(!mosq->password){
			mosquitto__free(mosq->username);
			mosq->username = NULL;
			return MOSQ_ERR_NOMEM;
		}
	}
	return MOSQ_ERR_SUCCESS;
}


int mosquitto_reconnect_delay_set(struct mosquitto *mosq, unsigned int reconnect_delay, unsigned int reconnect_delay_max, bool reconnect_exponential_backoff)
{
	if(!mosq) return MOSQ_ERR_INVAL;
	
	if(reconnect_delay == 0) reconnect_delay = 1;

	mosq->reconnect_delay = reconnect_delay;
	mosq->reconnect_delay_max = reconnect_delay_max;
	mosq->reconnect_exponential_backoff = reconnect_exponential_backoff;
	
	return MOSQ_ERR_SUCCESS;
	
}


int mosquitto_tls_set(struct mosquitto *mosq, const char *cafile, const char *capath, const char *certfile, const char *keyfile, int (*pw_callback)(char *buf, int size, int rwflag, void *userdata))
{
	return MOSQ_ERR_NOT_SUPPORTED;
}


int mosquitto_tls_opts_set(struct mosquitto *mosq, int cert_reqs, const char *tls_version, const char *ciphers)
{
	return MOSQ_ERR_NOT_SUPPORTED;

}


int mosquitto_tls_insecure_set(struct mosquitto *mosq, bool value)
{
	return MOSQ_ERR_NOT_SUPPORTED;
}


int mosquitto_string_option(struct mosquitto *mosq, enum mosq_opt_t option, const char *value)
{

	if(!mosq) return MOSQ_ERR_INVAL;

	switch(option){
		case MOSQ_OPT_TLS_ENGINE:
			return MOSQ_ERR_NOT_SUPPORTED;
			break;

		case MOSQ_OPT_TLS_KEYFORM:

			return MOSQ_ERR_NOT_SUPPORTED;
			break;


		case MOSQ_OPT_TLS_ENGINE_KPASS_SHA1:
			return MOSQ_ERR_NOT_SUPPORTED;
			break;

		case MOSQ_OPT_TLS_ALPN:
			return MOSQ_ERR_NOT_SUPPORTED;
			break;

		default:
			return MOSQ_ERR_INVAL;
	}
}



int mosquitto_opts_set(struct mosquitto *mosq, enum mosq_opt_t option, void *value)
{
	int ival;

	if(!mosq || !value) return MOSQ_ERR_INVAL;

	switch(option){
		case MOSQ_OPT_PROTOCOL_VERSION:
			ival = *((int *)value);
			return mosquitto_int_option(mosq, option, ival);
		case MOSQ_OPT_SSL_CTX:

			return MOSQ_ERR_NOT_SUPPORTED;
		default:
			return MOSQ_ERR_INVAL;
	}
	return MOSQ_ERR_SUCCESS;
}


int mosquitto_int_option(struct mosquitto *mosq, enum mosq_opt_t option, int value)
{
	if(!mosq) return MOSQ_ERR_INVAL;

	switch(option){
		case MOSQ_OPT_PROTOCOL_VERSION:
			if(value == MQTT_PROTOCOL_V31){
				mosq->protocol = mosq_p_mqtt31;
			}else if(value == MQTT_PROTOCOL_V311){
				mosq->protocol = mosq_p_mqtt311;
			}else if(value == MQTT_PROTOCOL_V5){
				mosq->protocol = mosq_p_mqtt5;
			}else{
				return MOSQ_ERR_INVAL;
			}
			break;

		case MOSQ_OPT_RECEIVE_MAXIMUM:
			if(value < 0 || value > 65535){
				return MOSQ_ERR_INVAL;
			}
			if(value == 0){
				mosq->msgs_in.inflight_maximum = 65535;
			}else{
				mosq->msgs_in.inflight_maximum = value;
			}
			break;

		case MOSQ_OPT_SEND_MAXIMUM:
			if(value < 0 || value > 65535){
				return MOSQ_ERR_INVAL;
			}
			if(value == 0){
				mosq->msgs_out.inflight_maximum = 65535;
			}else{
				mosq->msgs_out.inflight_maximum = value;
			}
			break;

		case MOSQ_OPT_SSL_CTX_WITH_DEFAULTS:
#if defined(WITH_TLS) && OPENSSL_VERSION_NUMBER >= 0x10100000L
			if(value){
				mosq->ssl_ctx_defaults = true;
			}else{
				mosq->ssl_ctx_defaults = false;
			}
			break;
#else
			return MOSQ_ERR_NOT_SUPPORTED;
#endif

		case MOSQ_OPT_TLS_OCSP_REQUIRED:
			return MOSQ_ERR_NOT_SUPPORTED;
			break;

		default:
			return MOSQ_ERR_INVAL;
	}
	return MOSQ_ERR_SUCCESS;
}


int mosquitto_void_option(struct mosquitto *mosq, enum mosq_opt_t option, void *value)
{
	if(!mosq || !value) return MOSQ_ERR_INVAL;

	switch(option){
		case MOSQ_OPT_SSL_CTX:
#ifdef WITH_TLS
			mosq->ssl_ctx = (SSL_CTX *)value;
			if(mosq->ssl_ctx){
#if (OPENSSL_VERSION_NUMBER >= 0x10100000L) && !defined(LIBRESSL_VERSION_NUMBER)
				SSL_CTX_up_ref(mosq->ssl_ctx);
#else
				CRYPTO_add(&(mosq->ssl_ctx)->references, 1, CRYPTO_LOCK_SSL_CTX);
#endif
			}
			break;
#else
			return MOSQ_ERR_NOT_SUPPORTED;
#endif
		default:
			return MOSQ_ERR_INVAL;
	}
	return MOSQ_ERR_SUCCESS;
}


void mosquitto_user_data_set(struct mosquitto *mosq, void *userdata)
{
	if(mosq){
		mosq->userdata = userdata;
	}
}

void *mosquitto_userdata(struct mosquitto *mosq)
{
	return mosq->userdata;
}
