/**
 * Thin runner for Simplicity Studio's GATT Configurator converter.
 *
 * Studio does not ship a standalone export CLI. The SIG → btconf conversion
 * lives in the GATT Configurator plugin bundle:
 *   ~/.silabs/slt/installs/archive/v6-base-.../gattConfigurator/js/vendor.HASH.js
 *
 * Loads vendor.*.js and calls the same functions the UI uses on Add Predefined:
 *   sigToGatt, sigToService, sigToCharacteristic, sigToDescriptor + gattToXml
 *
 * Usage:
 *   node export_from_studio_vendor.cjs
 *   node export_from_studio_vendor.cjs --out btconf
 */
const fs = require("fs");
const path = require("path");

const HERE = __dirname;

function findVendorJs() {
  const archive = path.join(
    process.env.USERPROFILE || process.env.HOME || "",
    ".silabs/slt/installs/archive",
  );
  if (!fs.existsSync(archive)) {
    throw new Error(`Studio archive not found: ${archive}`);
  }

  const bases = fs
    .readdirSync(archive)
    .filter((d) => d.startsWith("v6-base-"))
    .sort()
    .reverse();

  for (const base of bases) {
    const pluginsDir = path.join(archive, base, "SimplicityStudio-6/sts_back_end/plugins");
    if (!fs.existsSync(pluginsDir)) continue;

    const plugin = fs
      .readdirSync(pluginsDir)
      .find((d) => d.startsWith("com.silabs.ss.tool_gatt_configurator_"));
    if (!plugin) continue;

    const jsDir = path.join(pluginsDir, plugin, "btconf/gattConfigurator/js");
    const vendor = fs.readdirSync(jsDir).find((f) => /^vendor\..+\.js$/.test(f));
    if (vendor) return path.join(jsDir, vendor);
  }

  throw new Error(
    "GATT Configurator vendor.*.js not found. Install Simplicity Studio v6 with the GATT Configurator plugin.",
  );
}

function loadStudioConverter(vendorPath) {
  const allModules = {};
  global.window = global;
  global.self = global;
  global.webpackJsonp = [];
  global.webpackJsonp.push = function (chunk) {
    Object.assign(allModules, chunk[1]);
    return Array.prototype.push.call(this, chunk);
  };

  require(vendorPath);

  const cache = {};
  const wreq = (id) => {
    if (cache[id]) return cache[id].exports;
    if (!allModules[id]) throw new Error(`Studio module missing: ${id}`);
    const mod = { exports: {} };
    cache[id] = mod;
    allModules[id](mod, mod.exports, wreq);
    return mod.exports;
  };

  const ec72 = wreq("ec72");
  const models = wreq("2926");
  const { ServiceDeclarationType } = wreq("3e83");

  return {
    sigToGatt: ec72.sigToGatt,
    sigToService: ec72.sigToService,
    sigToCharacteristic: ec72.sigToCharacteristic,
    sigToDescriptor: ec72.sigToDescriptor,
    gattToXml: wreq("350f").default.gattToXml,
    SigMetadata: wreq("ec55").default,
    Gatt: models.Gatt,
    Service: models.Service,
    Characteristic: models.Characteristic,
    ServiceDeclarationType,
  };
}

function shortName(sigType) {
  const parts = sigType.split(".");
  return parts[parts.length - 1].replace(/-/g, "_");
}

function normalizeGattFragment(xml) {
  return xml
    .replace(/^\s*<\?xml[^?]*\?>\s*/i, "")
    .replace(/<gatt\b[^>]*>/, "<gatt>")
    .trimEnd();
}

function writeFragment(outDir, filename, body) {
  fs.mkdirSync(outDir, { recursive: true });
  fs.writeFileSync(path.join(outDir, filename), `${body}\n`, "utf8");
  console.log(`wrote ${path.relative(HERE, path.join(outDir, filename))}`);
}

function gattXml(converter, gatt) {
  return normalizeGattFragment(converter.gattToXml(gatt));
}

