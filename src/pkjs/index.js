// Import the Clay package
var Clay = require('@rebble/clay');
// Load our Clay configuration file
var clayConfig = require('./config');
// Load the custom function that adds accordion behavior to the config page
var customClay = require('./custom-clay');
// Initialize Clay
var clay = new Clay(clayConfig, customClay);