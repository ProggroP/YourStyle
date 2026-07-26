// Adds accordion behavior to the config page: each section's heading
// becomes clickable and collapses/expands the items that belong to it.
// Sections start collapsed so the page is short by default.

module.exports = function (minified) {
  var clayConfig = this;

  // One entry per collapsible section: id of its heading item, and the
  // group name shared by all items that belong to that section.
  var sections = [
    'colors',
    'lengths',
    'fillpos',
    'markpos',
    'hourmarkstyle',
    'minutemarkstyle',
    'numbers',
    'thickness',
    'date',
    'bluetooth',
    'battery',
    'appearance',
    'moon'
  ];

  clayConfig.on(clayConfig.EVENTS.AFTER_BUILD, function () {
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
            (collapsed ? '\u25B8 ' : '\u25BE ') +
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
  });
};
