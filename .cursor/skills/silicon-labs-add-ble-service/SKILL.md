# Silicon Labs BLE GATT Service Skill

Use this skill when adding, modifying, or validating BLE GATT services in a Silicon Labs SLC/Simplicity SDK project.

The skill uses one workflow: define the GATT database with Silicon Labs Profile Toolkit XML, place the XML under `config/btconf/`, run `slc generate`, then add application behavior using the generated `gattdb_*` symbols.

Do not add a BLE service by creating a new standalone service source/header pair and linking it in CMake. The GATT database belongs in GATT Configurator/Profile Toolkit XML. C code is only for behavior above that generated database.

## Project discovery

1. Locate the project `.slcp`.
2. Locate `config/btconf/`.
3. Locate the base GATT file if present, usually:
   - `config/btconf/gatt_configuration.btconf`
   - `config/btconf/gatt_configuration.xml`
4. Locate the Silicon Labs SDK package path from the project, environment, or user-provided command context.
5. Inspect existing `app.c` or the existing Bluetooth application event module before adding behavior.

Use placeholders in commands:

```sh
<project>.slcp
<project_dir>
<sdk_path>
```

## Add or modify the GATT database

Use Studio-generated Profile Toolkit XML resources from this skill:

```text
resources/btconf/profiles/
resources/btconf/services/
resources/btconf/characteristics/
resources/btconf/descriptors/
```

For a normal predefined service, copy from:

```text
resources/btconf/services/<service>.xml
```

For a predefined multi-service profile, copy from:

```text
resources/btconf/profiles/<profile>.xml
```

Destination:

```text
config/btconf/<service_or_profile>.xml
```

For a custom UUID-based service, create or edit Profile Toolkit XML in `config/btconf/`. It may be added as a separate XML fragment or inside the base `gatt_configuration.btconf`, matching the project style.

Example custom fragment shape:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<gatt gatt_caching="true" generic_attribute_service="true" header="gatt_db.h" name="Custom BLE GATT" out="gatt_db.c" prefix="gattdb_">
  <service advertise="false" id="custom_service" name="Custom Service" requirement="mandatory" sourceId="" type="primary" uuid="00000000-0000-0000-0000-000000000000">
    <characteristic const="false" id="custom_characteristic" name="Custom Characteristic" sourceId="" uuid="00000000-0000-0000-0000-000000000001">
      <value length="1" type="hex" variable_length="false">00</value>
      <properties>
        <read authenticated="false" bonded="false" encrypted="false"/>
        <write authenticated="false" bonded="false" encrypted="false"/>
        <notify authenticated="false" bonded="false" encrypted="false"/>
      </properties>
    </characteristic>
  </service>
</gatt>
```

Use project-appropriate UUIDs, IDs, names, lengths, value types, security properties, and notify/indicate descriptors.

Notes:

- `profiles/` and `services/` contain build-ready Profile Toolkit `<gatt>` fragments.
- `characteristics/` and `descriptors/` are useful when composing or inspecting GATT content, but they are normally not copied alone into `config/btconf/`.
- If the project already contains the same service or characteristic IDs, choose unique XML IDs before generation so generated `gattdb_*` symbols are unambiguous.
- Studio may suffix IDs when adding duplicate services through the GUI, for example `immediate_alert_0`.
- Do not copy SIG-native `gatt_xml` files into `config/btconf/`.

## Generate and validate

After changing `config/btconf`, run:

```sh
slc generate -p <project>.slcp -d <project_dir> --sdk-package-path <sdk_path>
```

Validate:

1. `slc generate` completes successfully.
2. `autogen/gatt_db.h` contains the expected `gattdb_*` handles.
3. `autogen/gatt_db.c` reflects the expected services, characteristics, descriptors, and properties.
4. Application code uses generated symbols from `gatt_db.h`, not hard-coded handles.

If the project uses CMake/Ninja, build with the project’s existing build command after generation.

## Make the service usable in application code

The XML adds the service to the GATT database. Application code makes the service do something.

Default to modifying the existing `app.c` or existing Bluetooth application event module. Do not create a new service `.c` / `.h` pair unless the project already has that pattern and the user explicitly wants a reusable service module.

Common application responsibilities:

- initialize characteristic values on boot
- respond to user read requests
- respond to user write requests
- track CCCD changes for notify/indicate characteristics
- send notifications or indications only when enabled by the client
- stop timers or clear state when the connection closes

Include headers already used by the project. Typical additions are:

```c
#include "gatt_db.h"
#include "sl_bt_api.h"
#include "app_assert.h"
#include "app_timer.h"
```

### Initialize generated attributes

Use this when a characteristic value should be initialized at boot or updated from application state:

```c
case sl_bt_evt_system_boot_id: {
  uint8_t value = 0;
  sc = sl_bt_gatt_server_write_attribute_value(gattdb_custom_characteristic,
                                               0,
                                               sizeof(value),
                                               &value);
  app_assert_status(sc);
  break;
}
```

### Handle user reads

Use this for characteristics whose value is provided by the application at read time:

```c
case sl_bt_evt_gatt_server_user_read_request_id:
  if (evt->data.evt_gatt_server_user_read_request.characteristic == gattdb_custom_characteristic) {
    uint8_t value = get_current_value();
    sc = sl_bt_gatt_server_send_user_read_response(
      evt->data.evt_gatt_server_user_read_request.connection,
      evt->data.evt_gatt_server_user_read_request.characteristic,
      0,
      sizeof(value),
      &value,
      NULL);
    app_assert_status(sc);
  }
  break;
