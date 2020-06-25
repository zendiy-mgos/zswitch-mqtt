#include "mgos.h"
#include "common/queue.h"
#include "mgos_mqtt.h"
#include "mgos_zthing_mqtt.h"
#include "mgos_zswitch_mqtt.h"

struct mg_zswitch_mqtt_entry {
  struct mgos_zswitch *handle;
  char *cmd_topic;
  char *state_topic;
  struct mgos_zswitch_mqtt_cfg cfg;
  SLIST_ENTRY(mg_zswitch_mqtt_entry) entry;
};

/* Execution context */
struct mg_zswitch_mqtt_ctx {
  int version;
  SLIST_HEAD(entries, mg_zswitch_mqtt_entry) entries;
};

/* Excecution context instance */
static struct mg_zswitch_mqtt_ctx *s_context = NULL;

void mg_zswitch_mqtt_cfg_close(struct mgos_zswitch_mqtt_cfg cfg) {
  free(((char *)cfg.state_on));
  free(((char *)cfg.state_off));
  free(((char *)cfg.cmd_on));
  free(((char *)cfg.cmd_off));
  free(((char *)cfg.cmd_toggle));
}

void mg_zswitch_mqtt_entry_free(struct mg_zswitch_mqtt_entry *entry) {
  if (entry != NULL) {
    mg_zswitch_mqtt_cfg_close(entry->cfg);
    free(entry->cmd_topic);
    free(entry->state_topic);
    free(entry);
  }
}

static void mg_zswitch_mqtt_on_event(struct mg_connection *nc,
                                     int ev,
                                     void *ev_data,
                                     void *user_data) {  
  (void) ev_data;
  (void) nc;
  (void) user_data;
  if (ev == MG_EV_MQTT_CONNACK) {
    // TODO: implement here
  } else if (ev == MG_EV_MQTT_DISCONNECT) {
  }
}

void mg_zswitch_mqtt_on_cmd_cb(struct mg_connection *nc, const char *topic,
                               int topic_len, const char *msg, int msg_len,
                               void *ud) {
  struct mg_zswitch_mqtt_entry *entry = (struct mg_zswitch_mqtt_entry *)ud;
  if (entry == NULL || msg == NULL) return;

  if (strncasecmp(entry->cfg.cmd_toggle, msg, msg_len) == 0) {
    mgos_zswitch_state_toggle(entry->handle);
  } else {
    bool state;
    if (strncasecmp(entry->cfg.cmd_on, msg, msg_len) == 0) {
      state = true;
    } else if (strncasecmp(entry->cfg.cmd_off, msg, msg_len) == 0) {
      state = false;
    } else {
      LOG(LL_ERROR, ("Invalid MQTT command to switch '%s'",
        entry->handle->id));
      return;
    }
    mgos_zswitch_state_set(entry->handle, state);
  }
  (void) nc;
  (void) topic;
  (void) topic_len;
}

struct mg_zswitch_mqtt_entry *mg_zswitch_mqtt_entry_get(struct mgos_zswitch *handle) {
  struct mg_zswitch_mqtt_entry *e;
  SLIST_FOREACH(e, &s_context->entries, entry) {
    if (((void *)e->handle) == ((void *)handle)) return e;
  }
  return NULL;
}

void mg_zswitch_mqtt_state_updated_cb(int ev, void *ev_data, void *ud) {
  struct mgos_zswitch_state *state = (struct mgos_zswitch_state *)ev_data;
  struct mg_zswitch_mqtt_entry *entry = (struct mg_zswitch_mqtt_entry *)ud;
  if (state != NULL && entry != NULL) {
    mgos_zthing_mqtt_pub(entry->state_topic, 
      (state->value ? entry->cfg.state_on : entry->cfg.state_off), false); 
  }
  (void) ev;
}

bool mg_zswitch_mqtt_entry_reset(struct mg_zswitch_mqtt_entry *entry) {
  if (!entry) return false;
  mgos_event_remove_handler(MGOS_EV_ZTHING_STATE_UPDATED, mg_zswitch_mqtt_state_updated_cb, entry);
  // TODO: waiting for completion of mgos_mqtt_unsub
  // At the moment 'mgos_mqtt_unsub' has only 'topic' 
  // parameter. Waiting for its completion.
  // return mgos_mqtt_unsub(entry->cmd_topic, mg_zswitch_mqtt_on_cmd_cb, entry);
  return false;
  (void) entry;
}

bool mg_zswitch_mqtt_entry_set(struct mg_zswitch_mqtt_entry *entry) {
  if (!entry || !entry->state_topic || !entry->cmd_topic) return false;
  if (mgos_event_add_handler(MGOS_EV_ZTHING_STATE_UPDATED, mg_zswitch_mqtt_state_updated_cb, entry)) {
    mgos_mqtt_sub(entry->cmd_topic, mg_zswitch_mqtt_on_cmd_cb, entry);
    return true;
  }
  mg_zswitch_mqtt_entry_reset(entry);
  return false;
}

