/*
 * Copyright (c) 2020 ZenDIY
 * All rights reserved
 *
 * Licensed under the Apache License, Version 2.0 (the ""License"");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an ""AS IS"" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MGOS_ZSWITCH_MQTT_H_
#define MGOS_ZSWITCH_MQTT_H_

#include <stdio.h>
#include <stdbool.h>
#include "mgos_zswitch.h"

#ifdef __cplusplus
extern "C" {
#endif


#define MGOS_ZSWITCH_MQTT_CFG { NULL, NULL }

struct mgos_zswitch_mqtt_cfg {
  const char *state_on;
  const char *state_off;
  const char *cmd_on;
  const char *cmd_off;
  const char *cmd_toggle; 
};

bool mgos_zswitch_mqtt_attach(struct mgos_zswitch *handle,
                              const char *state_topic,
                              const char *cmd_topic,
                              struct mgos_zswitch_mqtt_cfg *cfg);

bool mgos_zswitch_mqtt_detach(struct mgos_zswitch *handle);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MGOS_ZSWITCH_MQTT_H_ */