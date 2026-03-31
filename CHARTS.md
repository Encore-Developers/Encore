# Encore Chart Format
To work in Encore, a chart requires the following:
- Properly formatted `song.ini`
- At least 1 `song.ogg`, or multiple stems
- At least 1 valid instrument track in `<name>.mid`

Details about these files and the format will be noted below.

## notes.mid

This is a regular CH/RB midi file. Any 5-fret/Classic chart should be in a track named `PART_<inst>`. All `PAD_<inst>` tracks should be formatted for Encore pad gameplay.

*Note, `<inst>` is a placeholder for the instrument being charted. All acceptable instrument track names at this time are*

| Instrument  | Plastic      | Pad         |
|-------------|--------------|-------------|
| Guitar/Lead | `PART_GUITAR` | `PAD_GUITAR` |
| Bass/Groove | `PART_BASS`  | `PAD_BASS`  |
| Vocals*     | `PART_VOCALS` | `PAD_VOCALS` |
| Keys        | `PART_KEYS`  | `PAD_KEYS`  |
| Drums**     | `PART_DRUMS`  | `PAD_KEYS`   |  
*Only supports lyrics display at the moment  
**Only supports 4 lane/Pro drums

## Pad Specifics
Refer to the chart below for the different difficulties and pitch information.

### Tap Lanes
|Difficulty|1|2|3|4|5|
|:-|:-:|:-:|:-:|:-:|:-:|
|Expert|96  C6|97  C#6|98  D6|99  D#6|100  E6|
|Hard|84  C5|85  C#5|86  D5|87  D#5| |
|Medium|72  C4|73  C#4|74  D4|75  D#4| |
|Easy|60  C3|61  C#3|62  D3|63  D#3| |

### Lift Markers
|Difficulty|1|2|3|4|5|
|:-|:-:|:-:|:-:|:-:|:-:|
|Expert|102  F#6|103  G6|104  G#6|105  A6|106  B6|
|Hard|90  F#5|91  G5|92 G#5|93  A5| |
|Medium|78  F#4|79  G4|80  G#4|81  A4| |
|Easy|66  F#3|67  G3|68  G#3|69  A3| |

### Overdrive

|Difficulty|Lane|
|:-|:-:|
|All Diffs|116 G#7|



### Solos

|Difficulty|  Lane  |
|:-|:------:|
|All Diffs| 101 F6 |
(between Expert's lift and tap pitches)

## `EVENTS` and `BEAT`
Every `notes.mid` needs an `EVENTS` track and a `BEAT` track.

### Beat
`BEAT` changes how Overdrive drains. Do not touch it unless you're dealing with special tempo changes. Usually your template (if used) will do this for you. Measure start is on pitch 12 and every other beat is on pitch 13.

### Events
`EVENTS` has three required text events. 
- A `[music_start]` event tells the game where the first beatline should appear. 
- A `[music_end]` event tells the game where to stop displaying beatlines. 
- An `[end]` event should be placed at the very end of the chart MIDI, or where the game should cut to the Results screen.

`EVENTS` is also useful for creating practice sections. Refer to the [RBN documentation](http://docs.c3universe.com/rbndocs/index.php?title=Practice_Sections) for more practice section information.

## Audio
As Encore uses BASS and BASSOPUS, you can use WAV/AIFF/MP3/MP2/MP1/OGG/OPUS files for music. **OGG and OPUS are highly recommended.**

## Album art
Album art must be .png or .jpg, with a size of 512x512.
