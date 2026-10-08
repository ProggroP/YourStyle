// Import the Clay package
var Clay = require('@rebble/clay');
// Load our Clay configuration file
var clayConfig = require('./config');
// Load the custom function that adds accordion behavior and presets
var customClay = require('./custom-clay');

// autoHandleEvents is off because the preset storage (messageKey
// "presetblob") must never reach the watch: it is not declared in
// package.json, and Clay's automatic handler would try to send it,
// which makes sendAppMessage fail for the whole settings dictionary.
var clay = new Clay(clayConfig, customClay, { autoHandleEvents: false });

// Repairs stored settings whose type no longer matches the config.
//
// This is not cosmetic. Clay feeds every stored value straight into its
// component's set(), and those setters assume the type the component uses
// today. When "stepsontop" changed from a toggle to a radio group, phones
// still held a boolean for it, and the radio group's setter calls
// value.replace() -- which a boolean does not have. The exception aborted
// the page build half way through: everything below that item vanished,
// including the Save button, and no custom code ever ran.
//
// So whenever a setting's component type changes, old values must be
// converted rather than handed over as they are. Coercing here also repairs
// the stored copy, so it only ever happens once per phone.
function sanitizeStoredSettings() {
  var stored;
  try {
    stored = JSON.parse(localStorage.getItem('clay-settings') || '{}');
  } catch (e) {
    return;
  }
  if (!stored || typeof stored !== 'object') { return; }

  var changed = false;

  function visit(node) {
    if (Array.isArray(node)) { node.forEach(visit); return; }
    if (!node || typeof node !== 'object') { return; }
    if (node.items) { visit(node.items); }
    if (!node.messageKey || !(node.messageKey in stored)) { return; }

    var key = node.messageKey;
    var value = stored[key];

    if (node.type === 'radiogroup' || node.type === 'select') {
      var allowed = (node.options || []).map(function(o) { return String(o.value); });
      var asString = (typeof value === 'boolean') ? (value ? '1' : '0') : String(value);
      if (allowed.indexOf(asString) === -1) {
        delete stored[key];          // nothing sensible to map to: use the default
        changed = true;
      } else if (value !== asString) {
        stored[key] = asString;
        changed = true;
      }
    } else if (node.type === 'toggle') {
      var asBool = (value === true || value === 1 || value === '1');
      if (value !== asBool) { stored[key] = asBool; changed = true; }
    } else if (node.type === 'slider' || node.type === 'color') {
      if (typeof value !== 'number') {
        var asNum = (typeof value === 'string' && /^(0x|#)/.test(value)) ?
          parseInt(value.replace(/^#|^0x/, ''), 16) : parseFloat(value);
        if (isNaN(asNum)) { delete stored[key]; } else { stored[key] = asNum; }
        changed = true;
      }
    }
  }

  visit(clayConfig);

  if (changed) {
    localStorage.setItem('clay-settings', JSON.stringify(stored));
    console.log('Repaired stored settings that no longer matched their type');
  }
}

Pebble.addEventListener('showConfiguration', function() {
  sanitizeStoredSettings();

  // With autoHandleEvents off, Clay only fills clay.meta on its "ready"
  // handler. If that has not run, activeWatchInfo stays null and the color
  // picker silently falls back to its black/white layout -- every color
  // would snap to black or white. Populate it here so the page never
  // depends on event ordering.
  clay.meta = {
    activeWatchInfo: Pebble.getActiveWatchInfo ? Pebble.getActiveWatchInfo() : null,
    accountToken: Pebble.getAccountToken(),
    watchToken: Pebble.getWatchToken(),
    userData: {}
  };
  Pebble.openURL(clay.generateUrl());
});

Pebble.addEventListener('webviewclosed', function(e) {
  if (!e || !e.response) { return; }

  // convert=false: parse the response and persist everything (including
  // the preset blob) to localStorage, but leave the values unconverted
  // so we can filter before building the AppMessage dictionary.
  var settings;
  try {
    settings = clay.getSettings(e.response, false);
  } catch (err) {
    console.log('Config response invalid: ' + err.message);
    return;
  }

  // presets live only on the phone
  delete settings.presetblob;

  Pebble.sendAppMessage(Clay.prepareSettingsForAppMessage(settings),
    function() {
      console.log('Sent config data to Pebble');
    },
    function(error) {
      console.log('Failed to send config data! ' + JSON.stringify(error));
    });
});
