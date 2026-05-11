# Brother John Musicplayer

## Controls

Controls are sent over the serial interface through determined symbols.

### Immediate keys

| Key | Action |
|-----|--------|
| `m` | Toggle mute |
| `+` | Volume up |
| `-` | Volume down |

### Number entry keys

enter an integer, then press a command key to apply it.

| Suffix key | Action | Range |
|------------|--------|-------|
| `t` | Set tempo (BPM) | 120 – 240 |
| `k` | Set key (semitone transpose) | -5 – 5 |
| `c` | Cancel current input | — |

**Example:** type `2`, `4`, `0`, `t` → sets tempo to 240 BPM

### Message flow

```
playNote → setTone  (starts oscillator)
         → silence  (stops oscillator after note duration)
         → playNote (schedules next note)

toneGenerator → toneGenerator (self-rescheduling oscillator loop)
```
