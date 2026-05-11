# Brother John Musicplayer

## Controls

Controls are sent over the serial interface through determined symbols.

### Immediate keys

`m` - **toggle mute**

`u` - **increase volume**

`d` - **decrease volume**

### Number entry keys

enter an integer within the [range], then press a command key to apply it.

`t` - **Tempo [60, 240]**

`k` - **Key [-5, 5]**

`c` - **Cancel**

**Example:** type `2`, `4`, `0`, `t` → sets tempo to 240 BPM

### Message flow

```
playNote → setTone  (starts oscillator)
         → silence  (stops oscillator after note duration)
         → playNote (schedules next note)

toneGenerator → toneGenerator (self-rescheduling oscillator loop)
```