bool mgos_zswitch_mqtt_attach(struct mgos_zswitch *handle,
                              const char *state_topic,
                              const char *cmd_topic,
                              struct mgos_zswitch_mqtt_cfg *cfg) {
  if (handle == NULL ||
      state_topic == NULL || cmd_topic == NULL) return false;
  
  struct mg_zswitch_mqtt_entry *e = calloc(1,
    sizeof(struct mg_zswitch_mqtt_entry));
  if (e != NULL) {
    e->handle = handle;
    
    e->cfg.state_on = strdup((cfg == NULL ? MGOS_ZTHING_STR_ON :
      (cfg->state_on != NULL ? cfg->state_on : MGOS_ZTHING_STR_ON)));
    e->cfg.state_off = strdup((cfg == NULL ? MGOS_ZTHING_STR_OFF :
      (cfg->state_off != NULL ? cfg->state_off : MGOS_ZTHING_STR_OFF)));

    e->cfg.cmd_off = strdup((cfg == NULL ? MGOS_ZSWITCH_CMD_OFF :
      (cfg->cmd_off != NULL ? cfg->cmd_off : MGOS_ZSWITCH_CMD_OFF)));
    e->cfg.cmd_on = strdup((cfg == NULL ? MGOS_ZSWITCH_CMD_ON :
      (cfg->cmd_on != NULL ? cfg->cmd_on : MGOS_ZSWITCH_CMD_ON)));
    e->cfg.cmd_toggle = strdup((cfg == NULL ? MGOS_ZSWITCH_CMD_TOGGLE :
      (cfg->cmd_toggle != NULL ? cfg->cmd_toggle : MGOS_ZSWITCH_CMD_TOGGLE)));

    char *tmp_buf = NULL;
    // Normalize and clone state_topic 
    if (mgos_zthing_sreplaces(state_topic, &tmp_buf, 2,
          MGOS_ZTHING_ENV_DEVICEID, mgos_sys_config_get_device_id(),
          MGOS_ZTHING_ENV_THINGID, e->handle->id)) {
      e->state_topic = tmp_buf;
    } else {
      e->state_topic = strdup(state_topic);
    }
    LOG(LL_INFO, ("Switch '%s' will publish state to %s", e->handle->id,
      e->state_topic));

    // Normalize and clone cmd_topic 
    if (mgos_zthing_sreplaces(cmd_topic, &tmp_buf, 2,
          MGOS_ZTHING_ENV_DEVICEID, mgos_sys_config_get_device_id(),
          MGOS_ZTHING_ENV_THINGID, e->handle->id)) {
      e->cmd_topic = tmp_buf;
    } else {
      e->cmd_topic = strdup(cmd_topic);
    }
    LOG(LL_INFO, ("Switch '%s' will listen to %s", e->handle->id,
      e->cmd_topic));
   
    if (mg_zswitch_mqtt_entry_set(e)) {
      SLIST_INSERT_HEAD(&s_context->entries, e, entry);
    } else {
      mg_zswitch_mqtt_entry_free(e);
      e = NULL;
    }
  }
  
  return (e != NULL);
}

bool mgos_zswitch_mqtt_detach(struct mgos_zswitch *handle) {
  struct mg_zswitch_mqtt_entry *e = mg_zswitch_mqtt_entry_get(handle);
  if ((e != NULL) && mg_zswitch_mqtt_entry_reset(e)) {
    SLIST_REMOVE(&s_context->entries, e, mg_zswitch_mqtt_entry, entry);
    mg_zswitch_mqtt_entry_free(e);
    return true;
  }
  return false; 
}

#ifdef MGOS_HAVE_MJS

struct mgos_zswitch_mqtt_cfg *mjs_zswitch_mqtt_cfg_create(const char* state_on,
                                                          const char* state_off,
                                                          const char* cmd_on,
                                                          const char* cmd_off,
                                                          const char* cmd_toggle) {
  struct mgos_zswitch_mqtt_cfg *cfg = calloc(1,
    sizeof(struct mgos_zswitch_mqtt_cfg));
  cfg->state_on = strdup(state_on != NULL ? state_on : MGOS_ZTHING_STR_ON);
  cfg->state_off = strdup(state_off != NULL ? state_off : MGOS_ZTHING_STR_OFF);

  cfg->cmd_on = strdup(cmd_on != NULL ? cmd_on : MGOS_ZSWITCH_CMD_ON);
  cfg->cmd_off = strdup(cmd_off != NULL ? cmd_off : MGOS_ZSWITCH_CMD_OFF);
  cfg->cmd_toggle = strdup(cmd_toggle != NULL ? cmd_toggle : MGOS_ZSWITCH_CMD_TOGGLE);

  return cfg;
}

void mjs_zswitch_mqtt_cfg_free(struct mgos_zswitch_mqtt_cfg *cfg) {
  if (cfg != NULL) {
    mg_zswitch_mqtt_cfg_close(*cfg);
    free(cfg);
  }
}

#endif /* MGOS_HAVE_MJS */


bool mgos_zswitch_mqtt_init() {
  /* Create the context */
  s_context = calloc(1, sizeof(struct mg_zswitch_mqtt_ctx));
  if (!s_context) return false;
  
  /* Initialize the context */
  s_context->version = 1;
  SLIST_INIT(&s_context->entries);

  mgos_mqtt_add_global_handler(mg_zswitch_mqtt_on_event, NULL);

  return true;
}