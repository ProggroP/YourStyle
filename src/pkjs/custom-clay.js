// Adds two things to the generated Clay config page:
//
//  * accordion behaviour, so each section heading collapses the items below it
//    and the very long page stays manageable;
//  * the preset system: five named slots plus copy-and-paste preset codes.
//
// Note that Clay ships this whole function to the config page as source text
// (it is stringified with toSource), so everything it needs must live inside
// it. There is no require() and no access to the surrounding module here.

module.exports = function (minified) {
  var clayConfig = this;

  var PRESET_SLOTS = 5;

  // Filled from the built page; the codec compares against these and restores
  // them for any field a code does not mention.
  var defaultsByKey = {};

  // ------------------------------------------------------------------
  // Preset codes
  // ------------------------------------------------------------------
  //
  // A preset code is a short, self-contained string that carries a whole
  // configuration, so it can be written down, shared, and pasted back after
  // a reinstall or on another phone.
  //
  // FORMAT (bit stream, most significant bit first, then Crockford base32)
  //
  //   header   8 bit  number of fields the encoder knew (see PRESET_FIELDS)
  //            1 bit  mode: 0 = only the changed fields, 1 = every field
  //   mode 0   8 bit  how many changed fields follow
  //            per entry: 8 bit field index, then that field's value bits
  //   mode 1   every field's value bits, in PRESET_FIELDS order
  //   tail    16 bit  checksum over the payload bytes
  //
  // Mode 0 is normally much shorter; the encoder builds both and keeps the
  // smaller one, which is why codes vary in length. Nothing about that makes
  // them ambiguous: the header states the mode and the entry count, so the
  // reader always knows exactly how many bits to expect, and the checksum
  // rejects anything damaged rather than applying it half way.
  //
  // TWO RULES THAT KEEP OLD CODES WORKING -- please read before editing:
  //
  //  1. PRESET_FIELDS is append-only. Never insert, reorder or delete a row:
  //     the row position IS the field index stored inside every code that
  //     users already have. Adding new settings at the end is safe, and a
  //     code made before they existed simply leaves them at their default.
  //
  //  2. The width and offset columns are frozen, deliberately generous, and
  //     independent of config.json. If a slider's range in config.json is
  //     widened later (as moonradius went from 30 to 130), the stored width
  //     must stay as it is -- deriving widths from the live config would
  //     silently invalidate every code in circulation. Only if a value can
  //     no longer fit does the width need to grow, and that requires a new
  //     row rather than a changed one.
  //
  // Defaults must likewise stay put once shipped: decoding resets every
  // field to its default first and then applies the code, so a changed
  // default would quietly alter presets that omit that field.
  //
  // Column meaning: [messageKey, kind, bits, offset]
  //   kind 'b' toggle (1 bit)        'c' colour (6 bit palette index)
  //        'r' radio group (option number)
  //        'n' number, stored as value + offset
  var PRESET_FIELDS = [
    ['screenoffsety', 'n', 5, 8],
    ['screenoffsetx', 'n', 5, 8],
    ['backcolor', 'c', 6, 0],
    ['hourdialcolor', 'c', 6, 0],
    ['minsdialcolor', 'c', 6, 0],
    ['secsdialcolor', 'c', 6, 0],
    ['hourfillcolor', 'c', 6, 0],
    ['minsfillcolor', 'c', 6, 0],
    ['secsfillcolor', 'c', 6, 0],
    ['hourmarkcolor', 'c', 6, 0],
    ['minsmarkcolor', 'c', 6, 0],
    ['discretehands', 'b', 1, 0],
    ['hourlength', 'n', 8, 0],
    ['minslength', 'n', 8, 0],
    ['secslength', 'n', 8, 0],
    ['hourbalancelength', 'n', 8, 0],
    ['minsbalancelength', 'n', 8, 0],
    ['secsbalancelength', 'n', 8, 0],
    ['hourneedlestart', 'n', 8, 0],
    ['minsneedlestart', 'n', 8, 0],
    ['secsneedlestart', 'n', 8, 0],
    ['hourfillinnerpos', 'n', 8, 0],
    ['hourfillouterpos', 'n', 8, 0],
    ['minsfillinnerpos', 'n', 8, 0],
    ['minsfillouterpos', 'n', 8, 0],
    ['secsfillinnerpos', 'n', 8, 0],
    ['secsfillouterpos', 'n', 8, 0],
    ['hourmarkinnerpos', 'n', 8, 0],
    ['hourmarkouterpos', 'n', 8, 0],
    ['minsmarkinnerpos', 'n', 8, 0],
    ['minsmarkouterpos', 'n', 8, 0],
    ['hourmarkstyle', 'r', 4, 0],
    ['hourmarkrim', 'b', 1, 0],
    ['minutemarkstyle', 'r', 4, 0],
    ['minsmarkrim', 'b', 1, 0],
    ['shownumbers', 'b', 1, 0],
    ['numberfont', 'r', 4, 0],
    ['numberstyle', 'r', 4, 0],
    ['numberset', 'r', 4, 0],
    ['numberpos', 'n', 8, 0],
    ['numberrim', 'b', 1, 0],
    ['numbercolor', 'c', 6, 0],
    ['hourthickness', 'n', 6, 0],
    ['hourfillthickness', 'n', 6, 0],
    ['minsthickness', 'n', 6, 0],
    ['minsfillthickness', 'n', 6, 0],
    ['secsthickness', 'n', 6, 0],
    ['secsfillthickness', 'n', 6, 0],
    ['hourmarkthickness', 'n', 6, 0],
    ['minsmarkthickness', 'n', 6, 0],
    ['showcenterhub', 'b', 1, 0],
    ['centerhubauto', 'b', 1, 0],
    ['centerhubcolor', 'c', 6, 0],
    ['showcenteraxis', 'b', 1, 0],
    ['centeraxiscolor', 'c', 6, 0],
    ['date', 'b', 1, 0],
    ['dateshape', 'r', 4, 0],
    ['dateposdegree', 'n', 10, 0],
    ['datepositionrim', 'n', 8, 0],
    ['datesize', 'r', 4, 0],
    ['datetextcolor', 'c', 6, 0],
    ['datebackcolor', 'c', 6, 0],
    ['datebordercolor', 'c', 6, 0],
    ['bluetooth', 'b', 1, 0],
    ['btalways', 'b', 1, 0],
    ['btvibe', 'b', 1, 0],
    ['btshape', 'r', 4, 0],
    ['btposdegree', 'n', 10, 0],
    ['btpositionrim', 'n', 8, 0],
    ['btsize', 'r', 4, 0],
    ['btcolor', 'c', 6, 0],
    ['btbackcolor', 'c', 6, 0],
    ['btbordercolor', 'c', 6, 0],
    ['btoffcolor', 'c', 6, 0],
    ['btoffbackcolor', 'c', 6, 0],
    ['btoffbordercolor', 'c', 6, 0],
    ['battery', 'b', 1, 0],
    ['battwarnlevel', 'n', 8, 0],
    ['battshape', 'r', 4, 0],
    ['battposdegree', 'n', 10, 0],
    ['battpositionrim', 'n', 8, 0],
    ['battsize', 'r', 4, 0],
    ['battcolor', 'c', 6, 0],
    ['battbackcolor', 'c', 6, 0],
    ['battbordercolor', 'c', 6, 0],
    ['battwarncolor', 'c', 6, 0],
    ['battwarnbackcolor', 'c', 6, 0],
    ['battwarnbordercolor', 'c', 6, 0],
    ['fullscreen', 'b', 1, 0],
    ['showsecond', 'b', 1, 0],
    ['showminsmark', 'b', 1, 0],
    ['showhourmark', 'b', 1, 0],
    ['showmoonphase', 'b', 1, 0],
    ['moonposdegree', 'n', 10, 0],
    ['moonpositionrim', 'n', 8, 0],
    ['moonradius', 'n', 9, 0],
    ['moonlightcolor', 'c', 6, 0],
    ['moondarkcolor', 'c', 6, 0],
    ['moonborder', 'b', 1, 0],
    ['moonbordercolor', 'c', 6, 0],
    ['moonhemisphere', 'b', 1, 0],
    ['showsteps', 'b', 1, 0],
    ['stepsstyle', 'r', 4, 0],
    ['stepsontop', 'r', 4, 0],
    ['stepsposdegree', 'n', 10, 0],
    ['stepspositionrim', 'n', 8, 0],
    ['stepssize', 'r', 4, 0],
    ['stepsshape', 'r', 4, 0],
    ['stepslabelcolor', 'c', 6, 0],
    ['stepsvaluecolor', 'c', 6, 0],
    ['stepstextbackcolor', 'c', 6, 0],
    ['stepstextbordercolor', 'c', 6, 0],
    ['stepsdialradius', 'n', 8, 0],
    ['stepsneedlelength', 'n', 8, 0],
    ['stepsneedlebalancelength', 'n', 7, 0],
    ['stepsneedlestart', 'n', 8, 0],
    ['stepsneedlethickness', 'n', 5, 0],
    ['stepsneedlecolor', 'c', 6, 0],
    ['stepssegment', 'b', 1, 0],
    ['stepsfillinnerpos', 'n', 8, 0],
    ['stepsfillouterpos', 'n', 8, 0],
    ['stepsfillthickness', 'n', 5, 0],
    ['stepsfillcolor', 'c', 6, 0],
    ['stepsdialborder', 'b', 1, 0],
    ['stepsdialborderthickness', 'r', 4, 0],
    ['stepsdialbordercolor', 'c', 6, 0],
    ['stepsmarkcolor', 'c', 6, 0],
    ['stepsmarkthickness', 'n', 5, 0],
    ['stepsmarkinnerpos', 'n', 8, 0],
    ['stepsmarkouterpos', 'n', 8, 0]
  ];

  // Crockford base32: no I, L, O or U, so nothing looks like anything else.
  var CODE_ALPHABET = '0123456789ABCDEFGHJKMNPQRSTVWXYZ';
  var CODE_GROUP = 5;          // dashes every 5 characters, for readability
  var CODE_PREFIX = 'YS';      // marks the code as belonging to this watchface

  // Pebble's palette has four levels per channel, so a colour is 6 bits.
  var COLOR_LEVELS = [0, 85, 170, 255];

  function colorToIndex(value) {
    var v = (typeof value === 'string') ? parseInt(value.replace(/^#|^0x/, ''), 16) : (value | 0);
    var chan = [(v >> 16) & 0xFF, (v >> 8) & 0xFF, v & 0xFF];
    var idx = 0;
    for (var c = 0; c < 3; c++) {
      var best = 0, bestDiff = 999;
      for (var l = 0; l < 4; l++) {
        var d = Math.abs(COLOR_LEVELS[l] - chan[c]);
        if (d < bestDiff) { bestDiff = d; best = l; }
      }
      idx = (idx << 2) | best;
    }
    return idx;
  }

  function indexToColor(idx) {
    var r = COLOR_LEVELS[(idx >> 4) & 3];
    var g = COLOR_LEVELS[(idx >> 2) & 3];
    var b = COLOR_LEVELS[idx & 3];
    return (r << 16) | (g << 8) | b;
  }

  function BitWriter() {
    this.bytes = [];
    this.cur = 0;
    this.nbits = 0;
  }
  BitWriter.prototype.write = function (value, width) {
    for (var i = width - 1; i >= 0; i--) {
      this.cur = (this.cur << 1) | ((value >> i) & 1);
      this.nbits++;
      if (this.nbits === 8) { this.bytes.push(this.cur); this.cur = 0; this.nbits = 0; }
    }
  };
  BitWriter.prototype.finish = function () {
    if (this.nbits > 0) { this.bytes.push(this.cur << (8 - this.nbits)); this.cur = 0; this.nbits = 0; }
    return this.bytes;
  };

  function BitReader(bytes) {
    this.bytes = bytes;
    this.pos = 0;
  }
  BitReader.prototype.read = function (width) {
    var out = 0;
    for (var i = 0; i < width; i++) {
      var byteIdx = this.pos >> 3;
      if (byteIdx >= this.bytes.length) { throw new Error('code ends too early'); }
      var bit = (this.bytes[byteIdx] >> (7 - (this.pos & 7))) & 1;
      out = (out << 1) | bit;
      this.pos++;
    }
    return out;
  };

  // Small checksum over the payload. Only has to catch truncation and typos,
  // which it does reliably at 16 bits.
  function checksum16(bytes) {
    var h = 0x1234;
    for (var i = 0; i < bytes.length; i++) {
      h = (h ^ bytes[i]) & 0xFFFF;
      h = ((h << 5) - h + 0x9E37) & 0xFFFF;
    }
    return h & 0xFFFF;
  }

  function bytesToCode(bytes) {
    var bits = '', i;
    for (i = 0; i < bytes.length; i++) {
      var b = bytes[i].toString(2);
      while (b.length < 8) { b = '0' + b; }
      bits += b;
    }
    while (bits.length % 5 !== 0) { bits += '0'; }
    var out = '';
    for (i = 0; i < bits.length; i += 5) {
      out += CODE_ALPHABET.charAt(parseInt(bits.substr(i, 5), 2));
    }
    var grouped = '';
    for (i = 0; i < out.length; i += CODE_GROUP) {
      grouped += (grouped ? '-' : '') + out.substr(i, CODE_GROUP);
    }
    return CODE_PREFIX + '-' + grouped;
  }

  function codeToBytes(code) {
    var s = String(code).toUpperCase().replace(/[\s-]/g, '');
    if (s.indexOf(CODE_PREFIX) === 0) { s = s.slice(CODE_PREFIX.length); }
    // be forgiving about the characters the alphabet deliberately avoids
    s = s.replace(/O/g, '0').replace(/[IL]/g, '1').replace(/U/g, 'V');
    var bits = '', i;
    for (i = 0; i < s.length; i++) {
      var v = CODE_ALPHABET.indexOf(s.charAt(i));
      if (v < 0) { throw new Error('invalid character "' + s.charAt(i) + '"'); }
      var b = v.toString(2);
      while (b.length < 5) { b = '0' + b; }
      bits += b;
    }
    var bytes = [];
    for (i = 0; i + 8 <= bits.length; i += 8) {
      bytes.push(parseInt(bits.substr(i, 8), 2));
    }
    return bytes;
  }

  // Reads one field's value off the page and normalises it to an integer
  // that fits the frozen width.
  function fieldToInt(spec, raw) {
    var kind = spec[1], width = spec[2], offset = spec[3];
    var v;
    if (kind === 'b') {
      v = (raw === true || raw === 1 || raw === '1') ? 1 : 0;
    } else if (kind === 'c') {
      v = colorToIndex(raw);
    } else if (kind === 'r') {
      v = parseInt(raw, 10);
      if (isNaN(v) || v < 0) { v = 0; }
    } else {
      v = Math.round(parseFloat(raw));
      if (isNaN(v)) { v = 0; }
      v += offset;
    }
    var max = (1 << width) - 1;
    if (v < 0) { v = 0; }
    if (v > max) { v = max; }
    return v;
  }

  function intToField(spec, v) {
    var kind = spec[1], offset = spec[3];
    if (kind === 'b') { return v === 1; }
    if (kind === 'c') { return indexToColor(v); }
    if (kind === 'r') { return String(v); }
    return v - offset;
  }

  function encodePreset(values) {
    var n = PRESET_FIELDS.length, i, spec, ints = [], changed = [];
    for (i = 0; i < n; i++) {
      spec = PRESET_FIELDS[i];
      var raw = values[spec[0]];
      var def = defaultsByKey[spec[0]];
      // Compare in encoded form, not raw: a colour given as "0xFF0000" and one
      // given as 16711680 are the same setting and must not count as changed.
      var enc = fieldToInt(spec, typeof raw === 'undefined' ? def : raw);
      ints.push(enc);
      if (enc !== fieldToInt(spec, def)) {
        changed.push(i);
      }
    }

    function build(mode) {
      var w = new BitWriter();
      w.write(n, 8);
      w.write(mode, 1);
      if (mode === 0) {
        w.write(changed.length, 8);
        for (var j = 0; j < changed.length; j++) {
          w.write(changed[j], 8);
          w.write(ints[changed[j]], PRESET_FIELDS[changed[j]][2]);
        }
      } else {
        for (var k = 0; k < n; k++) { w.write(ints[k], PRESET_FIELDS[k][2]); }
      }
      var body = w.finish();
      var sum = checksum16(body);
      return body.concat([(sum >> 8) & 0xFF, sum & 0xFF]);
    }

    var deltaBytes = (changed.length <= 255) ? build(0) : null;
    var fullBytes = build(1);
    var chosen = (deltaBytes && deltaBytes.length <= fullBytes.length) ? deltaBytes : fullBytes;
    return bytesToCode(chosen);
  }

  // Returns a {key: value} map, or throws with a reason the user can act on.
  function decodePreset(code) {
    var bytes = codeToBytes(code);
    if (bytes.length < 4) { throw new Error('code is too short'); }
    var body = bytes.slice(0, bytes.length - 2);
    var want = (bytes[bytes.length - 2] << 8) | bytes[bytes.length - 1];
    if (checksum16(body) !== want) {
      throw new Error('code is damaged - please check you copied all of it');
    }

    var r = new BitReader(body);
    var n = r.read(8);
    if (n > PRESET_FIELDS.length) {
      throw new Error('code was made with a newer version of this watchface');
    }
    var mode = r.read(1);

    var out = {}, i, spec;
    // start from the defaults so fields the code omits are well defined
    for (i = 0; i < PRESET_FIELDS.length; i++) {
      spec = PRESET_FIELDS[i];
      out[spec[0]] = defaultsByKey[spec[0]];
    }

    if (mode === 0) {
      var count = r.read(8);
      for (i = 0; i < count; i++) {
        var idx = r.read(8);
        if (idx >= n) { throw new Error('code refers to an unknown setting'); }
        spec = PRESET_FIELDS[idx];
        out[spec[0]] = intToField(spec, r.read(spec[2]));
      }
    } else {
      for (i = 0; i < n; i++) {
        spec = PRESET_FIELDS[i];
        out[spec[0]] = intToField(spec, r.read(spec[2]));
      }
    }
    return out;
  }

  // One entry per collapsible section: id of its heading item, and the
  // group name shared by all items that belong to that section.
  var sections = [
    'presets',
    'screen',
    'colors',
    'lengths',
    'fillpos',
    'markpos',
    'hourmarkstyle',
    'minutemarkstyle',
    'numbers',
    'thickness',
    'center',
    'date',
    'bluetooth',
    'battery',
    'appearance',
    'moon',
    'steps'
  ];

  clayConfig.on(clayConfig.EVENTS.AFTER_BUILD, function () {
    // Defaults have to be read before anything else touches the page.
    clayConfig.getAllItems().forEach(function (item) {
      if (item.messageKey && item.messageKey !== 'presetblob') {
        defaultsByKey[item.messageKey] = item.config.defaultValue;
      }
    });

    sections.forEach(function (group) {
      var heading = clayConfig.getItemById('heading-' + group);
      if (!heading) {
        return;
      }

      var items = clayConfig.getItemsByGroup(group);
      var originalText = heading.get();
      var collapsed = true;

      function applyState() {
        items.forEach(function (item) {
          if (collapsed) {
            item.hide();
          } else {
            item.show();
          }
        });
        heading.set(
          '<span style="cursor:pointer">' +
            (collapsed ? '▸ ' : '▾ ') +
            originalText +
            '</span>'
        );
      }

      applyState();

      heading.on('click', function () {
        collapsed = !collapsed;
        applyState();
      });
    });

    // ---- Presets ----

    var blobItem = clayConfig.getItemByMessageKey('presetblob');
    var slotItem = clayConfig.getItemById('preset-slot');
    var nameItem = clayConfig.getItemById('preset-name');
    var saveBtn = clayConfig.getItemById('preset-save');
    var loadBtn = clayConfig.getItemById('preset-load');
    var codeItem = clayConfig.getItemById('preset-code');
    var codeGenBtn = clayConfig.getItemById('preset-code-gen');
    var codeApplyBtn = clayConfig.getItemById('preset-code-apply');

    if (!blobItem || !slotItem || !nameItem || !saveBtn || !loadBtn) {
      return;
    }

    // The blob input is pure storage; the user never edits it directly.
    // It stays hidden even when the presets section is expanded, so it
    // must be hidden after the accordion logic above has run.
    function hideBlob() {
      blobItem.hide();
    }
    hideBlob();
    clayConfig.getItemById('heading-presets').on('click', hideBlob);

    function readBlob() {
      var blob;
      try {
        blob = JSON.parse(blobItem.get() || '{}');
      } catch (e) {
        blob = {};
      }
      if (!blob || blob.v !== 1 ||
          !Array.isArray(blob.names) || !Array.isArray(blob.slots)) {
        blob = { v: 1, names: [], slots: [] };
      }
      while (blob.names.length < PRESET_SLOTS) { blob.names.push(''); }
      while (blob.slots.length < PRESET_SLOTS) { blob.slots.push(null); }
      return blob;
    }

    function writeBlob(blob) {
      blobItem.set(JSON.stringify(blob));
    }

    function slotIndex() {
      var i = parseInt(slotItem.get(), 10);
      return (i >= 0 && i < PRESET_SLOTS) ? i : 0;
    }

    // Show the stored names in the slot dropdown ("2: Worktime" etc.)
    function refreshSlotLabels(blob) {
      var select = slotItem.$manipulatorTarget[0];
      if (!select || !select.options) { return; }
      for (var i = 0; i < PRESET_SLOTS && i < select.options.length; i++) {
        select.options[i].text =
          (i + 1) + ': ' + (blob.names[i] || '—');
      }
      // sync the visible value label with the rewritten option text
      slotItem.trigger('change');
    }

    // Snapshot every real setting on the page (everything with a
    // messageKey except the blob itself).
    function snapshotForm() {
      var snapshot = {};
      clayConfig.getAllItems().forEach(function (item) {
        if (item.messageKey && item.messageKey !== 'presetblob') {
          snapshot[item.messageKey] = item.get();
        }
      });
      return snapshot;
    }

    function applySnapshot(preset) {
      Object.keys(preset).forEach(function (key) {
        if (key === 'presetblob') { return; }
        var item = clayConfig.getItemByMessageKey(key);
        if (item) {
          item.set(preset[key]);
        }
      });
    }

    // Briefly replace a button's label as feedback, then restore it.
    function flash(btn, text) {
      var original = btn.get();
      btn.set(text);
      setTimeout(function () { btn.set(original); }, 2200);
    }

    saveBtn.on('click', function () {
      var blob = readBlob();
      var i = slotIndex();
      blob.slots[i] = snapshotForm();
      blob.names[i] =
        String(nameItem.get() || '').trim() || 'Preset ' + (i + 1);
      writeBlob(blob);
      refreshSlotLabels(blob);
      flash(saveBtn, 'Saved ✓ — press Save Settings to keep');
    });

    loadBtn.on('click', function () {
      var blob = readBlob();
      var i = slotIndex();
      var preset = blob.slots[i];
      if (!preset) {
        flash(loadBtn, 'Slot is empty');
        return;
      }
      applySnapshot(preset);
      nameItem.set(blob.names[i] || '');
      if (codeItem) {
        try { codeItem.set(encodePreset(preset)); } catch (e) { /* leave field */ }
      }
      flash(loadBtn, 'Loaded ✓ — press Save Settings to apply');
    });

    slotItem.on('change', function () {
      nameItem.set(readBlob().names[slotIndex()] || '');
    });

    // ---- Preset codes ----

    if (codeItem && codeGenBtn && codeApplyBtn) {
      codeGenBtn.on('click', function () {
        var code;
        try {
          code = encodePreset(snapshotForm());
        } catch (e) {
          flash(codeGenBtn, 'Could not build a code');
          return;
        }
        codeItem.set(code);
        // Offer the code for copying, but never make the button depend on it:
        // clipboard access is blocked in some config webviews.
        var el = codeItem.$manipulatorTarget[0];
        var copied = false;
        try {
          el.focus();
          el.setSelectionRange(0, el.value.length);
          copied = document.execCommand && document.execCommand('copy');
        } catch (e2) { copied = false; }
        flash(codeGenBtn, copied ? 'Copied ✓' : 'Code ready — select and copy it');
      });

      codeApplyBtn.on('click', function () {
        var raw = String(codeItem.get() || '').trim();
        if (!raw) {
          flash(codeApplyBtn, 'Paste a code into the field first');
          return;
        }
        var preset;
        try {
          preset = decodePreset(raw);
        } catch (e) {
          flash(codeApplyBtn, 'Not usable: ' + e.message);
          return;
        }
        applySnapshot(preset);
        flash(codeApplyBtn, 'Applied ✓ — press Save Settings to keep');
      });
    }

    // initial state
    var initialBlob = readBlob();
    refreshSlotLabels(initialBlob);
    nameItem.set(initialBlob.names[slotIndex()] || '');

    // dev hooks for automated tests of the generated page
    if (typeof window !== 'undefined') {
      window._clayConfig = clayConfig;
      window._presetCodec = {
        encode: encodePreset,
        decode: decodePreset,
        fields: PRESET_FIELDS,
        defaults: defaultsByKey
      };
    }
  });
};