function extractElement(xml, tag, id) {
  const re = new RegExp(
    `(<!--[^\\n]*-->\\s*)?<${tag}\\b[^>]*\\bid="${id}"[\\s\\S]*?</${tag}>`,
    "m",
  );
  const match = xml.match(re);
  if (!match) {
    throw new Error(`could not extract <${tag} id="${id}"> from generated XML`);
  }
  return match[0].trim();
}

function stubService(converter, characteristics) {
  return new converter.Service({
    name: "Stub",
    uuid: "FFFF",
    sigType: "custom.stub",
    info: "",
    id: "stub",
    capabilities: [],
    advertise: false,
    declarationType: converter.ServiceDeclarationType.secondary,
    includes: [],
    characteristics,
  });
}

function exportProfiles(converter, outDir) {
  let wrote = 0;
  for (const [type, sigItem] of converter.SigMetadata.profiles) {
    const gatt = converter.sigToGatt(sigItem);
    const xml = gattXml(converter, gatt);
    writeFragment(outDir, `${shortName(type)}.xml`, xml);
    wrote++;
  }
  return wrote;
}

function exportServices(converter, outDir) {
  let wrote = 0;
  for (const [type, sigItem] of converter.SigMetadata.services) {
    const service = converter.sigToService(sigItem);
    const gatt = new converter.Gatt({
      gattCaching: true,
      genericAttributeService: false,
      name: sigItem.name,
      services: [service],
    });
    writeFragment(outDir, `${shortName(type)}.xml`, gattXml(converter, gatt));
    wrote++;
  }
  return wrote;
}

function exportCharacteristics(converter, outDir) {
  let wrote = 0;
  for (const [type, sigItem] of converter.SigMetadata.characteristics) {
    const characteristic = converter.sigToCharacteristic(sigItem);
    const gatt = new converter.Gatt({
      gattCaching: true,
      genericAttributeService: false,
      name: sigItem.name,
      services: [stubService(converter, [characteristic])],
    });
    const xml = extractElement(gattXml(converter, gatt), "characteristic", characteristic.id);
    writeFragment(outDir, `${shortName(type)}.xml`, xml);
    wrote++;
  }
  return wrote;
}

function exportDescriptors(converter, outDir) {
  let wrote = 0;
  for (const [type, sigItem] of converter.SigMetadata.descriptors) {
    const descriptor = converter.sigToDescriptor(sigItem);
    const stubChar = new converter.Characteristic({
      name: "Stub",
      uuid: "FFFE",
      sigType: "custom.stub_char",
      info: "",
      id: "stub_char",
      capabilities: [],
      value: descriptor.value,
      permissions: descriptor.permissions,
      descriptors: [descriptor],
    });
    const gatt = new converter.Gatt({
      gattCaching: true,
      genericAttributeService: false,
      name: sigItem.name,
      services: [stubService(converter, [stubChar])],
    });
    const xml = extractElement(gattXml(converter, gatt), "descriptor", descriptor.id);
    writeFragment(outDir, `${shortName(type)}.xml`, xml);
    wrote++;
  }
  return wrote;
}

function parseArgs(argv) {
  const outIdx = argv.indexOf("--out");
  return {
    out: path.resolve(HERE, outIdx >= 0 ? argv[outIdx + 1] : "btconf"),
  };
}

function main() {
  const { out } = parseArgs(process.argv.slice(2));
  const vendorPath = findVendorJs();
  console.log(`studio vendor: ${vendorPath}`);
  const converter = loadStudioConverter(vendorPath);

  const counts = {
    profiles: exportProfiles(converter, path.join(out, "profiles")),
    services: exportServices(converter, path.join(out, "services")),
    characteristics: exportCharacteristics(converter, path.join(out, "characteristics")),
    descriptors: exportDescriptors(converter, path.join(out, "descriptors")),
  };

  console.log(
    `exported ${counts.profiles} profiles, ${counts.services} services, ` +
      `${counts.characteristics} characteristics, ${counts.descriptors} descriptors → ${out}`,
  );
}

main();
