load('api_zswitch.js');

ZenSwitch.MQTT = {
  _att: ffi('bool mgos_zswitch_mqtt_attach(void *, char *, char *, void *)'),
  _det: ffi('bool mgos_zswitch_mqtt_detach(void *)'),
  _cfgc: ffi('void *mjs_zswitch_mqtt_cfg_create(char *,char *,char *,char *,char *)'),
  _cfgf: ffi('void mjs_zswitch_mqtt_cfg_free(void *)'),

  _proto: {
    _switch: null,
    _getHandle: function() {
      return this._switch.handle
    },

    // ## **`object.MQTT.attach(stateTopic, cmdTopic, cfg)`**
    //
    // Example:
    // ```javascript
    // let res = sw.MQTT.attach('mgos/${device_id}/${zthing_id}/state',
    //   'mgos/${device_id}/${zthing_id}/cmd');
    //
    // if (res) {
    //   /* success */
    // } else {
    //   /* error */
    // }
    // ```
    attach: function(stateTopic, cmdTopic, cfg) {
      let cfgo = null;
      if (cfg) {
        cfgo = ZenSwitch.MQTT._cfgc(
          cfg.stateOn || null,
          cfg.stateOff || null,
          cfg.cmdOn || null,
          cfg.cmdOff || null,
          cfg.cmdToggle || null
        );
      }
      let result = ZenSwitch.MQTT._att(this._getHandle(),
        stateTopic, cmdTopic, cfgo);
      ZenSwitch.MQTT._cfgf(cfgo);
      return result;
    },

    // ## **`object.MQTT.detach()`**
    //
    // Example:
    // ```javascript
    // let success = sw.MQTT.detach();
    // ```
    detach: function() {
      return ZenSwitch.MQTT._det(this._getHandle());
    },
  },
};

ZenSwitch._onCreateSub(function(obj) {
  obj.MQTT = Object.create(ZenSwitch.MQTT._proto);
  obj.MQTT._switch = obj;
});
