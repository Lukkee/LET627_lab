# Brother John Musicplayer

## Controls

Controls are sent over the serial interface through determined symbols.

### Modes

`.` - **switches between the two modes**

#### Musician Mode
Listens to commands from the CAN bus.

#### Conductor Mode
Listens to commands from keyboard input.

### Keys
toggle-keys work as toggles.

#### Immediate keys

`u` - **increase volume**

`d` - **decrease volume**

#### Toggle-keys

`m` - **toggle mute**

`p` - **toggle playback**

toggle-keys are entered using a flag prefix.

**Example:**

type `1`, `m` → Mutes the player

type `0`, `m` → Unmutes the player

### Number entry keys

enter an integer within the [range], then press a command key to apply it.

`t` - **Tempo [30, 300]**

`k` - **Key [-5, 5]**

`v` - **Volume [0, 20]**

`c` - **Cancel**

**Example:** type `2`, `4`, `0`, `t` → sets tempo to 240 BPM

### User-button
The user-button is used for setting and resetting the tempo, tapping a minimum of three times with an interval between 100ms - 3000ms will set the tempo to
the average of the last three presses.

Holding the user-button for at least two seconds will reset the tempo to 120bpm.

## Led
The led will flash with the tempo while playback is active.

## Changes
### Since last submission
- Fixed: press-and-hold never went into 2s mode, now it does
- Fixed: button-trigger initialized as rising-edge, changed to falling-edge