```

### Handle user writes

Validate length, apply the value, and send a write response:

```c
case sl_bt_evt_gatt_server_user_write_request_id:
  if (evt->data.evt_gatt_server_user_write_request.characteristic == gattdb_custom_characteristic) {
    uint8_t att_errorcode = 0;

    if (evt->data.evt_gatt_server_user_write_request.value.len == 1) {
      set_current_value(evt->data.evt_gatt_server_user_write_request.value.data[0]);
    } else {
      att_errorcode = 0x0D; // Invalid Attribute Value Length
    }

    sc = sl_bt_gatt_server_send_user_write_response(
      evt->data.evt_gatt_server_user_write_request.connection,
      evt->data.evt_gatt_server_user_write_request.characteristic,
      att_errorcode);
    app_assert_status(sc);
  }
  break;
```

### Track notification or indication enablement

Use the characteristic status event to detect CCCD changes:

```c
static uint8_t app_connection = 0xff;
static bool custom_notify_enabled = false;

case sl_bt_evt_gatt_server_characteristic_status_id:
  if ((evt->data.evt_gatt_server_characteristic_status.characteristic == gattdb_custom_characteristic)
      && (evt->data.evt_gatt_server_characteristic_status.status_flags == sl_bt_gatt_server_client_config)) {
    app_connection = evt->data.evt_gatt_server_characteristic_status.connection;
    custom_notify_enabled = (evt->data.evt_gatt_server_characteristic_status.client_config_flags != sl_bt_gatt_disable);
  }
  break;
```

### Send a notification

```c
if (custom_notify_enabled) {
  uint8_t value = get_current_value();
  sc = sl_bt_gatt_server_send_notification(app_connection,
                                           gattdb_custom_characteristic,
                                           sizeof(value),
                                           &value);
  app_assert_status(sc);
}
```

### Send an indication

```c
if (custom_indication_enabled) {
  uint8_t value = get_current_value();
  sc = sl_bt_gatt_server_send_indication(app_connection,
                                         gattdb_custom_characteristic,
                                         sizeof(value),
                                         &value);
  app_assert_status(sc);
}
```

For indications, also handle confirmation if the application needs pacing or reliability state:

```c
case sl_bt_evt_gatt_server_characteristic_status_id:
  if ((evt->data.evt_gatt_server_characteristic_status.characteristic == gattdb_custom_characteristic)
      && (evt->data.evt_gatt_server_characteristic_status.status_flags == sl_bt_gatt_server_confirmation)) {
    // Indication confirmed by the client.
  }
  break;
```

### Clean up on disconnect

```c
case sl_bt_evt_connection_closed_id:
  app_connection = 0xff;
  custom_notify_enabled = false;
  custom_indication_enabled = false;
  (void)app_timer_stop(&custom_timer);
  break;
```

## Resource refresh

The XML catalog is generated from the Simplicity Studio GATT Configurator bundle.

Refresh only when updating the skill resources from a newer Studio installation:

```sh
cd resources
node export_from_studio_vendor.cjs
```

The exporter is dependency-free. It uses Node built-ins and the GATT Configurator JavaScript bundle shipped with Studio. Do not check in `node_modules`.

Keep generated XML resources checked in so the skill works without requiring every user or agent run to execute the exporter.

## Common checks

- Keep GATT database changes in `config/btconf` Profile Toolkit XML.
- Use `resources/btconf/services/` or `resources/btconf/profiles/` for predefined services and profiles.
- Use custom Profile Toolkit XML for custom UUID-based services.
- Run `slc generate` after every `config/btconf` change.
- Use `autogen/gatt_db.h` symbols in C code.
- Do not edit `autogen/gatt_db.c` or `autogen/gatt_db.h` manually.
- Do not add service source files as a substitute for GATT XML configuration.
