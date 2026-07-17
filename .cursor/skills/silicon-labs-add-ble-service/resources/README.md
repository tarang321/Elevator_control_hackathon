# GATT Configurator resources

This directory contains Studio-generated Profile Toolkit XML used by the skill.

```text
btconf/
  profiles/          multi-service profile bundles generated like Studio Add Predefined profiles
  services/          build-ready single-service fragments for config/btconf/
  characteristics/   standalone characteristic fragments for lookup/composition
  descriptors/       standalone descriptor fragments for lookup/composition
studio-sig-catalog.json
export_from_studio_vendor.cjs
```

## Normal CLI workflow

For a service with no SDK `.slcc` component:

```text
copy resources/btconf/services/<service>.xml -> <project>/config/btconf/<service>.xml
run slc generate
```

Use `btconf/profiles/` when the intended addition is a full profile bundle that contains multiple services.

Do not copy `btconf/characteristics/*.xml` or `btconf/descriptors/*.xml` directly into `config/btconf/` as standalone build inputs. Use them to inspect UUIDs/properties or to compose a service XML.

## Refreshing the catalog

The checked-in XMLs are the normal payload. The exporter is only for refreshing the catalog from a newer Simplicity Studio install.

```powershell
cd .cursor/skills/silicon-labs-add-ble-service/resources
node export_from_studio_vendor.cjs
```

The exporter uses only Node built-ins (`fs`, `path`) and Studio's bundled GATT Configurator `vendor.*.js`; it does not require `node_modules` or npm packages.

Expected Studio bundle location pattern:

```text
%USERPROFILE%\.silabs\slt\installs\archive\v6-base-*\SimplicityStudio-6\sts_back_end\plugins\com.silabs.ss.tool_gatt_configurator_*\btconf\gattConfigurator\js\vendor.*.js
```

Do not store `node_modules/`, `package-lock.json`, scratch snippets, or SIG-native `gatt_xml` files here.
