# ZenSwitch MQTT
## Overview
Mongoose OS library for enabling MQTT over ZenSwitches.
## GET STARTED
Build up your own device in few minutes just starting from one of the following samples.

|Sample|Notes|
|--|--|
|[zswitch-mqtt-demo](https://github.com/zendiy-mgos/zswitch-mqtt-demo)|Mongoose OS demo firmware for using MQTT to drive ZenSwitches.|
## Usage
Include the library into your `mos.yml` file.
```yaml
libs:
  - origin: https://github.com/zendiy-mgos/zswitch-mqtt
```
If you are developing a JavaScript firmware, load `api_zswitch_mqtt.js` in your `init.js` file.
```js
load('api_zswitch_mqtt.js');
```
## C/C++ API Reference
### mgos_zswitch_mqtt_cfg
```c
struct mgos_zswitch_mqtt_cfg {
  const char *state_on;
  const char *state_off;
  const char *cmd_on;
  const char *cmd_off;
  const char *cmd_toggle; 
};
```
MQTT configuration settings for `mgos_zswitch_mqtt_attach()`.

|Field||
|--|--|
|state_on|The state message payload to publish when the siwtch is ON.|
|state_off|The state message payload to publish when the siwtch is OFF.|
|cmd_on|The command message payload for turning the siwtch ON.|
|cmd_off|The command message payload for turning the siwtch OFF.|
|cmd_toggle|The command message payload for toggling the siwtch state.|
### mgos_zswitch_mqtt_attach()
```c
bool mgos_zswitch_mqtt_attach(struct mgos_zswitch *handle,
                              const char *state_topic,
                              const char *cmd_topic,
                              struct mgos_zswitch_mqtt_cfg *cfg);
```
Attach the switch to MQTT services. Returns `true` on success, `false` otherwise. The switch will publish its state on `state_topic` when connected and everytime its stage changes. The switch subscribes to `cmd_topic` for receiving commands, like turn ON, turn OFF and toggle. Command's payloads are case-insentivive.

|Parameter||
|--|--|
|handle|Switch handle.|
|state_topic|The MQTT topic for publishing switch state payload.|
|cmd_topic|The MQTT topic to subscribe to for receiving command payload.|
|cfg|Optional. MQTT configuration. If `NULL`, following default configuration values are used: state_on=`MGOS_ZTHING_STR_ON`, state_off=`MGOS_ZTHING_STR_OFF`, cmd_on=`MGOS_ZSWITCH_CMD_ON`, cmd_off=`MGOS_ZSWITCH_CMD_ON`, cmd_toggle=`MGOS_ZSWITCH_CMD_TOGGLE`.|

**Environment variables for MQTT topics**
Both `state_topic` and `cmd_topic` parameters can contain one or more of following environment variables.

|Environment variable||
|--|--|
|${device_id}|The device ID.|
|${zthing_id}|The switch ID.|

**Example** - Create a switch using default configuration values and attach it to MQTT services.
```c
struct mgos_zswitch *sw = mgos_zswitch_create("sw-1", NULL);
struct mgos_zswitch_mqtt_cfg cfg = {"on", "off", "turn_on", "turn_off", "toggle"};
mgos_zswitch_mqtt_attach(sw, "$zt/${device_id}/${zthing_id}/state", "$zt/${device_id}/${zthing_id}/cmd", &cfg);
}
```
### mgos_zswitch_mqtt_detach()
```c
bool mgos_zswitch_mqtt_detach(struct mgos_zswitch *handle);
```
Detach the switch from MQTT services that were previously attached using `mgos_zswitch_mqtt_attach()`. Returns `true` on success, `false` otherwise.

|Parameter||
|--|--|
|handle|Switch handle.|
## JS API Reference
### .MQTT.attach()
```js
let success = sw.MQTT.attach(stateTopic, cmdTopic, cfg);
```
Attach the switch to MQTT services. Returns `true` on success, `false` otherwise. The switch will publish its state on `stateTopic` when connected and everytime its stage changes. The switch subscribes to `cmdTopic` for receiving commands, like turn ON, turn OFF and toggle. Command's payloads are case-insentivive.

|Parameter|Type||
|--|--|--|
|stateTopic|string|The MQTT topic for publishing switch state payload.|
|cmdTopic|string|The MQTT topic to subscribe to for receiving command payload.|
|cfg|object|Optional. MQTT configuration. If missing, default configuration values are used.|

**MQTT configuration properties**
|Property|Type||
|--|--|--|
|*cfg*.stateOn|string|Optional. The state message payload to publish when the siwtch is ON. Default value `'ON'`.|
|*cfg*.stateOff|string|Optional. The state message payload to publish when the siwtch is OFF. Default value `'OFF'`.|
|*cfg*.cmdOn|string|Optional. The command message payload for turning the siwtch ON. Default value `'ON'`.|
|*cfg*.cmdOff|string|Optional. The command message payload for turning the siwtch OFF. Default value `'OFF'`.|
|*cfg*.cmdToggle|string|Optional. The command message payload for toggling the siwtch state. Default value `'TOGGLE'`.|

**Environment variables for MQTT topics**
Both `stateTopic` and `cmdTopic` parameters can contain one or more of following environment variables.

|Environment variable||
|--|--|
|${device_id}|The device ID.|
|${zthing_id}|The switch ID.|

**Example** - Create a switch using default configuration values and attach it to MQTT services.
```js
let sw = ZenSwitch.create('sw-1');
let success = sw.MQTT.attach('$zt/${device_id}/${zthing_id}/state', '$zt/${device_id}/${zthing_id}/cmd', {
    stateOn: 'on',
    stateOff: 'off',
    cmdOn: 'turn_on',
    cmdOff: 'turn_off',
    cmdToggle: 'toggle'
  });
```
### .MQTT.detach()
```js
sw.MQTT.detach();
```
Detach the switch from GPIO services that were previously attached using `.MQTT.attach()`.
## Additional resources
Take a look to some other demo samples.

|Sample|Notes|
|--|--|
|[zswitch-demo](https://github.com/zendiy-mgos/zswitch-demo)|Mongoose OS demo firmware for using ZenSwitches.|
|[zswitch-gpio-demo](https://github.com/zendiy-mgos/zswitch-gpio-demo)|Mongoose OS demo firmware for using GPIO-enabled ZenSwitches.|
