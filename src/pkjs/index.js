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

Pebble.addEventListener('showConfiguration', function() {
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
