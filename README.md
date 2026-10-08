# Your Style

*An analogue watchface you design yourself.*

A Pebble watchface (v2.3.0). Every part of the dial is adjustable on the
settings page: hands, hand fillings, hour and minute marks, colours and
proportions, so the same face can look like a sports watch, a dress watch or
something entirely your own.

## Settings

- **Presets**: store up to five complete configurations, or copy a whole
  configuration as a text code and apply it elsewhere
- **Screen adjustment**: nudge everything a few pixels if the display is not
  perfectly centred
- **Colours**: background, each hand, each hand filling, hour and minute marks
- **Lengths**: each hand from the centre to the rim, the inner start of the
  hands, and optional snapping of the hands to 6° steps
- **Hand fillings**: start and end position
- **Hour marks**: all, even or odd hours, gaps at certain hours, all except
  12 o'clock, optionally extended to the case rim on rectangular watches
- **Minute marks**: every 5th or 15th minute, with gaps around the hour marks

## Platforms

- Pebble Time 2 (`emery`)
- Pebble Round 2 (`gabbro`)
- Pebble Time Round (`chalk`)
- Pebble Time (`basalt`)
- Pebble 2 (`diorite`)
- Pebble 2 Duo (`flint`)

## Building

With the [Pebble SDK](https://developer.repebble.com/sdk/):

```bash
pebble build
pebble install --emulator emery
```

The repository can also be imported into CloudPebble as is.

## License

MIT License, see [LICENSE](LICENSE).
